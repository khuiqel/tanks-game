#include "tricky-maneuvering-level.h"

#include "../constants.h"

#include "../reset-things.h"
#include "../level-helper.h"
#include "../powerup-manager.h"
#include "../wall-manager.h"
#include "../hazard-manager.h"
#include "../level-manager.h"

std::unordered_map<std::string, float> TrickyManeuveringLevel::getWeights() const {
	std::unordered_map<std::string, float> weights;
	weights.insert({ "vanilla-extra", 0.5f });
	weights.insert({ "random-vanilla", 0.25f });
	weights.insert({ "old", 0.25f });
	weights.insert({ "random-old", 0.25f });
	return weights;
}

void TrickyManeuveringLevel::initialize() {
	ResetThings::tankPositionReset(40);

	//in JS, power mixing was turned off
	ColorValueHolder color = getDefaultColor();
	//PositionHolder pos;
	GenericFactoryConstructionData constructionData;
	double* posArr;
	std::string* types;
	std::string* names;

	LevelHelper::pushClassicWalls(color);
	const PositionHolder* wallArray = LevelHelper::getClassicWallPositions();

	LevelHelper::pushSymmetricWalls_Corners(GAME_WIDTH/2, GAME_HEIGHT/2, 40, 40, 20, 20, color);

	posArr = new double[4]{ GAME_WIDTH/2 - 40, GAME_HEIGHT/2 - 40 - 20, 40*2, 20 };
	constructionData = GenericFactoryConstructionData(4, posArr);
	HazardManager::pushRectHazard("vanilla", "horizontal_lightning", constructionData);
	posArr = new double[4]{ GAME_WIDTH/2 - 40, GAME_HEIGHT/2 + 40, 40*2, 20 };
	constructionData = GenericFactoryConstructionData(4, posArr);
	HazardManager::pushRectHazard("vanilla", "horizontal_lightning", constructionData);
	posArr = new double[4]{ GAME_WIDTH/2 - 40 - 20, GAME_HEIGHT/2 - 40, 20, 40*2 };
	constructionData = GenericFactoryConstructionData(4, posArr);
	HazardManager::pushRectHazard("vanilla", "vertical_lightning", constructionData);
	posArr = new double[4]{ GAME_WIDTH/2 + 40, GAME_HEIGHT/2 - 40, 20, 40*2 };
	constructionData = GenericFactoryConstructionData(4, posArr);
	HazardManager::pushRectHazard("vanilla", "vertical_lightning", constructionData);

	posArr = new double[4]{ wallArray[0].first+32, 0, wallArray[3].first - (wallArray[0].first+32), 16 };
	constructionData = GenericFactoryConstructionData(4, posArr);
	HazardManager::pushRectHazard("vanilla", "lava", constructionData);
	posArr = new double[4]{ wallArray[0].first+32, GAME_HEIGHT - 16, wallArray[3].first - (wallArray[0].first+32), 16 };
	constructionData = GenericFactoryConstructionData(4, posArr);
	HazardManager::pushRectHazard("vanilla", "lava", constructionData);

	//traps:
	//LevelHelper::pushSymmetricPowerups_Corners(GAME_WIDTH/2, GAME_HEIGHT/2, 60-20/2, 60+20/2, "vanilla", "speed");
	//LevelHelper::pushSymmetricPowerups_Corners(GAME_WIDTH/2, GAME_HEIGHT/2, 60+20/2, 60-20/2, "vanilla", "speed");

	//LevelHelper::pushSymmetricPowerups_Corners(GAME_WIDTH/2, GAME_HEIGHT/2, 80+20/2, 80+20/2, "vanilla", "banana");
	//LevelHelper::pushSymmetricPowerups_Corners(GAME_WIDTH/2, GAME_HEIGHT/2, 16/2, 16/2, "vanilla", "speed"); //not trap

	//[insert LightningCorner's center powerup here] (JS has it in its comments)

	//names = new std::string[3]{ "big", "blast", "banana" }; //JS
	names = new std::string[3]{ "big", "banana", "banana" };
	PowerupManager::pushPowerup(new PowerSquare(GAME_WIDTH/2, GAME_HEIGHT/2, "vanilla", names, 3));
	delete[] names;
	//in JS, blast's bullet acceleration did not affect banana
	//TODO: power constructors take arguments, increase second banana's bananaCount to 16

	//did this level come first, or did LightningCorners? probably LightningCorners

	LevelHelper::pushSymmetricPowerups_Corners(GAME_WIDTH/2, GAME_HEIGHT/2, 16, 16, "vanilla", "invincible");

	types = new std::string[2]{ "vanilla", "vanilla-extra" };
	names = new std::string[2]{ "speed", "barrier" };
	LevelHelper::pushSymmetricPowerups_Corners(GAME_WIDTH/2, GAME_HEIGHT/2, (wallArray[3].first - (wallArray[0].first+32))/2 - 16, GAME_HEIGHT/2 - (16+16), types, names, 2);
	delete[] types; delete[] names;

	//not here in JS:
	//LevelHelper::pushSymmetricPowerups_UD(GAME_WIDTH/2, GAME_HEIGHT/2, 80-20/2, "dev", "inversion");

	delete[] wallArray;
}

Level* TrickyManeuveringLevel::factory() {
	return new TrickyManeuveringLevel();
}

TrickyManeuveringLevel::TrickyManeuveringLevel() {
	bool* temp = new bool[1]{ true };
	GenericFactoryConstructionData constructionData(1, temp);
	effects.push_back(LevelManager::makeLevelEffect("vanilla", "wind", constructionData));
}
