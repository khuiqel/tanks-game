#include "gravity-well-hazard.h"

#include "../constants.h"
#include <cmath>
#include <algorithm> //std::copy
#include <iostream>
#include "../rng.h"

#include "../renderer.h"
#include "../color-mixer.h"
#include "../background-rect.h"

#include "../collision-handler.h"
#include "../tank-manager.h"
#include "../bullet-manager.h"
#include "../wall-manager.h"
#include "../hazard-manager.h"
#include "../powerup-manager.h"

SimpleVector2D GravityWellHazard::body_vertices[Circle::NumOfSides+1];
unsigned int GravityWellHazard::body_indices[Circle::NumOfSides*3];
unsigned int GravityWellHazard::outline_indices[Circle::NumOfSides*2*3];
SimpleVector2D GravityWellHazard::vertices_arrow[7];
unsigned int GravityWellHazard::indices_arrow[3*3];
bool GravityWellHazard::initialized_vertices = false;

const ColorValueHolder GravityWellHazard::color = ColorValueHolder(0.25f, 0.25f, 0.25f);

std::unordered_map<std::string, float> GravityWellHazard::getWeights() const {
	std::unordered_map<std::string, float> weights;
	weights.insert({ "vanilla-extra", 1.0f });
	weights.insert({ "random-vanilla", 0.5f });
	weights.insert({ "random", 0.25f });
	return weights;
}

GravityWellHazard::GravityWellHazard(double xpos, double ypos, double radius, double range, double minGravity, double maxGravity) : CircleHazard(HAZARD_TEAM) {
	x = xpos;
	y = ypos;
	r = radius;

	gravityRange = range;
	minGravityStrength = minGravity;
	maxGravityStrength = maxGravity;
	tickCount = 0;
	tickCycle = 200;
	//TODO: should the gravity cycle on/off? maybe transition states?

	//canAcceptPowers = false;

	initializeVertices();
}

GravityWellHazard::GravityWellHazard(double xpos, double ypos, double radius, double gravityRange) : GravityWellHazard(xpos, ypos, radius, gravityRange, 4.0/100, 75.0/100) {}

GravityWellHazard::GravityWellHazard(double xpos, double ypos, double radius) : GravityWellHazard(xpos, ypos, radius, radius*4) {}

GravityWellHazard::~GravityWellHazard() {
	//nothing
}

bool GravityWellHazard::initializeVertices() {
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

	for (int i = 0; i < Circle::NumOfSides; i++) {
		outline_indices[i*6]   =  i*2;
		outline_indices[i*6+1] =  i*2+1;
		outline_indices[i*6+2] = (i*2+3) % (Circle::NumOfSides*2);
		outline_indices[i*6+3] = (i*2+3) % (Circle::NumOfSides*2);
		outline_indices[i*6+4] = (i*2+2) % (Circle::NumOfSides*2);
		outline_indices[i*6+5] =  i*2;
	}

	//from WindLevelEffect:
	vertices_arrow[0] = SimpleVector2D(-1.0f, -0.125f); //bottom left
	vertices_arrow[1] = SimpleVector2D(-1.0f,  0.125f); //top left
	vertices_arrow[2] = SimpleVector2D( 0.5f,  0.125f); //top right
	vertices_arrow[3] = SimpleVector2D( 0.5f,  0.5f  );
	vertices_arrow[4] = SimpleVector2D( 1.0f,  0.0f  ); //arrow tip
	vertices_arrow[5] = SimpleVector2D( 0.5f, -0.5f  );
	vertices_arrow[6] = SimpleVector2D( 0.5f, -0.125f); //bottom right

	unsigned int arrow_temp[] = {
		0, 1, 2,
		2, 6, 0,
		3, 4, 5
	};
	std::copy(arrow_temp, arrow_temp + (3*3), indices_arrow);

	initialized_vertices = true;
	return true;
}

CircleHazard* GravityWellHazard::factory(const GenericFactoryConstructionData& args) {
	if (args.getDataCount() >= 1) [[likely]] {
		int count = args.getDataPortionLength(0);

		if (count >= 3) [[likely]] {
			const double* arr = static_cast<const double*>(args.getDataPortion(0).get());
			double x = arr[0];
			double y = arr[1];
			double r = arr[2];

			if (count >= 4) {
				double outerRange = arr[3];

				if ((args.getDataCount() >= 2) && (args.getDataPortionLength(1) >= 2)) {
					const double* arr_strength = static_cast<const double*>(args.getDataPortion(1).get());
					double minStrength = arr_strength[0];
					double maxStrength = arr_strength[1];
					return new GravityWellHazard(x, y, r, outerRange, minStrength, maxStrength);
				}

				return new GravityWellHazard(x, y, r, outerRange);
			}

			return new GravityWellHazard(x, y, r);
		}
	}

	return new GravityWellHazard(0, 0, 0);
}

inline float GravityWellHazard::getGravityStrength(double dist) const {
	return (1 - dist/(this->gravityRange - this->r)) * (maxGravityStrength - minGravityStrength) + minGravityStrength;
}

inline float GravityWellHazard::getInnerGravityCircleRadius() const {
	if (isGravityReversed()) {
		return     ((tickCount/tickCycle)) * (gravityRange - r) + r;
	} else {
		return (1 - (tickCount/tickCycle)) * (gravityRange - r) + r;
	}
}

inline Circle GravityWellHazard::getGravityRangeCircle() const {
	Circle c;
	c.x = this->x;
	c.y = this->y;
	c.r = this->gravityRange;
	return c;
}

void GravityWellHazard::tick() {
	const Circle gravityBounds = getGravityRangeCircle();
	for (int i = 0; i < TankManager::getNumTanks(); i++) {
		Tank* t = TankManager::getTank(i);
		if (CollisionHandler::partiallyCollidedIgnoreEdge(t, &gravityBounds)) {
			const SimpleVector2D distToTank = SimpleVector2D(t->x - this->x, t->y - this->y);
			const double distFromSurfaceToClosestPoint = distToTank.getMagnitude() - (t->r + this->r);
			SimpleVector2D tankMovement = SimpleVector2D(distToTank.getAngle(), getGravityStrength(distFromSurfaceToClosestPoint), true);

			t->x -= tankMovement.getXComp();
			t->y -= tankMovement.getYComp();
		}
	}
	for (int i = 0; i < BulletManager::getNumBullets(); i++) {
		Bullet* b = BulletManager::getBullet(i);
		if (CollisionHandler::partiallyCollidedIgnoreEdge(b, &gravityBounds)) {
			const SimpleVector2D distToBullet = SimpleVector2D(b->x - this->x, b->y - this->y);
			const double distFromSurfaceToClosestPoint = distToBullet.getMagnitude() - (b->r + this->r);
			SimpleVector2D bulletMovement = SimpleVector2D(distToBullet.getAngle(), getGravityStrength(distFromSurfaceToClosestPoint), true);

			b->x -= bulletMovement.getXComp();
			b->y -= bulletMovement.getYComp();
		}
	}
	//doesn't change angles because that's complex

	tickCount++;
	if (tickCount >= tickCycle) {
		if (tickCycle <= 0) {
			tickCount = 0;
		} else {
			tickCount -= tickCycle;
		}
	}
}

bool GravityWellHazard::reasonableLocation() const {
	for (int i = 0; i < WallManager::getNumWalls(); i++) {
		if (CollisionHandler::partiallyCollided(this, WallManager::getWall(i))) {
			return false;
		}
	}

	for (int i = 0; i < HazardManager::getNumCircleHazards(); i++) {
		const CircleHazard* ch = HazardManager::getCircleHazard(i);
		if (ch->getGameID() != this->getGameID()) [[unlikely]] {
			if (CollisionHandler::partiallyCollided(this, ch)) {
				return false;
			}
		}
	}
	for (int i = 0; i < HazardManager::getNumRectHazards(); i++) {
		const RectHazard* rh = HazardManager::getRectHazard(i);
		if (CollisionHandler::partiallyCollided(this, rh)) {
			return false;
		}
	}

	return validLocation();
}

void GravityWellHazard::draw() const {
	drawGravityCircle();
	drawGravityArrows();
	drawBody();
	drawOutline();
}

void GravityWellHazard::draw(DrawingLayers layer) const {
	switch (layer) {
		case DrawingLayers::under:
			drawGravityCircle();
			drawGravityArrows();
			break;

		default:
			std::cerr << "WARNING: unknown DrawingLayer for " + getName() + " draw!" << std::endl;
			[[fallthrough]];
		case DrawingLayers::normal:
			drawBody();
			drawOutline();
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

void GravityWellHazard::poseDraw() const {
	drawBody();
	drawOutline();
}

void GravityWellHazard::poseDraw(DrawingLayers layer) const {
	switch (layer) {
		case DrawingLayers::under:
			//nothing
			break;

		default:
			std::cerr << "WARNING: unknown DrawingLayer for " + getName() + " poseDraw!" << std::endl;
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

void GravityWellHazard::ghostDraw(float alpha) const {
	drawBody(alpha);
	drawOutline(alpha);
}

void GravityWellHazard::ghostDraw(DrawingLayers layer, float alpha) const {
	switch (layer) {
		case DrawingLayers::under:
			//nothing
			break;

		default:
			std::cerr << "WARNING: unknown DrawingLayer for " + getName() + " ghostDraw!" << std::endl;
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

void GravityWellHazard::drawBody(float alpha) const {
	alpha = alpha * alpha;
	ColorValueHolder bodyColor = color;
	bodyColor = ColorMixer::mix(BackgroundRect::getBackColor(), bodyColor, alpha);

	float coordsAndColor[(Circle::NumOfSides+1)*(2+4)];
	coordsAndColor[0] = x;
	coordsAndColor[1] = y;
	coordsAndColor[2] = bodyColor.getRf();
	coordsAndColor[3] = bodyColor.getGf();
	coordsAndColor[4] = bodyColor.getBf();
	coordsAndColor[5] = bodyColor.getAf();
	for (int i = 1; i < Circle::NumOfSides+1; i++) {
		coordsAndColor[i*6]   = static_cast<float>(x) + static_cast<float>(r) * body_vertices[i].getXComp();
		coordsAndColor[i*6+1] = static_cast<float>(y) + static_cast<float>(r) * body_vertices[i].getYComp();
		coordsAndColor[i*6+2] = bodyColor.getRf();
		coordsAndColor[i*6+3] = bodyColor.getGf();
		coordsAndColor[i*6+4] = bodyColor.getBf();
		coordsAndColor[i*6+5] = bodyColor.getAf();
	}

	Renderer::SubmitBatchedDraw(coordsAndColor, (Circle::NumOfSides+1)*(2+4), body_indices, Circle::NumOfSides*3);
}

void GravityWellHazard::drawOutline(float alpha) const {
	alpha = alpha * alpha;
	ColorValueHolder color = ColorValueHolder(0.0f, 0.0f, 0.0f);
	color = ColorMixer::mix(BackgroundRect::getBackColor(), color, alpha);
	const float lineWidth = 0.5f;

	float coordsAndColor[(Circle::NumOfSides*2)*(2+4)];
	for (int i = 0; i < Circle::NumOfSides; i++) {
		coordsAndColor[(i*2)  *6]   = static_cast<float>(x) + (static_cast<float>(gravityRange)-lineWidth) * body_vertices[i+1].getXComp();
		coordsAndColor[(i*2)  *6+1] = static_cast<float>(y) + (static_cast<float>(gravityRange)-lineWidth) * body_vertices[i+1].getYComp();
		coordsAndColor[(i*2+1)*6]   = static_cast<float>(x) + (static_cast<float>(gravityRange)+lineWidth) * body_vertices[i+1].getXComp();
		coordsAndColor[(i*2+1)*6+1] = static_cast<float>(y) + (static_cast<float>(gravityRange)+lineWidth) * body_vertices[i+1].getYComp();

		coordsAndColor[(i*2)  *6+2] = color.getRf();
		coordsAndColor[(i*2)  *6+3] = color.getGf();
		coordsAndColor[(i*2)  *6+4] = color.getBf();
		coordsAndColor[(i*2)  *6+5] = color.getAf();
		coordsAndColor[(i*2+1)*6+2] = color.getRf();
		coordsAndColor[(i*2+1)*6+3] = color.getGf();
		coordsAndColor[(i*2+1)*6+4] = color.getBf();
		coordsAndColor[(i*2+1)*6+5] = color.getAf();
	}

	Renderer::SubmitBatchedDraw(coordsAndColor, (Circle::NumOfSides*2)*(2+4), outline_indices, Circle::NumOfSides*6);
}

void GravityWellHazard::drawGravityCircle(float alpha) const {
	alpha = alpha * alpha;
	ColorValueHolder color = ColorMixer::mix(BackgroundRect::getBackColor(), this->color, .25f);
	color = ColorMixer::mix(BackgroundRect::getBackColor(), color, alpha);
	const float lineWidth = this->r / 4;

	float coordsAndColor[(Circle::NumOfSides*2)*(2+4)];
	for (int i = 0; i < Circle::NumOfSides; i++) {
		coordsAndColor[(i*2)  *6]   = static_cast<float>(x) + (getInnerGravityCircleRadius()-lineWidth) * body_vertices[i+1].getXComp();
		coordsAndColor[(i*2)  *6+1] = static_cast<float>(y) + (getInnerGravityCircleRadius()-lineWidth) * body_vertices[i+1].getYComp();
		coordsAndColor[(i*2+1)*6]   = static_cast<float>(x) + (getInnerGravityCircleRadius())           * body_vertices[i+1].getXComp();
		coordsAndColor[(i*2+1)*6+1] = static_cast<float>(y) + (getInnerGravityCircleRadius())           * body_vertices[i+1].getYComp();

		coordsAndColor[(i*2)  *6+2] = color.getRf();
		coordsAndColor[(i*2)  *6+3] = color.getGf();
		coordsAndColor[(i*2)  *6+4] = color.getBf();
		coordsAndColor[(i*2)  *6+5] = color.getAf();
		coordsAndColor[(i*2+1)*6+2] = color.getRf();
		coordsAndColor[(i*2+1)*6+3] = color.getGf();
		coordsAndColor[(i*2+1)*6+4] = color.getBf();
		coordsAndColor[(i*2+1)*6+5] = color.getAf();
	}

	Renderer::SubmitBatchedDraw(coordsAndColor, (Circle::NumOfSides*2)*(2+4), outline_indices, Circle::NumOfSides*6);
}

void GravityWellHazard::drawGravityArrows(float alpha) const {
	alpha = alpha * alpha;
	ColorValueHolder color = ColorMixer::mix(BackgroundRect::getBackColor(), ColorValueHolder(0.0f, 0.0f, 0.0f));
	color = ColorMixer::mix(BackgroundRect::getBackColor(), color, alpha);
	const float arrowScale = this->r / 2; //x-coords are in [-1,1]

	float coordsAndColor_arrow[7*(2+4)];
	for (int i = 0; i < 4; i++) {
		const float rotateAngle = i*(PI/2) + PI/4;
		SimpleVector2D translateAmount = SimpleVector2D(rotateAngle, -arrowScale + getInnerGravityCircleRadius(), true);
		for (int j = 0; j < 7; j++) {
			SimpleVector2D vertex = SimpleVector2D(vertices_arrow[j]);
			if (isGravityReversed()) {
				vertex.scaleAndRotate(arrowScale, rotateAngle);
			} else {
				vertex.scaleAndRotate(arrowScale, rotateAngle + float(PI)); //flip arrow direction
			}

			coordsAndColor_arrow[j*6]   = static_cast<float>(this->x) + translateAmount.getXComp() + vertex.getXComp();
			coordsAndColor_arrow[j*6+1] = static_cast<float>(this->y) + translateAmount.getYComp() + vertex.getYComp();
			coordsAndColor_arrow[j*6+2] = color.getRf();
			coordsAndColor_arrow[j*6+3] = color.getGf();
			coordsAndColor_arrow[j*6+4] = color.getBf();
			coordsAndColor_arrow[j*6+5] = color.getAf();
		}

		Renderer::SubmitBatchedDraw(coordsAndColor_arrow, 7*(2+4), indices_arrow, 3*3);
	}
}

CircleHazard* GravityWellHazard::randomizingFactory(double x_start, double y_start, double area_width, double area_height, const GenericFactoryConstructionData& args) {
	int attempts = 0;
	CircleHazard* randomized = nullptr;
	double xpos, ypos, radius, gravityRange;
	double strengthMin, strengthMax;

	bool randomizeR;
	bool randomizeGR;
	int count = 0;
	if (args.getDataCount() >= 1) {
		count = args.getDataPortionLength(0);
	}
	if (count >= 1) {
		const double* arr = static_cast<const double*>(args.getDataPortion(0).get());
		radius = arr[0];
		randomizeR = false;
		if (count >= 2) {
			gravityRange = arr[1];
			randomizeGR = false;
		} else {
			randomizeGR = true;
		}
	} else {
		randomizeR = true;
		randomizeGR = true;
	}

	strengthMin = LevelRNG::randNumInRange(.04, .10);
	strengthMax = LevelRNG::randNumInRange(.55, .90);
	if (LevelRNG::randFunc() < .05) {
		strengthMin *= -1;
		strengthMax *= -1;
	}

	do {
		if (randomizeR) {
			radius = LevelRNG::randNumInRange(8, 16); //TODO: where should these constants be?
		}
		if (randomizeGR) {
			gravityRange = LevelRNG::randNumInRange(radius*2.5, radius*4);
		}
		const double realRadius = gravityRange;
		xpos = LevelRNG::randNumInRange(x_start + realRadius, x_start + area_width - realRadius);
		ypos = LevelRNG::randNumInRange(y_start + realRadius, y_start + area_height - realRadius);
		CircleHazard* testGravityWell = new GravityWellHazard(xpos, ypos, radius, gravityRange, strengthMin, strengthMax);
		if (testGravityWell->reasonableLocation()) {
			randomized = testGravityWell;
			break;
		} else {
			delete testGravityWell;
		}
		attempts++;
	} while (attempts < 64);

	return randomized;
}
