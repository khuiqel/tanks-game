#include "unnamed-level-13.h"

#include "../constants.h"
#include <iostream>
#include "../rng.h"

#include "../reset-things.h"
#include "../level-helper.h"
#include "../powerup-manager.h"
#include "../wall-manager.h"
#include "../hazard-manager.h"
#include "../level-manager.h"

#include "../level-effects/respawning-powerups-level-effect.h"

ColorValueHolder UnnamedLevel13::getDefaultColor() const {
	return ColorValueHolder(1, 1, 1);
}

std::unordered_map<std::string, float> UnnamedLevel13::getWeights() const {
	std::unordered_map<std::string, float> weights;
	weights.insert({ "dev", 0.5f });
	weights.insert({ "random-dev", 0.5f });
	return weights;
}

void UnnamedLevel13::initialize() {
	ResetThings::tankPositionReset(ResetThings::default_tankToEdgeDist, GAME_HEIGHT*(3.0/5), 2);

	ColorValueHolder color = getDefaultColor();
	PositionHolder pos;
	GenericFactoryConstructionData constructionData;
	double* posArr;
	//PowerSquare* p;

	if (effects.size() < 1) [[unlikely]] {
		throw std::logic_error("ERROR: \"" + getName() + "\" level does not have sufficient level effects!");
	}
	RespawningPowerupsLevelEffect* respawning = static_cast<RespawningPowerupsLevelEffect*>(effects[0]);
	if (respawning->getName() != "respawning_powerups") [[unlikely]] {
		throw std::logic_error("ERROR: \"" + getName() + "\" level does not have \"respawning_powerups\" level effect!");
	}

	posArr = new double[4]{ GAME_WIDTH/2, GAME_HEIGHT/2, 16, 64 };
	double* strengthArr = new double[2]{ .10, .95 };
	constructionData = GenericFactoryConstructionData(4, posArr);
	constructionData.pushData(2, strengthArr);
	HazardManager::pushCircleHazard("vanilla-extra", "gravity_well", constructionData);

	pos = LevelHelper::getSymmetricWallPositions_UD(0, GAME_WIDTH/2, GAME_HEIGHT/2, 64+20, 64*2, 20);
	posArr = new double[4]{ pos.first, pos.second, 64*2, 20 };
	constructionData = GenericFactoryConstructionData(4, posArr);
	HazardManager::pushRectHazard("dev", "reflecktor", constructionData);
	pos = LevelHelper::getSymmetricWallPositions_UD(1, GAME_WIDTH/2, GAME_HEIGHT/2, 64+20, 64*2, 20);
	posArr = new double[4]{ pos.first, pos.second, 64*2, 20 };
	constructionData = GenericFactoryConstructionData(4, posArr);
	HazardManager::pushRectHazard("dev", "reflecktor", constructionData);

	if (LevelRNG::randFunc() < .5) {
		LevelHelper::pushSymmetricPowerups_UD(GAME_WIDTH/2, GAME_HEIGHT/2, 64+10, "vanilla-extra", "barrier");
		LevelHelper::pushSymmetricPowerups_UD(GAME_WIDTH/2, GAME_HEIGHT/2, 64+50, "vanilla", "multishot");
	} else {
		LevelHelper::pushSymmetricPowerups_UD(GAME_WIDTH/2, GAME_HEIGHT/2, 64+10, "vanilla", "multishot");
		LevelHelper::pushSymmetricPowerups_UD(GAME_WIDTH/2, GAME_HEIGHT/2, 64+50, "vanilla-extra", "barrier");
	}

	for (int i = 0; i < 4; i++) {
		pos = LevelHelper::getSymmetricWallPositions_Corners(i, GAME_WIDTH/2, GAME_HEIGHT/2, 64+40, 22, 20, 48*2);
		posArr = new double[4]{ pos.first, pos.second, 20, 48*2 };
		constructionData = GenericFactoryConstructionData(4, posArr);
		HazardManager::pushRectHazard("dev", "reflecktor", constructionData);
	}
	for (int i = 0; i < 4; i++) {
		pos = LevelHelper::getSymmetricPowerupPositions_Corners(i, GAME_WIDTH/2, GAME_HEIGHT/2, (64+40)+20+10+48, 22+48);
		posArr = new double[4]{ pos.first, pos.second, 16, 48 };
		constructionData = GenericFactoryConstructionData(4, posArr);
		HazardManager::pushCircleHazard("vanilla-extra", "gravity_well", constructionData);
	}
	for (int i = 0; i < 4; i++) {
		pos = LevelHelper::getSymmetricWallPositions_Corners(i, GAME_WIDTH/2, GAME_HEIGHT/2, ((64+40)+20+10+48) + 48+10, 22, 20, 48*2);
		posArr = new double[4]{ pos.first, pos.second, 20, 48*2 };
		constructionData = GenericFactoryConstructionData(4, posArr);
		HazardManager::pushRectHazard("dev", "reflecktor", constructionData);
	}

	LevelHelper::pushSymmetricPowerups_Corners_Alternate(GAME_WIDTH/2, GAME_HEIGHT/2, (64+40)+20+10/2, 22+48,
		"vanilla", "speed", "vanilla", "blast");
	LevelHelper::pushSymmetricPowerups_Corners_Alternate(GAME_WIDTH/2, GAME_HEIGHT/2, ((64+40)+20+(10+48)*2)-10/2, 22+48,
		"dev", "inversion", "dev", "edge_sparks");
	respawning->watchLastPowerSquaresPushed(4, 1000);
}

Level* UnnamedLevel13::factory() {
	return new UnnamedLevel13();
}

UnnamedLevel13::UnnamedLevel13() {
	bool* temp = new bool[1]{ false };
	GenericFactoryConstructionData constructionData(1, temp);
	effects.push_back(LevelManager::makeLevelEffect("vanilla", "respawning_powerups", constructionData));
}
