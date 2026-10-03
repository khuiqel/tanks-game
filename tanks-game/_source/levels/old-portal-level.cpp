#include "old-portal-level.h"

#include "../constants.h"
#include "../rng.h"

#include "../reset-things.h"
#include "../level-helper.h"
#include "../wall-manager.h"
#include "../powerup-manager.h"
#include "../hazard-manager.h"

std::unordered_map<std::string, float> OldPortalLevel::getWeights() const {
	std::unordered_map<std::string, float> weights;
	weights.insert({ "old", 0.25f });
	return weights;
}

void OldPortalLevel::initialize() {
	ResetThings::tankPositionReset(40);

	//in JS, power mixing was turned off
	ColorValueHolder color = getDefaultColor();
	PositionHolder pos;
	GenericFactoryConstructionData constructionData;
	//double* posArr;

	LevelHelper::pushClassicWalls(color); //in JS, initial walls were given +40 height (to ensure smooth portal transitions)
	const PositionHolder* wallArray = LevelHelper::getClassicWallPositions();

	//LR:
	LevelHelper::pushRandomWalls(6, wallArray[0].first+32 + 40, 40, 64, GAME_HEIGHT - 40*2, color, 8, 16, 12, 64);
	LevelHelper::pushRandomWalls(6, wallArray[3].first-64 - 40, 40, 64, GAME_HEIGHT - 40*2, color, 8, 16, 12, 64);

	//UD:
	LevelHelper::pushRandomWalls(4, (wallArray[0].first+32 + 40) + 64, 40,                (wallArray[3].first-64 - 40)-(wallArray[0].first+32 + 40 + 64), 64, color, 12, 64, 8, 16); //TODO: that 152 depends on the initial walls; also this doesn't do anything with the 424-152 stuff, find out what that does
	LevelHelper::pushRandomWalls(4, (wallArray[0].first+32 + 40) + 64, GAME_HEIGHT-64-40, (wallArray[3].first-64 - 40)-(wallArray[0].first+32 + 40 + 64), 64, color, 12, 64, 8, 16); //TODO: that 152 depends on the initial walls; also this doesn't do anything with the 424-152 stuff, find out what that does

	PowerupManager::pushPowerup(new PowerSquare(GAME_WIDTH/2, GAME_HEIGHT/2, "vanilla", "wallhack"));

	LevelHelper::pushSymmetricPowerups_Corners_Alternate(GAME_WIDTH/2, GAME_HEIGHT/2, (wallArray[3].first - (wallArray[0].first + 32))/2 - 20, 32+20,
		"vanilla", "multishot", "vanilla", "triple");

	//traps:
	//LevelHelper::pushSymmetricPowerups_LR(GAME_WIDTH/2, GAME_HEIGHT/2, 64, "vanilla", "speed");

	LevelHelper::pushSymmetricPowerups_UD_Alternate(GAME_WIDTH/2, GAME_HEIGHT/2, 20,
		"vanilla", "speed", "vanilla-extra", "shotgun");
}

Level* OldPortalLevel::factory() {
	return new OldPortalLevel();
}

OldPortalLevel::OldPortalLevel() { return; }
