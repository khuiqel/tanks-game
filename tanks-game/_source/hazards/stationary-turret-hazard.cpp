#include "stationary-turret-hazard.h"

#include "../constants.h"
#include <cmath>
#include <algorithm> //std::copy, std::clamp
#include <iostream>
#include "../rng.h"

#include "../renderer.h"
#include "../color-mixer.h"
#include "../background-rect.h"

#include "../collision-handler.h"
#include "../tank.h"
#include "../tank-manager.h"
#include "../bullet-manager.h"
#include "../wall-manager.h"
#include "../hazard-manager.h"

SimpleVector2D StationaryTurretHazard::body_vertices[Circle::NumOfSides+1];
unsigned int StationaryTurretHazard::body_indices[Circle::NumOfSides*3];
unsigned int StationaryTurretHazard::outline_indices[Circle::NumOfSides*2*3];
bool StationaryTurretHazard::initialized_vertices = false;

const ColorValueHolder StationaryTurretHazard::stateColors[maxState] = { {0.5f, 0.5f, 0.5f}, {1.0f, 0x22/255.0, 0x11/255.0}, {0.0f, 0.5f, 1.0f} };

std::unordered_map<std::string, float> StationaryTurretHazard::getWeights() const {
	std::unordered_map<std::string, float> weights;
	weights.insert({ "vanilla", 1.0f });
	weights.insert({ "random-vanilla", 1.0f });
	weights.insert({ "old", 1.0f });
	weights.insert({ "random-old", 1.0f });
	weights.insert({ "random", 1.0f });
	return weights;
}

StationaryTurretHazard::StationaryTurretHazard(double xpos, double ypos, double angle) : CircleHazard(HAZARD_TEAM) {
	x = xpos;
	y = ypos;
	r = TANK_RADIUS / 4;
	velocity = SimpleVector2D(angle, 0, true);

	tickCount = 0;
	tickCycle = 100; //100 is JS default (because of shooting speed)
	currentState = 0;
	constexpr double temp[maxState] = { 2, 1, 2 };
	std::copy(temp, temp + maxState, stateMultiplier);

	//canAcceptPowers = false;

	initializeVertices();
}

StationaryTurretHazard::~StationaryTurretHazard() {
	//nothing
}

bool StationaryTurretHazard::initializeVertices() {
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

	initialized_vertices = true;
	return true;
}

CircleHazard* StationaryTurretHazard::factory(const GenericFactoryConstructionData& args) {
	if (args.getDataCount() >= 1) [[likely]] {
		int count = args.getDataPortionLength(0);

		if (count >= 3) [[likely]] {
			const double* arr = static_cast<const double*>(args.getDataPortion(0).get());
			double x = arr[0];
			double y = arr[1];
			double a = arr[2];
			return new StationaryTurretHazard(x, y, a);
		}
	}

	return new StationaryTurretHazard(0, 0, 0);
}

void StationaryTurretHazard::tick() {
	tickCount++;
	bool mustShoot = false; //in case two state cycles happen at once (this will have annoying unit tests)
	while (tickCount >= tickCycle * stateMultiplier[currentState]) {
		if (tickCycle * stateMultiplier[currentState] <= 0) {
			tickCount = 0;
			currentState = 2;
			mustShoot = true;
			break;
		} else {
			tickCount -= tickCycle * stateMultiplier[currentState];
			++currentState %= maxState;
			if (currentState == 2) {
				mustShoot = true;
			}
		}
	}
	if (mustShoot) {
		BulletManager::pushBullet(new Bullet(x + r*std::cos(velocity.getAngle()), y + r*std::sin(velocity.getAngle()), r*(BULLET_TO_TANK_RADIUS_RATIO*2), velocity.getAngle(), Tank::default_maxSpeed*BULLET_TO_TANK_SPEED_RATIO, this->getTeamID(), BulletParentType::individual, this->getGameID()));
	}
}

bool StationaryTurretHazard::canSeeTank(const Tank* t) const {
	const SimpleVector2D distToTank = SimpleVector2D(t->getX() - x, t->getY() - y);

	//line of sight
	float angleDiff = SimpleVector2D::angleBetween(distToTank, this->velocity);
	float projMag = distToTank.getMagnitude() * std::cos(angleDiff);
	float distToLineOfSightSquared = distToTank.getMagnitude()*distToTank.getMagnitude() - projMag*projMag;
	if (distToLineOfSightSquared > t->r * t->r) {
		return false;
	}

	//check walls
	for (int i = 0; i < WallManager::getNumWalls(); i++) {
		const Wall* wa = WallManager::getWall(i);
		if (CollisionHandler::lineRectCollision(x, y, x + distToTank.getMagnitude()*std::cos(this->velocity.getAngle()), y + distToTank.getMagnitude()*std::sin(this->velocity.getAngle()), wa)) {
			return false;
		}
	}

	return true;
}

bool StationaryTurretHazard::reasonableLocation() const {
	for (int i = 0; i < TankManager::getNumTanks(); i++) {
		if (canSeeTank(TankManager::getTank(i))) {
			return false;
		}
	}

	for (int i = 0; i < WallManager::getNumWalls(); i++) {
		if (CollisionHandler::partiallyCollided(this, WallManager::getWall(i))) {
			return false;
		}
	}

	for (int i = 0; i < HazardManager::getNumCircleHazards(); i++) {
		const CircleHazard* ch = HazardManager::getCircleHazard(i);
		if (ch->getGameID() != this->getGameID()) [[likely]] {
			if (CollisionHandler::partiallyCollided(this, ch)) {
				return false;
			}
		}
	}
	for (int i = 0; i < HazardManager::getNumRectHazards(); i++) {
		if (CollisionHandler::partiallyCollided(this, HazardManager::getRectHazard(i))) {
			return false;
		}
	}

	return validLocation();
}

ColorValueHolder StationaryTurretHazard::getColor() const {
	return ColorMixer::mix(stateColors[currentState], stateColors[(currentState+1)%maxState], std::clamp<float>(tickCount/(tickCycle*stateMultiplier[currentState]), 0, 1));
}

ColorValueHolder StationaryTurretHazard::getColor(unsigned int state) const {
	return stateColors[state % maxState];
}

void StationaryTurretHazard::draw() const {
	drawBody();
	drawOutline();
	drawBarrel();
}

void StationaryTurretHazard::draw(DrawingLayers layer) const {
	switch (layer) {
		case DrawingLayers::under:
			//nothing
			break;

		default:
			std::cerr << "WARNING: unknown DrawingLayer for " + getName() + " draw!" << std::endl;
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

void StationaryTurretHazard::poseDraw() const {
	drawBody();
	drawOutline();
	drawBarrel();
}

void StationaryTurretHazard::poseDraw(DrawingLayers layer) const {
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

void StationaryTurretHazard::ghostDraw(float alpha) const {
	drawBody(alpha);
	drawOutline(alpha);
	drawBarrel(alpha);
}

void StationaryTurretHazard::ghostDraw(DrawingLayers layer, float alpha) const {
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

void StationaryTurretHazard::drawBody(float alpha) const {
	alpha = alpha * alpha;
	ColorValueHolder color = getColor();
	color = ColorMixer::mix(BackgroundRect::getBackColor(), color, alpha);

	float coordsAndColor[(Circle::NumOfSides+1)*(2+4)];
	coordsAndColor[0] = x;
	coordsAndColor[1] = y;
	coordsAndColor[2] = color.getRf();
	coordsAndColor[3] = color.getGf();
	coordsAndColor[4] = color.getBf();
	coordsAndColor[5] = color.getAf();
	for (int i = 1; i < Circle::NumOfSides+1; i++) {
		coordsAndColor[i*6]   = static_cast<float>(x) + static_cast<float>(r) * body_vertices[i].getXComp();
		coordsAndColor[i*6+1] = static_cast<float>(y) + static_cast<float>(r) * body_vertices[i].getYComp();
		coordsAndColor[i*6+2] = color.getRf();
		coordsAndColor[i*6+3] = color.getGf();
		coordsAndColor[i*6+4] = color.getBf();
		coordsAndColor[i*6+5] = color.getAf();
	}

	Renderer::SubmitBatchedDraw(coordsAndColor, (Circle::NumOfSides+1)*(2+4), body_indices, Circle::NumOfSides*3);
}

void StationaryTurretHazard::drawOutline(float alpha) const {
	drawOutline(alpha, 0.5f);
}
void StationaryTurretHazard::drawOutline(float alpha, float lineWidth) const {
	alpha = alpha * alpha;
	ColorValueHolder color = ColorValueHolder(0.0f, 0.0f, 0.0f);
	color = ColorMixer::mix(BackgroundRect::getBackColor(), color, alpha);

	float coordsAndColor[(Circle::NumOfSides*2)*(2+4)];
	for (int i = 0; i < Circle::NumOfSides; i++) {
		coordsAndColor[(i*2)  *6]   = static_cast<float>(x) + (static_cast<float>(r)-lineWidth) * body_vertices[i+1].getXComp();
		coordsAndColor[(i*2)  *6+1] = static_cast<float>(y) + (static_cast<float>(r)-lineWidth) * body_vertices[i+1].getYComp();
		coordsAndColor[(i*2+1)*6]   = static_cast<float>(x) + (static_cast<float>(r)+lineWidth) * body_vertices[i+1].getXComp();
		coordsAndColor[(i*2+1)*6+1] = static_cast<float>(y) + (static_cast<float>(r)+lineWidth) * body_vertices[i+1].getYComp();

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

void StationaryTurretHazard::drawBarrel(float alpha) const {
	drawBarrel(alpha, 0.75f);
}
void StationaryTurretHazard::drawBarrel(float alpha, float lineWidth) const {
	alpha = alpha * alpha;
	ColorValueHolder color = ColorValueHolder(0.0f, 0.0f, 0.0f);
	color = ColorMixer::mix(BackgroundRect::getBackColor(), color, alpha);

	float coordsAndColor[4*(2+4)];
	unsigned int indices[6];

	SimpleVector2D dist = SimpleVector2D(velocity.getAngle(), r, true);
	SimpleVector2D distCW = SimpleVector2D(velocity.getAngle() - float(PI/2), lineWidth, true);

	coordsAndColor[0*6]   = static_cast<float>(x)                   + distCW.getXComp();
	coordsAndColor[0*6+1] = static_cast<float>(y)                   + distCW.getYComp();
	coordsAndColor[1*6]   = static_cast<float>(x) + dist.getXComp() + distCW.getXComp();
	coordsAndColor[1*6+1] = static_cast<float>(y) + dist.getYComp() + distCW.getYComp();
	coordsAndColor[2*6]   = static_cast<float>(x) + dist.getXComp() - distCW.getXComp();
	coordsAndColor[2*6+1] = static_cast<float>(y) + dist.getYComp() - distCW.getYComp();
	coordsAndColor[3*6]   = static_cast<float>(x)                   - distCW.getXComp();
	coordsAndColor[3*6+1] = static_cast<float>(y)                   - distCW.getYComp();

	for (int i = 0; i < 4; i++) {
		coordsAndColor[i*6+2] = color.getRf();
		coordsAndColor[i*6+3] = color.getGf();
		coordsAndColor[i*6+4] = color.getBf();
		coordsAndColor[i*6+5] = color.getAf();
	}

	indices[0] = 0;
	indices[1] = 1;
	indices[2] = 2;
	indices[3] = 2;
	indices[4] = 3;
	indices[5] = 0;

	Renderer::SubmitBatchedDraw(coordsAndColor, 4*(2+4), indices, 6);
}

CircleHazard* StationaryTurretHazard::randomizingFactory(double x_start, double y_start, double area_width, double area_height, const GenericFactoryConstructionData& args) {
	int attempts = 0;
	CircleHazard* randomized = nullptr;
	double xpos, ypos, angle;

	int count = 0;
	if (args.getDataCount() >= 1) {
		count = args.getDataPortionLength(0);
	}
	if (count >= 1) {
		const double* arr = static_cast<const double*>(args.getDataPortion(0).get());
		angle = arr[0];
	} else {
		angle = LevelRNG::randFunc() * (2*PI);
	}

	const double realRadius = (TANK_RADIUS/4);

	do {
		xpos = LevelRNG::randNumInRange(x_start + realRadius, x_start + area_width - realRadius);
		ypos = LevelRNG::randNumInRange(y_start + realRadius, y_start + area_height - realRadius);
		CircleHazard* testStationaryTurret = new StationaryTurretHazard(xpos, ypos, angle);
		if (testStationaryTurret->reasonableLocation()) {
			randomized = testStationaryTurret;
			break;
		} else {
			delete testStationaryTurret;
		}
		attempts++;
	} while (attempts < 128);

	return randomized;
}
