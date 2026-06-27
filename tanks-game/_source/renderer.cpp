#include "renderer.h"

#include "constants.h"
#include <algorithm> //std::fill
#include <iostream>
#include <gtc/matrix_transform.hpp>
#include "bullet.h" //to avoid passing in the bullet's vertices every time

#include "graphics/opengl-rendering-context.h"
#include "graphics/software-rendering-context.h"
#include "graphics/null-rendering-context.h"

#include "frame-time-graph.h"
#include "game-manager.h" //for isDebugDrawingEnabled(), also settings (bullet alpha draw)

#include <tracy_include.h>

glm::mat4 Renderer::proj = glm::ortho(0.0f, (float)GAME_WIDTH, 0.0f, (float)GAME_HEIGHT);
RenderingContext* Renderer::renderingBackend = nullptr;
AvailableRenderingContexts Renderer::renderingBackendType;

std::unordered_map<std::string, Shader*> Renderer::shaderStorage;
Shader* Renderer::boundShader = nullptr;

VertexArrayObject* Renderer::batched_vao;
VertexBuffer* Renderer::batched_vb;
IndexBuffer* Renderer::batched_ib;
VertexArrayObject* Renderer::instanced_vao;
VertexBuffer* Renderer::instanced_vb_pos;
VertexBuffer* Renderer::instanced_vb_bvalues;
IndexBuffer* Renderer::instanced_ib;
bool Renderer::initialized_GPU = false;

std::unordered_map<std::string, std::vector<Renderer::VertexDrawingData*>> Renderer::sceneData;
std::vector<std::string> Renderer::sceneList;
std::string Renderer::currentSceneName = "";
const unsigned int Renderer::MainBatched_VertexData::maxVerticesDataLength = (1 << 16) / sizeof(float); //2^16 - 2^18 have basically identical performance; lower results in problems, higher results in stuttering/spikes with worse performance
const unsigned int Renderer::MainBatched_VertexData::maxIndicesDataLength = (1 << 16) / sizeof(unsigned int); //TODO: size (fills up faster)
const unsigned int Renderer::Bullet_VertexData::maxDataLength = (1 << 16) / sizeof(float);

void Renderer::SetContext(AvailableRenderingContexts API) {
	if (renderingBackend != nullptr) {
		throw std::logic_error("ERROR: Cannot change rendering context!");
	}

	switch (API) {
		default:
			std::cerr << "Rendering context unknown! Defaulting to OpenGL..." << std::endl;
			[[fallthrough]];
		case AvailableRenderingContexts::OpenGL:
			renderingBackend = new OpenGLRenderingContext();
			renderingBackendType = AvailableRenderingContexts::OpenGL;
			break;
		case AvailableRenderingContexts::software:
			renderingBackend = new SoftwareRenderingContext();
			renderingBackendType = AvailableRenderingContexts::software;
			break;
		case AvailableRenderingContexts::null_rendering:
			renderingBackend = new NullRenderingContext();
			renderingBackendType = AvailableRenderingContexts::null_rendering;
			break;
	}
}

void Renderer::SetContext(const std::string& API) {
	AvailableRenderingContexts context;
	if (API == "OpenGL") {
		context = AvailableRenderingContexts::OpenGL;
	} else if (API == "software") {
		context = AvailableRenderingContexts::software;
	} else if (API == "null" || API == "NULL") {
		context = AvailableRenderingContexts::null_rendering;
	} else {
		std::cerr << "Rendering context unknown! Defaulting to OpenGL..." << std::endl;
		context = AvailableRenderingContexts::OpenGL;
	}

	SetContext(context);
}

void Renderer::PrintRendererInfo() {
	renderingBackend->PrintRendererInfo();
}

void Renderer::Initialize() {
	renderingBackend->Initialize();

	//TODO: need to move shader initialization to renderingBackend

	Shader* shader = Shader::MakeShader("res/shaders/OpenGL/main.vert", "res/shaders/OpenGL/main.frag");
	shaderStorage.insert({ "main", shader });

	//shader = Shader::MakeShader("res/shaders/OpenGL/default.vert", "res/shaders/OpenGL/default.frag");
	//shaderStorage.insert({ "default", shader });

	shader = Shader::MakeShader("res/shaders/OpenGL/bullet.vert", "res/shaders/OpenGL/bullet.frag");
	shaderStorage.insert({ "bullet", shader });

	initializeGPU();
}

void Renderer::Uninitialize() {
	//uninitializeGPU();
}

bool Renderer::MainBatched_VertexData::enoughRoomForMoreVertices(unsigned int pushLength) {
	return (m_vertices.size() + pushLength <= Renderer::MainBatched_VertexData::maxVerticesDataLength);
}
bool Renderer::MainBatched_VertexData::enoughRoomForMoreIndices(unsigned int pushLength) {
	return (m_indices.size() + pushLength <= Renderer::MainBatched_VertexData::maxIndicesDataLength);
}

bool Renderer::Bullet_VertexData::enoughRoomForMoreBulletValues(unsigned int pushLength) {
	return (m_bulletValues.size() + pushLength <= Renderer::Bullet_VertexData::maxDataLength);
}

Renderer::MainBatched_VertexData::MainBatched_VertexData() {
	m_shaderName = "main";
	#ifdef NDEBUG
	m_vertices.reserve(MainBatched_VertexData::maxVerticesDataLength);
	m_indices.reserve(MainBatched_VertexData::maxIndicesDataLength);
	#else
	//performance is awful //TODO: check again
	#endif
}

Renderer::Bullet_VertexData::Bullet_VertexData() {
	m_shaderName = "bullet";
	#ifdef NDEBUG
	m_bulletValues.reserve(Bullet_VertexData::maxDataLength);
	#else
	//performance is awful //TODO: check again
	#endif
}

glm::mat4 Renderer::GenerateModelMatrix(float scaleX, float scaleY, float rotateAngle, float transX, float transY) {
	glm::mat4 trans = glm::translate(glm::mat4(1.0f), glm::vec3(transX, transY, 0.0f));
	glm::mat4 rot = glm::rotate(trans, rotateAngle, glm::vec3(0.0f, 0.0f, 1.0f));
	return glm::scale(rot, glm::vec3(scaleX, scaleY, 0));
}

glm::mat4 Renderer::GenerateModelMatrix_NoRotate(float scaleX, float scaleY, float transX, float transY) {
	glm::mat4 trans = glm::translate(glm::mat4(1.0f), glm::vec3(transX, transY, 0.0f));
	return glm::scale(trans, glm::vec3(scaleX, scaleY, 0));
}

void Renderer::SetViewMatrix(float cameraX, float cameraY, float cameraZ, float targetX, float targetY, float targetZ) {
	boundShader->setUniformMat4f("u_ViewMatrix", glm::lookAt(glm::vec3(cameraX, cameraY, cameraZ), glm::vec3(targetX, targetY, targetZ), glm::vec3(0, 1, 0)));
	//boundShader->setUniformMat4f("u_ViewMatrix", glm::mat4(1.0f));
}

void Renderer::SetProjectionMatrix() {
	boundShader->setUniformMat4f("u_ProjectionMatrix", Renderer::proj);
}

inline void Renderer::bindShader(Shader* shader) {
	shader->Bind();
	boundShader = shader;
	SetViewMatrix(0, 0, 1, 0, 0, 0);
	SetProjectionMatrix();
}

inline void Renderer::bindVertexArrayObject(const VertexArrayObject* vao) {
	vao->Bind();
}

inline void Renderer::bindIndexBuffer(const IndexBuffer* ib) {
	ib->Bind();
}

Shader* Renderer::getShader(const std::string& s) {
	auto get = shaderStorage.find(s);
	if (get != shaderStorage.end()) [[likely]] {
		return get->second;
	}
	//else shader wasn't found
	std::cerr << "Could not find '" << s << "' shader!" << std::endl;

	//return the magenta shader, just so there's something
	get = shaderStorage.find("default");
	if (get != shaderStorage.end()) [[likely]] {
		return get->second;
	}
	//else big uh-oh
	std::cerr << "Could not find the default shader!" << std::endl;

	return nullptr; //default magenta shader is missing
}

void Renderer::UnbindAll() {
	batched_vao->Unbind(); //doesn't matter which one because they call the same thing
	boundShader->Unbind();

	boundShader = nullptr;
}

void Renderer::Clear() {
	renderingBackend->Clear();
}

void Renderer::ActuallyFlush() {
	auto start = FrameTimeGraph::getTime();
	std::string currentShaderName = "";
	for (int i = 0; i < sceneList.size(); i++) {
		const std::string& sceneName = sceneList[i];
		std::vector<VertexDrawingData*>& sceneDrawCalls = sceneData[sceneName];
		for (int j = 0; j < sceneDrawCalls.size(); j++) {
			if (sceneDrawCalls[j]->m_shaderName != currentShaderName) {
				if (sceneDrawCalls[j]->m_shaderName == "main") {
					bindShader(Renderer::getShader("main"));
					bindVertexArrayObject(batched_vao);
				} else if (sceneDrawCalls[j]->m_shaderName == "bullet") {
					bindShader(Renderer::getShader("bullet"));
					boundShader->setUniform1i("u_DrawTransparency", (int)GameManager::get_settings().Bullet_DrawAlpha);
					bindVertexArrayObject(instanced_vao);

					instanced_vb_pos->modifyData(Bullet::instanced_vertices, sizeof(Bullet::instanced_vertices));
					instanced_ib->modifyData(Bullet::instanced_indices, sizeof(Bullet::instanced_indices));
					//TODO: could use a different IB when alpha is enabled to avoid the death circle altogether
				} else {
					//oh no
				}
				currentShaderName = sceneDrawCalls[j]->m_shaderName;
			}

			if (currentShaderName == "main") {
				BatchedFlush(static_cast<MainBatched_VertexData*>(sceneDrawCalls[j]));
				delete static_cast<MainBatched_VertexData*>(sceneDrawCalls[j]);
			} else if (currentShaderName == "bullet") {
				BulletFlush(static_cast<Bullet_VertexData*>(sceneDrawCalls[j]));
				delete static_cast<Bullet_VertexData*>(sceneDrawCalls[j]);
			} else {
				//oh no
				delete sceneDrawCalls[j]; //TODO
			}
		}
		sceneDrawCalls.clear();
	}
	sceneList.clear(); //TODO: this is a surprising area for CPU time (_Tidy from reallocating) (or is it the line above?)
	auto end = FrameTimeGraph::getTime();

	FrameTimeGraph::pushGraphTime("draw", FrameTimeGraph::getDiff(start, end));
	FrameTimeGraph::pushGraphSumTime("all");

	const GameSettings& game_settings = GameManager::get_settings();
	if (game_settings.PerformanceGraphEnable) {
		FrameTimeGraph::drawGraphTimes();
	}

	renderingBackend->Flush();

	UnbindAll();
}

void Renderer::Flush() {
	ZoneScoped;
	//TODO: this function is separate in case graphics will be put on a separate thread
	//(which wouldn't be particularly useful, as rendering is typically <1ms in worst case scenarios)

	ActuallyFlush();
}

void Renderer::Cleanup() {
	//TODO?
}

bool Renderer::initializeGPU() {
	if (initialized_GPU) {
		return false;
	}

	float* positions = new float[MainBatched_VertexData::maxVerticesDataLength];
	std::fill(positions, positions + MainBatched_VertexData::maxVerticesDataLength, 0);

	unsigned int* indices = new unsigned int[MainBatched_VertexData::maxIndicesDataLength];
	std::fill(indices, indices + MainBatched_VertexData::maxIndicesDataLength, 0);

	batched_vb = VertexBuffer::MakeVertexBuffer(positions, MainBatched_VertexData::maxVerticesDataLength * sizeof(float), VertexBuffer::RenderingHints::dynamic_draw);
	VertexBufferLayout layout = {
		{ ShaderDataType::Float2, "a_Position" },
		{ ShaderDataType::Float4, "a_Color" }
	};
	batched_vb->SetLayout(layout);

	batched_ib = IndexBuffer::MakeIndexBuffer(indices, MainBatched_VertexData::maxIndicesDataLength, IndexBuffer::RenderingHints::dynamic_draw);

	batched_vao = VertexArrayObject::MakeVertexArrayObject();
	batched_vao->AddVertexBuffer(batched_vb);
	batched_vao->SetIndexBuffer(batched_ib);

	delete[] positions;
	delete[] indices;

	float* positions_instanced = new float[Bullet_VertexData::maxDataLength];
	std::fill(positions_instanced, positions_instanced + Bullet_VertexData::maxDataLength, 0);

	unsigned int* indices_instanced = new unsigned int[MainBatched_VertexData::maxIndicesDataLength]; //TODO: might want a better size
	std::fill(indices_instanced, indices_instanced + MainBatched_VertexData::maxIndicesDataLength, 0);

	instanced_vb_pos = VertexBuffer::MakeVertexBuffer(positions_instanced, Bullet_VertexData::maxDataLength * sizeof(float), VertexBuffer::RenderingHints::stream_draw);
	VertexBufferLayout layout_instanced_pos = {
		{ ShaderDataType::Float2, "a_VertexPosition" }
	};
	instanced_vb_pos->SetLayout(layout_instanced_pos);

	instanced_vb_bvalues = VertexBuffer::MakeVertexBuffer(positions_instanced, Bullet_VertexData::maxDataLength * sizeof(float), VertexBuffer::RenderingHints::dynamic_draw);
	VertexBufferLayout layout_instanced_mat = {
		{ ShaderDataType::Float3, "a_BulletValuesInstanced", false, true },
		{ ShaderDataType::Float4, "a_ColorInstanced",        false, true }
	};
	instanced_vb_bvalues->SetLayout(layout_instanced_mat);

	instanced_ib = IndexBuffer::MakeIndexBuffer(indices_instanced, MainBatched_VertexData::maxIndicesDataLength, IndexBuffer::RenderingHints::stream_draw);

	instanced_vao = VertexArrayObject::MakeVertexArrayObject();
	instanced_vao->AddVertexBuffer(instanced_vb_pos);
	instanced_vao->AddVertexBuffer(instanced_vb_bvalues);
	instanced_vao->SetIndexBuffer(instanced_ib);

	delete[] positions_instanced;
	delete[] indices_instanced;

	initialized_GPU = true;
	return true;
}

bool Renderer::uninitializeGPU() {
	if (!initialized_GPU) {
		return false;
	}

	delete batched_vao;
	delete batched_vb;
	delete batched_ib;
	delete instanced_vao;
	delete instanced_vb_pos;
	delete instanced_vb_bvalues;
	delete instanced_ib;

	initialized_GPU = false;
	return true;
}

void Renderer::SubmitBatchedDraw(const float* posAndColor, unsigned int posAndColorLength, const unsigned int* indices, unsigned int indicesLength) {
	if (currentSceneName == "") [[unlikely]] {
		//only happens for FrameTimeGraph
		bindShader(Renderer::getShader("main"));
		bindVertexArrayObject(batched_vao);
		MainBatched_VertexData* tempVertexDataGroup = new MainBatched_VertexData();
		tempVertexDataGroup->m_vertices.insert(tempVertexDataGroup->m_vertices.end(), posAndColor, posAndColor + posAndColorLength);
		tempVertexDataGroup->m_indices.insert(tempVertexDataGroup->m_indices.end(), indices, indices + indicesLength);
		BatchedFlush(tempVertexDataGroup);
		delete tempVertexDataGroup;
	} else {
		MainBatched_VertexData* currentVertexDataGroup;
		std::vector<VertexDrawingData*>& sceneDrawGroup = sceneData[currentSceneName];
		if (sceneDrawGroup.empty() || sceneDrawGroup[sceneDrawGroup.size()-1]->m_shaderName != "main") {
			currentVertexDataGroup = new MainBatched_VertexData();
			sceneDrawGroup.push_back(static_cast<VertexDrawingData*>(currentVertexDataGroup));
		} else {
			currentVertexDataGroup = static_cast<MainBatched_VertexData*>(sceneDrawGroup[sceneDrawGroup.size()-1]);
			if (!currentVertexDataGroup->enoughRoomForMoreVertices(posAndColorLength) || !currentVertexDataGroup->enoughRoomForMoreIndices(indicesLength)) {
				currentVertexDataGroup = new MainBatched_VertexData();
				sceneDrawGroup.push_back(static_cast<VertexDrawingData*>(currentVertexDataGroup));

				if (posAndColorLength > MainBatched_VertexData::maxVerticesDataLength || indicesLength > MainBatched_VertexData::maxIndicesDataLength) [[unlikely]] {
					std::cerr << "GPU buffer overrun!" << std::endl;
				}
			}
		}

		std::vector<float>& verticesData = currentVertexDataGroup->m_vertices;
		std::vector<unsigned int>& indicesData = currentVertexDataGroup->m_indices;

		unsigned int currVerticesLength = verticesData.size();
		verticesData.insert(verticesData.end(), posAndColor, posAndColor + posAndColorLength);

		unsigned int currIndicesLength = indicesData.size();
		indicesData.insert(indicesData.end(), indices, indices + indicesLength);
		for (unsigned int i = currIndicesLength; i < indicesData.size(); i++) {
			indicesData[i] += currVerticesLength/(2+4);
			//could glDrawElementsBaseVertex() be usable to avoid fixing indices?
		}
	}
}

void Renderer::SubmitBulletDrawCall(float x, float y, float radius, float red, float green, float blue, float bulletLife) {
	if (currentSceneName == "") [[unlikely]] {
		//TODO
	} else {
		Bullet_VertexData* currentVertexDataGroup;
		std::vector<VertexDrawingData*>& sceneDrawGroup = sceneData[currentSceneName];
		if (sceneDrawGroup.empty() || sceneDrawGroup[sceneDrawGroup.size()-1]->m_shaderName != "bullet") {
			currentVertexDataGroup = new Bullet_VertexData();
			sceneDrawGroup.push_back(static_cast<VertexDrawingData*>(currentVertexDataGroup));
		} else {
			currentVertexDataGroup = static_cast<Bullet_VertexData*>(sceneDrawGroup[sceneDrawGroup.size()-1]);
			if (!currentVertexDataGroup->enoughRoomForMoreBulletValues(7)) {
				currentVertexDataGroup = new Bullet_VertexData();
				sceneDrawGroup.push_back(static_cast<VertexDrawingData*>(currentVertexDataGroup));
			}
		}

		std::vector<float>& bulletValuesData = currentVertexDataGroup->m_bulletValues;

		bulletValuesData.insert(bulletValuesData.end(), { x, y, radius, red, green, blue, bulletLife });
	}
}

void Renderer::BatchedFlush(const MainBatched_VertexData* drawData) {
	ZoneScoped;
	//TODO: check if there's nothing?

	batched_vb->modifyData(drawData->m_vertices.data(), drawData->m_vertices.size() * sizeof(float));
	batched_ib->modifyData(drawData->m_indices.data(), drawData->m_indices.size() * sizeof(unsigned int));

	renderingBackend->DrawUsingIndices(batched_vao->GetIndexBuffer()->getCount());
}

void Renderer::BulletFlush(const Bullet_VertexData* drawData) {
	ZoneScoped;
	//TODO: check if there's nothing?

	instanced_vb_bvalues->modifyData(drawData->m_bulletValues.data(), drawData->m_bulletValues.size() * sizeof(float));

	const unsigned int bulletCount = drawData->m_bulletValues.size() / 7;
	renderingBackend->DrawInstanced(sizeof(Bullet::instanced_indices)/sizeof(*Bullet::instanced_indices), bulletCount);
}

void Renderer::BeginScene(const std::string& name) {
	currentSceneName = name;
	sceneList.push_back(name);
}

void Renderer::EndScene() {
	currentSceneName = "";
	//not sure what to put...
}

bool Renderer::isDebugDrawingEnabled(const std::string& name) {
	const BasicINIParser::BasicINIData& ini_data = GameManager::get_INI();

	bool drawingEnabled = false;
	if (ini_data.exists("DEBUG", "EnableDebugDrawingObjects")) {
		int length = ini_data.length("DEBUG", "EnableDebugDrawingObjects");
		for (int i = 0; i < length; i++) {
			if (ini_data.get("DEBUG", "EnableDebugDrawingObjects", i) == name) {
				drawingEnabled = true;
				break;
			}
		}
	}
	return drawingEnabled;
}
