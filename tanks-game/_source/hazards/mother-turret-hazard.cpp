#include "mother-turret-hazard.h"

#include "../constants.h"
#include <cmath>
#include <algorithm> //std::copy, std::count, std::clamp
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
#include "../hazard-data-governor.h"
#include "targeting-turret-hazard.h" //in case mother turrets no longer extend targeting turrets

std::unordered_map<std::string, float> MotherTurretHazard::getWeights() const {
	std::unordered_map<std::string, float> weights;
	weights.insert({ "vanilla", 0.5f });
	weights.insert({ "random-vanilla", 0.25f });
	weights.insert({ "random", 0.125f });
	return weights;
}

MotherTurretHazard::MotherTurretHazard(double xpos, double ypos, double angle, int maxChildren, int startChildren, double childDistMultiplier) : TargetingTurretHazard(xpos, ypos, angle) {
	r = TANK_RADIUS * 2; // that's a little big

	stateMultiplier[1] *= 4;
	turningIncrement *= 2;

	maxChildTurrets = (maxChildren/2)*2; //even numbers only
	childDistFromMother = r * childDistMultiplier;
	initialAngle = angle;
	targetingNum = -1;
	targetingChild = false;

	childTurretIDs = std::vector<Game_ID>(maxChildTurrets, -1);
	childTurretAlive = std::vector<char>(maxChildTurrets, (char)false);
	initialChildren = startChildren;

	//canAcceptPowers = false; //not sure

	//idea: mitosis turrets; grows for some time, then splits (that'd be hard to program...)
}

MotherTurretHazard::MotherTurretHazard(double xpos, double ypos, double angle, int maxChildren, int startChildren) : MotherTurretHazard(xpos, ypos, angle, maxChildren, startChildren, .75) {
	//child distance options: r, r/2, r*.75
}

MotherTurretHazard::MotherTurretHazard(double xpos, double ypos, double angle) : MotherTurretHazard(xpos, ypos, angle, 8, 4) {}

MotherTurretHazard::~MotherTurretHazard() {
	//nothing
}

CircleHazard* MotherTurretHazard::factory(const GenericFactoryConstructionData& args) {
	if (args.getDataCount() >= 1) [[likely]] {
		int count_basic = args.getDataPortionLength(0);

		if (count_basic >= 3) [[likely]] {
			const double* arr_basic = static_cast<const double*>(args.getDataPortion(0).get());
			double x = arr_basic[0];
			double y = arr_basic[1];
			double a = arr_basic[2];

			if (args.getDataCount() >= 2) {
				int count_num = args.getDataPortionLength(1);

				if (count_num >= 1) {
					const int* arr_num = static_cast<const int*>(args.getDataPortion(1).get());

					int child_max = arr_num[0];
					int child_start = ((count_num >= 2) ? arr_num[1] : 0);

					if ((args.getDataCount() >= 3) && (args.getDataPortionLength(2) >= 1)) {
						const double* arr_dist = static_cast<const double*>(args.getDataPortion(2).get());
						return new MotherTurretHazard(x, y, a, child_max, child_start, arr_dist[0]);
					}

					return new MotherTurretHazard(x, y, a, child_max, child_start);
				}
			}

			return new MotherTurretHazard(x, y, a);
		}
	}

	return new MotherTurretHazard(0, 0, 0, 0, 0);
}

void MotherTurretHazard::pushInitialChildren(int childCount) {
	//childCount = (childCount/2)*2; //guaranteed to be even already

	if ((maxChildTurrets == 0) || (childCount == 0)) [[unlikely]] {
		return;
	}

	//for every power of 2 of maxChildren, split working area in half
	/* pretend this is a circle:
	 * 
	 *         @
	 *       @   @
	 *     @       @
	 *   @           @
	 * @       X       @
	 *   @           @
	 *     @       @
	 *       @   @
	 *         @
	 * 
	 *         @
	 *       @   @
	 *     @       @
	 *   @           @
	 * @-------X-------@
	 *   @           @
	 *     @       @
	 *       @   @
	 *         @
	 * 
	 *         @
	 *       @ | @
	 *     @   |   @
	 *   @     |     @
	 * @-------X-------@
	 *   @     |     @
	 *     @   |   @
	 *       @ | @
	 *         @
	 * 
	 *         @
	 *       @ | @
	 *     @ \ | / @
	 *   @    \|/    @
	 * @-------X-------@
	 *   @    /|\    @
	 *     @ / | \ @
	 *       @ | @
	 *         @
	 * 
	 */
	//after splitting the working area, place the initial children in there, with the same pattern for each part

	if (childCount >= maxChildTurrets) [[unlikely]] {
		for (int i = 0; i < maxChildTurrets; i++) {
			pushChild(i);
		}
		return;
	}

	int currentMaxChildren = maxChildTurrets;
	int currentInitialChildren = childCount;
	int angleSplit = 1;
	while ((currentMaxChildren%2 == 0) && (currentInitialChildren%2 == 0)) {
		angleSplit *= 2;
		currentMaxChildren /= 2;
		currentInitialChildren /= 2;
	}

	const float turretAngleDiff = float(2*PI) / maxChildTurrets;
	const float workingAngle = float(2*PI) / angleSplit;
	for (int i = 0; i < angleSplit; i++) {
		for (int j = 1; j <= currentInitialChildren; j++) {
			pushChild(i*currentMaxChildren + static_cast<int>(std::round((j * workingAngle/float(currentInitialChildren+1)) / turretAngleDiff)));
		}
	}
}

CircleHazard* MotherTurretHazard::makeTurret(int turretNum) const {
	GenericFactoryConstructionData constructionData;
	const float angle = getChildTurretAngle(turretNum);
	double* posArr = new double[3]{ this->x + (this->r+childDistFromMother) * std::cos(angle), this->y + (this->r+childDistFromMother) * std::sin(angle), double(angle) };
	constructionData = GenericFactoryConstructionData(3, posArr);
	CircleHazard* childTurret = HazardDataGovernor::getCircleHazardFactory("vanilla", "targeting_turret")(constructionData);
	return childTurret;
}

void MotherTurretHazard::pushChild(int turretNum) {
	//make turret
	CircleHazard* childTurret = makeTurret(turretNum);

	//add turret to child list
	this->childTurretIDs[turretNum] = childTurret->getGameID();
	this->childTurretAlive[turretNum] = true;

	//push turret
	HazardManager::pushCircleHazard(childTurret);
}

float MotherTurretHazard::getChildTurretAngle(int turretNum) const {
	const float turretAngleDiff = float(2*PI) / maxChildTurrets;
	return initialAngle + turretNum * turretAngleDiff;
}

void MotherTurretHazard::initialize() {
	pushInitialChildren(initialChildren);
}

void MotherTurretHazard::tick() {
	//tickCount is unused... maybe targetingCount should replace it...

	updateChildCount();

	if (currentState == 0) { //either turning to next turret spot or doing nothing
		if (targetingChild) { //tracking spot
			tick_trackSpot();
		}
		if (!targetingChild) { //looking for an open spot
			tick_chooseSpot();
		}
	}

	if (currentState == 1) { //charging up
		tick_chargeUp();
	}

	if (currentState == 2) { //cooling down
		tick_cooldown();
	}

	//maxState is 3; not using else in case tickCycle is zero
}

void MotherTurretHazard::tick_chooseSpot() {
	//choose the spot that will result in the most time spent turning
	if (getChildCount() < maxChildTurrets) {
		const float childAngleDiff = float(2*PI) / maxChildTurrets;
		float maxAngleDiff = -2*PI;
		for (int i = 0; i < maxChildTurrets; i++) {
			if (!childTurretAlive[i]) {
				float a = std::abs(SimpleVector2D::angleBetween(this->velocity, SimpleVector2D(getChildTurretAngle(i), 0, true)));
				if (a > maxAngleDiff) {
					maxAngleDiff = a;
					targetingNum = i;
				}
			}
		}
		targetingChild = true;
	} else {
		targetingChild = false;
		targetingCount = 0;
	}
}

void MotherTurretHazard::tick_trackSpot() {
	turnTowardsPoint(targetingNum);
	if (isPointedAt(targetingNum)) {
		targetingCount++;
		if (targetingCount >= stateMultiplier[0] * tickCycle) {
			currentState = 1;
			targetingCount = 0;
		}
	} else {
		targetingCount--;
		if (targetingCount < 0) {
			targetingCount = 0;
		}
	}
}

void MotherTurretHazard::tick_chargeUp() {
	targetingCount++;
	if (targetingCount >= stateMultiplier[1] * tickCycle) {
		pushChild(targetingNum);
		currentState = 2;
		targetingCount = 0;
		targetingChild = false; //allows target to change
		//targetingNum = -1; //needed?
	}
}

int MotherTurretHazard::getChildCount() const {
	return std::count(childTurretAlive.begin(), childTurretAlive.end(), (char)true);
}

int MotherTurretHazard::updateChildCount() {
	//loop through current children; it's alive if there's an ID match
	int count = 0;
	for (int i = 0; i < maxChildTurrets; i++) {
		if (!childTurretAlive[i]) {
			continue;
		}

		if (HazardManager::getCircleHazardByID(childTurretIDs[i]) == nullptr) {
			//not found, dead
			childTurretIDs[i] = -1;
			childTurretAlive[i] = false;
		} else {
			//found, alive
			count++;
		}
	}

	return count;
}

void MotherTurretHazard::turnTowardsPoint(int turretNum) {
	//see TargetingTurretHazard::turnTowardsTank
	SimpleVector2D distToChild = SimpleVector2D(getChildTurretAngle(turretNum), this->r+childDistFromMother, true);
	float theta = SimpleVector2D::angleBetween(distToChild, velocity);
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

bool MotherTurretHazard::isPointedAt(int turretNum) const {
	float angle = getChildTurretAngle(turretNum);
	return (std::abs(SimpleVector2D::angleBetween(velocity, SimpleVector2D(angle, 0, true))) < float(PI)/turningIncrement);
}

bool MotherTurretHazard::reasonableLocation() const {
	for (int i = 0; i < maxChildTurrets; i++) {
		CircleHazard* testChild = makeTurret(i);
		if (!testChild->reasonableLocation()) {
			//note: hazards made after this mother turret don't check for these future child turrets
			delete testChild;
			return false;
		}
		for (int j = 0; j < WallManager::getNumWalls(); j++) {
			if (CollisionHandler::lineRectCollision(this->x, this->y, testChild->getX(), testChild->getY(), WallManager::getWall(j))) {
				delete testChild;
				return false;
			}
		}
		delete testChild;
	}
	//probably enough

	return TargetingTurretHazard::reasonableLocation(); //this tests whether it can see the tank, which is unnecessary, but oh well
}

void MotherTurretHazard::draw() const {
	drawShootingTimer();
	drawBody();
	drawOutline();
	drawBarrel();
}

void MotherTurretHazard::draw(DrawingLayers layer) const {
	switch (layer) {
		case DrawingLayers::under:
			//nothing
			break;

		default:
			std::cerr << "WARNING: unknown DrawingLayer for " + getName() + " draw!\n";
			[[fallthrough]];
		case DrawingLayers::normal:
			drawShootingTimer();
			drawBody();
			drawOutline();
			drawBarrel();
			break;

		case DrawingLayers::effects:
			//nothing
			break;

		case DrawingLayers::top:
			//nothing
			break;

		case DrawingLayers::debug:
			if (Renderer::isDebugDrawingEnabled(this->getName())) {
				drawChildTurretLocations();
			}
			break;
	}
}

void MotherTurretHazard::poseDraw() const {
	drawBody();
	drawOutline();
	drawBarrel();
}

void MotherTurretHazard::poseDraw(DrawingLayers layer) const {
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
			//nothing
			break;

		case DrawingLayers::top:
			//nothing
			break;

		case DrawingLayers::debug:
			if (Renderer::isDebugDrawingEnabled(this->getName())) {
				drawChildTurretLocations();
			}
			break;
	}
}

void MotherTurretHazard::ghostDraw(float alpha) const {
	drawBody(alpha);
	drawOutline(alpha);
	drawBarrel(alpha);
}

void MotherTurretHazard::ghostDraw(DrawingLayers layer, float alpha) const {
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

void MotherTurretHazard::drawShootingTimer(float alpha) const {
	if (currentState != 1) {
		return;
	}

	float shootingOutlinePercent;
	if (tickCycle <= 0) {
		shootingOutlinePercent = 0;
	} else {
		shootingOutlinePercent = std::clamp<float>(targetingCount / (stateMultiplier[1] * tickCycle), 0, 1);
	}
	unsigned int shootingOutlineTriangles = Circle::NumOfSides * shootingOutlinePercent;

	if (shootingOutlineTriangles > 0) {
		ColorValueHolder color = ColorValueHolder(1.0f, 1.0f, 1.0f);
		color = ColorMixer::mix(BackgroundRect::getBackColor(), color, alpha);
		const float rotateAngle = velocity.getAngle() + float(2*PI)*(1 - float(shootingOutlineTriangles)/Circle::NumOfSides)/2;

		float coordsAndColor[(Circle::NumOfSides+1)*(2+4)];
		coordsAndColor[0] = x;
		coordsAndColor[1] = y;
		coordsAndColor[2] = color.getRf();
		coordsAndColor[3] = color.getGf();
		coordsAndColor[4] = color.getBf();
		coordsAndColor[5] = color.getAf();
		for (unsigned int i = 0; i <= shootingOutlineTriangles && i < Circle::NumOfSides; i++) {
			SimpleVector2D vertex = SimpleVector2D(body_vertices[i % Circle::NumOfSides + 1]);
			vertex.scaleAndRotate(static_cast<float>(r)*(5.0f/4.0f), rotateAngle);

			coordsAndColor[(i+1)*6]   = static_cast<float>(x) + vertex.getXComp();
			coordsAndColor[(i+1)*6+1] = static_cast<float>(y) + vertex.getYComp();
			coordsAndColor[(i+1)*6+2] = color.getRf();
			coordsAndColor[(i+1)*6+3] = color.getGf();
			coordsAndColor[(i+1)*6+4] = color.getBf();
			coordsAndColor[(i+1)*6+5] = color.getAf();
		}

		Renderer::SubmitBatchedDraw(coordsAndColor, (shootingOutlineTriangles < Circle::NumOfSides ? (shootingOutlineTriangles+2)*(2+4) : (shootingOutlineTriangles+1)*(2+4)), body_indices, shootingOutlineTriangles*3);
	}
}

void MotherTurretHazard::drawChildTurretLocations(float alpha) const {
	ColorValueHolder color = ColorValueHolder(0.0f, 0.0f, 0.0f);
	color = ColorMixer::mix(BackgroundRect::getBackColor(), color, alpha);

	int chosenChild;
	if (targetingChild) {
		chosenChild = targetingNum;
	} else {
		chosenChild = -1;
	}

	float coordsAndColor[(Circle::NumOfSides+1)*(2+4)];

	for (int i = 0; i < maxChildTurrets; i++) {
		const CircleHazard* testChild = makeTurret(i); //yeah it's super improper but whatever
		const float radius = (i==chosenChild ? (1.5)*((TANK_RADIUS/2) / 2) : (TANK_RADIUS/2) / 2);

		coordsAndColor[0] = testChild->x;
		coordsAndColor[1] = testChild->y;
		coordsAndColor[2] = color.getRf();
		coordsAndColor[3] = color.getGf();
		coordsAndColor[4] = color.getBf();
		coordsAndColor[5] = color.getAf();
		for (int j = 1; j < Circle::NumOfSides+1; j++) {
			coordsAndColor[j*6]   = static_cast<float>(testChild->x) + radius * body_vertices[j].getXComp();
			coordsAndColor[j*6+1] = static_cast<float>(testChild->y) + radius * body_vertices[j].getYComp();
			coordsAndColor[j*6+2] = color.getRf();
			coordsAndColor[j*6+3] = color.getGf();
			coordsAndColor[j*6+4] = color.getBf();
			coordsAndColor[j*6+5] = color.getAf();
		}
		delete testChild;

		Renderer::SubmitBatchedDraw(coordsAndColor, (Circle::NumOfSides+1)*(2+4), body_indices, Circle::NumOfSides*3);
	}
}

CircleHazard* MotherTurretHazard::randomizingFactory(double x_start, double y_start, double area_width, double area_height, const GenericFactoryConstructionData& args) {
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

	const double realRadius = (TANK_RADIUS*2) * 1.75 + (TANK_RADIUS/2);

	do {
		xpos = LevelRNG::randNumInRange(x_start + realRadius, x_start + area_width - realRadius);
		ypos = LevelRNG::randNumInRange(y_start + realRadius, y_start + area_height - realRadius);
		CircleHazard* testMotherTurret = new MotherTurretHazard(xpos, ypos, angle);
		if (testMotherTurret->reasonableLocation()) {
			randomized = testMotherTurret;
			break;
		} else {
			delete testMotherTurret;
		}
		attempts++;
	} while (attempts < 128);

	return randomized;
}
