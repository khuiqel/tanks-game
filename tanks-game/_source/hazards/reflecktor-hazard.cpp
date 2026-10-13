#include "reflecktor-hazard.h"

#include "../constants.h"
#include <cmath>
//#include <algorithm> //nothing
#include <iostream>
#include "../rng.h"

#include "../renderer.h"
#include "../color-mixer.h"
#include "../background-rect.h"

#include "../collision-handler.h"
#include "../tank-manager.h"
#include "../wall-manager.h"
#include "../hazard-manager.h"
//#include "../level-manager.h"

std::unordered_map<std::string, float> ReflecktorHazard::getWeights() const {
	std::unordered_map<std::string, float> weights;
	weights.insert({ "dev", 1.0f });
	weights.insert({ "random-dev", 1.0f });
	return weights;
}

ReflecktorHazard::ReflecktorHazard(double xpos, double ypos, double width, double height) : RectHazard(HAZARD_TEAM) {
	x = xpos;
	y = ypos;
	w = width;
	h = height;

	color = ColorValueHolder(.95f, .95f, .95f);

	//canAcceptPowers = false;
}

ReflecktorHazard::~ReflecktorHazard() {
	//nothing
}

RectHazard* ReflecktorHazard::factory(const GenericFactoryConstructionData& args) {
	if (args.getDataCount() >= 1) [[likely]] {
		int count = args.getDataPortionLength(0);

		if (count >= 4) [[likely]] {
			const double* arr = static_cast<const double*>(args.getDataPortion(0).get());
			double x = arr[0];
			double y = arr[1];
			double w = arr[2];
			double h = arr[3];
			return new ReflecktorHazard(x, y, w, h);
		}
	}

	return new ReflecktorHazard(0, 0, 0, 0);
}

InteractionUpdateHolder<TankUpdateStruct, RectHazardUpdateStruct> ReflecktorHazard::modifiedTankCollision(const Tank* t) const {
	//copied from PowerFunctionHelper::superbounceGeneric()

	double t_xDelta, t_yDelta;
	float  t_angleDelta;

	if (t->y - this->y <= (this->h / this->w) * (t->x - this->x)) {
		if (t->y - (this->y + this->h) <= (-this->h / this->w) * (t->x - this->x)) { //bottom
			t_yDelta = -1 * ((t->y + t->r - this->y) * 2);
			t_angleDelta = -2 * t->velocity.getAngle();
			t_xDelta = 0;
		} else { //right
			t_xDelta = (this->x + this->w - (t->x - t->r)) * 2;
			t_angleDelta = float(PI) - 2*t->velocity.getAngle();
			t_yDelta = 0;
		}
	} else {
		if (t->y - (this->y + this->h) <= (-this->h / this->w) * (t->x - this->x)) { //left
			t_xDelta = -1 * ((t->x + t->r - this->x) * 2);
			t_angleDelta = float(PI) - 2*t->velocity.getAngle();
			t_yDelta = 0;
		} else { //top
			t_yDelta = (this->y + this->h - (t->y - t->r)) * 2;
			t_angleDelta = -2 * t->velocity.getAngle();
			t_xDelta = 0;
		}
	}

	return { false, false, new TankUpdateStruct(t_xDelta, t_yDelta, 0, t_angleDelta), nullptr };
}

bool ReflecktorHazard::reasonableLocation() const {
	for (int i = 0; i < WallManager::getNumWalls(); i++) {
		if (CollisionHandler::partiallyCollided(this, WallManager::getWall(i))) {
			return false;
		}
	}

	for (int i = 0; i < HazardManager::getNumCircleHazards(); i++) {
		const CircleHazard* ch = HazardManager::getCircleHazard(i);
		if (CollisionHandler::partiallyCollided(this, ch)) {
			return false;
		}
	}
	for (int i = 0; i < HazardManager::getNumRectHazards(); i++) {
		const RectHazard* rh = HazardManager::getRectHazard(i);
		if (rh->getGameID() != this->getGameID()) [[unlikely]] {
			if (CollisionHandler::partiallyCollided(this, rh)) {
				return false;
			}
		}
	}

	return validLocation();
}

void ReflecktorHazard::draw() const {
	ghostDraw(1.0f);
}

void ReflecktorHazard::draw(DrawingLayers layer) const {
	switch (layer) {
		case DrawingLayers::under:
			//nothing
			break;

		default:
			std::cerr << "WARNING: unknown DrawingLayer for " + getName() + " draw!\n";
			[[fallthrough]];
		case DrawingLayers::normal:
			draw();
			break;

		case DrawingLayers::effects:
			//nothing
			break;

		case DrawingLayers::top:
			//nothing
			break;

		case DrawingLayers::debug:
			//later
			break;
	}
}

void ReflecktorHazard::poseDraw() const {
	draw();
}

void ReflecktorHazard::poseDraw(DrawingLayers layer) const {
	switch (layer) {
		case DrawingLayers::under:
			//nothing
			break;

		default:
			std::cerr << "WARNING: unknown DrawingLayer for " + getName() + " poseDraw!\n";
			[[fallthrough]];
		case DrawingLayers::normal:
			poseDraw();
			break;

		case DrawingLayers::effects:
			//nothing
			break;

		case DrawingLayers::top:
			//nothing
			break;

		case DrawingLayers::debug:
			//later
			break;
	}
}

void ReflecktorHazard::ghostDraw(float alpha) const {
	ColorValueHolder c = this->color;
	c = ColorMixer::mix(BackgroundRect::getBackColor(), c, alpha);

	float coordsAndColor[] = {
		x,   y,     c.getRf(), c.getGf(), c.getBf(), c.getAf(),
		x+w, y,     c.getRf(), c.getGf(), c.getBf(), c.getAf(),
		x+w, y+h,   c.getRf(), c.getGf(), c.getBf(), c.getAf(),
		x,   y+h,   c.getRf(), c.getGf(), c.getBf(), c.getAf()
	};
	unsigned int indices[] = {
		0, 1, 2,
		2, 3, 0
	};

	Renderer::SubmitBatchedDraw(coordsAndColor, 4 * (2+4), indices, 2 * 3);
}

void ReflecktorHazard::ghostDraw(DrawingLayers layer, float alpha) const {
	switch (layer) {
		case DrawingLayers::under:
			//nothing
			break;

		default:
			std::cerr << "WARNING: unknown DrawingLayer for " + getName() + " ghostDraw!\n";
			[[fallthrough]];
		case DrawingLayers::normal:
			ghostDraw(alpha);
			break;

		case DrawingLayers::effects:
			//nothing
			break;

		case DrawingLayers::top:
			//nothing
			break;

		case DrawingLayers::debug:
			//later
			break;
	}
}

RectHazard* ReflecktorHazard::randomizingFactory(double x_start, double y_start, double area_width, double area_height, const GenericFactoryConstructionData& args) {
	int attempts = 0;
	RectHazard* randomized = nullptr;
	double xpos, ypos, width, height;

	bool randomizeWH;
	int count = 0;
	if (args.getDataCount() >= 1) {
		count = args.getDataPortionLength(0);
	}
	if (count >= 2) {
		const double* arr = static_cast<const double*>(args.getDataPortion(0).get());
		width = arr[0];
		height = arr[1];
		randomizeWH = false;
	} else {
		randomizeWH = true;
	}

	do {
		if (randomizeWH) {
			width = LevelRNG::randNumInRange(12, 64); //from LevelHelper::makeNewRandomWall
			height = LevelRNG::randNumInRange(8, 96);
		}
		xpos = LevelRNG::randNumInRange(x_start, x_start + area_width - width);
		ypos = LevelRNG::randNumInRange(y_start, y_start + area_height - height);
		RectHazard* testReflecktor = new ReflecktorHazard(xpos, ypos, width, height);
		if (testReflecktor->reasonableLocation()) {
			randomized = testReflecktor;
			break;
		} else {
			delete testReflecktor;
		}
		attempts++;
	} while (attempts < 64);

	return randomized;
}
