#include "circular-lava-hazard.h"

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

SimpleVector2D CircularLavaHazard::bubble_vertices[CircularLavaHazard::BubbleSideCount + 1];
unsigned int CircularLavaHazard::bubble_indices[CircularLavaHazard::BubbleSideCount * 3];
unsigned int CircularLavaHazard::outline_indices[CircularLavaHazard::BubbleSideCount * 2*3];
bool CircularLavaHazard::initialized_vertices = false;

std::unordered_map<std::string, float> CircularLavaHazard::getWeights() const {
	std::unordered_map<std::string, float> weights;
	weights.insert({ "vanilla", 1.0f });
	weights.insert({ "random-vanilla", 1.0f });
	weights.insert({ "old", 1.0f });
	weights.insert({ "random-old", 1.0f });
	weights.insert({ "random", 1.0f });
	return weights;
}

CircularLavaHazard::CircularLavaHazard(double xpos, double ypos, double radius) : CircleHazard(HAZARD_TEAM) {
	cir.x = xpos;
	cir.y = ypos;
	cir.r = radius;

	tickCount = 0;
	tickCycle = 2400;
	bubbleChance = 1.0f/400;
	bubbles.assign(maxBubbles, LavaBubble());

	//canAcceptPowers = false;

	initializeVertices();
}

CircularLavaHazard::~CircularLavaHazard() {
	//nothing
}

bool CircularLavaHazard::initializeVertices() {
	if (initialized_vertices) [[likely]] {
		return false;
	}

	bubble_vertices[0] = SimpleVector2D(0, 0);
	for (int i = 1; i < CircularLavaHazard::BubbleSideCount+1; i++) {
		bubble_vertices[i] = SimpleVector2D(std::cos((i-1) * (2*PI / CircularLavaHazard::BubbleSideCount)), std::sin((i-1) * (2*PI / CircularLavaHazard::BubbleSideCount)));
	}

	for (int i = 0; i < CircularLavaHazard::BubbleSideCount; i++) {
		bubble_indices[i*3]   = 0;
		bubble_indices[i*3+1] = i+1;
		bubble_indices[i*3+2] = (i+1) % CircularLavaHazard::BubbleSideCount + 1;
	}

	for (int i = 0; i < CircularLavaHazard::BubbleSideCount; i++) {
		outline_indices[i*6]   =  i*2;
		outline_indices[i*6+1] =  i*2+1;
		outline_indices[i*6+2] = (i*2+3) % (CircularLavaHazard::BubbleSideCount*2);
		outline_indices[i*6+3] = (i*2+3) % (CircularLavaHazard::BubbleSideCount*2);
		outline_indices[i*6+4] = (i*2+2) % (CircularLavaHazard::BubbleSideCount*2);
		outline_indices[i*6+5] =  i*2;
	}

	initialized_vertices = true;
	return true;
}

CircleHazard* CircularLavaHazard::factory(const GenericFactoryConstructionData& args) {
	if (args.getDataCount() >= 1) [[likely]] {
		int count = args.getDataPortionLength(0);

		if (count >= 3) [[likely]] {
			const double* arr = static_cast<const double*>(args.getDataPortion(0).get());
			double x = arr[0];
			double y = arr[1];
			double r = arr[2];
			return new CircularLavaHazard(x, y, r);
		}
	}

	return new CircularLavaHazard(0, 0, 0);
}

void CircularLavaHazard::tryToPushNewBubble(float bubbleRadius) {
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

	float r0, a0, r1, a1;
	a0 = VisualRNG::randFuncf() * float(2*PI);
	r0 = VisualRNG::randFuncf() * (float(cir.r) - bubbleRadius);
	x0 = r0 * std::cos(a0);
	y0 = r0 * std::sin(a0);
	do {
		a1 = VisualRNG::randFuncf() * float(2*PI);
		r1 = VisualRNG::randFuncf() * (float(cir.r) - bubbleRadius);
		x1 = r1 * std::cos(a1);
		y1 = r1 * std::sin(a1);
		attempts++;
	} while ((attempts < 8) && (std::abs(x0-x1) < float(cir.r)/16 || std::abs(y0-y1) < float(cir.r)/16));

	if (attempts < 8) {
		float maxTick = std::floor(VisualRNG::randFloatInRange(200, 300+1));
		bubbles[availableIndex] = LavaBubble(bubbleRadius, x0/float(cir.r), y0/float(cir.r), x1/float(cir.r), y1/float(cir.r), maxTick);
	}
}

bool CircularLavaHazard::reasonableLocation() const {
	for (int i = 0; i < WallManager::getNumWalls(); i++) {
		if (CollisionHandler::partiallyCollided(this->cir, WallManager::getWall(i)->rec)) {
			return false;
		}
	}

	for (int i = 0; i < HazardManager::getNumCircleHazards(); i++) {
		const CircleHazard* ch = HazardManager::getCircleHazard(i);
		if (ch->getName() != this->getName()) {
			if (CollisionHandler::partiallyCollided(this->cir, ch->cir)) {
				return false;
			}
		}
	}
	//don't allow the other version, it looks weird
	for (int i = 0; i < HazardManager::getNumRectHazards(); i++) {
		if (CollisionHandler::partiallyCollided(this->cir, HazardManager::getRectHazard(i)->rec)) {
			return false;
		}
	}

	return validLocation();
}

void CircularLavaHazard::draw() const {
	drawBackground(false);
	drawBubbles(false);
}

void CircularLavaHazard::draw(DrawingLayers layer) const {
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

void CircularLavaHazard::poseDraw() const {
	drawBackground(true);
}

void CircularLavaHazard::poseDraw(DrawingLayers layer) const {
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

void CircularLavaHazard::ghostDraw(float alpha) const {
	drawBackground(false, alpha);
	//no bubbles
}

void CircularLavaHazard::ghostDraw(DrawingLayers layer, float alpha) const {
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

void CircularLavaHazard::drawBackground(bool pose, float alpha) const {
	ColorValueHolder color = (pose ? getBackgroundColor_Pose() : getBackgroundColor());
	color = ColorMixer::mix(BackgroundRect::getBackColor(), color, alpha);

	float coordsAndColor[(CircularLavaHazard::BubbleSideCount+1)*(2+4)];
	coordsAndColor[0] = cir.x;
	coordsAndColor[1] = cir.y;
	coordsAndColor[2] = color.getRf();
	coordsAndColor[3] = color.getGf();
	coordsAndColor[4] = color.getBf();
	coordsAndColor[5] = color.getAf();
	for (int i = 1; i < CircularLavaHazard::BubbleSideCount+1; i++) {
		coordsAndColor[i*6]   = static_cast<float>(cir.x) + static_cast<float>(cir.r) * bubble_vertices[i].getXComp();
		coordsAndColor[i*6+1] = static_cast<float>(cir.y) + static_cast<float>(cir.r) * bubble_vertices[i].getYComp();
		coordsAndColor[i*6+2] = color.getRf();
		coordsAndColor[i*6+3] = color.getGf();
		coordsAndColor[i*6+4] = color.getBf();
		coordsAndColor[i*6+5] = color.getAf();
	}

	Renderer::SubmitBatchedDraw(coordsAndColor, (CircularLavaHazard::BubbleSideCount+1)*(2+4), bubble_indices, CircularLavaHazard::BubbleSideCount*3);
}

void CircularLavaHazard::drawBubbles(bool pose, float alpha) const {
	//bubbles are already sorted by their alpha
	for (int j = 0; j < bubbles.size(); j++) {
		if (bubbles[j].isDead()) {
			continue;
		}

		ColorValueHolder color = (pose ? getBubbleColor_Pose(bubbles[j]) : getBubbleColor(bubbles[j]));
		color = ColorMixer::mix(BackgroundRect::getBackColor(), color, alpha);
		const float lineWidth = 0.75f;

		float coordsAndColor[(CircularLavaHazard::BubbleSideCount*2)*(2+4)];
		for (int i = 0; i < CircularLavaHazard::BubbleSideCount; i++) {
			coordsAndColor[(i*2)  *6]   = (bubbles[j].getX()*static_cast<float>(this->cir.r) + static_cast<float>(this->cir.x)) + (bubbles[j].getR() - lineWidth) * bubble_vertices[i+1].getXComp();
			coordsAndColor[(i*2)  *6+1] = (bubbles[j].getY()*static_cast<float>(this->cir.r) + static_cast<float>(this->cir.y)) + (bubbles[j].getR() - lineWidth) * bubble_vertices[i+1].getYComp();
			coordsAndColor[(i*2+1)*6]   = (bubbles[j].getX()*static_cast<float>(this->cir.r) + static_cast<float>(this->cir.x)) + (bubbles[j].getR() + lineWidth) * bubble_vertices[i+1].getXComp();
			coordsAndColor[(i*2+1)*6+1] = (bubbles[j].getY()*static_cast<float>(this->cir.r) + static_cast<float>(this->cir.y)) + (bubbles[j].getR() + lineWidth) * bubble_vertices[i+1].getYComp();

			coordsAndColor[(i*2)  *6+2] = color.getRf();
			coordsAndColor[(i*2)  *6+3] = color.getGf();
			coordsAndColor[(i*2)  *6+4] = color.getBf();
			coordsAndColor[(i*2)  *6+5] = color.getAf();
			coordsAndColor[(i*2+1)*6+2] = color.getRf();
			coordsAndColor[(i*2+1)*6+3] = color.getGf();
			coordsAndColor[(i*2+1)*6+4] = color.getBf();
			coordsAndColor[(i*2+1)*6+5] = color.getAf();
		}

		Renderer::SubmitBatchedDraw(coordsAndColor, (CircularLavaHazard::BubbleSideCount*2)*(2+4), outline_indices, CircularLavaHazard::BubbleSideCount*6);
	}
}

CircleHazard* CircularLavaHazard::randomizingFactory(double x_start, double y_start, double area_width, double area_height, const GenericFactoryConstructionData& args) {
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
			radius = LevelRNG::randNumInRange(20, 40); //TODO: where should these constants be?
		}
		xpos = LevelRNG::randNumInRange(x_start + radius, x_start + area_width - radius);
		ypos = LevelRNG::randNumInRange(y_start + radius, y_start + area_height - radius);
		CircleHazard* testCircularLava = new CircularLavaHazard(xpos, ypos, radius);
		if (testCircularLava->reasonableLocation()) {
			randomized = testCircularLava;
			break;
		} else {
			delete testCircularLava;
		}
		attempts++;
	} while (attempts < 64);

	return randomized;
}
