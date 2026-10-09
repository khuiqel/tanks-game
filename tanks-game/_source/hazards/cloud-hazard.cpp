#include "cloud-hazard.h"

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
#include "../powerup-manager.h"

SimpleVector2D CloudHazard::body_vertices[Circle::NumOfSides+1];
unsigned int CloudHazard::body_indices[Circle::NumOfSides*3];
bool CloudHazard::initialized_vertices = false;

const ColorValueHolder CloudHazard::color = ColorValueHolder(1.0f, 1.0f, 1.0f);

std::unordered_map<std::string, float> CloudHazard::getWeights() const {
	std::unordered_map<std::string, float> weights;
	weights.insert({ "dev", 1.0f });
	weights.insert({ "random-dev", 1.0f });
	return weights;
}

//not the main one
//TODO: this can work (and put this hazard in vanilla!) if hazards could check the type of the level, then pull powers from that type
CloudHazard::CloudHazard(double xpos, double ypos, double radius, double speed, double acc) : CircleHazard(HAZARD_TEAM) {
	x = xpos;
	y = ypos;
	r = radius;

	powerChoices = { {"vanilla", "speed"} }; //TODO: pull from the level?
	initialSpeed = speed;
	acceleration = acc;
	tickCount = 0;
	tickCycle = 400;

	powerupsToMove = std::vector<Game_ID>();
	powerupsVelocity = std::vector<SimpleVector2D>();

	//canAcceptPowers = false;

	initializeVertices();
}

CloudHazard::CloudHazard(double xpos, double ypos, double radius) : CloudHazard(xpos, ypos, radius, 1, -1.0/100) {}

//the main one
CloudHazard::CloudHazard(double xpos, double ypos, double radius, double speed, double acc, const std::vector<std::vector<std::string>>& powerList) : CircleHazard(HAZARD_TEAM) {
	x = xpos;
	y = ypos;
	r = radius;

	powerChoices = std::vector<std::vector<std::string>>(powerList);
	initialSpeed = speed;
	acceleration = acc;
	tickCount = 0;
	tickCycle = 400;

	powerupsToMove = std::vector<Game_ID>();
	powerupsVelocity = std::vector<SimpleVector2D>();

	//canAcceptPowers = false;

	initializeVertices();
}

CloudHazard::~CloudHazard() {
	//nothing
}

bool CloudHazard::initializeVertices() {
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

	initialized_vertices = true;
	return true;
}

CircleHazard* CloudHazard::factory(const GenericFactoryConstructionData& args) {
	if (args.getDataCount() >= 1) [[likely]] {
		int count = args.getDataPortionLength(0);

		if (count >= 3) [[likely]] {
			const double* arr = static_cast<const double*>(args.getDataPortion(0).get());
			double x = arr[0];
			double y = arr[1];
			double r = arr[2];

			if ((args.getDataCount() >= 2) && (args.getDataPortionLength(1) >= 2)) {
				const double* arr_extra = static_cast<const double*>(args.getDataPortion(1).get());
				double speed = arr_extra[0];
				double acc = arr_extra[1];

				if ((args.getDataCount() >= 3) && (args.getDataPortionLength(2) >= 1)) {
					const auto* list = static_cast<const std::vector<std::vector<std::string>>*>(args.getDataPortion(2).get());
					return new CloudHazard(x, y, r, speed, acc, *list);
				}

				return new CloudHazard(x, y, r, speed, acc);
			}

			return new CloudHazard(x, y, r);
		}
	}

	return new CloudHazard(0, 0, 0);
}

PowerSquare* CloudHazard::makePowerup(int powerupIndex) const {
	const int typeCount = powerChoices[powerupIndex].size() / 2;
	std::string* types = new std::string[typeCount];
	std::string* names = new std::string[typeCount];

	for (int i = 0; i < typeCount; i++) {
		types[i] = powerChoices[powerupIndex][i*2];
		names[i] = powerChoices[powerupIndex][i*2+1];
	}

	PowerSquare* powerup = new PowerSquare(this->x, this->y, types, names, typeCount);
	delete[] types; delete[] names;
	return powerup;
}

void CloudHazard::pushPowerup() {
	int powerupIndex = GameRNG::randFunc() * powerChoices.size();
	PowerSquare* powerup = makePowerup(powerupIndex);
	powerupsToMove.push_back(powerup->getGameID());
	powerupsVelocity.push_back(SimpleVector2D(float(2*PI) * GameRNG::randFuncf(), this->initialSpeed, true));
	PowerupManager::pushPowerup(powerup);
}

void CloudHazard::tick() {
	for (int i = 0; i < powerupsToMove.size(); i++) {
		PowerSquare* p = PowerupManager::getPowerupByID(powerupsToMove[i]);
		if (p == nullptr) [[unlikely]] {
			powerupsToMove.erase(powerupsToMove.begin() + i);
			powerupsVelocity.erase(powerupsVelocity.begin() + i);
			i--;
			continue;
		}

		SimpleVector2D& vel = powerupsVelocity[i];
		vel.changeMagnitude(this->acceleration);

		if (vel.getMagnitude() > 0) {
			p->x += vel.getXComp();
			p->y += vel.getYComp();
		} else [[unlikely]] {
			powerupsToMove.erase(powerupsToMove.begin() + i);
			powerupsVelocity.erase(powerupsVelocity.begin() + i);
			i--;
			continue;
		}
	}

	tickCount++;
	if (tickCount >= tickCycle) [[unlikely]] {
		//don't bother checking tickCycle <= 0
		tickCount -= tickCycle;
		pushPowerup();
	}
}

bool CloudHazard::reasonableLocation() const {
	for (int i = 0; i < WallManager::getNumWalls(); i++) {
		if (CollisionHandler::partiallyCollided(this, WallManager::getWall(i))) {
			return false;
		}
	}

	for (int i = 0; i < HazardManager::getNumCircleHazards(); i++) {
		const CircleHazard* ch = HazardManager::getCircleHazard(i);
		if (ch->getGameID() != this->getGameID()) [[unlikely]] {
			if ((ch->getCollisionType() == CircleHazardCollisionType::solid) && (CollisionHandler::partiallyCollided(this, ch))) {
				return false;
			}
		}
	}
	for (int i = 0; i < HazardManager::getNumRectHazards(); i++) {
		const RectHazard* rh = HazardManager::getRectHazard(i);
		if ((rh->getCollisionType() == RectHazardCollisionType::solid) && CollisionHandler::partiallyCollided(this, rh)) {
			return false;
		}
	}

	return validLocation();
}

void CloudHazard::draw() const {
	ghostDraw(1.0f);
}

void CloudHazard::draw(DrawingLayers layer) const {
	switch (layer) {
		case DrawingLayers::under:
			//nothing
			break;

		default:
			std::cerr << "WARNING: unknown DrawingLayer for " + getName() + " draw!\n";
			[[fallthrough]];
		case DrawingLayers::normal:
			//nothing
			break;

		case DrawingLayers::effects:
			draw();
			break;

		case DrawingLayers::top:
			//nothing
			break;

		case DrawingLayers::debug:
			//later
			break;
	}
}

void CloudHazard::poseDraw() const {
	draw();
}

void CloudHazard::poseDraw(DrawingLayers layer) const {
	switch (layer) {
		case DrawingLayers::under:
			//nothing
			break;

		default:
			std::cerr << "WARNING: unknown DrawingLayer for " + getName() + " poseDraw!\n";
			[[fallthrough]];
		case DrawingLayers::normal:
			//nothing
			break;

		case DrawingLayers::effects:
			poseDraw();
			break;

		case DrawingLayers::top:
			//nothing
			break;

		case DrawingLayers::debug:
			//later
			break;
	}
}

void CloudHazard::ghostDraw(float alpha) const {
	//TODO: multiple circles

	alpha = alpha * alpha;
	ColorValueHolder cloudColor = color;
	cloudColor = ColorMixer::mix(BackgroundRect::getBackColor(), cloudColor, alpha);

	float coordsAndColor[(Circle::NumOfSides+1)*(2+4)];
	coordsAndColor[0] = x;
	coordsAndColor[1] = y;
	coordsAndColor[2] = cloudColor.getRf();
	coordsAndColor[3] = cloudColor.getGf();
	coordsAndColor[4] = cloudColor.getBf();
	coordsAndColor[5] = cloudColor.getAf();
	for (int i = 1; i < Circle::NumOfSides+1; i++) {
		coordsAndColor[i*6]   = static_cast<float>(x) + static_cast<float>(r) * body_vertices[i].getXComp();
		coordsAndColor[i*6+1] = static_cast<float>(y) + static_cast<float>(r) * body_vertices[i].getYComp();
		coordsAndColor[i*6+2] = cloudColor.getRf();
		coordsAndColor[i*6+3] = cloudColor.getGf();
		coordsAndColor[i*6+4] = cloudColor.getBf();
		coordsAndColor[i*6+5] = cloudColor.getAf();
	}

	Renderer::SubmitBatchedDraw(coordsAndColor, (Circle::NumOfSides+1)*(2+4), body_indices, Circle::NumOfSides*3);
}

void CloudHazard::ghostDraw(DrawingLayers layer, float alpha) const {
	switch (layer) {
		case DrawingLayers::under:
			//nothing
			break;

		default:
			std::cerr << "WARNING: unknown DrawingLayer for " + getName() + " ghostDraw!\n";
			[[fallthrough]];
		case DrawingLayers::normal:
			//nothing
			break;

		case DrawingLayers::effects:
			ghostDraw(alpha);
			break;

		case DrawingLayers::top:
			//nothing
			break;

		case DrawingLayers::debug:
			//later
			break;
	}
}

CircleHazard* CloudHazard::randomizingFactory(double x_start, double y_start, double area_width, double area_height, const GenericFactoryConstructionData& args) {
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
			radius = LevelRNG::randNumInRange(16, 24); //TODO: where should these constants be?
		}
		xpos = LevelRNG::randNumInRange(x_start + radius, x_start + area_width - radius);
		ypos = LevelRNG::randNumInRange(y_start + radius, y_start + area_height - radius);
		CircleHazard* testCloud = new CloudHazard(xpos, ypos, radius);
		if (testCloud->reasonableLocation()) {
			randomized = testCloud;
			break;
		} else {
			delete testCloud;
		}
		attempts++;
	} while (attempts < 64);

	return randomized;
}
