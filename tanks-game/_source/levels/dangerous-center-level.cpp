#include "dangerous-center-level.h"

#include "../constants.h"
#include <cmath> //floor
#include <stdexcept>
#include <iostream>
#include "../rng.h"

#include "../reset-things.h"
#include "../level-helper.h"
#include "../powerup-manager.h"
#include "../wall-manager.h"
#include "../hazard-manager.h"
#include "../level-manager.h"

#include "../level-effects/respawning-powerups-level-effect.h"

ColorValueHolder DangerousCenterLevel::getDefaultColor() const {
	//return ColorValueHolder(0xE6/255.0, 0xE6/255.0, 0xFA/255.0); //lavender
	return ColorValueHolder(0xE6/255.0, 0x91/255.0, 0xA9/255.0);
}

std::unordered_map<std::string, float> DangerousCenterLevel::getWeights() const {
	std::unordered_map<std::string, float> weights;
	weights.insert({ "vanilla", 1.0f });
	weights.insert({ "random-vanilla", 0.5f }); //TODO: seems too common
	weights.insert({ "random", 0.5f });
	return weights;
}

void DangerousCenterLevel::initialize() {
	ResetThings::tankPositionReset(ResetThings::default_tankToEdgeDist, (2.0/5) * GAME_HEIGHT, 3);

	ColorValueHolder color = getDefaultColor();
	PositionHolder pos;
	GenericFactoryConstructionData constructionData;
	double* posArr;
	std::string* types;
	std::string* names;
	//PowerSquare* p;

	if (effects.size() < 1) [[unlikely]] {
		throw std::logic_error("ERROR: \"" + getName() + "\" level does not have sufficient level effects!");
	}
	RespawningPowerupsLevelEffect* respawning = static_cast<RespawningPowerupsLevelEffect*>(effects[0]);
	if (respawning->getName() != "respawning_powerups") [[unlikely]] {
		throw std::logic_error("ERROR: \"" + getName() + "\" level does not have \"respawning_powerups\" level effect!");
	}

	posArr = new double[3]{ GAME_WIDTH/2, GAME_HEIGHT/2, PI/2 * (std::floor(LevelRNG::randFunc()*2)*2-1) };
	int* childArr = new int[2]{ 16, 8 };
	constructionData = GenericFactoryConstructionData(3, posArr);
	constructionData.pushData(2, childArr);
	HazardManager::pushCircleHazard("vanilla", "mother_turret", constructionData);

	//hiding areas from lone turret level, pushed out
	const double top_gap = TANK_RADIUS*2;
	const double side_length = 60;
	/*
	//equal x and y offsets from center with gap from top
	LevelHelper::pushSymmetricWalls_Corners(GAME_WIDTH/2, GAME_HEIGHT/2, GAME_HEIGHT/2-(top_gap+side_length), GAME_HEIGHT/2-(top_gap+side_length), side_length, 20, color);
	LevelHelper::pushSymmetricWalls_Corners(GAME_WIDTH/2, GAME_HEIGHT/2, GAME_HEIGHT/2-(top_gap+side_length), GAME_HEIGHT/2-(top_gap+side_length), 20, side_length, color);
	//x offset from center equals y offset from edge
	LevelHelper::pushSymmetricWalls_Corners(GAME_WIDTH/2, GAME_HEIGHT/2, GAME_WIDTH/2-(top_gap+side_length), GAME_HEIGHT/2-(top_gap+side_length), side_length, 20, color);
	LevelHelper::pushSymmetricWalls_Corners(GAME_WIDTH/2, GAME_HEIGHT/2, GAME_WIDTH/2-(top_gap+side_length), GAME_HEIGHT/2-(top_gap+side_length), 20, side_length, color);
	//middle of the two
	LevelHelper::pushSymmetricWalls_Corners(GAME_WIDTH/2, GAME_HEIGHT/2, (GAME_WIDTH/2+GAME_HEIGHT/2)/2-(top_gap+side_length), GAME_HEIGHT/2-(top_gap+side_length), side_length, 20, color);
	LevelHelper::pushSymmetricWalls_Corners(GAME_WIDTH/2, GAME_HEIGHT/2, (GAME_WIDTH/2+GAME_HEIGHT/2)/2-(top_gap+side_length), GAME_HEIGHT/2-(top_gap+side_length), 20, side_length, color);
	*/
	//just use a custom distance
	LevelHelper::pushSymmetricWalls_Corners(GAME_WIDTH/2, GAME_HEIGHT/2, (GAME_HEIGHT/2+40)-(top_gap+side_length)+20, GAME_HEIGHT/2-(top_gap+side_length), side_length-20, 20, color);
	LevelHelper::pushSymmetricWalls_Corners(GAME_WIDTH/2, GAME_HEIGHT/2, (GAME_HEIGHT/2+40)-(top_gap+side_length), GAME_HEIGHT/2-(top_gap+side_length), 20, side_length, color);

	for (int i = 0; i < 4; i++) {
		pos = LevelHelper::getSymmetricWallPositions_Corners(i, GAME_WIDTH/2, GAME_HEIGHT/2, (GAME_HEIGHT/2+40)-(top_gap+side_length), GAME_HEIGHT/2-(top_gap), 20, top_gap);
		posArr = new double[4]{ pos.first, pos.second, 20, top_gap };
		constructionData = GenericFactoryConstructionData(4, posArr);
		HazardManager::pushRectHazard("vanilla", "vertical_lightning", constructionData);
	}

	//top/bottom walls
	LevelHelper::pushSymmetricWalls_UD(GAME_WIDTH/2, GAME_HEIGHT/2, GAME_HEIGHT/2-(top_gap+10), 60, 10, color);
	//little hooks
	LevelHelper::pushSymmetricWalls_Corners(GAME_WIDTH/2, GAME_HEIGHT/2, 60/2-10, GAME_HEIGHT/2-(top_gap+10)-(10), 10, 10, color);

	//starting walls
	LevelHelper::pushSymmetricWalls_LR(GAME_WIDTH/2, GAME_HEIGHT/2, GAME_WIDTH/2-(40+20), 20, GAME_HEIGHT-(top_gap+side_length-20)*2, color);

	//pocket for a power
	LevelHelper::pushSymmetricWalls_Corners(GAME_WIDTH/2, GAME_HEIGHT/2, GAME_WIDTH/2-(40+20)-(40), 20, 40, 10, color);

	//pocket powers:
	LevelHelper::pushSymmetricPowerups_LR(GAME_WIDTH/2, GAME_HEIGHT/2, GAME_WIDTH/2-(40+20)-(40)/2, "vanilla", "big");
	respawning->watchLastPowerSquaresPushed(2);

	//top/bottom powers
	types = new std::string[2]{ "vanilla", "vanilla" };
	names = new std::string[2]{ "multishot", "bounce" };
	LevelHelper::pushSymmetricPowerups_UD(GAME_WIDTH/2, GAME_HEIGHT/2, GAME_HEIGHT/2-(top_gap+10)-(10), types, names, 2);
	delete[] types; delete[] names;

	//concealed powers:
	types = new std::string[2]{ "vanilla", "vanilla" };
	names = new std::string[2]{ "grenade", "triple" };
	LevelHelper::pushSymmetricPowerups_Corners(GAME_WIDTH/2, GAME_HEIGHT/2, (GAME_HEIGHT/2+40)-(top_gap+side_length)+(side_length/2), GAME_HEIGHT/2-(top_gap+side_length)+(side_length/2), types, names, 2);
	//TODO: should these be switched with the invincible powers?
	delete[] types; delete[] names;

	//not concealed powers:
	LevelHelper::pushSymmetricPowerups_Corners_Alternate(GAME_WIDTH/2, GAME_HEIGHT/2, (GAME_HEIGHT/2+40)-(top_gap+side_length)-(10), GAME_HEIGHT/2-(top_gap+side_length)-(10),
		"vanilla", "fire", "vanilla", "invincible");
}

Level* DangerousCenterLevel::factory() {
	return new DangerousCenterLevel();
}

DangerousCenterLevel::DangerousCenterLevel() {
	bool* temp = new bool[1]{ false };
	GenericFactoryConstructionData constructionData(1, temp);
	effects.push_back(LevelManager::makeLevelEffect("vanilla", "respawning_powerups", constructionData));
}
