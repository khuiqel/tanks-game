#include "lightning-corners-level.h"

#include "../constants.h"

#include "../reset-things.h"
#include "../level-helper.h"
#include "../powerup-manager.h"
#include "../wall-manager.h"
#include "../hazard-manager.h"

std::unordered_map<std::string, float> LightningCornersLevel::getWeights() const {
	std::unordered_map<std::string, float> weights;
	weights.insert({ "old", 0.25f });
	return weights;
}

void LightningCornersLevel::initialize() {
	ResetThings::tankPositionReset(40);

	ColorValueHolder color = getDefaultColor();
	PositionHolder pos;
	GenericFactoryConstructionData constructionData;
	double* posArr;
	std::string* names;

	LevelHelper::pushClassicWalls(color);

	LevelHelper::pushSymmetricWalls_Corners(GAME_WIDTH/2, GAME_HEIGHT/2, 80-20, 80-20, 20, 20, color);

	//I don't think this should exist
	posArr = new double[4]{ GAME_WIDTH/2 - 80 + 20, GAME_HEIGHT/2 - 80 + 20, (80-20)*2, (80-20)*2 };
	constructionData = GenericFactoryConstructionData(4, posArr);
	//HazardManager::pushRectHazard("vanilla", "no_bullet_zone", constructionData);

	posArr = new double[4]{ GAME_WIDTH/2 - 80 + 20, GAME_HEIGHT/2 - 80, (80-20)*2, 20 };
	constructionData = GenericFactoryConstructionData(4, posArr);
	HazardManager::pushRectHazard("vanilla", "horizontal_lightning", constructionData);
	posArr = new double[4]{ GAME_WIDTH/2 - 80 + 20, GAME_HEIGHT/2 + 80 - 20, (80-20)*2, 20 };
	constructionData = GenericFactoryConstructionData(4, posArr);
	HazardManager::pushRectHazard("vanilla", "horizontal_lightning", constructionData);
	posArr = new double[4]{ GAME_WIDTH/2 - 80, GAME_HEIGHT/2 - 80 + 20, 20, (80-20)*2 };
	constructionData = GenericFactoryConstructionData(4, posArr);
	HazardManager::pushRectHazard("vanilla", "vertical_lightning", constructionData);
	posArr = new double[4]{ GAME_WIDTH/2 + 80 - 20, GAME_HEIGHT/2 - 80 + 20, 20, (80-20)*2 };
	constructionData = GenericFactoryConstructionData(4, posArr);
	HazardManager::pushRectHazard("vanilla", "vertical_lightning", constructionData);

	//traps:
	//LevelHelper::pushSymmetricPowerups_Corners(GAME_WIDTH/2, GAME_HEIGHT/2, 80-20/2, 80+20/2, "vanilla", "speed");
	//LevelHelper::pushSymmetricPowerups_Corners(GAME_WIDTH/2, GAME_HEIGHT/2, 80+20/2, 80-20/2, "vanilla", "speed");

	//unused:
	//LevelHelper::pushSymmetricPowerups_Corners(GAME_WIDTH/2, GAME_HEIGHT/2, 80+20/2, 80+20/2, "vanilla", "banana");

	names = new std::string[5]{ "speed", "speed", "speed", "big", "big" };
	PowerupManager::pushPowerup(new PowerSquare(GAME_WIDTH/2, GAME_HEIGHT/2, "vanilla", names, 5));
	delete[] names;
	//JS used 4 speed to accomplish the same thing (big bullet speed decrease stacked there) (if using old big power, do what JS does)
	//if the powerup at the center is split, use this:
	//LevelHelper::pushSymmetricPowerups_Corners(GAME_WIDTH/2, GAME_HEIGHT/2, 10, 10, "vanilla", "speed");

	//not here in the JS level but I feel it should be here:
	pos = LevelHelper::getSymmetricPowerupPositions_LR(0, GAME_WIDTH/2, GAME_HEIGHT/2, 80+20);
	posArr = new double[3]{ pos.first, pos.second, 0 };
	constructionData = GenericFactoryConstructionData(3, posArr);
	HazardManager::pushCircleHazard("vanilla", "stationary_turret", constructionData);
	pos = LevelHelper::getSymmetricPowerupPositions_LR(1, GAME_WIDTH/2, GAME_HEIGHT/2, 80+20);
	posArr = new double[3]{ pos.first, pos.second, PI };
	constructionData = GenericFactoryConstructionData(3, posArr);
	HazardManager::pushCircleHazard("vanilla", "stationary_turret", constructionData);
	pos = LevelHelper::getSymmetricPowerupPositions_UD(0, GAME_WIDTH/2, GAME_HEIGHT/2, 80+20);
	posArr = new double[3]{ pos.first, pos.second, PI/2 };
	constructionData = GenericFactoryConstructionData(3, posArr);
	HazardManager::pushCircleHazard("vanilla", "stationary_turret", constructionData);
	pos = LevelHelper::getSymmetricPowerupPositions_UD(1, GAME_WIDTH/2, GAME_HEIGHT/2, 80+20);
	posArr = new double[3]{ pos.first, pos.second, -PI/2 };
	constructionData = GenericFactoryConstructionData(3, posArr);
	HazardManager::pushCircleHazard("vanilla", "stationary_turret", constructionData);

	//just trying stuff
	//posArr = new double[4]{ wallArray[0].first+32, 0, 16, GAME_HEIGHT };
	//constructionData = GenericFactoryConstructionData(4, posArr);
	//HazardManager::pushRectHazard("vanilla", "vertical_lightning", constructionData);
	//posArr = new double[4]{ wallArray[3].first-16, 0, 16, GAME_HEIGHT };
	//constructionData = GenericFactoryConstructionData(4, posArr);
	//HazardManager::pushRectHazard("vanilla", "vertical_lightning", constructionData);
}

Level* LightningCornersLevel::factory() {
	return new LightningCornersLevel();
}

LightningCornersLevel::LightningCornersLevel() { return; }
