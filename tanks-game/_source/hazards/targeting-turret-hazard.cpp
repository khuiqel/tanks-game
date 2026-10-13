#include "targeting-turret-hazard.h"

#include "../constants.h"
#include <cmath>
#include <algorithm> //std::clamp
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

const ColorValueHolder TargetingTurretHazard::reticuleColors[2] = { {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f} };

std::unordered_map<std::string, float> TargetingTurretHazard::getWeights() const {
	std::unordered_map<std::string, float> weights;
	weights.insert({ "vanilla", 1.0f });
	weights.insert({ "random-vanilla", 1.0f });
	weights.insert({ "old", 1.0f });
	weights.insert({ "random-old", 1.0f });
	weights.insert({ "random", 1.0f });
	return weights;
}

TargetingTurretHazard::TargetingTurretHazard(double xpos, double ypos, double angle) : StationaryTurretHazard(xpos, ypos, angle) {
	r = TANK_RADIUS / 2;

	targeting = false;
	targetingX = xpos;
	targetingY = ypos;
	targetingCount = 0;
	trackingID = this->getGameID();

	//canAcceptPowers = false; //... true?
}

TargetingTurretHazard::~TargetingTurretHazard() {
	//nothing
}

CircleHazard* TargetingTurretHazard::factory(const GenericFactoryConstructionData& args) {
	if (args.getDataCount() >= 1) [[likely]] {
		int count = args.getDataPortionLength(0);

		if (count >= 3) [[likely]] {
			const double* arr = static_cast<const double*>(args.getDataPortion(0).get());
			double x = arr[0];
			double y = arr[1];
			double a = arr[2];
			return new TargetingTurretHazard(x, y, a);
		}
	}

	return new TargetingTurretHazard(0, 0, 0);
}

inline void TargetingTurretHazard::updateTrackingPos(const Tank* t, bool pointedAt) {
	if (pointedAt) {
		targetingX = t->x;
		targetingY = t->y;
	} else {
		float dist = std::sqrt((t->x - x)*(t->x - x) + (t->y - y)*(t->y - y));
		targetingX = static_cast<float>(x) + dist * std::cos(velocity.getAngle());
		targetingY = static_cast<float>(y) + dist * std::sin(velocity.getAngle());
	}
}

void TargetingTurretHazard::tick() {
	//tickCount is unused... maybe targetingCount should replace it...

	if (currentState == 0) { //either tracking a tank or doing nothing
		if (targeting) { //tracking tank
			tick_continueTracking();
		}
		if (!targeting) { //looking for a tank
			targetingCount = 0;
			tick_lookForNewTarget();
		}
	}

	if (currentState == 1) { //charging up to shoot (and shooting)
		tick_chargeUp();
	}

	if (currentState == 2) { //cooling down
		tick_cooldown();
	}

	//maxState is 3; not using else in case tickCycle is zero
}

void TargetingTurretHazard::tick_continueTracking() {
	bool tankIsVisible = false;
	const Tank* t = TankManager::getTankByID(this->trackingID);
	if (t != nullptr) { //exists
		if (canSeeTank(t)) { //no obstructions
			tankIsVisible = true;
			//main logic here
			turnTowardsTank(t);
			if (isPointedAt(t)) {
				targetingCount++;
				if (targetingCount >= stateMultiplier[0] * tickCycle) {
					currentState = 1;
					targetingCount = 0;
				}
				updateTrackingPos(t, true);
			} else {
				targetingCount--;
				if (targetingCount < 0) {
					targetingCount = 0;
				}
				updateTrackingPos(t, false);
			}
		}
	}
	if (!tankIsVisible) {
		targeting = false;
	}
}

void TargetingTurretHazard::tick_lookForNewTarget() {
	//targetingCount = 0;

	std::vector<char> tankVisibility; tankVisibility.reserve(TankManager::getNumTanks()); //not using regular arrays so people (including future me) can actually read this
	std::vector<double> distancesSquaredToTank; distancesSquaredToTank.reserve(TankManager::getNumTanks()); //TODO: option for angle-based selection (look at homing in PowerFunctionHelper)
	for (int i = 0; i < TankManager::getNumTanks(); i++) {
		const Tank* t = TankManager::getTank(i);
		tankVisibility.push_back((char)canSeeTank(t));
		if (tankVisibility[i]) {
			distancesSquaredToTank.push_back((t->x - x)*(t->x - x) + (t->y - y)*(t->y - y));
		} else {
			distancesSquaredToTank.push_back(GAME_WIDTH*GAME_WIDTH*4 + GAME_HEIGHT*GAME_HEIGHT*4);
		}
	}

	double minDist = GAME_WIDTH*GAME_WIDTH*4 + GAME_HEIGHT*GAME_HEIGHT*4;
	std::vector<int> tankIndices; tankIndices.reserve(TankManager::getNumTanks()); //multiple tanks can have same distance
	for (int i = 0; i < TankManager::getNumTanks(); i++) {
		if (distancesSquaredToTank[i] == minDist) {
			tankIndices.push_back(i);
		} else if (distancesSquaredToTank[i] < minDist) {
			tankIndices.clear();
			tankIndices.push_back(i);
			minDist = distancesSquaredToTank[i];
		}
	}

	int indexOfTargetedTank;
	if (tankIndices.size() == 1) {
		indexOfTargetedTank = tankIndices[0];
	} else {
		indexOfTargetedTank = tankIndices[static_cast<int>(GameRNG::randFunc() * tankIndices.size())];
	}

	if (tankVisibility[indexOfTargetedTank]) {
		const Tank* t = TankManager::getTank(indexOfTargetedTank);
		trackingID = t->getGameID();
		targeting = true;
		updateTrackingPos(t, isPointedAt(t));
	}
}

void TargetingTurretHazard::tick_chargeUp() {
	targetingCount++;
	if (targetingCount >= stateMultiplier[1] * tickCycle) {
		BulletManager::pushBullet(new Bullet(x + r*std::cos(velocity.getAngle()), y + r*std::sin(velocity.getAngle()), r*BULLET_TO_TANK_RADIUS_RATIO, velocity.getAngle(), Tank::default_maxSpeed*BULLET_TO_TANK_SPEED_RATIO, this->getTeamID(), BulletParentType::individual, this->getGameID()));
		currentState = 2;
		targetingCount = 0;
		targeting = false; //allows target to change (also controls whether the reticule is drawn)
	}
}

void TargetingTurretHazard::tick_cooldown() {
	targetingCount++;
	if (targetingCount >= stateMultiplier[2] * tickCycle) {
		targetingCount = 0;
		currentState = 0;
	}
}

bool TargetingTurretHazard::canSeeTank(const Tank* t) const {
	for (int i = 0; i < WallManager::getNumWalls(); i++) {
		const Wall* wa = WallManager::getWall(i);
		if (CollisionHandler::lineRectCollision(x, y, t->x, t->y, wa)) {
			return false;
		}
	}
	return true;
}

void TargetingTurretHazard::turnTowardsTank(const Tank* t) {
	//see PowerFunctionHelper::homingGeneric
	SimpleVector2D distToTank = SimpleVector2D(t->getX() - this->x, t->getY() - this->y);
	float theta = SimpleVector2D::angleBetween(distToTank, velocity);
	if (std::abs(theta) < float(PI)/turningIncrement) {
		//too small to adjust angle
	} else {
		//large angle adjustment needed
		if (theta < 0) {
			this->velocity.changeAngle(float(PI)/turningIncrement);
		} else {
			this->velocity.changeAngle(float(-PI)/turningIncrement);
		}
	}
}

bool TargetingTurretHazard::isPointedAt(const Tank* t) const {
	return (std::abs(SimpleVector2D::angleBetween(velocity, SimpleVector2D(t->x - x, t->y - y))) < float(PI)/turningIncrement);
}

bool TargetingTurretHazard::reasonableLocation() const {
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

ColorValueHolder TargetingTurretHazard::getColor() const {
	return ColorMixer::mix(stateColors[currentState], stateColors[(currentState+1)%maxState], std::clamp<float>(targetingCount/(tickCycle*stateMultiplier[currentState]), 0, 1));
}

ColorValueHolder TargetingTurretHazard::getReticuleColor() const {
	if (currentState == 0) {
		return reticuleColors[0];
	}
	if (currentState == 1) {
		return reticuleColors[1];
	}
	return ColorValueHolder(0.0f, 0.0f, 0.0f); //shouldn't be drawn
}

void TargetingTurretHazard::draw() const {
	drawBody();
	drawOutline();
	drawBarrel();
	drawReticule();
}

void TargetingTurretHazard::draw(DrawingLayers layer) const {
	switch (layer) {
		case DrawingLayers::under:
			//nothing
			break;

		default:
			std::cerr << "WARNING: unknown DrawingLayer for " + getName() + " draw!\n";
			[[fallthrough]];
		case DrawingLayers::normal:
			drawBody();
			drawOutline();
			drawBarrel();
			break;

		case DrawingLayers::effects:
			drawReticule();
			break;

		case DrawingLayers::top:
			//nothing
			break;

		case DrawingLayers::debug:
			//later
			break;
	}
}

void TargetingTurretHazard::poseDraw() const {
	drawBody();
	drawOutline();
	drawBarrel();
}

void TargetingTurretHazard::poseDraw(DrawingLayers layer) const {
	switch (layer) {
		case DrawingLayers::under:
			//nothing
			break;

		default:
			std::cerr << "WARNING: unknown DrawingLayer for " + getName() + " poseDraw!\n";
			[[fallthrough]];
		case DrawingLayers::normal:
			drawBody();
			drawOutline();
			drawBarrel();
			break;

		case DrawingLayers::effects:
			//drawReticule();
			break;

		case DrawingLayers::top:
			//nothing
			break;

		case DrawingLayers::debug:
			//later
			break;
	}
}

void TargetingTurretHazard::ghostDraw(float alpha) const {
	drawBody(alpha);
	drawOutline(alpha);
	drawBarrel(alpha);
}

void TargetingTurretHazard::ghostDraw(DrawingLayers layer, float alpha) const {
	switch (layer) {
		case DrawingLayers::under:
			//nothing
			break;

		default:
			std::cerr << "WARNING: unknown DrawingLayer for " + getName() + " ghostDraw!\n";
			[[fallthrough]];
		case DrawingLayers::normal:
			drawBody(alpha);
			drawOutline(alpha);
			drawBarrel(alpha);
			break;

		case DrawingLayers::effects:
			//drawReticule(alpha);
			break;

		case DrawingLayers::top:
			//nothing
			break;

		case DrawingLayers::debug:
			//later
			break;
	}
}

void TargetingTurretHazard::drawReticule(float alpha) const {
	if (!targeting) {
		return;
	}

	ColorValueHolder color_outline = getReticuleColor();
	color_outline = ColorMixer::mix(BackgroundRect::getBackColor(), color_outline, alpha);
	const float lineWidth = 1.0f;
	const float reticuleRadius = (2*TANK_RADIUS);
	//optional rotate angle: (-PI/2) * (targetingCount/(stateMultiplier[0] * tickCycle))

	float coordsAndColor_outline[(Circle::NumOfSides*2 + 4*4*2)*(2+4)];
	for (int i = 0; i < Circle::NumOfSides; i++) {
		coordsAndColor_outline[(i*2)  *6]   = targetingX + (reticuleRadius - lineWidth) * body_vertices[i+1].getXComp();
		coordsAndColor_outline[(i*2)  *6+1] = targetingY + (reticuleRadius - lineWidth) * body_vertices[i+1].getYComp();
		coordsAndColor_outline[(i*2+1)*6]   = targetingX + (reticuleRadius + lineWidth) * body_vertices[i+1].getXComp();
		coordsAndColor_outline[(i*2+1)*6+1] = targetingY + (reticuleRadius + lineWidth) * body_vertices[i+1].getYComp();

		coordsAndColor_outline[(i*2)  *6+2] = color_outline.getRf();
		coordsAndColor_outline[(i*2)  *6+3] = color_outline.getGf();
		coordsAndColor_outline[(i*2)  *6+4] = color_outline.getBf();
		coordsAndColor_outline[(i*2)  *6+5] = color_outline.getAf();
		coordsAndColor_outline[(i*2+1)*6+2] = color_outline.getRf();
		coordsAndColor_outline[(i*2+1)*6+3] = color_outline.getGf();
		coordsAndColor_outline[(i*2+1)*6+4] = color_outline.getBf();
		coordsAndColor_outline[(i*2+1)*6+5] = color_outline.getAf();
	}

	coordsAndColor_outline[(Circle::NumOfSides*2 + (4*0))  *6]   = targetingX + ( .75f*reticuleRadius - lineWidth);
	coordsAndColor_outline[(Circle::NumOfSides*2 + (4*0))  *6+1] = targetingY - lineWidth;
	coordsAndColor_outline[(Circle::NumOfSides*2 + (4*0+1))*6]   = targetingX + (1.25f*reticuleRadius + lineWidth);
	coordsAndColor_outline[(Circle::NumOfSides*2 + (4*0+1))*6+1] = targetingY - lineWidth;
	coordsAndColor_outline[(Circle::NumOfSides*2 + (4*0+2))*6]   = targetingX + (1.25f*reticuleRadius + lineWidth);
	coordsAndColor_outline[(Circle::NumOfSides*2 + (4*0+2))*6+1] = targetingY + lineWidth;
	coordsAndColor_outline[(Circle::NumOfSides*2 + (4*0+3))*6]   = targetingX + ( .75f*reticuleRadius - lineWidth);
	coordsAndColor_outline[(Circle::NumOfSides*2 + (4*0+3))*6+1] = targetingY + lineWidth;

	coordsAndColor_outline[(Circle::NumOfSides*2 + (4*1))  *6]   = targetingX + lineWidth;
	coordsAndColor_outline[(Circle::NumOfSides*2 + (4*1))  *6+1] = targetingY + ( .75f*reticuleRadius - lineWidth);
	coordsAndColor_outline[(Circle::NumOfSides*2 + (4*1+1))*6]   = targetingX + lineWidth;
	coordsAndColor_outline[(Circle::NumOfSides*2 + (4*1+1))*6+1] = targetingY + (1.25f*reticuleRadius + lineWidth);
	coordsAndColor_outline[(Circle::NumOfSides*2 + (4*1+2))*6]   = targetingX - lineWidth;
	coordsAndColor_outline[(Circle::NumOfSides*2 + (4*1+2))*6+1] = targetingY + (1.25f*reticuleRadius + lineWidth);
	coordsAndColor_outline[(Circle::NumOfSides*2 + (4*1+3))*6]   = targetingX - lineWidth;
	coordsAndColor_outline[(Circle::NumOfSides*2 + (4*1+3))*6+1] = targetingY + ( .75f*reticuleRadius - lineWidth);

	coordsAndColor_outline[(Circle::NumOfSides*2 + (4*2))  *6]   = targetingX - ( .75f*reticuleRadius - lineWidth);
	coordsAndColor_outline[(Circle::NumOfSides*2 + (4*2))  *6+1] = targetingY - lineWidth;
	coordsAndColor_outline[(Circle::NumOfSides*2 + (4*2+1))*6]   = targetingX - (1.25f*reticuleRadius + lineWidth);
	coordsAndColor_outline[(Circle::NumOfSides*2 + (4*2+1))*6+1] = targetingY - lineWidth;
	coordsAndColor_outline[(Circle::NumOfSides*2 + (4*2+2))*6]   = targetingX - (1.25f*reticuleRadius + lineWidth);
	coordsAndColor_outline[(Circle::NumOfSides*2 + (4*2+2))*6+1] = targetingY + lineWidth;
	coordsAndColor_outline[(Circle::NumOfSides*2 + (4*2+3))*6]   = targetingX - ( .75f*reticuleRadius - lineWidth);
	coordsAndColor_outline[(Circle::NumOfSides*2 + (4*2+3))*6+1] = targetingY + lineWidth;

	coordsAndColor_outline[(Circle::NumOfSides*2 + (4*3))  *6]   = targetingX - lineWidth;
	coordsAndColor_outline[(Circle::NumOfSides*2 + (4*3))  *6+1] = targetingY - ( .75f*reticuleRadius - lineWidth);
	coordsAndColor_outline[(Circle::NumOfSides*2 + (4*3+1))*6]   = targetingX - lineWidth;
	coordsAndColor_outline[(Circle::NumOfSides*2 + (4*3+1))*6+1] = targetingY - (1.25f*reticuleRadius + lineWidth);
	coordsAndColor_outline[(Circle::NumOfSides*2 + (4*3+2))*6]   = targetingX + lineWidth;
	coordsAndColor_outline[(Circle::NumOfSides*2 + (4*3+2))*6+1] = targetingY - (1.25f*reticuleRadius + lineWidth);
	coordsAndColor_outline[(Circle::NumOfSides*2 + (4*3+3))*6]   = targetingX + lineWidth;
	coordsAndColor_outline[(Circle::NumOfSides*2 + (4*3+3))*6+1] = targetingY - ( .75f*reticuleRadius - lineWidth);

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			coordsAndColor_outline[(Circle::NumOfSides*2 + (4*i+j))*6+2] = color_outline.getRf();
			coordsAndColor_outline[(Circle::NumOfSides*2 + (4*i+j))*6+3] = color_outline.getGf();
			coordsAndColor_outline[(Circle::NumOfSides*2 + (4*i+j))*6+4] = color_outline.getBf();
			coordsAndColor_outline[(Circle::NumOfSides*2 + (4*i+j))*6+5] = color_outline.getAf();
		}
	}

	unsigned int indices_outline[Circle::NumOfSides*6 + 4*6];
	std::copy(StationaryTurretHazard::outline_indices, StationaryTurretHazard::outline_indices + Circle::NumOfSides*6, indices_outline);

	for (int i = 0; i < 4; i++) {
		indices_outline[Circle::NumOfSides*6 + i*6]   = (Circle::NumOfSides*2 + 4*i);
		indices_outline[Circle::NumOfSides*6 + i*6+1] = (Circle::NumOfSides*2 + 4*i+1);
		indices_outline[Circle::NumOfSides*6 + i*6+2] = (Circle::NumOfSides*2 + 4*i+2);
		indices_outline[Circle::NumOfSides*6 + i*6+3] = (Circle::NumOfSides*2 + 4*i+2);
		indices_outline[Circle::NumOfSides*6 + i*6+4] = (Circle::NumOfSides*2 + 4*i+3);
		indices_outline[Circle::NumOfSides*6 + i*6+5] = (Circle::NumOfSides*2 + 4*i);
	}

	Renderer::SubmitBatchedDraw(coordsAndColor_outline, (Circle::NumOfSides*2 + 4*4*2)*(2+4), indices_outline, Circle::NumOfSides*6 + 4*6);
}

CircleHazard* TargetingTurretHazard::randomizingFactory(double x_start, double y_start, double area_width, double area_height, const GenericFactoryConstructionData& args) {
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

	const double realRadius = (TANK_RADIUS/2);

	do {
		xpos = LevelRNG::randNumInRange(x_start + realRadius, x_start + area_width - realRadius);
		ypos = LevelRNG::randNumInRange(y_start + realRadius, y_start + area_height - realRadius);
		CircleHazard* testTargetingTurret = new TargetingTurretHazard(xpos, ypos, angle);
		if (testTargetingTurret->reasonableLocation()) {
			randomized = testTargetingTurret;
			break;
		} else {
			delete testTargetingTurret;
		}
		attempts++;
	} while (attempts < 128);

	return randomized;
}
