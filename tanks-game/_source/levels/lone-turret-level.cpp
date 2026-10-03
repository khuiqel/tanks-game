#include "lone-turret-level.h"

#include "../constants.h"
#include <cmath> //floor
#include "../rng.h"

#include "../reset-things.h"
#include "../level-helper.h"
#include "../powerup-manager.h"
#include "../wall-manager.h"
#include "../hazard-manager.h"

std::unordered_map<std::string, float> LoneTurretLevel::getWeights() const {
	std::unordered_map<std::string, float> weights;
	weights.insert({ "old", 1.0f });
	weights.insert({ "random-old", 0.5f });
	return weights;
}

void LoneTurretLevel::initialize() {
	ResetThings::tankPositionReset(40);

	ColorValueHolder color = getDefaultColor();
	PositionHolder pos;
	GenericFactoryConstructionData constructionData;
	double* posArr;

	//LevelHelper::pushClassicWalls(color); //JS

	posArr = new double[3]{ GAME_WIDTH/2, GAME_HEIGHT/2, PI/2 * (std::floor(LevelRNG::randFunc()*2)*2-1) };
	constructionData = GenericFactoryConstructionData(3, posArr);
	HazardManager::pushCircleHazard("vanilla", "targeting_turret", constructionData);

	//rectangular lightning surrounding turret:
	//posArr = new double[4]{ GAME_WIDTH/2 - 80, GAME_HEIGHT/2 - 40, 80*2, 40*2 };
	//constructionData = GenericFactoryConstructionData(4, posArr);
	//HazardManager::pushRectHazard("vanilla", "lightning", constructionData);
	//posArr = new double[4]{ GAME_WIDTH/2 - 40, GAME_HEIGHT/2 - 40, 40*2, 40*2 };
	//constructionData = GenericFactoryConstructionData(4, posArr);
	//HazardManager::pushRectHazard("vanilla", "lightning", constructionData);

	pos = LevelHelper::getSymmetricWallPositions_LR(0, GAME_WIDTH/2, GAME_HEIGHT/2, 80, 20, 80);
	posArr = new double[4]{ pos.first, pos.second, 20, 80 };
	constructionData = GenericFactoryConstructionData(4, posArr);
	HazardManager::pushRectHazard("vanilla", "no_bullet_zone", constructionData);
	pos = LevelHelper::getSymmetricWallPositions_LR(1, GAME_WIDTH/2, GAME_HEIGHT/2, 80, 20, 80);
	posArr = new double[4]{ pos.first, pos.second, 20, 80 };
	constructionData = GenericFactoryConstructionData(4, posArr);
	HazardManager::pushRectHazard("vanilla", "no_bullet_zone", constructionData);

	LevelHelper::pushSymmetricWalls_Corners(GAME_WIDTH/2, GAME_HEIGHT/2, 80, 40, 20, 80, color);
	LevelHelper::pushSymmetricWalls_Corners(GAME_WIDTH/2, GAME_HEIGHT/2, 80+20, 40, 80-20, 20, color);

	//outer powers:
	LevelHelper::pushSymmetricPowerups_Corners_Alternate(GAME_WIDTH/2, GAME_HEIGHT/2, 80+20+(80-20)/2, 40+20+(80-20)/2,
		"vanilla-extra", "barrier", "vanilla-extra", "tracking");

	//inner powers:
	LevelHelper::pushSymmetricPowerups_Corners_Alternate(GAME_WIDTH/2, GAME_HEIGHT/2, 80/2, 40+20+(80-20)/2,
		"vanilla", "grenade", "vanilla", "blast");

	//alternate inner powers:
	//LevelHelper::pushSymmetricPowerups_DiagForwardSlash_Alternate(GAME_WIDTH/2, GAME_HEIGHT/2, 80/2, 40+20+(80-20)/2,
	//	"vanilla", "fire", "vanilla", "grenade");
	//LevelHelper::pushSymmetricPowerups_DiagBackwardSlash_Alternate(GAME_WIDTH/2, GAME_HEIGHT/2, 80/2, 40+20+(80-20)/2,
	//	"vanilla", "bounce", "vanilla", "blast");
}

Level* LoneTurretLevel::factory() {
	return new LoneTurretLevel();
}

LoneTurretLevel::LoneTurretLevel() { return; }
