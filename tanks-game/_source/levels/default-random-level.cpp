#include "default-random-level.h"

#include "../constants.h"
#include <iostream>
#include "../rng.h"
#include "../mylib.h" //weightedSelect

#include "../reset-things.h"
#include "../level-helper.h"
#include "../powerup-manager.h"
#include "../wall-manager.h"
#include "../hazard-manager.h"
#include "../hazard-data-governor.h"

std::unordered_map<std::string, float> DefaultRandomLevel::getWeights() const {
	std::unordered_map<std::string, float> weights;
	weights.insert({ "vanilla", 4.0f });
	weights.insert({ "random-vanilla", 4.0f });
	weights.insert({ "random", 2.0f });
	return weights;
}

ColorValueHolder DefaultRandomLevel::getDefaultColor() const {
	//return ColorValueHolder(LevelRNG::randFunc(), LevelRNG::randFunc(), LevelRNG::randFunc()); //funky mode
	return currentColor;
}

void DefaultRandomLevel::initialize() {
	ResetThings::tankPositionReset();

	ColorValueHolder randColor = getDefaultColor();
	PositionHolder pos;
	//GenericFactoryConstructionData constructionData;
	//double* posArr;

	//some random walls
	LevelHelper::pushRandomWalls(LevelRNG::randIntInRange(12, 16+1), TANK_RADIUS*2.5*2, TANK_RADIUS*2.5, GAME_WIDTH - 2*(TANK_RADIUS*2.5*2), GAME_HEIGHT - 2*(TANK_RADIUS*2.5), randColor);

	//randomize hazards:
	//get the weight of each hazard
	float* circleHazardWeights = new float[HazardDataGovernor::getNumCircleHazardTypes("random-vanilla")];
	float* rectHazardWeights = new float[HazardDataGovernor::getNumRectHazardTypes("random-vanilla")];
	for (int i = 0; i < HazardDataGovernor::getNumCircleHazardTypes("random-vanilla"); i++) {
		std::string n = HazardDataGovernor::getCircleHazardName("random-vanilla", i);
		GenericFactoryConstructionData constructionData;
		CircleHazard* ch = HazardDataGovernor::getCircleHazardFactory("random-vanilla", n)(constructionData);
		circleHazardWeights[i] = ch->getWeights()["random-vanilla"];
		delete ch;
	}
	for (int i = 0; i < HazardDataGovernor::getNumRectHazardTypes("random-vanilla"); i++) {
		std::string n = HazardDataGovernor::getRectHazardName("random-vanilla", i);
		GenericFactoryConstructionData constructionData;
		RectHazard* rh = HazardDataGovernor::getRectHazardFactory("random-vanilla", n)(constructionData);
		rectHazardWeights[i] = rh->getWeights()["random-vanilla"];
		delete rh;
	}

	//add the hazards, but randomly based on their weight (if one has a high weight, it can get randomized multiple times)
	float choosingHazardWeights[] = { 0.5f, 1.5f, 2.0f, 1.0f };
	double circleVsRectHazardChoice = double(HazardDataGovernor::getNumCircleHazardTypes("random-vanilla"))/double(HazardDataGovernor::getNumCircleHazardTypes("random-vanilla") + HazardDataGovernor::getNumRectHazardTypes("random-vanilla"));
	int addHazardCount = LevelRNG::randIntInRange(8, 12+1); //v0.2.4 had 5 circle hazards and 5 rect hazards, meaning there used to be ~10 chances to insert hazards
	for (int i = 0; i < addHazardCount; i++) {
		int count = weightedSelect(choosingHazardWeights, 4); //{0, 1, 2, 3}
		if (LevelRNG::randFunc() < circleVsRectHazardChoice) {
			int index = weightedSelect(circleHazardWeights, HazardDataGovernor::getNumCircleHazardTypes("random-vanilla")); //randomize which hazard gets chosen (it could be one that was already chosen)
			CircleHazardRandomizationFunction f = HazardDataGovernor::getCircleHazardRandomizationFunction("random-vanilla", HazardDataGovernor::getCircleHazardName("random-vanilla", index));
			for (int i = 0; i < count; i++) {
				GenericFactoryConstructionData constructionData;
				CircleHazard* ch = f(TANK_RADIUS*2.5*2, TANK_RADIUS*2.5, GAME_WIDTH - 2*(TANK_RADIUS*2.5*2), GAME_HEIGHT - 2*(TANK_RADIUS*2.5), constructionData);
				if (ch != nullptr) {
					HazardManager::pushCircleHazard(ch);
				}
			}
		} else {
			int index = weightedSelect(rectHazardWeights, HazardDataGovernor::getNumRectHazardTypes("random-vanilla"));
			RectHazardRandomizationFunction f = HazardDataGovernor::getRectHazardRandomizationFunction("random-vanilla", HazardDataGovernor::getRectHazardName("random-vanilla", index));
			for (int i = 0; i < count; i++) {
				GenericFactoryConstructionData constructionData;
				RectHazard* rh = f(TANK_RADIUS*2.5*2, TANK_RADIUS*2.5, GAME_WIDTH - 2*(TANK_RADIUS*2.5*2), GAME_HEIGHT - 2*(TANK_RADIUS*2.5), constructionData);
				if (rh != nullptr) {
					HazardManager::pushRectHazard(rh);
				}
			}
		}
	}
	//std::cout << (HazardManager::getNumCircleHazards() + HazardManager::getNumRectHazards()) << std::endl;

	delete[] circleHazardWeights;
	delete[] rectHazardWeights;

	//randomize powers:
	float choosingPowerWeights[] = { 4.0f, 4.0f, 1.0f, .5f };
	for (int i = 0; i < 4; i++) {
		int count = 1 + weightedSelect(choosingPowerWeights, 4); //{1, 2, 3, 4}
		std::string* randPowers = LevelHelper::getRandomPowers(count, "random-vanilla");
		pos = LevelHelper::getSymmetricPowerupPositions_Corners(i, GAME_WIDTH/2, GAME_HEIGHT/2, GAME_WIDTH/2-60, GAME_HEIGHT/2-16);
		PowerupManager::pushPowerup(new PowerSquare(pos.first, pos.second, "random-vanilla", randPowers, count));
		delete[] randPowers;
	}

	//throw somethin' special in the center
	std::string* ultimatePowerName = LevelHelper::getRandomPowers(1, "ultimate-vanilla");
	PowerupManager::pushPowerup(new PowerSquare(GAME_WIDTH/2, GAME_HEIGHT/2, "ultimate-vanilla", ultimatePowerName[0]));
	delete[] ultimatePowerName;
	//semi-classic speed & invincible
	LevelHelper::pushSymmetricPowerups_UD_Alternate(GAME_WIDTH/2, GAME_HEIGHT/2, GAME_HEIGHT*(3.0/8),
		"vanilla", "speed", "vanilla", "invincible");

	//TODO: should have more random levels, but not sure how they should be different...
	//like less hazards? more open area? no idea
}

Level* DefaultRandomLevel::factory() {
	return new DefaultRandomLevel();
}

DefaultRandomLevel::DefaultRandomLevel() {
	currentColor = ColorValueHolder(LevelRNG::randFunc(), LevelRNG::randFunc(), LevelRNG::randFunc());
}
