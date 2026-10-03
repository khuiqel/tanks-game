#include "untraditional-divider-level.h"

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
#include "../level-effects/magnetism-level-effect.h"

ColorValueHolder UntraditionalDividerLevel::getDefaultColor() const {
	return ColorValueHolder(0x76/255.0, 0xC6/255.0, 0x7D/255.0); //darker and less red shield color
}

std::unordered_map<std::string, float> UntraditionalDividerLevel::getWeights() const {
	std::unordered_map<std::string, float> weights;
	weights.insert({ "vanilla", 0.5f });
	weights.insert({ "random-vanilla", 0.5f });
	weights.insert({ "random", 0.25f });
	return weights;
}

void UntraditionalDividerLevel::initialize() {
	ResetThings::tankPositionReset(ResetThings::default_tankToEdgeDist, GAME_HEIGHT*(3.0/5), 2);

	ColorValueHolder color = getDefaultColor();
	PositionHolder pos;
	GenericFactoryConstructionData constructionData;
	double* posArr;
	std::string* types;
	std::string* names;
	//PowerSquare* p;

	if (effects.size() < 2) [[unlikely]] {
		throw std::logic_error("ERROR: \"" + getName() + "\" level does not have sufficient level effects!");
	}
	RespawningPowerupsLevelEffect* respawning = static_cast<RespawningPowerupsLevelEffect*>(effects[0]);
	if (respawning->getName() != "respawning_powerups") [[unlikely]] {
		throw std::logic_error("ERROR: \"" + getName() + "\" level does not have \"respawning_powerups\" level effect!");
	}
	MagnetismLevelEffect* magnetism = static_cast<MagnetismLevelEffect*>(effects[1]);
	if (magnetism->getName() != "magnetism") [[unlikely]] {
		throw std::logic_error("ERROR: \"" + getName() + "\" level does not have \"magnetism\" level effect!");
	}

	WallManager::pushWall(new Wall(GAME_WIDTH/2 - 240, GAME_HEIGHT/2 - 10, 240*2, 10*2, color));

	for (int i = 0; i < 4; i++) {
		PositionHolder posOuter = LevelHelper::getSymmetricWallPositions_Corners(i, GAME_WIDTH/2, GAME_HEIGHT/2, 240-20,           10+50, 20, GAME_HEIGHT/2 - (10+50));
		posArr = new double[4]{ posOuter.first, posOuter.second, 20, GAME_HEIGHT/2 - (10+50) };
		constructionData = GenericFactoryConstructionData(4, posArr);
		HazardManager::pushRectHazard("vanilla", "lava", constructionData);

		PositionHolder posInner = LevelHelper::getSymmetricWallPositions_Corners(i, GAME_WIDTH/2, GAME_HEIGHT/2, 240-20 - (80+20), 10+50, 20, GAME_HEIGHT/2 - (10+50));
		posArr = new double[4]{ posInner.first, posInner.second, 20, GAME_HEIGHT/2 - (10+50) };
		constructionData = GenericFactoryConstructionData(4, posArr);
		HazardManager::pushRectHazard("vanilla", "lava", constructionData);
	}

	types = new std::string[2]{ "vanilla", "vanilla" };
	names = new std::string[2]{ "invincible", "speed" }; //TODO: this probably needs two speed
	LevelHelper::pushSymmetricPowerups_Corners(GAME_WIDTH/2, GAME_HEIGHT/2, 240-20 - 80/2, GAME_HEIGHT/2 - 20, types, names, 2);
	delete[] types; delete[] names;
	respawning->watchLastPowerSquaresPushed(4, 1000);
	magnetism->watchLastPowerSquaresPushed(4);

	const int turretCount = 3;
	for (int j = 0; j < turretCount; j++) {
		for (int i = 0; i < 4; i++) {
			pos = LevelHelper::getSymmetricPowerupPositions_Corners(i, GAME_WIDTH/2, GAME_HEIGHT/2, 240-20 - (80+20) + 20/2 + .01, (10+50) + j*(GAME_HEIGHT/2 - (10+50) - 20)/(turretCount-1) + 20/2);
			posArr = new double[3]{ pos.first, pos.second, PI * (i%2) };
			constructionData = GenericFactoryConstructionData(3, posArr);
			HazardManager::pushCircleHazard("vanilla", "stationary_turret", constructionData);
			//TODO: the bullets don't collide with each other because they don't actually hit each other; maybe have a better solution than adjust positioning by .01?
		}
	}

	//LevelHelper::pushSymmetricPowerups_Corners_Alternate(GAME_WIDTH/2, GAME_HEIGHT/2 + (10+50) + (GAME_HEIGHT/2 - (10+50))/2, 40, 40,
	//	"vanilla", "multishot", "vanilla", "fire");
	//LevelHelper::pushSymmetricPowerups_Corners_Alternate(GAME_WIDTH/2, GAME_HEIGHT/2 - (10+50) - (GAME_HEIGHT/2 - (10+50))/2, 40, 40,
	//	"vanilla", "multishot", "vanilla", "fire");
	LevelHelper::pushSymmetricPowerups_Corners_Alternate(GAME_WIDTH/2, GAME_HEIGHT/2, 80, (10+50) + (GAME_HEIGHT/2 - (10+50))/2 + 40,
		"vanilla-extra", "barrier", "vanilla", "triple");
	LevelHelper::pushSymmetricPowerups_Corners_Alternate(GAME_WIDTH/2, GAME_HEIGHT/2, 80, (10+50) + (GAME_HEIGHT/2 - (10+50))/2 - 40,
		"vanilla", "multishot", "vanilla", "fire");

	types = new std::string[2]{ "vanilla", "vanilla" };
	names = new std::string[2]{ "homing", "wallhack" };
	LevelHelper::pushSymmetricPowerups_UD(GAME_WIDTH/2, GAME_HEIGHT/2, (10+50) + (GAME_HEIGHT/2 - (10+50))/2, types, names, 2);
	delete[] types; delete[] names;
	respawning->watchLastPowerSquaresPushed(2);

	posArr = new double[3]{ GAME_WIDTH/2, GAME_HEIGHT/2, 24 };
	constructionData = GenericFactoryConstructionData(3, posArr);
	HazardManager::pushCircleHazard("vanilla", "no_bullet_zone", constructionData); //excluding random levels, this is currently the only circular no bullet zone in the game...

	//screw it, put down some circular lightnings (because these will currently be the only ones in the game)
	posArr = new double[3]{ GAME_WIDTH/2, 0, 16 };
	constructionData = GenericFactoryConstructionData(3, posArr);
	HazardManager::pushCircleHazard("vanilla", "lightning", constructionData);
	posArr = new double[3]{ GAME_WIDTH/2, GAME_HEIGHT, 16 };
	constructionData = GenericFactoryConstructionData(3, posArr);
	HazardManager::pushCircleHazard("vanilla", "lightning", constructionData);

	//alternate screw it:
	/*
	for (int i = 0; i < 4; i++) {
		pos = LevelHelper::getSymmetricPowerupPositions_Corners(i, GAME_WIDTH/2, GAME_HEIGHT/2, 240-20 - 80/2, GAME_HEIGHT/2 - 20);
		posArr = new double[3]{ pos.first, pos.second, 16 };
		constructionData = GenericFactoryConstructionData(3, posArr);
		HazardManager::pushCircleHazard("vanilla", "lightning", constructionData);
	}
	*/
}

Level* UntraditionalDividerLevel::factory() {
	return new UntraditionalDividerLevel();
}

UntraditionalDividerLevel::UntraditionalDividerLevel() {
	bool* temp1 = new bool[1]{ false };
	GenericFactoryConstructionData constructionData(1, temp1);
	effects.push_back(LevelManager::makeLevelEffect("vanilla", "respawning_powerups", constructionData));
	bool* temp2 = new bool[1]{ false };
	constructionData = GenericFactoryConstructionData(1, temp2);
	effects.push_back(LevelManager::makeLevelEffect("vanilla", "magnetism", constructionData));

	std::shared_ptr<int[]> mineCount = std::shared_ptr<int[]>(new int[2]{ 4, 16 }, GenericFactoryConstructionData::deleteArrFunc);
	for (int i = 0; i < 4; i++) {
		PositionHolder posOuter = LevelHelper::getSymmetricWallPositions_Corners(i, GAME_WIDTH/2, GAME_HEIGHT/2, 240-20,           10+50, 20, GAME_HEIGHT/2 - (10+50));
		PositionHolder posInner = LevelHelper::getSymmetricWallPositions_Corners(i, GAME_WIDTH/2, GAME_HEIGHT/2, 240-20 - (80+20), 10+50, 20, GAME_HEIGHT/2 - (10+50));

		double* minefieldData = new double[4]{ (posOuter.first < posInner.first ? posOuter.first : posInner.first) + 20, posOuter.second, 80, GAME_HEIGHT/2 - (10+50) };
		GenericFactoryConstructionData constructionData(4, minefieldData);
		constructionData.pushData(2, mineCount);
		effects.push_back(LevelManager::makeLevelEffect("vanilla", "minefield", constructionData));
	}
}
