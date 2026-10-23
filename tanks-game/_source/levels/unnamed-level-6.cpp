#include "unnamed-level-6.h"

#include "../constants.h"
#include <iostream>

#include "../reset-things.h"
#include "../level-helper.h"
#include "../powerup-manager.h"
#include "../wall-manager.h"
#include "../hazard-manager.h"
//#include "../level-manager.h"
#include "../tank-manager.h"

ColorValueHolder UnnamedLevel6::getDefaultColor() const {
	return ColorValueHolder(1, 1, 1);
}

std::unordered_map<std::string, float> UnnamedLevel6::getWeights() const {
	std::unordered_map<std::string, float> weights;
	weights.insert({ "dev", 0.25f });
	weights.insert({ "random-dev", 0.25f });
	return weights;
}

void UnnamedLevel6::initialize() {
	ResetThings::tankPositionReset(ResetThings::default_tankToEdgeDist, GAME_HEIGHT - 2*20, 2);
	if (TankManager::getTank(0)->cir.getY() > GAME_HEIGHT/2) {
		TankManager::getTank(0)->velocity.changeAngle(-PI/2);
		TankManager::getTank(1)->velocity.changeAngle(-PI/2);
	} else {
		TankManager::getTank(0)->velocity.changeAngle(PI/2);
		TankManager::getTank(1)->velocity.changeAngle(PI/2);
	}

	ColorValueHolder color = getDefaultColor();
	PositionHolder pos;
	GenericFactoryConstructionData constructionData;
	double* posArr;

	//LevelHelper::pushSymmetricWalls_Corners(GAME_WIDTH/2, GAME_HEIGHT/2, GAME_WIDTH/2 - 80, GAME_HEIGHT/2 - (40+20), 80, 20, color);
	LevelHelper::pushSymmetricWalls_Corners(GAME_WIDTH/2, GAME_HEIGHT/2, GAME_WIDTH/2 - (40+20), GAME_HEIGHT/2 - 60, 20, 60, color);

	//although I would like to make a real level with two ginormous turrets, they take up like the entire level
	posArr = new double[3]{ GAME_WIDTH/2 - 96, GAME_HEIGHT/2, 0 };
	constructionData = GenericFactoryConstructionData(3, posArr);
	HazardManager::pushCircleHazard("vanilla", "ginormous_turret", constructionData);
	posArr = new double[3]{ GAME_WIDTH/2 + 96, GAME_HEIGHT/2, PI };
	constructionData = GenericFactoryConstructionData(3, posArr);
	HazardManager::pushCircleHazard("vanilla", "ginormous_turret", constructionData);

	LevelHelper::pushSymmetricPowerups_Corners(GAME_WIDTH/2, GAME_HEIGHT/2, GAME_WIDTH/2 - (40+20) - 20, GAME_HEIGHT/2 - 20, "vanilla", "speed");

	LevelHelper::pushSymmetricPowerups_UD(GAME_WIDTH/2, GAME_HEIGHT/2, GAME_HEIGHT/2 - 20, "vanilla", "big");
	PowerupManager::pushPowerup(new PowerSquare(GAME_WIDTH/2, GAME_HEIGHT/2, "vanilla", "godmode"));
}

Level* UnnamedLevel6::factory() {
	return new UnnamedLevel6();
}

UnnamedLevel6::UnnamedLevel6() { return; }
