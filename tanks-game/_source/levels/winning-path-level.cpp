#include "winning-path-level.h"

#include "../constants.h"
#include "../rng.h"

#include "../reset-things.h"
#include "../level-helper.h"
#include "../powerup-manager.h"
#include "../wall-manager.h"
#include "../level-manager.h"

ColorValueHolder WinningPathLevel::getDefaultColor() const {
	return ColorValueHolder(0.0f, 0.0f, 0.0f);
	//in JS, it just used the background rect's color
}

std::unordered_map<std::string, float> WinningPathLevel::getWeights() const {
	std::unordered_map<std::string, float> weights;
	weights.insert({ "vanilla-extra", 0.25f });
	weights.insert({ "random-vanilla", 0.125f });
	weights.insert({ "old", 0.25f });
	weights.insert({ "random-old", 0.25f });
	return weights;
}

void WinningPathLevel::initialize() {
	ResetThings::tankPositionReset(40);

	ColorValueHolder color = getDefaultColor();
	PositionHolder pos;
	//GenericFactoryConstructionData constructionData;
	//double* posArr;

	LevelHelper::pushClassicWalls(color);
	const PositionHolder* wallArray = LevelHelper::getClassicWallPositions();

	PowerupManager::pushPowerup(new PowerSquare(GAME_WIDTH/2, GAME_HEIGHT/2, "vanilla", "godmode"));

	LevelHelper::pushSymmetricWalls_LR(GAME_WIDTH/2, GAME_HEIGHT/2, 20, 20, GAME_HEIGHT/2, color);

	const double WALL_MIN_WIDTH = 8;
	const double WALL_MAX_WIDTH = 32;
	const double WALL_MIN_HEIGHT = 4;
	const double WALL_MAX_HEIGHT = 16;
	const int WALL_COUNT = 8;

	//converting this into LevelHelper calls is effort...
	for (int i = 0; i < 4*WALL_COUNT; i++) {
		double w = LevelRNG::randNumInRange(WALL_MIN_WIDTH, WALL_MAX_WIDTH);
		double h = LevelRNG::randNumInRange(WALL_MIN_HEIGHT, WALL_MAX_HEIGHT);
		//JS floored w and h (also, it didn't allow w=8 or h=4 for some reason)
		double x_offset = LevelRNG::randFunc() * ((wallArray[3].first - 40) - (GAME_WIDTH/2 + 20+20) - w);
		double y_offset = LevelRNG::randFunc() * ((GAME_HEIGHT/2 - 40) - (GAME_HEIGHT/4) - h);
		//space from LR walls' corner to classic walls, minus 40 for the tanks to move around
		pos = LevelHelper::getSymmetricWallPositions_Corners(i%4, GAME_WIDTH/2, GAME_HEIGHT/2, 20+20 + x_offset, GAME_HEIGHT/4 + y_offset, w, h);
		WallManager::pushWall(new Wall(pos.first, pos.second, w, h, color));
	}

	delete[] wallArray;
}

Level* WinningPathLevel::factory() {
	return new WinningPathLevel();
}

WinningPathLevel::WinningPathLevel() {
	GenericFactoryConstructionData constructionData;
	effects.push_back(LevelManager::makeLevelEffect("vanilla-extra", "invisible_walls", constructionData));
}
