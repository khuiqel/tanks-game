#include "unnamed-level-10.h"

#include "../constants.h"
#include <iostream>

#include "../reset-things.h"
#include "../level-helper.h"
#include "../powerup-manager.h"
#include "../wall-manager.h"
#include "../hazard-manager.h"

ColorValueHolder UnnamedLevel10::getDefaultColor() const {
	return ColorValueHolder(1, 1, 1);
}

std::unordered_map<std::string, float> UnnamedLevel10::getWeights() const {
	std::unordered_map<std::string, float> weights;
	weights.insert({ "dev", 1.0f });
	weights.insert({ "random-dev", 1.0f });
	return weights;
}

void UnnamedLevel10::initialize() {
	ResetThings::tankPositionReset(ResetThings::default_tankToEdgeDist, GAME_HEIGHT - 2*20, 2);

	ColorValueHolder color = getDefaultColor();
	PositionHolder pos;
	GenericFactoryConstructionData constructionData;
	double* posArr;
	std::string* types;
	std::string* names;

	LevelHelper::pushSymmetricWalls_Corners(GAME_WIDTH/2, GAME_HEIGHT/2, GAME_WIDTH/2 - 80, GAME_HEIGHT/2 - (40+20), 80, 20, color);

	LevelHelper::pushSymmetricWalls_LR(GAME_WIDTH/2, GAME_HEIGHT/2, GAME_WIDTH/2 - 80, 20, 120, color);
	LevelHelper::pushSymmetricWalls_LR(GAME_WIDTH/2, GAME_HEIGHT/2, GAME_WIDTH/2 - (80+20+40), 20, 120, color);

	LevelHelper::pushSymmetricPowerups_Corners_Alternate(GAME_WIDTH/2, GAME_HEIGHT/2, GAME_WIDTH/2 - 80 - 40/2, 120/2 - 10,
		"vanilla-extra", "barrier", "dev", "wall_sparks");

	types = new std::string[2]{ "dev", "dev" };
	names = new std::string[2]{ "inversion", "ultrabounce" };
	LevelHelper::pushSymmetricPowerups_LR(GAME_WIDTH/2, GAME_HEIGHT/2, GAME_WIDTH/2-60/2, types, names, 2);
	delete[] types; delete[] names;

	const double centerOffset = 30;
	types = new std::string[3]{ "vanilla", "dev", "vanilla" };
	names = new std::string[3]{ "multishot", "inversion", "banana" };
	LevelHelper::pushSymmetricPowerups_LR(GAME_WIDTH/2, GAME_HEIGHT/2, 20/2 + centerOffset, types, names, 3);
	delete[] types; delete[] names;

	LevelHelper::pushSymmetricWalls_UD(GAME_WIDTH/2, GAME_HEIGHT/2, centerOffset, 20, 20, color);
	LevelHelper::pushSymmetricWalls_Corners(GAME_WIDTH/2, GAME_HEIGHT/2, 20/2 + 2*centerOffset, centerOffset, 20, 20, color);
	//WallManager::pushWall(new Wall(GAME_WIDTH/2 - 20/2, GAME_HEIGHT/2 - centerOffset, 20, 2*centerOffset, color));

	pos = LevelHelper::getSymmetricWallPositions_LR(0, GAME_WIDTH/2, GAME_HEIGHT/2, 20/2 + 2*centerOffset, 20, 2*centerOffset);
	posArr = new double[4]{ pos.first, pos.second, 20, 2*centerOffset };
	constructionData = GenericFactoryConstructionData(4, posArr);
	HazardManager::pushRectHazard("vanilla", "vertical_lightning", constructionData);
	pos = LevelHelper::getSymmetricWallPositions_LR(1, GAME_WIDTH/2, GAME_HEIGHT/2, 20/2 + 2*centerOffset, 20, 2*centerOffset);
	posArr = new double[4]{ pos.first, pos.second, 20, 2*centerOffset };
	constructionData = GenericFactoryConstructionData(4, posArr);
	HazardManager::pushRectHazard("vanilla", "vertical_lightning", constructionData);

	posArr = new double[4]{ GAME_WIDTH/2 - 20/2, GAME_HEIGHT/2 - centerOffset, 20, 2*centerOffset };
	constructionData = GenericFactoryConstructionData(4, posArr);
	HazardManager::pushRectHazard("vanilla", "vertical_lightning", constructionData);

	for (int i = 0; i < 4; i++) {
		pos = LevelHelper::getSymmetricWallPositions_Corners(i, GAME_WIDTH/2, GAME_HEIGHT/2, 20/2, centerOffset, 2*centerOffset, 20);
		posArr = new double[4]{ pos.first, pos.second, 2*centerOffset, 20 };
		constructionData = GenericFactoryConstructionData(4, posArr);
		HazardManager::pushRectHazard("vanilla", "horizontal_lightning", constructionData);
	}

	//old version with one "center" instead of two:
	/*
	types = new std::string[3]{ "vanilla", "dev", "vanilla" };
	names = new std::string[3]{ "multishot", "inversion", "banana" };
	PowerupManager::pushPowerup(new PowerSquare(GAME_WIDTH/2, GAME_HEIGHT/2, types, names, 3));
	delete[] types; delete[] names;

	const double centerOffset = 30;
	LevelHelper::pushSymmetricWalls_Corners(GAME_WIDTH/2, GAME_HEIGHT/2, centerOffset, centerOffset, 20, 20, color);

	pos = LevelHelper::getSymmetricWallPositions_LR(0, GAME_WIDTH/2, GAME_HEIGHT/2, centerOffset, 20, 2*centerOffset);
	posArr = new double[4]{ pos.first, pos.second, 20, 2*centerOffset };
	constructionData = GenericFactoryConstructionData(4, posArr);
	HazardManager::pushRectHazard("vanilla", "vertical_lightning", constructionData);
	pos = LevelHelper::getSymmetricWallPositions_LR(1, GAME_WIDTH/2, GAME_HEIGHT/2, centerOffset, 20, 2*centerOffset);
	posArr = new double[4]{ pos.first, pos.second, 20, 2*centerOffset };
	constructionData = GenericFactoryConstructionData(4, posArr);
	HazardManager::pushRectHazard("vanilla", "vertical_lightning", constructionData);

	pos = LevelHelper::getSymmetricWallPositions_UD(0, GAME_WIDTH/2, GAME_HEIGHT/2, centerOffset, 2*centerOffset, 20);
	posArr = new double[4]{ pos.first, pos.second, 2*centerOffset, 20 };
	constructionData = GenericFactoryConstructionData(4, posArr);
	HazardManager::pushRectHazard("vanilla", "horizontal_lightning", constructionData);
	pos = LevelHelper::getSymmetricWallPositions_UD(1, GAME_WIDTH/2, GAME_HEIGHT/2, centerOffset, 2*centerOffset, 20);
	posArr = new double[4]{ pos.first, pos.second, 2*centerOffset, 20 };
	constructionData = GenericFactoryConstructionData(4, posArr);
	HazardManager::pushRectHazard("vanilla", "horizontal_lightning", constructionData);
	*/

	//honestly a bit worse than unnamed 9
}

Level* UnnamedLevel10::factory() {
	return new UnnamedLevel10();
}

UnnamedLevel10::UnnamedLevel10() { return; }
