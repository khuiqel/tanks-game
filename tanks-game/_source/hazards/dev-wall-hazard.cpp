#include "dev-wall-hazard.h"

#include <iostream>
#include "../rng.h"

#include "../renderer.h"
#include "../color-mixer.h"
#include "../background-rect.h"

#include "../collision-handler.h"
#include "../hazard-manager.h"
#include "../level-manager.h"

std::unordered_map<std::string, float> DevWallHazard::getWeights() const {
	std::unordered_map<std::string, float> weights;
	weights.insert({ "dev", 1.0f });
	weights.insert({ "random-dev", 0.5f });
	return weights;
}

DevWallHazard::DevWallHazard(double xpos, double ypos, double width, double height) : RectHazard(DEFAULT_TEAM) {
	rec.x = xpos;
	rec.y = ypos;
	rec.w = width;
	rec.h = height;
	if (LevelManager::getNumLevels() > 0) [[likely]] {
		color = LevelManager::getLevel(0)->getDefaultColor();
	} else {
		color = ColorValueHolder(0, 0, 0);
	}

	//canAcceptPowers = false;
}

DevWallHazard::~DevWallHazard() {
	//nothing
}

RectHazard* DevWallHazard::factory(const GenericFactoryConstructionData& args) {
	if (args.getDataCount() >= 1) [[likely]] {
		int count = args.getDataPortionLength(0);

		if (count >= 4) [[likely]] {
			const double* arr = static_cast<const double*>(args.getDataPortion(0).get());
			double x = arr[0];
			double y = arr[1];
			double w = arr[2];
			double h = arr[3];
			return new DevWallHazard(x, y, w, h);
		}
	}

	return new DevWallHazard(0, 0, 0, 0);
}

bool DevWallHazard::reasonableLocation() const {
	/*
	for (int i = 0; i < WallManager::getNumWalls(); i++) {
		if (CollisionHandler::partiallyCollided(this->rec, WallManager::getWall(i)->rec)) {
			return false;
		}
	}
	*/

	for (int i = 0; i < HazardManager::getNumCircleHazards(); i++) {
		const CircleHazard* ch = HazardManager::getCircleHazard(i);
		//note: circle wall hazards don't exist, so this check is unnecessary
		if (ch->getName() != this->getName()) {
			if (CollisionHandler::partiallyCollided(this->rec, ch->cir)) {
				return false;
			}
		}
	}
	for (int i = 0; i < HazardManager::getNumRectHazards(); i++) {
		const RectHazard* rh = HazardManager::getRectHazard(i);
		if (rh->getName() != this->getName()) {
			if (CollisionHandler::partiallyCollided(this->rec, rh->rec)) {
				return false;
			}
		}
	}

	return validLocation();
}

void DevWallHazard::draw() const {
	ghostDraw(1.0f);
}

void DevWallHazard::draw(DrawingLayers layer) const {
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

void DevWallHazard::poseDraw() const {
	draw();
}

void DevWallHazard::poseDraw(DrawingLayers layer) const {
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

void DevWallHazard::ghostDraw(float alpha) const {
	ColorValueHolder c = this->color;
	c = ColorMixer::mix(BackgroundRect::getBackColor(), c, alpha);

	float coordsAndColor[] = {
		rec.x,       rec.y,         c.getRf(), c.getGf(), c.getBf(), c.getAf(),
		rec.x+rec.w, rec.y,         c.getRf(), c.getGf(), c.getBf(), c.getAf(),
		rec.x+rec.w, rec.y+rec.h,   c.getRf(), c.getGf(), c.getBf(), c.getAf(),
		rec.x,       rec.y+rec.h,   c.getRf(), c.getGf(), c.getBf(), c.getAf()
	};
	unsigned int indices[] = {
		0, 1, 2,
		2, 3, 0
	};

	Renderer::SubmitBatchedDraw(coordsAndColor, 4 * (2+4), indices, 2 * 3);
}

void DevWallHazard::ghostDraw(DrawingLayers layer, float alpha) const {
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

RectHazard* DevWallHazard::randomizingFactory(double x_start, double y_start, double area_width, double area_height, const GenericFactoryConstructionData& args) {
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
		RectHazard* testWall = new DevWallHazard(xpos, ypos, width, height);
		if (testWall->reasonableLocation()) {
			randomized = testWall;
			break;
		} else {
			delete testWall;
		}
		attempts++;
	} while (attempts < 64);

	return randomized;
}
