#include "unnamed-level-11.h"

#include "../constants.h"
#include <iostream>

#include "../reset-things.h"
#include "../level-helper.h"
#include "../powerup-manager.h"
#include "../wall-manager.h"
#include "../hazard-manager.h"

ColorValueHolder UnnamedLevel11::getDefaultColor() const {
	return ColorValueHolder(1, 1, 1);
}

std::unordered_map<std::string, float> UnnamedLevel11::getWeights() const {
	std::unordered_map<std::string, float> weights;
	weights.insert({ "dev", 0.5f });
	weights.insert({ "random-dev", 0.5f });
	return weights;
}

void UnnamedLevel11::initialize() {
	ResetThings::tankPositionReset(40);

	ColorValueHolder color = getDefaultColor();
	PositionHolder pos;
	GenericFactoryConstructionData constructionData;
	double* posArr;

	LevelHelper::pushClassicWalls(color);
	const PositionHolder* wallArray = LevelHelper::getClassicWallPositions();

	const double centerOffset = 40;
	const double zoneWidth = 10;
	LevelHelper::pushSymmetricWalls_Corners(GAME_WIDTH/2, GAME_HEIGHT/2, centerOffset, centerOffset, zoneWidth, zoneWidth, color);
	PowerupManager::pushPowerup(new PowerSquare(GAME_WIDTH/2, GAME_HEIGHT/2, "vanilla", "godmode"));

	pos = LevelHelper::getSymmetricWallPositions_LR(0, GAME_WIDTH/2, GAME_HEIGHT/2, centerOffset, zoneWidth, 2*centerOffset);
	posArr = new double[4]{ pos.first, pos.second, zoneWidth, 2*centerOffset };
	constructionData = GenericFactoryConstructionData(4, posArr);
	HazardManager::pushRectHazard("vanilla", "no_bullet_zone", constructionData);
	pos = LevelHelper::getSymmetricWallPositions_LR(1, GAME_WIDTH/2, GAME_HEIGHT/2, centerOffset, zoneWidth, 2*centerOffset);
	posArr = new double[4]{ pos.first, pos.second, zoneWidth, 2*centerOffset };
	constructionData = GenericFactoryConstructionData(4, posArr);
	HazardManager::pushRectHazard("vanilla", "no_bullet_zone", constructionData);

	pos = LevelHelper::getSymmetricWallPositions_UD(0, GAME_WIDTH/2, GAME_HEIGHT/2, centerOffset, 2*centerOffset, zoneWidth);
	posArr = new double[4]{ pos.first, pos.second, 2*centerOffset, zoneWidth };
	constructionData = GenericFactoryConstructionData(4, posArr);
	HazardManager::pushRectHazard("vanilla", "no_bullet_zone", constructionData);
	pos = LevelHelper::getSymmetricWallPositions_UD(1, GAME_WIDTH/2, GAME_HEIGHT/2, centerOffset, 2*centerOffset, zoneWidth);
	posArr = new double[4]{ pos.first, pos.second, 2*centerOffset, zoneWidth };
	constructionData = GenericFactoryConstructionData(4, posArr);
	HazardManager::pushRectHazard("vanilla", "no_bullet_zone", constructionData);

	const double wallGap = 60;
	LevelHelper::pushSymmetricWalls_LR(GAME_WIDTH/2, GAME_HEIGHT/2, GAME_WIDTH/2 - (wallArray[0].first+32) - (wallGap+20), 20, 200, color);

	pos = LevelHelper::getSymmetricPowerupPositions_UD(0, (wallArray[0].first+32) + wallGap/2, GAME_HEIGHT/2, GAME_HEIGHT/2 - 10);
	posArr = new double[3]{ pos.first, pos.second, PI/2 };
	constructionData = GenericFactoryConstructionData(3, posArr);
	HazardManager::pushCircleHazard("vanilla", "stationary_turret", constructionData);
	pos = LevelHelper::getSymmetricPowerupPositions_UD(1, (wallArray[0].first+32) + wallGap/2, GAME_HEIGHT/2, GAME_HEIGHT/2 - 10);
	posArr = new double[3]{ pos.first, pos.second, -PI/2 };
	constructionData = GenericFactoryConstructionData(3, posArr);
	HazardManager::pushCircleHazard("vanilla", "stationary_turret", constructionData);
	pos = LevelHelper::getSymmetricPowerupPositions_UD(0, (wallArray[3].first) - wallGap/2, GAME_HEIGHT/2, GAME_HEIGHT/2 - 10);
	posArr = new double[3]{ pos.first, pos.second, PI/2 };
	constructionData = GenericFactoryConstructionData(3, posArr);
	HazardManager::pushCircleHazard("vanilla", "stationary_turret", constructionData);
	pos = LevelHelper::getSymmetricPowerupPositions_UD(1, (wallArray[3].first) - wallGap/2, GAME_HEIGHT/2, GAME_HEIGHT/2 - 10);
	posArr = new double[3]{ pos.first, pos.second, -PI/2 };
	constructionData = GenericFactoryConstructionData(3, posArr);
	HazardManager::pushCircleHazard("vanilla", "stationary_turret", constructionData);

	pos = LevelHelper::getSymmetricPowerupPositions_UD(0, (wallArray[0].first+32) + (wallGap+20+wallGap/2), GAME_HEIGHT/2, GAME_HEIGHT/2 - 10);
	posArr = new double[3]{ pos.first, pos.second, PI/2 };
	constructionData = GenericFactoryConstructionData(3, posArr);
	HazardManager::pushCircleHazard("vanilla", "stationary_turret", constructionData);
	pos = LevelHelper::getSymmetricPowerupPositions_UD(1, (wallArray[0].first+32) + (wallGap+20+wallGap/2), GAME_HEIGHT/2, GAME_HEIGHT/2 - 10);
	posArr = new double[3]{ pos.first, pos.second, -PI/2 };
	constructionData = GenericFactoryConstructionData(3, posArr);
	HazardManager::pushCircleHazard("vanilla", "stationary_turret", constructionData);
	pos = LevelHelper::getSymmetricPowerupPositions_UD(0, (wallArray[3].first) - (wallGap+20+wallGap/2), GAME_HEIGHT/2, GAME_HEIGHT/2 - 10);
	posArr = new double[3]{ pos.first, pos.second, PI/2 };
	constructionData = GenericFactoryConstructionData(3, posArr);
	HazardManager::pushCircleHazard("vanilla", "stationary_turret", constructionData);
	pos = LevelHelper::getSymmetricPowerupPositions_UD(1, (wallArray[3].first) - (wallGap+20+wallGap/2), GAME_HEIGHT/2, GAME_HEIGHT/2 - 10);
	posArr = new double[3]{ pos.first, pos.second, -PI/2 };
	constructionData = GenericFactoryConstructionData(3, posArr);
	HazardManager::pushCircleHazard("vanilla", "stationary_turret", constructionData);

	delete[] wallArray;
}

Level* UnnamedLevel11::factory() {
	return new UnnamedLevel11();
}

UnnamedLevel11::UnnamedLevel11() { return; }
