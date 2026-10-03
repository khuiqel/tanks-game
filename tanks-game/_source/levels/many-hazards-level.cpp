#include "many-hazards-level.h"

#include "../constants.h"
#include <iostream>

#include "../reset-things.h"
#include "../level-helper.h"
#include "../powerup-manager.h"
#include "../wall-manager.h"
#include "../hazard-manager.h"

std::unordered_map<std::string, float> ManyHazardsLevel::getWeights() const {
	std::unordered_map<std::string, float> weights;
	weights.insert({ "vanilla", 0.5f });
	weights.insert({ "random-vanilla", 0.5f });
	weights.insert({ "old", 0.5f });
	weights.insert({ "random-old", 0.5f });
	weights.insert({ "random", 0.5f });
	return weights;
}

void ManyHazardsLevel::initialize() {
	ResetThings::tankPositionReset(40);

	//in JS, power mixing was turned off
	ColorValueHolder color = getDefaultColor();
	PositionHolder pos;
	GenericFactoryConstructionData constructionData;
	double* posArr;

	LevelHelper::pushClassicWalls(color);
	const PositionHolder* wallArray = LevelHelper::getClassicWallPositions();

	//TODO: this level could use a homing powerup, possibly where the "useless traps" are

	posArr = new double[4]{ GAME_WIDTH/2 - 10, 128, 10*2, GAME_HEIGHT - 128*2 };
	constructionData = GenericFactoryConstructionData(4, posArr);
	HazardManager::pushRectHazard("vanilla", "no_bullet_zone", constructionData);

	const int TURRET_COUNT = 16; //stationary turret count, not regular turret count
	//const double turretDistance = (wallArray[3].first - (wallArray[0].first+32)) / (TURRET_COUNT+1);
	const double turretDistance = (wallArray[3].first - (wallArray[0].first+32) - 8) / (TURRET_COUNT+1);
	for (int i = 0; i < TURRET_COUNT; i++) {
		//bottom half
		//posArr = new double[3]{ wallArray[0].first+32 + turretDistance * (i+1), 16, PI/2 };
		posArr = new double[3]{ wallArray[0].first+32+8/2 + turretDistance * (i+1), 16, PI/2 };
		constructionData = GenericFactoryConstructionData(3, posArr);
		HazardManager::pushCircleHazard("vanilla", "stationary_turret", constructionData);
	}
	for (int i = 0; i < TURRET_COUNT; i++) {
		//top half
		//posArr = new double[3]{ wallArray[0].first+32 + turretDistance * (i+1), GAME_HEIGHT-16, -PI/2 };
		posArr = new double[3]{ wallArray[0].first+32+8/2 + turretDistance * (i+1), GAME_HEIGHT-16, -PI/2 };
		constructionData = GenericFactoryConstructionData(3, posArr);
		HazardManager::pushCircleHazard("vanilla", "stationary_turret", constructionData);
	}
	//JS x positions: [140, 164, ..., 476, 500]; C++ x positions: [136.471, 160.941, ..., 479.059, 503.529]
	//positions have been updated to JS version; old version left commented out

	//this is too much:
	//for (int i = 0; i < TURRET_COUNT-1; i++) {
	//	posArr = new double[3]{ wallArray[0].first+32+8/2 + turretDistance * (i+1) + 4+8, 16, PI/2 };
	//	constructionData = GenericFactoryConstructionData(3, posArr);
	//	HazardManager::pushCircleHazard("vanilla", "targeting_turret", constructionData);
	//}
	//for (int i = 0; i < TURRET_COUNT-1; i++) {
	//	posArr = new double[3]{ wallArray[0].first+32+8/2 + turretDistance * (i+1) + 4+8, GAME_HEIGHT-16, -PI/2 };
	//	constructionData = GenericFactoryConstructionData(3, posArr);
	//	HazardManager::pushCircleHazard("vanilla", "targeting_turret", constructionData);
	//}

	for (int i = 0; i < 4; i++) {
		pos = LevelHelper::getSymmetricPowerupPositions_Corners(i, GAME_WIDTH/2, GAME_HEIGHT/2, GAME_WIDTH/2 - ((wallArray[0].first+32) + (TANK_RADIUS/2 * 1.5)), 32 + 20 + (TANK_RADIUS/2 * 2.5));
		//pos = LevelHelper::getSymmetricPowerupPositions_Corners(i, GAME_WIDTH/2, GAME_HEIGHT/2, GAME_WIDTH/2 - ((wallArray[0].first+32) + (TANK_RADIUS/2 * 1.5) + 4), 32 + 20 + (TANK_RADIUS/2 * 2.5));
		posArr = new double[3]{ pos.first, pos.second, PI * (i%2) };
		constructionData = GenericFactoryConstructionData(3, posArr);
		HazardManager::pushCircleHazard("vanilla", "targeting_turret", constructionData);
	}

	//useless traps:
	//LevelHelper::pushSymmetricPowerups_Corners(GAME_WIDTH/2, GAME_HEIGHT/2, GAME_WIDTH/2 - (wallArray[0].first+32 + 20), GAME_HEIGHT/2 - 48, "vanilla", "speed");

	PositionHolder wallPos1 = LevelHelper::getSymmetricWallPositions_UD(0, GAME_WIDTH/2, GAME_HEIGHT/2, (GAME_HEIGHT - 128*2)/2, 160, 20);
	WallManager::pushWall(new Wall(wallPos1.first, wallPos1.second, 160, 20, color));
	PositionHolder wallPos2 = LevelHelper::getSymmetricWallPositions_UD(1, GAME_WIDTH/2, GAME_HEIGHT/2, (GAME_HEIGHT - 128*2)/2, 160, 20);
	WallManager::pushWall(new Wall(wallPos2.first, wallPos2.second, 160, 20, color));

	for (int i = 0; i < 4; i++) {
		pos = LevelHelper::getSymmetricWallPositions_Corners(i, GAME_WIDTH/2, GAME_HEIGHT/2, 160/2, (GAME_HEIGHT - 128*2)/2, (GAME_WIDTH/2 - 160/2 - (wallArray[0].first+32)), 20);
		posArr = new double[4]{ pos.first, pos.second, GAME_WIDTH/2 - 160/2 - (wallArray[0].first+32), 20 };
		constructionData = GenericFactoryConstructionData(4, posArr);
		HazardManager::pushRectHazard("vanilla", "horizontal_lightning", constructionData);
	}

	pos = LevelHelper::getSymmetricPowerupPositions_LR(0, GAME_WIDTH/2, GAME_HEIGHT/2, 160/4 * 1.5);
	PowerupManager::pushPowerup(new PowerSquare(pos.first, pos.second, "vanilla-extra", "barrier")); //replace barrier with speed?
	pos = LevelHelper::getSymmetricPowerupPositions_LR(1, GAME_WIDTH/2, GAME_HEIGHT/2, 160/4 * 1.5);
	PowerupManager::pushPowerup(new PowerSquare(pos.first, pos.second, "vanilla-extra", "barrier"));

	pos = LevelHelper::getSymmetricWallPositions_UD(0, GAME_WIDTH/2, GAME_HEIGHT/2, GAME_HEIGHT/2 - wallPos1.second, 160/2, 50);
	posArr = new double[4]{ pos.first, pos.second, 160/2, 50 };
	constructionData = GenericFactoryConstructionData(4, posArr);
	HazardManager::pushRectHazard("vanilla", "lava", constructionData);
	pos = LevelHelper::getSymmetricWallPositions_UD(1, GAME_WIDTH/2, GAME_HEIGHT/2, GAME_HEIGHT/2 - wallPos1.second, 160/2, 50);
	posArr = new double[4]{ pos.first, pos.second, 160/2, 50 };
	constructionData = GenericFactoryConstructionData(4, posArr);
	HazardManager::pushRectHazard("vanilla", "lava", constructionData);

	for (int i = 0; i < 4; i++) {
		pos = LevelHelper::getSymmetricWallPositions_Corners(i, GAME_WIDTH/2, GAME_HEIGHT/2, 160/4, GAME_HEIGHT/2 - wallPos1.second, 160/4, 50);
		posArr = new double[4]{ pos.first, pos.second, 160/4, 50 };
		constructionData = GenericFactoryConstructionData(4, posArr);
		HazardManager::pushRectHazard("vanilla", "lightning", constructionData);
	}

	LevelHelper::pushSymmetricPowerups_Corners(GAME_WIDTH/2, GAME_HEIGHT/2, 160/4 * 1.5, (GAME_HEIGHT - 128*2)/2 + 20 + 50 + 21, "vanilla", "invincible");
	//(160 - 32 - 20 - 50 - 16)/2 = 21
	//(GAME_HEIGHT/2 - (GAME_HEIGHT - 128*2)/2 - [wall height] - [lava height] - [stationary turret distance to edge])/2

	//traps:
	//LevelHelper::pushSymmetricPowerups_UD(GAME_WIDTH/2, GAME_HEIGHT/2, (GAME_HEIGHT - 128*2)/2 + 20 + 50 + 21, "vanilla", "speed");
	//yes, they were evil

	delete[] wallArray;
}

Level* ManyHazardsLevel::factory() {
	return new ManyHazardsLevel();
}

ManyHazardsLevel::ManyHazardsLevel() { return; }
