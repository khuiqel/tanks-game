#include "circular-no-bullet-zone-hazard.h"

#include "../constants.h"
#include <cmath>
#include <algorithm> //std::copy
#include <iostream>
#include "../rng.h"

#include "../renderer.h"
#include "../color-mixer.h"
#include "../background-rect.h"

#include "../collision-handler.h"
#include "../wall-manager.h"
#include "../hazard-manager.h"

SimpleVector2D CircularNoBulletZoneHazard::body_vertices[Circle::NumOfSides+1];
unsigned int CircularNoBulletZoneHazard::body_indices[Circle::NumOfSides*3];
SimpleVector2D* CircularNoBulletZoneHazard::redX_vertices = nullptr;
unsigned int* CircularNoBulletZoneHazard::redX_indices = nullptr;
int CircularNoBulletZoneHazard::redX_vertices_count = 0;
int CircularNoBulletZoneHazard::redX_indices_count = 0;
bool CircularNoBulletZoneHazard::initialized_vertices = false;

std::unordered_map<std::string, float> CircularNoBulletZoneHazard::getWeights() const {
	std::unordered_map<std::string, float> weights;
	weights.insert({ "vanilla", 1.0f });
	weights.insert({ "random-vanilla", 0.5f }); //TODO: should this be decreased?
	weights.insert({ "old", 1.0f });
	weights.insert({ "random-old", 0.5f });
	weights.insert({ "random", 0.25f });
	return weights;
}

CircularNoBulletZoneHazard::CircularNoBulletZoneHazard(double xpos, double ypos, double radius) : CircleHazard(HAZARD_TEAM) {
	x = xpos;
	y = ypos;
	r = radius;

	//canAcceptPowers = false;

	initializeVertices();
}

CircularNoBulletZoneHazard::~CircularNoBulletZoneHazard() {
	//nothing
}

bool CircularNoBulletZoneHazard::initializeVertices() {
	if (initialized_vertices) [[likely]] {
		return false;
	}

	body_vertices[0] = SimpleVector2D(0, 0);
	for (int i = 1; i < Circle::NumOfSides+1; i++) {
		body_vertices[i] = SimpleVector2D(std::cos((i-1) * (2*PI / Circle::NumOfSides)), std::sin((i-1) * (2*PI / Circle::NumOfSides)));
	}

	for (int i = 0; i < Circle::NumOfSides; i++) {
		body_indices[i*3]   = 0;
		body_indices[i*3+1] = i+1;
		body_indices[i*3+2] = (i+1) % Circle::NumOfSides + 1;
	}

	const int num_vertices_fromSlashCenter = std::floor(Circle::NumOfSides/4 * X_WIDTH); //vertices from the center of a slash
	const int num_vertices_outside = (2*num_vertices_fromSlashCenter + 1) * 4; //all the vertices on the outside
	std::vector<float> coords_extra;
	std::vector<unsigned int> indices_extra;

	coords_extra.push_back(0);
	coords_extra.push_back(0);

	for (int k = 0; k < 4; k++) {
		for (int i = -1*num_vertices_fromSlashCenter; i <= num_vertices_fromSlashCenter; i++) {
			int val = (k*Circle::NumOfSides/4 + Circle::NumOfSides/8) + i;
			coords_extra.push_back(std::cos(val * (2*PI / Circle::NumOfSides)));
			coords_extra.push_back(std::sin(val * (2*PI / Circle::NumOfSides)));
		}

		for (int i = 0; i < num_vertices_outside/4-1; i++) {
			indices_extra.push_back(0);
			indices_extra.push_back(k*(num_vertices_outside/4) + i+1);
			indices_extra.push_back(k*(num_vertices_outside/4) + i+2);
		}
	}

	indices_extra.push_back(0);
	indices_extra.push_back(3*(num_vertices_outside/4) + 0);
	indices_extra.push_back(0*(num_vertices_outside/4) + 1); //DL to UR, DR half
	indices_extra.push_back(0);
	indices_extra.push_back(1*(num_vertices_outside/4) + 0);
	indices_extra.push_back(2*(num_vertices_outside/4) + 1); //DL to UR, UL half
	indices_extra.push_back(0);
	indices_extra.push_back(4*(num_vertices_outside/4) + 0);
	indices_extra.push_back(1*(num_vertices_outside/4) + 1); //UL to DR, UR half
	indices_extra.push_back(0);
	indices_extra.push_back(2*(num_vertices_outside/4) + 0);
	indices_extra.push_back(3*(num_vertices_outside/4) + 1); //UL to DR, DL half
	//these draw over each other but that's easier than adding more vertices

	redX_vertices = new SimpleVector2D[coords_extra.size() / 2];
	redX_vertices_count = coords_extra.size() / 2;
	redX_indices = new unsigned int[indices_extra.size()];
	redX_indices_count = indices_extra.size() / 3;

	for (int i = 0; i < redX_vertices_count; i++) {
		redX_vertices[i] = SimpleVector2D(coords_extra[i*2], coords_extra[i*2+1]);
	}
	std::copy(indices_extra.begin(), indices_extra.end(), redX_indices);

	initialized_vertices = true;
	return true;
}

CircleHazard* CircularNoBulletZoneHazard::factory(const GenericFactoryConstructionData& args) {
	if (args.getDataCount() >= 1) [[likely]] {
		int count = args.getDataPortionLength(0);

		if (count >= 3) [[likely]] {
			const double* arr = static_cast<const double*>(args.getDataPortion(0).get());
			double x = arr[0];
			double y = arr[1];
			double r = arr[2];
			return new CircularNoBulletZoneHazard(x, y, r);
		}
	}

	return new CircularNoBulletZoneHazard(0, 0, 0);
}

bool CircularNoBulletZoneHazard::reasonableLocation() const {
	for (int i = 0; i < WallManager::getNumWalls(); i++) {
		if (CollisionHandler::fullyCollided(this, WallManager::getWall(i))) {
			return false;
		}
	}

	for (int i = 0; i < HazardManager::getNumCircleHazards(); i++) {
		const CircleHazard* ch = HazardManager::getCircleHazard(i);
		if (ch->getName() != this->getName()) {
			if (CollisionHandler::partiallyCollided(this, ch)) {
				return false;
			}
		}
	}
	//don't allow the other version, it looks weird
	for (int i = 0; i < HazardManager::getNumRectHazards(); i++) {
		if (CollisionHandler::partiallyCollided(this, HazardManager::getRectHazard(i))) {
			return false;
		}
	}

	return validLocation();
}

void CircularNoBulletZoneHazard::draw() const {
	ghostDraw(1.0f);
}

void CircularNoBulletZoneHazard::draw(DrawingLayers layer) const {
	switch (layer) {
		default:
			std::cerr << "WARNING: unknown DrawingLayer for " + getName() + " draw!\n";
			[[fallthrough]];
		case DrawingLayers::under:
			draw();
			break;

		case DrawingLayers::normal:
			//nothing
			break;

		case DrawingLayers::effects:
			//nothing
			break;

		case DrawingLayers::top:
			//nothing
			break;

		case DrawingLayers::debug:
			//nothing
			break;
	}
}

void CircularNoBulletZoneHazard::poseDraw() const {
	draw();
}

void CircularNoBulletZoneHazard::poseDraw(DrawingLayers layer) const {
	switch (layer) {
		default:
			std::cerr << "WARNING: unknown DrawingLayer for " + getName() + " poseDraw!\n";
			[[fallthrough]];
		case DrawingLayers::under:
			poseDraw();
			break;

		case DrawingLayers::normal:
			//nothing
			break;

		case DrawingLayers::effects:
			//nothing
			break;

		case DrawingLayers::top:
			//nothing
			break;

		case DrawingLayers::debug:
			//nothing
			break;
	}
}

void CircularNoBulletZoneHazard::ghostDraw(float alpha) const {
	//background:
	ColorValueHolder color_background = GeneralizedNoBulletZone::getColor();
	color_background = ColorMixer::mix(BackgroundRect::getBackColor(), color_background, alpha);

	float coordsAndColor_background[(Circle::NumOfSides+1)*(2+4)];
	coordsAndColor_background[0] = x;
	coordsAndColor_background[1] = y;
	coordsAndColor_background[2] = color_background.getRf();
	coordsAndColor_background[3] = color_background.getGf();
	coordsAndColor_background[4] = color_background.getBf();
	coordsAndColor_background[5] = color_background.getAf();
	for (int i = 1; i < Circle::NumOfSides+1; i++) {
		coordsAndColor_background[i*6]   = static_cast<float>(x) + static_cast<float>(r) * body_vertices[i].getXComp();
		coordsAndColor_background[i*6+1] = static_cast<float>(y) + static_cast<float>(r) * body_vertices[i].getYComp();
		coordsAndColor_background[i*6+2] = color_background.getRf();
		coordsAndColor_background[i*6+3] = color_background.getGf();
		coordsAndColor_background[i*6+4] = color_background.getBf();
		coordsAndColor_background[i*6+5] = color_background.getAf();
	}

	Renderer::SubmitBatchedDraw(coordsAndColor_background, (Circle::NumOfSides+1)*(2+4), body_indices, Circle::NumOfSides*3);

	//red X (JS didn't have this, though the circular version didn't exist in JS):
	ColorValueHolder color_extra = X_COLOR;
	color_extra = ColorMixer::mix(BackgroundRect::getBackColor(), color_extra, alpha);

	float* coordsAndColor_extra = new float[redX_vertices_count * (2+4)];
	coordsAndColor_extra[0] = x;
	coordsAndColor_extra[1] = y;
	coordsAndColor_extra[2] = color_extra.getRf();
	coordsAndColor_extra[3] = color_extra.getGf();
	coordsAndColor_extra[4] = color_extra.getBf();
	coordsAndColor_extra[5] = color_extra.getAf();
	for (int i = 1; i < redX_vertices_count; i++) {
		coordsAndColor_extra[i*6]   = static_cast<float>(x) + static_cast<float>(r) * redX_vertices[i].getXComp();
		coordsAndColor_extra[i*6+1] = static_cast<float>(y) + static_cast<float>(r) * redX_vertices[i].getYComp();
		coordsAndColor_extra[i*6+2] = color_extra.getRf();
		coordsAndColor_extra[i*6+3] = color_extra.getGf();
		coordsAndColor_extra[i*6+4] = color_extra.getBf();
		coordsAndColor_extra[i*6+5] = color_extra.getAf();
	}

	Renderer::SubmitBatchedDraw(coordsAndColor_extra, redX_vertices_count*(2+4), redX_indices, redX_indices_count*3);
	delete[] coordsAndColor_extra;
}

void CircularNoBulletZoneHazard::ghostDraw(DrawingLayers layer, float alpha) const {
	switch (layer) {
		default:
			std::cerr << "WARNING: unknown DrawingLayer for " + getName() + " ghostDraw!\n";
			[[fallthrough]];
		case DrawingLayers::under:
			ghostDraw(alpha);
			break;

		case DrawingLayers::normal:
			//nothing
			break;

		case DrawingLayers::effects:
			//nothing
			break;

		case DrawingLayers::top:
			//nothing
			break;

		case DrawingLayers::debug:
			//nothing
			break;
	}
}

CircleHazard* CircularNoBulletZoneHazard::randomizingFactory(double x_start, double y_start, double area_width, double area_height, const GenericFactoryConstructionData& args) {
	int attempts = 0;
	CircleHazard* randomized = nullptr;
	double xpos, ypos, radius;

	bool randomizeR;
	int count = 0;
	if (args.getDataCount() >= 1) {
		count = args.getDataPortionLength(0);
	}
	if (count >= 1) {
		const double* arr = static_cast<const double*>(args.getDataPortion(0).get());
		radius = arr[0];
		randomizeR = false;
	} else {
		randomizeR = true;
	}

	do {
		if (randomizeR) {
			radius = LevelRNG::randNumInRange(10, 20); //TODO: where should these constants be?
		}
		xpos = LevelRNG::randNumInRange(x_start + radius, x_start + area_width - radius);
		ypos = LevelRNG::randNumInRange(y_start + radius, y_start + area_height - radius);
		CircleHazard* testCircularNoBulletZone = new CircularNoBulletZoneHazard(xpos, ypos, radius);
		if (testCircularNoBulletZone->reasonableLocation()) {
			randomized = testCircularNoBulletZone;
			break;
		} else {
			delete testCircularNoBulletZone;
		}
		attempts++;
	} while (attempts < 64);

	return randomized;
}
