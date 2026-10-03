#include "concealed-powerups-level.h"

#include "../constants.h"

#include "../reset-things.h"
#include "../level-helper.h"
#include "../powerup-manager.h"
#include "../wall-manager.h"
#include "../hazard-manager.h"

std::unordered_map<std::string, float> ConcealedPowerupsLevel::getWeights() const {
	std::unordered_map<std::string, float> weights;
	weights.insert({ "old", 1.0f });
	weights.insert({ "random-old", 1.0f });
	return weights;
}

void ConcealedPowerupsLevel::initialize() {
	ResetThings::tankPositionReset(40);

	//in JS, power mixing was turned off
	ColorValueHolder color = getDefaultColor();
	PositionHolder pos;
	GenericFactoryConstructionData constructionData;
	double* posArr;

	LevelHelper::pushClassicWalls(color);

	pos = LevelHelper::getSymmetricWallPositions_UD(0, GAME_WIDTH/2, GAME_HEIGHT/2, 70, 160, 20);
	posArr = new double[4]{ pos.first, pos.second, 160, 20 };
	constructionData = GenericFactoryConstructionData(4, posArr);
	HazardManager::pushRectHazard("vanilla", "horizontal_lightning", constructionData);
	pos = LevelHelper::getSymmetricWallPositions_UD(1, GAME_WIDTH/2, GAME_HEIGHT/2, 70, 160, 20);
	posArr = new double[4]{ pos.first, pos.second, 160, 20 };
	constructionData = GenericFactoryConstructionData(4, posArr);
	HazardManager::pushRectHazard("vanilla", "horizontal_lightning", constructionData);

	LevelHelper::pushSymmetricWalls_LR(GAME_WIDTH/2, GAME_HEIGHT/2, 80, 20, GAME_HEIGHT-70*2, color);

	//traps:
	//LevelHelper::pushSymmetricPowerups_LR(GAME_WIDTH/2, GAME_HEIGHT/2, 80+20+20, "vanilla", "speed");

	PowerupManager::pushPowerup(new PowerSquare(GAME_WIDTH/2, GAME_HEIGHT/2, "old", "old_big"));

	LevelHelper::pushSymmetricPowerups_UD_Alternate(GAME_WIDTH/2, GAME_HEIGHT/2, 20,
		"vanilla", "speed", "vanilla", "invincible");

	LevelHelper::pushSymmetricPowerups_LR(GAME_WIDTH/2, GAME_HEIGHT/2, 20, "vanilla", "grenade");

	LevelHelper::pushSymmetricPowerups_Corners(GAME_WIDTH/2, GAME_HEIGHT/2, GAME_WIDTH/2-(80+32+20), GAME_HEIGHT/2-20, "vanilla", "homing");
}

Level* ConcealedPowerupsLevel::factory() {
	return new ConcealedPowerupsLevel();
}

ConcealedPowerupsLevel::ConcealedPowerupsLevel() { return; }
