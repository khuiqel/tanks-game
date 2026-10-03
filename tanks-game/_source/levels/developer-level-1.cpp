#include "developer-level-1.h"

#include "../constants.h"
//#include <iostream>

#include "../reset-things.h"
#include "../level-helper.h"
#include "../powerup-manager.h"
#include "../wall-manager.h"
#include "../hazard-manager.h"

ColorValueHolder DeveloperLevel1::getDefaultColor() const {
	return ColorValueHolder(0.25f, 0.25f, 0.25f);
}

std::unordered_map<std::string, float> DeveloperLevel1::getWeights() const {
	std::unordered_map<std::string, float> weights;
	weights.insert({ "dev", 1.0f });
	weights.insert({ "random-dev", 1.0f });
	return weights;
}

void DeveloperLevel1::initialize() {
	ResetThings::tankPositionReset(ResetThings::default_tankToEdgeDist, GAME_HEIGHT/2);

	ColorValueHolder color = getDefaultColor();
	PositionHolder pos;
	GenericFactoryConstructionData constructionData;
	double* posArr;
	std::string* names;

	PositionHolder wallPos1 = LevelHelper::getSymmetricWallPositions_LR(0, GAME_WIDTH/2, GAME_HEIGHT/2, 60, 20, 80);
	WallManager::pushWall(new Wall(wallPos1.first, wallPos1.second, 20, 80, color));
	PositionHolder wallPos2 = LevelHelper::getSymmetricWallPositions_LR(1, GAME_WIDTH/2, GAME_HEIGHT/2, 60, 20, 80);
	WallManager::pushWall(new Wall(wallPos2.first, wallPos2.second, 20, 80, color));
	WallManager::pushWall(new Wall(wallPos2.first, GAME_HEIGHT-20, 20, 20, color));

	posArr = new double[4]{ wallPos2.first, wallPos2.second+80, 20, (GAME_HEIGHT-20)-(wallPos2.second+80) };
	constructionData = GenericFactoryConstructionData(4, posArr);
	HazardManager::pushRectHazard("vanilla", "vertical_lightning", constructionData);
	posArr = new double[4]{ wallPos1.first, wallPos1.second+80, 20, GAME_HEIGHT-(wallPos1.second+80) };
	constructionData = GenericFactoryConstructionData(4, posArr);
	HazardManager::pushRectHazard("vanilla", "vertical_lightning", constructionData);
	posArr = new double[4]{ GAME_WIDTH/2 - 80 + 20, GAME_HEIGHT/2 - 40, 60*2, 20*2 - 10 };
	constructionData = GenericFactoryConstructionData(4, posArr);
	HazardManager::pushRectHazard("vanilla", "horizontal_lightning", constructionData);
	posArr = new double[4]{ GAME_WIDTH/2 - 80 + 20, GAME_HEIGHT/2 - 120, 30*2, 30*2 };
	constructionData = GenericFactoryConstructionData(4, posArr);
	HazardManager::pushRectHazard("vanilla", "lightning", constructionData);
	posArr = new double[3]{ GAME_WIDTH/2 + 30, GAME_HEIGHT/2 - 90, 30 };
	constructionData = GenericFactoryConstructionData(3, posArr);
	HazardManager::pushCircleHazard("vanilla", "lightning", constructionData);
	posArr = new double[4]{ GAME_WIDTH/2 - 80 + 20, GAME_HEIGHT/2 + 10, 60*2, 20*2 - 10 };
	constructionData = GenericFactoryConstructionData(4, posArr);
	HazardManager::pushRectHazard("vanilla", "lava", constructionData);
	posArr = new double[3]{ GAME_WIDTH/2, GAME_HEIGHT/2 + 100, 40 };
	constructionData = GenericFactoryConstructionData(3, posArr);
	HazardManager::pushCircleHazard("vanilla", "lava", constructionData);
	posArr = new double[3]{ GAME_WIDTH/2, GAME_HEIGHT/2, PI/2 };
	constructionData = GenericFactoryConstructionData(3, posArr);
	HazardManager::pushCircleHazard("vanilla", "targeting_turret", constructionData);

	//assumption: TANK_RADIUS=16 (why it would ever be changed is beyond me)
	PowerupManager::pushPowerup(new PowerSquare(20, 20, "vanilla", "speed"));
	PowerupManager::pushPowerup(new PowerSquare(40, 20, "vanilla", "wallhack"));
	PowerupManager::pushPowerup(new PowerSquare(60, 20, "vanilla", "invincible"));
	PowerupManager::pushPowerup(new PowerSquare(80, 20, "vanilla", "multishot"));
	PowerupManager::pushPowerup(new PowerSquare(100, 20, "vanilla", "big"));
	PowerupManager::pushPowerup(new PowerSquare(120, 20, "vanilla", "homing"));
	PowerupManager::pushPowerup(new PowerSquare(140, 20, "vanilla", "grenade"));
	PowerupManager::pushPowerup(new PowerSquare(160, 20, "vanilla", "banana"));

	PowerupManager::pushPowerup(new PowerSquare(GAME_WIDTH-20, 20, "dev", "ultrabounce"));

	names = new std::string[3]{ "multishot", "multishot", "invincible" };
	PowerupManager::pushPowerup(new PowerSquare(GAME_WIDTH-20, GAME_HEIGHT-20, "vanilla", names, 3));
	names[0] = "multishot", names[1] = "big", names[2] = "big";
	PowerupManager::pushPowerup(new PowerSquare(GAME_WIDTH-40, GAME_HEIGHT-20, "vanilla", names, 3));
	names[0] = "bounce", names[1] = "homing";
	PowerupManager::pushPowerup(new PowerSquare(GAME_WIDTH-60, GAME_HEIGHT-20, "vanilla", names, 2));
	delete[] names;
}

Level* DeveloperLevel1::factory() {
	return new DeveloperLevel1();
}

DeveloperLevel1::DeveloperLevel1() { return; }
