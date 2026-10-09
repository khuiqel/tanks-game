#include "rectangular-no-bullet-zone-hazard.h"

#include <cmath>
#include <iostream>
#include "../rng.h"

#include "../renderer.h"
#include "../color-mixer.h"
#include "../background-rect.h"

#include "../collision-handler.h"
#include "../wall-manager.h"
#include "../hazard-manager.h"

std::unordered_map<std::string, float> RectangularNoBulletZoneHazard::getWeights() const {
	std::unordered_map<std::string, float> weights;
	weights.insert({ "vanilla", 1.0f });
	weights.insert({ "random-vanilla", 0.5f }); //TODO: should this be decreased?
	weights.insert({ "old", 1.0f });
	weights.insert({ "random-old", 0.5f });
	weights.insert({ "random", 0.25f });
	return weights;
}

RectangularNoBulletZoneHazard::RectangularNoBulletZoneHazard(double xpos, double ypos, double width, double height) : RectHazard(HAZARD_TEAM) {
	x = xpos;
	y = ypos;
	w = width;
	h = height;

	//canAcceptPowers = false;
}

RectangularNoBulletZoneHazard::~RectangularNoBulletZoneHazard() {
	//nothing
}

RectHazard* RectangularNoBulletZoneHazard::factory(const GenericFactoryConstructionData& args) {
	if (args.getDataCount() >= 1) [[likely]] {
		int count = args.getDataPortionLength(0);

		if (count >= 4) [[likely]] {
			const double* arr = static_cast<const double*>(args.getDataPortion(0).get());
			double x = arr[0];
			double y = arr[1];
			double w = arr[2];
			double h = arr[3];
			return new RectangularNoBulletZoneHazard(x, y, w, h);
		}
	}

	return new RectangularNoBulletZoneHazard(0, 0, 0, 0);
}

bool RectangularNoBulletZoneHazard::reasonableLocation() const {
	for (int i = 0; i < WallManager::getNumWalls(); i++) {
		if (CollisionHandler::fullyCollided(this, WallManager::getWall(i))) {
			return false;
		}
	}

	//don't allow the other version, it looks weird
	for (int i = 0; i < HazardManager::getNumCircleHazards(); i++) {
		if (CollisionHandler::partiallyCollided(this, HazardManager::getCircleHazard(i))) {
			return false;
		}
	}
	for (int i = 0; i < HazardManager::getNumRectHazards(); i++) {
		const RectHazard* rh = HazardManager::getRectHazard(i);
		if (rh->getName() != this->getName()) {
			if (CollisionHandler::partiallyCollided(this, rh)) {
				return false;
			}
		}
	}

	return validLocation();
}

void RectangularNoBulletZoneHazard::draw() const {
	ghostDraw(1.0f);
}

void RectangularNoBulletZoneHazard::draw(DrawingLayers layer) const {
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

void RectangularNoBulletZoneHazard::poseDraw() const {
	draw();
}

void RectangularNoBulletZoneHazard::poseDraw(DrawingLayers layer) const {
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

void RectangularNoBulletZoneHazard::ghostDraw(float alpha) const {
	alpha = alpha * alpha;

	//background:
	ColorValueHolder color_background = GeneralizedNoBulletZone::getColor();
	color_background = ColorMixer::mix(BackgroundRect::getBackColor(), color_background, alpha);

	float coordsAndColor_background[] = {
		x,   y,     color_background.getRf(), color_background.getGf(), color_background.getBf(), color_background.getAf(),
		x+w, y,     color_background.getRf(), color_background.getGf(), color_background.getBf(), color_background.getAf(),
		x+w, y+h,   color_background.getRf(), color_background.getGf(), color_background.getBf(), color_background.getAf(),
		x,   y+h,   color_background.getRf(), color_background.getGf(), color_background.getBf(), color_background.getAf()
	};
	unsigned int indices_background[] = {
		0, 1, 2,
		2, 3, 0
	};

	Renderer::SubmitBatchedDraw(coordsAndColor_background, 4 * (2+4), indices_background, 2 * 3);

	//red X (JS didn't have this):
	ColorValueHolder color_extra = X_COLOR;
	color_extra = ColorMixer::mix(BackgroundRect::getBackColor(), color_extra, alpha);
	float coordsAndColor_extra[] = {
		//bottom left:
		x,               y,                 color_extra.getRf(), color_extra.getGf(), color_extra.getBf(), color_extra.getAf(), //0
		x + w*X_WIDTH,   y,                 color_extra.getRf(), color_extra.getGf(), color_extra.getBf(), color_extra.getAf(),
		x,               y + h*X_WIDTH,     color_extra.getRf(), color_extra.getGf(), color_extra.getBf(), color_extra.getAf(),

		//top right:
		x+w,             y+h,               color_extra.getRf(), color_extra.getGf(), color_extra.getBf(), color_extra.getAf(), //3
		x+w - w*X_WIDTH, y+h,               color_extra.getRf(), color_extra.getGf(), color_extra.getBf(), color_extra.getAf(),
		x+w,             y+h - h*X_WIDTH,   color_extra.getRf(), color_extra.getGf(), color_extra.getBf(), color_extra.getAf(),

		//top left:
		x,               y+h,               color_extra.getRf(), color_extra.getGf(), color_extra.getBf(), color_extra.getAf(), //6
		x,               y+h - h*X_WIDTH,   color_extra.getRf(), color_extra.getGf(), color_extra.getBf(), color_extra.getAf(),
		x + w*X_WIDTH,   y+h,               color_extra.getRf(), color_extra.getGf(), color_extra.getBf(), color_extra.getAf(),

		//bottom right:
		x+w,             y,                 color_extra.getRf(), color_extra.getGf(), color_extra.getBf(), color_extra.getAf(), //9
		x+w,             y + h*X_WIDTH,     color_extra.getRf(), color_extra.getGf(), color_extra.getBf(), color_extra.getAf(),
		x+w - w*X_WIDTH, y,                 color_extra.getRf(), color_extra.getGf(), color_extra.getBf(), color_extra.getAf()
	};
	unsigned int indices_extra[] = {
		0,  1,  2,  //bottom left tip
		3,  4,  5,  //top right tip
		1,  5,  4,  //rect between
		4,  2,  1,

		6,  7,  8,  //top left tip
		9,  10, 11, //bottom right tip
		8,  7,  11, //rect between
		11, 10, 8
	};

	Renderer::SubmitBatchedDraw(coordsAndColor_extra, (3*4) * (2+4), indices_extra, (4+4) * 3);
}

void RectangularNoBulletZoneHazard::ghostDraw(DrawingLayers layer, float alpha) const {
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

RectHazard* RectangularNoBulletZoneHazard::randomizingFactory(double x_start, double y_start, double area_width, double area_height, const GenericFactoryConstructionData& args) {
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
			width = LevelRNG::randNumInRange(20, 40); //TODO: where should these constants be?
			height = LevelRNG::randNumInRange(20, 50); //TODO: where should these constants be?
		}
		xpos = LevelRNG::randNumInRange(x_start, x_start + area_width - width);
		ypos = LevelRNG::randNumInRange(y_start, y_start + area_height - height);
		RectHazard* testRectangularNoBulletZone = new RectangularNoBulletZoneHazard(xpos, ypos, width, height);
		if (testRectangularNoBulletZone->reasonableLocation()) {
			randomized = testRectangularNoBulletZone;
			break;
		} else {
			delete testRectangularNoBulletZone;
		}
		attempts++;
	} while (attempts < 64);

	return randomized;
}
