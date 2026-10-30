#pragma once
#include <string>
#include <utility>
#include <vector>
#include <unordered_map>
#include <glm.hpp>

#include "graphics/rendering-context.h"
#include "graphics/vertex-array-object.h"
#include "graphics/vertex-buffer.h"
#include "graphics/index-buffer.h"
#include "graphics/shader.h"

enum class AvailableRenderingContexts {
	OpenGL,
	software,
	null_rendering
};

class Renderer {
	friend class WindowInitializer; //only to set the projection matrix

private:
	static glm::mat4 proj;

	static AvailableRenderingContexts renderingBackendType;
	static RenderingContext* renderingBackend;

private:
	static std::unordered_map<std::string, Shader*> shaderStorage;
	static Shader* getShader(const std::string&); //does not bind the shader
	static Shader* boundShader;

	static void SetViewMatrix(float cameraX, float cameraY, float cameraZ, float targetX, float targetY, float targetZ);
	static void SetProjectionMatrix();
	static inline void bindShader(Shader*);
	static inline void bindVertexArrayObject(const VertexArrayObject*);
	static inline void bindIndexBuffer(const IndexBuffer*);

	static void UnbindAll();
	static void Cleanup();

private:
	enum class ShaderType {
		None,
		MainBatched,
		Bullet,
	};

	struct VertexDrawingData {
		ShaderType m_shaderType;
		//NOTE: normally a virtual destructor would be required (to avoid an enormous memory leak)
		//however, the pointer is cast to the derived class before deleting, thus avoiding this requirement
		//virtual ~VertexDrawingData() {}
	};

	struct MainBatched_VertexData final : public VertexDrawingData {
		//TODO: pad to 64B?
		std::vector<float> m_vertices;
		std::vector<unsigned int> m_indices;
		static const unsigned int maxVerticesDataLength;
		static const unsigned int maxIndicesDataLength;
		bool enoughRoomForMoreVertices(unsigned int pushLength);
		bool enoughRoomForMoreIndices(unsigned int pushLength);
		MainBatched_VertexData();
	};

	struct Bullet_VertexData final : public VertexDrawingData {
		std::vector<float> m_bulletValues; //x, y, r, rgba
		//packing 8-bit color into a float does slightly improve rendering times, but only slightly
		static const unsigned int maxDataLength; //TODO
		bool enoughRoomForMoreBulletValues(unsigned int pushLength);
		Bullet_VertexData();
	};

	static std::unordered_map<std::string, std::vector<VertexDrawingData*>> sceneData; //TODO: store the vectors somewhere, move them there at end of frame, pull them out as needed
	static std::vector<std::string> sceneList;
	static std::string currentSceneName;
	static void ActuallyFlush();
	static void BatchedFlush(const MainBatched_VertexData* drawData);
	static void BulletFlush(const Bullet_VertexData* drawData);

	static VertexArrayObject* batched_vao;
	static VertexBuffer* batched_vb;
	static IndexBuffer* batched_ib;
	static VertexArrayObject* instanced_vao;
	static VertexBuffer* instanced_vb_pos;
	static VertexBuffer* instanced_vb_bvalues;
	static IndexBuffer* instanced_ib;
	static bool initialized_GPU;

	static bool initializeGPU();
	static bool uninitializeGPU();

public:
	//TODO: because window creation depends on the context being used, maybe these functions should go to WindowInitializer
	static void SetContext(AvailableRenderingContexts);
	static void SetContext(const std::string&);
	static AvailableRenderingContexts GetContext() { return renderingBackendType; }
	static void PrintRendererInfo(); //only valid after window initialization

	static void Initialize();
	static void Uninitialize();

	static void Clear();
	static void BeginScene(const std::string& name); //name of scene, intended for FrameTimeGraph, but drawing is so fast that there's little point
	static void SubmitBatchedDraw(const float* posAndColor, unsigned int posAndColorLength, const unsigned int* indices, unsigned int indicesLength);
	static void SubmitBulletDrawCall(float x, float y, float radius, float red, float green, float blue, float bulletLife);
	static void EndScene();
	static void Flush();
	static bool isDebugDrawingEnabled(const std::string& name); //should this really be here?

	static glm::mat4 GenerateModelMatrix(float scaleX, float scaleY, float rotateAngle, float transX, float transY);
	static glm::mat4 GenerateModelMatrix_NoRotate(float scaleX, float scaleY, float transX, float transY);

private:
	Renderer() = delete;
	Renderer(const Renderer&) = delete;
};
