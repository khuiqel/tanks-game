#include "rectangular-lava-hazard.h"

#include "../constants.h"
#include <cmath>
#include <algorithm> //std::sort
#include <iostream>
#include "../rng.h"

#include "../renderer.h"
#include "../color-mixer.h"
#include "../background-rect.h"

#include "../collision-handler.h"
#include "../wall-manager.h"
#include "../hazard-manager.h"

SimpleVector2D RectangularLavaHazard::bubble_vertices[RectangularLavaHazard::BubbleSideCount + 1];
unsigned int RectangularLavaHazard::bubble_indices[RectangularLavaHazard::BubbleSideCount * 3];
unsigned int RectangularLavaHazard::outline_indices[RectangularLavaHazard::BubbleSideCount * 2*3];
bool RectangularLavaHazard::initialized_vertices = false;

std::unordered_map<std::string, float> RectangularLavaHazard::getWeights() const {
	std::unordered_map<std::string, float> weights;
	weights.insert({ "vanilla", 1.0f });
	weights.insert({ "random-vanilla", 1.0f });
	weights.insert({ "old", 1.0f });
	weights.insert({ "random-old", 1.0f });
	weights.insert({ "random", 1.0f });
	return weights;
}

RectangularLavaHazard::RectangularLavaHazard(double xpos, double ypos, double width, double height) : RectHazard(HAZARD_TEAM) {
	x = xpos;
	y = ypos;
	w = width;
	h = height;

	tickCount = 0;
	tickCycle = 2400;
	bubbleChance = 1.0f/400;
	bubbles.assign(maxBubbles, LavaBubble());

	//canAcceptPowers = false;

	initializeVertices();
}

RectangularLavaHazard::~RectangularLavaHazard() {
	//nothing
}

bool RectangularLavaHazard::initializeVertices() {
	if (initialized_vertices) [[likely]] {
		return false;
	}

	bubble_vertices[0] = SimpleVector2D(0, 0);
	for (int i = 1; i < RectangularLavaHazard::BubbleSideCount+1; i++) {
		bubble_vertices[i] = SimpleVector2D(std::cos((i-1) * (2*PI / RectangularLavaHazard::BubbleSideCount)), std::sin((i-1) * (2*PI / RectangularLavaHazard::BubbleSideCount)));
	}

	for (int i = 0; i < RectangularLavaHazard::BubbleSideCount; i++) {
		bubble_indices[i*3]   = 0;
		bubble_indices[i*3+1] = i+1;
		bubble_indices[i*3+2] = (i+1) % RectangularLavaHazard::BubbleSideCount + 1;
	}

	for (int i = 0; i < RectangularLavaHazard::BubbleSideCount; i++) {
		outline_indices[i*6]   =  i*2;
		outline_indices[i*6+1] =  i*2+1;
		outline_indices[i*6+2] = (i*2+3) % (RectangularLavaHazard::BubbleSideCount*2);
		outline_indices[i*6+3] = (i*2+3) % (RectangularLavaHazard::BubbleSideCount*2);
		outline_indices[i*6+4] = (i*2+2) % (RectangularLavaHazard::BubbleSideCount*2);
		outline_indices[i*6+5] =  i*2;
	}

	initialized_vertices = true;
	return true;
}

RectHazard* RectangularLavaHazard::factory(const GenericFactoryConstructionData& args) {
	if (args.getDataCount() >= 1) [[likely]] {
		int count = args.getDataPortionLength(0);

		if (count >= 4) [[likely]] {
			const double* arr = static_cast<const double*>(args.getDataPortion(0).get());
			double x = arr[0];
			double y = arr[1];
			double w = arr[2];
			double h = arr[3];
			return new RectangularLavaHazard(x, y, w, h);
		}
	}

	return new RectangularLavaHazard(0, 0, 0, 0);
}

void RectangularLavaHazard::tryToPushNewBubble(float bubbleRadius) {
	int availableIndex = -1;
	for (int i = 0; i < bubbles.size(); i++) {
		if (bubbles[i].isDead()) {
			availableIndex = i;
			break;
		}
	}
	if (availableIndex < 0) {
		return;
	}

	float x0, y0, x1, y1;
	int attempts = 0;

	x0 = VisualRNG::randFloatInRange(bubbleRadius, float(w) - bubbleRadius);
	y0 = VisualRNG::randFloatInRange(bubbleRadius, float(h) - bubbleRadius);
	do {
		x1 = VisualRNG::randFloatInRange(bubbleRadius, float(w) - bubbleRadius);
		y1 = VisualRNG::randFloatInRange(bubbleRadius, float(h) - bubbleRadius);
		attempts++;
	} while ((attempts < 8) && (std::abs(x0-x1) < float(w)/16 || std::abs(y0-y1) < float(h)/16)); //JS Tanks used w/8 and h/8

	if (attempts < 8) {
		float maxTick = std::floor(VisualRNG::randFloatInRange(200, 300+1));
		bubbles[availableIndex] = LavaBubble(bubbleRadius, x0/float(w), y0/float(h), x1/float(w), y1/float(h), maxTick);
	}
}

bool RectangularLavaHazard::reasonableLocation() const {
	for (int i = 0; i < WallManager::getNumWalls(); i++) {
		if (CollisionHandler::partiallyCollided(this, WallManager::getWall(i))) {
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

void RectangularLavaHazard::draw() const {
	drawBackground(false);
	drawBubbles(false);
}

void RectangularLavaHazard::draw(DrawingLayers layer) const {
	switch (layer) {
		default:
			std::cerr << "WARNING: unknown DrawingLayer for " + getName() + " draw!\n";
			[[fallthrough]];
		case DrawingLayers::under:
			drawBackground(false);
			break;

		case DrawingLayers::normal:
			drawBubbles(false);
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

void RectangularLavaHazard::poseDraw() const {
	drawBackground(true);
}

void RectangularLavaHazard::poseDraw(DrawingLayers layer) const {
	switch (layer) {
		default:
			std::cerr << "WARNING: unknown DrawingLayer for " + getName() + " poseDraw!\n";
			[[fallthrough]];
		case DrawingLayers::under:
			drawBackground(true);
			break;

		case DrawingLayers::normal:
			//drawBubbles(true);
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

void RectangularLavaHazard::ghostDraw(float alpha) const {
	drawBackground(false, alpha);
	//no bubbles
}

void RectangularLavaHazard::ghostDraw(DrawingLayers layer, float alpha) const {
	switch (layer) {
		default:
			std::cerr << "WARNING: unknown DrawingLayer for " + getName() + " ghostDraw!\n";
			[[fallthrough]];
		case DrawingLayers::under:
			drawBackground(false, alpha);
			break;

		case DrawingLayers::normal:
			//drawBubbles(false, alpha);
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

void RectangularLavaHazard::drawBackground(bool pose, float alpha) const {
	alpha = alpha * alpha;
	ColorValueHolder color = (pose ? getBackgroundColor_Pose() : getBackgroundColor());
	color = ColorMixer::mix(BackgroundRect::getBackColor(), color, alpha);

	float coordsAndColor[] = {
		x,   y,     color.getRf(), color.getGf(), color.getBf(), color.getAf(),
		x+w, y,     color.getRf(), color.getGf(), color.getBf(), color.getAf(),
		x+w, y+h,   color.getRf(), color.getGf(), color.getBf(), color.getAf(),
		x,   y+h,   color.getRf(), color.getGf(), color.getBf(), color.getAf()
	};
	unsigned int indices[] = {
		0, 1, 2,
		2, 3, 0
	};

	Renderer::SubmitBatchedDraw(coordsAndColor, 4 * (2+4), indices, 2 * 3);
}

void RectangularLavaHazard::drawBubbles(bool pose, float alpha) const {
	alpha = alpha * alpha;

	//bubbles are already sorted by their alpha
	for (int j = 0; j < bubbles.size(); j++) {
		if (bubbles[j].isDead()) {
			continue;
		}

		ColorValueHolder color = (pose ? getBubbleColor_Pose(bubbles[j]) : getBubbleColor(bubbles[j]));
		color = ColorMixer::mix(BackgroundRect::getBackColor(), color, alpha);
		const float lineWidth = 0.75f;

		float coordsAndColor[(RectangularLavaHazard::BubbleSideCount*2)*(2+4)];
		for (int i = 0; i < RectangularLavaHazard::BubbleSideCount; i++) {
			coordsAndColor[(i*2)  *6]   = (bubbles[j].getX()*static_cast<float>(this->w) + static_cast<float>(this->x)) + (bubbles[j].getR() - lineWidth) * bubble_vertices[i+1].getXComp();
			coordsAndColor[(i*2)  *6+1] = (bubbles[j].getY()*static_cast<float>(this->h) + static_cast<float>(this->y)) + (bubbles[j].getR() - lineWidth) * bubble_vertices[i+1].getYComp();
			coordsAndColor[(i*2+1)*6]   = (bubbles[j].getX()*static_cast<float>(this->w) + static_cast<float>(this->x)) + (bubbles[j].getR() + lineWidth) * bubble_vertices[i+1].getXComp();
			coordsAndColor[(i*2+1)*6+1] = (bubbles[j].getY()*static_cast<float>(this->h) + static_cast<float>(this->y)) + (bubbles[j].getR() + lineWidth) * bubble_vertices[i+1].getYComp();

			coordsAndColor[(i*2)  *6+2] = color.getRf();
			coordsAndColor[(i*2)  *6+3] = color.getGf();
			coordsAndColor[(i*2)  *6+4] = color.getBf();
			coordsAndColor[(i*2)  *6+5] = color.getAf();
			coordsAndColor[(i*2+1)*6+2] = color.getRf();
			coordsAndColor[(i*2+1)*6+3] = color.getGf();
			coordsAndColor[(i*2+1)*6+4] = color.getBf();
			coordsAndColor[(i*2+1)*6+5] = color.getAf();
		}

		Renderer::SubmitBatchedDraw(coordsAndColor, (RectangularLavaHazard::BubbleSideCount*2)*(2+4), outline_indices, RectangularLavaHazard::BubbleSideCount*6);
	}
}

RectHazard* RectangularLavaHazard::randomizingFactory(double x_start, double y_start, double area_width, double area_height, const GenericFactoryConstructionData& args) {
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
			width = LevelRNG::randNumInRange(30, 120); //TODO: where should these constants be?
			height = LevelRNG::randNumInRange(20, 80); //TODO: where should these constants be?
		}
		xpos = LevelRNG::randNumInRange(x_start, x_start + area_width - width);
		ypos = LevelRNG::randNumInRange(y_start, y_start + area_height - height);
		RectHazard* testRectangularLava = new RectangularLavaHazard(xpos, ypos, width, height);
		if (testRectangularLava->reasonableLocation()) {
			randomized = testRectangularLava;
			break;
		} else {
			delete testRectangularLava;
		}
		attempts++;
	} while (attempts < 64);

	return randomized;
}
