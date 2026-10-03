#include "mine-heaven-level.h"

#include "../constants.h"

#include "../reset-things.h"
#include "../level-helper.h"
#include "../powerup-manager.h"
#include "../wall-manager.h"
#include "../hazard-manager.h"
#include "../level-manager.h"

std::unordered_map<std::string, float> MineHeavenLevel::getWeights() const {
	std::unordered_map<std::string, float> weights;
	weights.insert({ "vanilla-extra", 0.25f });
	weights.insert({ "random-vanilla", 0.125f }); //people just do not like ice physics
	weights.insert({ "old", 0.25f });
	weights.insert({ "random-old", 0.125f });
	return weights;
}

void MineHeavenLevel::initialize() {
	ResetThings::tankPositionReset(40);

	//in JS, power mixing was turned off
	ColorValueHolder color = getDefaultColor();
	PositionHolder pos;
	//GenericFactoryConstructionData constructionData;
	//double* posArr;
	std::string* types;
	std::string* names;

	LevelHelper::pushClassicWalls(color);

	//total of 77 mine powerups

	types = new std::string[2]{ "vanilla-extra", "old" };
	names = new std::string[2]{ "mines", "old_mines" };
	PowerupManager::pushPowerup(new PowerSquare(GAME_WIDTH/2, GAME_HEIGHT/2, types, names, 2));
	delete[] types; delete[] names;
	//the reason there were two was to increase shooting speed

	const double mineSpacing = 16;
	const double horzDistFromEdge = 40*2 + 32 + mineSpacing; //= 128

	//8*4 on the diagonals from the center
	for (int i = 1; i <= 8; i++) {
		LevelHelper::pushSymmetricPowerups_Corners(GAME_WIDTH/2, GAME_HEIGHT/2, i * (GAME_WIDTH/2 - horzDistFromEdge)/8.0, i * (GAME_HEIGHT/2 - mineSpacing)/8.0, "vanilla-extra", "mines");
	}

	//15*2 on the top and bottom
	for (int i = -7; i <= 7; i++) {
		LevelHelper::pushSymmetricPowerups_UD(GAME_WIDTH/2 - i * (GAME_WIDTH/2 - horzDistFromEdge)/8.0, GAME_HEIGHT/2, GAME_HEIGHT/2 - mineSpacing, "vanilla-extra", "mines");
	}

	//7*2 along the center
	for (int i = 1; i <= 7; i++) {
		LevelHelper::pushSymmetricPowerups_UD(GAME_WIDTH/2, GAME_HEIGHT/2, i * (GAME_HEIGHT/2 - mineSpacing)/8.0, "vanilla-extra", "mines");
		//JS did 14 in a row, not 7 pairs
	}
}

Level* MineHeavenLevel::factory() {
	return new MineHeavenLevel();
}

MineHeavenLevel::MineHeavenLevel() {
	const PositionHolder* wallArray = LevelHelper::getClassicWallPositions();

	GenericFactoryConstructionData constructionData;
	effects.push_back(LevelManager::makeLevelEffect("vanilla-extra", "ice", constructionData));
	double* temp = new double[4]{ wallArray[0].first+32, 0, wallArray[3].first - (wallArray[0].first+32), GAME_HEIGHT };
	constructionData = GenericFactoryConstructionData(4, temp);
	effects.push_back(LevelManager::makeLevelEffect("vanilla", "minefield", constructionData));

	delete[] wallArray;
}
