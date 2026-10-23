#include "terrifying-chaos-level.h"

#include "../constants.h"
#include <cmath> //floor
#include <iostream>
#include "../rng.h"

//#include "../reset-things.h"
#include "../level-helper.h"
#include "../powerup-manager.h"
#include "../wall-manager.h"
#include "../hazard-manager.h"
#include "../level-manager.h"
#include "../tank-manager.h"

ColorValueHolder TerrifyingChaosLevel::getDefaultColor() const {
	return ColorValueHolder(0x00/255.0, 0xA6/255.0, 0x53/255.0);
}

std::unordered_map<std::string, float> TerrifyingChaosLevel::getWeights() const {
	std::unordered_map<std::string, float> weights;
	weights.insert({ "vanilla-extra", 0.25f });
	weights.insert({ "random-vanilla", 0.25f });
	weights.insert({ "dev", 0.5f });
	weights.insert({ "random-dev", 0.5f });
	//only semi-finished so it shouldn't really be in a regular group... but it's fine
	return weights;
}

void TerrifyingChaosLevel::initialize() {
	TankManager::getTank(0)->cir.x = GAME_WIDTH/2 - 240/4;
	TankManager::getTank(1)->cir.x = GAME_WIDTH/2 + 240/4;
	TankManager::getTank(0)->cir.y = GAME_HEIGHT - 20;
	TankManager::getTank(1)->cir.y = 20;
	TankManager::getTank(0)->velocity.setAngle(-PI/2);
	TankManager::getTank(1)->velocity.setAngle(PI/2);

	ColorValueHolder color = getDefaultColor();
	PositionHolder pos;
	GenericFactoryConstructionData constructionData;
	double* posArr;
	std::string* types;
	std::string* names;

	LevelHelper::pushSymmetricWalls_UD(GAME_WIDTH/2, GAME_HEIGHT/2, GAME_HEIGHT/2 - (40+20), 240, 20, color);

	posArr = new double[3]{ GAME_WIDTH/2, GAME_HEIGHT/2, PI/2 * (std::floor(LevelRNG::randFunc()*2)*2-1) };
	int* childArr = new int[2]{ 32, 16 }; //this is a lot
	constructionData = GenericFactoryConstructionData(3, posArr);
	constructionData.pushData(2, childArr);
	HazardManager::pushCircleHazard("vanilla", "ginormous_turret", constructionData);

	LevelHelper::pushSymmetricPowerups_Corners(GAME_WIDTH/2, GAME_HEIGHT/2, GAME_WIDTH/2 - 20, GAME_HEIGHT/2 - 20, "vanilla-extra", "barrier");

	types = new std::string[2]{ "vanilla", "vanilla" };
	names = new std::string[2]{ "speed", "wallhack" }; //what an ugly color
	LevelHelper::pushSymmetricPowerups_LR(GAME_WIDTH/2, GAME_HEIGHT/2, 128, types, names, 2);
	delete[] types; delete[] names;

	LevelHelper::pushSymmetricPowerups_LR(GAME_WIDTH/2, GAME_HEIGHT/2, GAME_WIDTH/2 - 20, "vanilla", "big");
	LevelHelper::pushSymmetricPowerups_Corners_Alternate(GAME_WIDTH/2, GAME_HEIGHT/2, GAME_WIDTH/2 - 20, 40,
		"vanilla", "multishot", "dev", "triple_spread");

	LevelHelper::pushSymmetricPowerups_Corners_Alternate(GAME_WIDTH/2, GAME_HEIGHT/2, 240/2 - 20/2, GAME_HEIGHT/2 - 40 - 20/2,
		"dev", "edge_sparks", "vanilla", "blast"); //maybe homing instead of edge sparks, though homing is very powerful on this level
}

Level* TerrifyingChaosLevel::factory() {
	return new TerrifyingChaosLevel();
}

TerrifyingChaosLevel::TerrifyingChaosLevel() {
	GenericFactoryConstructionData constructionData;
	effects.push_back(LevelManager::makeLevelEffect("vanilla", "respawning_powerups", constructionData));
}
