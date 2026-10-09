#include "patrolling-turret-hazard.h"

#include "../constants.h"
#include <cmath>
#include <algorithm> //std::copy, std::fill
#include <iostream>
#include "../rng.h"
#include "../mylib.h" //pointInPolygon, weightedSelect

#include "../renderer.h"
#include "../color-mixer.h"
#include "../background-rect.h"

#include "../collision-handler.h"
#include "../tank.h"
#include "../tank-manager.h"
#include "../bullet-manager.h"
#include "../wall-manager.h"
#include "../hazard-manager.h"

std::unordered_map<std::string, float> PatrollingTurretHazard::getWeights() const {
	std::unordered_map<std::string, float> weights;
	weights.insert({ "vanilla", 1.0f });
	weights.insert({ "random-vanilla", 1.0f });
	weights.insert({ "random", 1.0f });
	return weights;
}

PatrollingTurretHazard::PatrollingTurretHazard(double xpos, double ypos, double angle, int pairNum, const double* posList, const double* waitList) : TargetingTurretHazard(xpos, ypos, angle) {
	velocity = SimpleVector2D(angle, Tank::default_maxSpeed/2, true);

	currentPatrolTarget = 0;
	waitCount = 0;

	routePosPairNum = pairNum;
	if (posList == nullptr) {
		routePosList = nullptr;
		routeWaitCount = nullptr;
	} else {
		routePosList = new double[pairNum*2];
		std::copy(posList, posList+(pairNum*2), routePosList);

		routeWaitCount = new double[pairNum];
		if (waitList == nullptr) {
			std::fill(routeWaitCount, routeWaitCount + pairNum, 200.0);
		} else {
			std::copy(waitList, waitList+(pairNum), routeWaitCount);
		}
	}

	//canAcceptPowers = true;
}

PatrollingTurretHazard::~PatrollingTurretHazard() {
	if (routePosList != nullptr) {
		delete[] routePosList;
	}
	if (routeWaitCount != nullptr) {
		delete[] routeWaitCount;
	}
}

CircleHazard* PatrollingTurretHazard::factory(const GenericFactoryConstructionData& args) {
	if (args.getDataCount() >= 3) [[likely]] {
		int count_basic = args.getDataPortionLength(0);
		int count_num = args.getDataPortionLength(1);
		int count_list = args.getDataPortionLength(2);

		if (count_basic >= 3) [[likely]] {
			const double* arr_basic = static_cast<const double*>(args.getDataPortion(0).get());
			double x = arr_basic[0];
			double y = arr_basic[1];
			double a = arr_basic[2];

			if ((count_num >= 1) && (count_list >= 1)) {
				const int* arr_count = static_cast<const int*>(args.getDataPortion(1).get());
				const double* arr_list = static_cast<const double*>(args.getDataPortion(2).get());
				int count = arr_count[0];

				if ((args.getDataCount() >= 4) && (args.getDataPortionLength(3) >= 1)) {
					const double* wait = static_cast<const double*>(args.getDataPortion(3).get());
					return new PatrollingTurretHazard(x, y, a, count, arr_list, wait);
				} else {
					return new PatrollingTurretHazard(x, y, a, count, arr_list, nullptr);
				}
			} else {
				return new PatrollingTurretHazard(x, y, a, 0, nullptr, nullptr);
			}
		}
	}

	return new PatrollingTurretHazard(0, 0, 0, 0, nullptr, nullptr);
}

inline double PatrollingTurretHazard::getRoutePosX(int i) const {
	return routePosList[i*2];
}
inline double PatrollingTurretHazard::getRoutePosY(int i) const {
	return routePosList[i*2+1];
}

void PatrollingTurretHazard::tick() {
	TargetingTurretHazard::tick();
	//overrides tick_lookForNewTarget to do patrolling and stuff
}

void PatrollingTurretHazard::tick_lookForNewTarget() {
	targetingCount = 0;
	TargetingTurretHazard::tick_lookForNewTarget();

	if (targeting) {
		return;
	}

	tick_patrol();
}

void PatrollingTurretHazard::tick_patrol() {
	if (isWaitingAtPoint()) {
		tick_patrolWait();
	} else {
		turnTowardsPoint(/* TODO */);
		if (isPointedAtPoint(/* TODO */)) {
			tick_moveForward();
		}
	}
}

void PatrollingTurretHazard::tick_moveForward() {
	this->x += velocity.getXComp();
	this->y += velocity.getYComp();
}

void PatrollingTurretHazard::tick_patrolWait() {
	if (waitCount >= routeWaitCount[currentPatrolTarget]) {
		waitCount = 0;
		currentPatrolTarget = (currentPatrolTarget+1) % routePosPairNum;
	} else {
		waitCount++;
	}
}

bool PatrollingTurretHazard::isCloseAsPossibleToCurrentPoint() const {
	//find distance from current position to target point
	//if distance is less than current velocity, not close enough
	double xDist = this->x - getRoutePosX(currentPatrolTarget);
	double yDist = this->y - getRoutePosY(currentPatrolTarget);
	float distSquared = xDist*xDist + yDist*yDist;
	return (distSquared < velocity.getMagnitude() * velocity.getMagnitude());
}

bool PatrollingTurretHazard::isWaitingAtPoint() const {
	return isCloseAsPossibleToCurrentPoint();
}

void PatrollingTurretHazard::turnTowardsPoint() {
	//see TargetingTurretHazard::turnTowardsTank
	SimpleVector2D distToPoint = SimpleVector2D(getRoutePosX(currentPatrolTarget) - this->x, getRoutePosY(currentPatrolTarget) - this->y);
	float theta = SimpleVector2D::angleBetween(distToPoint, velocity);
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

bool PatrollingTurretHazard::isPointedAtPoint() const {
	SimpleVector2D distToPoint = SimpleVector2D(getRoutePosX(currentPatrolTarget) - this->x, getRoutePosY(currentPatrolTarget) - this->y);
	float theta = SimpleVector2D::angleBetween(distToPoint, velocity);
	return (std::abs(theta) < float(PI)/turningIncrement);
}

bool PatrollingTurretHazard::reasonableLocation() const {
	//this is only an approximation but there should only be problems for ultra-tiny walls
	//for each segment of the path, generate an "inner" and "outer" path, r distance away
	//check the inner, outer, and normal paths against every wall and hazard (ignore other patrolling turrets)

	if (routePosPairNum <= 1) {
		return TargetingTurretHazard::reasonableLocation();
	}

	for (int i = 0; i < routePosPairNum; i++) {
		const int start_pos = i;
		const int end_pos = (i+1) % routePosPairNum;

		const SimpleVector2D path = SimpleVector2D(getRoutePosX(end_pos) - getRoutePosX(start_pos), getRoutePosY(end_pos), getRoutePosY(start_pos));
		double outerPath[4] = {
			getRoutePosX(start_pos) + getR() * std::cos(path.getAngle() - float(PI/2)), getRoutePosY(start_pos) + getR() * std::sin(path.getAngle() - float(PI/2)),
			getRoutePosX(end_pos)   + getR() * std::cos(path.getAngle() - float(PI/2)), getRoutePosY(end_pos)   + getR() * std::sin(path.getAngle() - float(PI/2)) };
		double mainPath[4] = {
			getRoutePosX(start_pos), getRoutePosY(start_pos),
			getRoutePosX(end_pos), getRoutePosY(end_pos) };
		double innerPath[4] = {
			getRoutePosX(start_pos) + getR() * std::cos(path.getAngle() + float(PI/2)), getRoutePosY(start_pos) + getR() * std::sin(path.getAngle() + float(PI/2)),
			getRoutePosX(end_pos)   + getR() * std::cos(path.getAngle() + float(PI/2)), getRoutePosY(end_pos)   + getR() * std::sin(path.getAngle() + float(PI/2)) };

		for (int j = 0; j < WallManager::getNumWalls(); j++) {
			if (CollisionHandler::lineRectCollision(outerPath[0], outerPath[1], outerPath[2], outerPath[3], WallManager::getWall(j))) {
				return false;
			}
			if (CollisionHandler::lineRectCollision(mainPath[0], mainPath[1], mainPath[2], mainPath[3], WallManager::getWall(j))) {
				return false;
			}
			if (CollisionHandler::lineRectCollision(innerPath[0], innerPath[1], innerPath[2], innerPath[3], WallManager::getWall(j))) {
				return false;
			}
		}

		Circle testBody = Circle(getRoutePosX(i), getRoutePosY(i), this->r);
		for (int j = 0; j < WallManager::getNumWalls(); j++) {
			if (CollisionHandler::partiallyCollided(&testBody, WallManager::getWall(j))) {
				return false;
			}
		}

		//testing if the turret can see the tank at any point along its path: just accept doing some marching and call it a day
		for (int k = 0; k < TankManager::getNumTanks(); k++) {
			const Tank* t = TankManager::getTank(k);
			const SimpleVector2D testVelocity = SimpleVector2D(path.getAngle(), Tank::default_maxSpeed/2, true);
			double testX = getRoutePosX(i), testY = getRoutePosY(i);
			double distLeftToTravel = path.getMagnitude();
			do {
				bool wallBlocksSight = false;
				for (int j = 0; j < WallManager::getNumWalls(); j++) {
					if (CollisionHandler::lineRectCollision(t->x, t->y, testX, testY, WallManager::getWall(j))) {
						wallBlocksSight = true;
						break;
					}
				}
				if (!wallBlocksSight) {
					return false;
				}
				testX += testVelocity.getXComp();
				testY += testVelocity.getYComp();
				distLeftToTravel -= testVelocity.getMagnitude();
			} while (distLeftToTravel > 0);
		}

		//other hazards: just test the stopping location, don't bother with the full path
		for (int j = 0; j < HazardManager::getNumCircleHazards(); j++) {
			const CircleHazard* ch = HazardManager::getCircleHazard(j);
			if (ch->getGameID() != this->getGameID()) [[likely]] {
				if ((ch->getCollisionType() != CircleHazardCollisionType::under) && CollisionHandler::partiallyCollided(&testBody, ch)) {
					return false;
				}
			}
		}
		for (int j = 0; j < HazardManager::getNumRectHazards(); j++) {
			const RectHazard* rh = HazardManager::getRectHazard(j);
			if ((rh->getCollisionType() != RectHazardCollisionType::under) && CollisionHandler::partiallyCollided(&testBody, rh)) {
				return false;
			}
		}
	}

	return TargetingTurretHazard::reasonableLocation();
}

void PatrollingTurretHazard::draw() const {
	drawBody();
	drawOutline();
	drawBarrel();
	drawReticule();
}

void PatrollingTurretHazard::draw(DrawingLayers layer) const {
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
			if (Renderer::isDebugDrawingEnabled(this->getName())) {
				drawPath();
			}
			break;
	}
}

void PatrollingTurretHazard::poseDraw() const {
	drawBody();
	drawOutline();
	drawBarrel();
}

void PatrollingTurretHazard::poseDraw(DrawingLayers layer) const {
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
			if (Renderer::isDebugDrawingEnabled(this->getName())) {
				drawPath();
			}
			break;
	}
}

void PatrollingTurretHazard::ghostDraw(float alpha) const {
	drawBody(alpha);
	drawOutline(alpha);
	drawBarrel(alpha);
}

void PatrollingTurretHazard::ghostDraw(DrawingLayers layer, float alpha) const {
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

void PatrollingTurretHazard::drawPath(float alpha) const {
	alpha = alpha * alpha;
	ColorValueHolder color = ColorValueHolder(0.0f, 0.0f, 0.0f);
	color = ColorMixer::mix(BackgroundRect::getBackColor(), color, alpha);
	const float radius = this->r / 2;
	const float lineWidth = 1.0f;

	for (int i = 0; i < routePosPairNum; i++) {
		float coordsAndColor_circle[(Circle::NumOfSides+1)*(2+4)];
		float coordsAndColor_line[4*(2+4)];
		unsigned int indices_line[6];

		//circle
		coordsAndColor_circle[0] = getRoutePosX(i);
		coordsAndColor_circle[1] = getRoutePosY(i);
		coordsAndColor_circle[2] = color.getRf();
		coordsAndColor_circle[3] = color.getGf();
		coordsAndColor_circle[4] = color.getBf();
		coordsAndColor_circle[5] = color.getAf();
		for (int j = 1; j < Circle::NumOfSides+1; j++) {
			coordsAndColor_circle[j*6]   = static_cast<float>(getRoutePosX(i)) + radius * body_vertices[j].getXComp();
			coordsAndColor_circle[j*6+1] = static_cast<float>(getRoutePosY(i)) + radius * body_vertices[j].getYComp();
			coordsAndColor_circle[j*6+2] = color.getRf();
			coordsAndColor_circle[j*6+3] = color.getGf();
			coordsAndColor_circle[j*6+4] = color.getBf();
			coordsAndColor_circle[j*6+5] = color.getAf();
		}

		Renderer::SubmitBatchedDraw(coordsAndColor_circle, (Circle::NumOfSides+1)*(2+4), body_indices, Circle::NumOfSides*3);

		//line
		SimpleVector2D dist = SimpleVector2D(getRoutePosX((i+1) % routePosPairNum) - getRoutePosX(i), getRoutePosY((i+1) % routePosPairNum) - getRoutePosY(i));
		SimpleVector2D distCW  = SimpleVector2D(dist.getAngle() - float(PI/2), lineWidth, true);
		//SimpleVector2D distCCW = SimpleVector2D(dist.getAngle() + PI/2, lineWidth, true);

		coordsAndColor_line[0*6]   = static_cast<float>(getRoutePosX(i))                   + distCW.getXComp();
		coordsAndColor_line[0*6+1] = static_cast<float>(getRoutePosY(i))                   + distCW.getYComp();
		coordsAndColor_line[1*6]   = static_cast<float>(getRoutePosX(i)) + dist.getXComp() + distCW.getXComp();
		coordsAndColor_line[1*6+1] = static_cast<float>(getRoutePosY(i)) + dist.getYComp() + distCW.getYComp();
		coordsAndColor_line[2*6]   = static_cast<float>(getRoutePosX(i)) + dist.getXComp() - distCW.getXComp();
		coordsAndColor_line[2*6+1] = static_cast<float>(getRoutePosY(i)) + dist.getYComp() - distCW.getYComp();
		coordsAndColor_line[3*6]   = static_cast<float>(getRoutePosX(i))                   - distCW.getXComp();
		coordsAndColor_line[3*6+1] = static_cast<float>(getRoutePosY(i))                   - distCW.getYComp();

		for (int j = 0; j < 4; j++) {
			coordsAndColor_line[j*6+2] = color.getRf();
			coordsAndColor_line[j*6+3] = color.getGf();
			coordsAndColor_line[j*6+4] = color.getBf();
			coordsAndColor_line[j*6+5] = color.getAf();
		}

		indices_line[0] = 0;
		indices_line[1] = 1;
		indices_line[2] = 2;
		indices_line[3] = 2;
		indices_line[4] = 3;
		indices_line[5] = 0;

		Renderer::SubmitBatchedDraw(coordsAndColor_line, 4*(2+4), indices_line, 6);
	}
}

CircleHazard* PatrollingTurretHazard::randomizingFactory(double x_start, double y_start, double area_width, double area_height, const GenericFactoryConstructionData& args) {
	int attempts = 0;
	CircleHazard* randomized = nullptr;
	double angle;

	int count = 0;
	if (args.getDataCount() >= 1) {
		count = args.getDataPortionLength(0);
	}
	if (count >= 1) {
		const double* arr = static_cast<const double*>(args.getDataPortion(0).get());
		angle = arr[0];
		//this array is shared with optional starting position
	} else {
		angle = LevelRNG::randFunc() * (2*PI);
	}

	const double realRadius = (TANK_RADIUS/2);

	float stoppingLocationWeights[] = { 1.0f, 3.0f, 5.0f };
	do {
		int stoppingLocationCount = 3 + weightedSelect<float>(stoppingLocationWeights, 3); //{3, 4, 5}
		double* stoppingLocations = new double[stoppingLocationCount*2];
		double* waitTimes = new double[stoppingLocationCount];

		waitTimes[0] = std::floor(LevelRNG::randNumInRange(150, 200+1));
		if (count >= 3) {
			stoppingLocations[0] = static_cast<const double*>(args.getDataPortion(0).get())[1];
			stoppingLocations[1] = static_cast<const double*>(args.getDataPortion(0).get())[2];
			//assume the starting position is valid
		} else {
			int location_attempts = 0;
			bool blockedPosition;
			do {
				blockedPosition = false;
				stoppingLocations[0] = LevelRNG::randNumInRange(x_start + realRadius, x_start + area_width - realRadius);
				stoppingLocations[1] = LevelRNG::randNumInRange(y_start + realRadius, y_start + area_height - realRadius);

				Circle testStop = Circle(stoppingLocations[0], stoppingLocations[1], 0);
				for (int j = 0; j < WallManager::getNumWalls(); j++) {
					if (CollisionHandler::partiallyCollided(&testStop, WallManager::getWall(j))) {
						blockedPosition = true;
						break;
					}
				}

				location_attempts++;
			} while (blockedPosition && (location_attempts < 16));
			//note: no need to worry if the position is still blocked after all the attempts; reasonableLocation() will figure it out
		}

		bool outOfBounds;
		for (int i = 1; i < stoppingLocationCount; i++) {
			waitTimes[i] = std::floor(LevelRNG::randNumInRange(150, 200+1));
			int location_attempts = 0;
			bool blockedPosition;
			do {
				blockedPosition = false;
				outOfBounds = false;
				const double angleChange = LevelRNG::randFunc() * (2*PI);
				const double distanceChange = LevelRNG::randNumInRange(50, 150);
				const SimpleVector2D displacement = SimpleVector2D(angleChange, distanceChange, true);
				stoppingLocations[i*2]   = stoppingLocations[(i-1)*2]   + displacement.getXComp();
				stoppingLocations[i*2+1] = stoppingLocations[(i-1)*2+1] + displacement.getYComp();
				if ((stoppingLocations[i*2]   - (TANK_RADIUS/2) < x_start) || (stoppingLocations[i*2]   + (TANK_RADIUS/2) > x_start+area_width) ||
				    (stoppingLocations[i*2+1] - (TANK_RADIUS/2) < y_start) || (stoppingLocations[i*2+1] + (TANK_RADIUS/2) > y_start+area_height)) {
					//out of bounds, try again; done this way to avoid a bunch of paths clumping against the border
					outOfBounds = true;
					blockedPosition = true; //otherwise it will exit early
				} else {
					Circle testStop = Circle(stoppingLocations[i*2], stoppingLocations[i*2+1], 0);
					for (int j = 0; j < WallManager::getNumWalls(); j++) {
						if (CollisionHandler::partiallyCollided(&testStop, WallManager::getWall(j))) {
							blockedPosition = true;
							break;
						}
					}
				}

				location_attempts++;
			} while (blockedPosition && (location_attempts < 32));
			if (outOfBounds) {
				break;
			}
		}
		if (outOfBounds) {
			delete[] stoppingLocations;
			delete[] waitTimes;
			attempts++;
			continue;
		}

		CircleHazard* testPatrollingTurret = new PatrollingTurretHazard(stoppingLocations[0], stoppingLocations[1], angle, stoppingLocationCount, stoppingLocations, waitTimes);
		if (testPatrollingTurret->reasonableLocation()) {
			randomized = testPatrollingTurret;
			delete[] stoppingLocations;
			delete[] waitTimes;
			break;
		} else {
			delete testPatrollingTurret;
			delete[] stoppingLocations;
			delete[] waitTimes;
		}
		attempts++;
	} while (attempts < 128);

	return randomized;
}
