#include "unnamed-level-12.h"

#include "../constants.h"
#include <iostream>

#include "../reset-things.h"
#include "../level-helper.h"
#include "../powerup-manager.h"
#include "../wall-manager.h"
#include "../hazard-manager.h"
//#include "../level-manager.h"
#include "../tank-manager.h"

ColorValueHolder UnnamedLevel12::getDefaultColor() const {
	return ColorValueHolder(1, 1, 1);
}

std::unordered_map<std::string, float> UnnamedLevel12::getWeights() const {
	std::unordered_map<std::string, float> weights;
	weights.insert({ "dev", 0.125f });
	weights.insert({ "random-dev", 0.125f });
	//very boring
	return weights;
}

void UnnamedLevel12::initialize() {
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

	LevelHelper::pushSymmetricWalls_Corners(GAME_WIDTH/2, GAME_HEIGHT/2, GAME_WIDTH/2 - (40+20), GAME_HEIGHT/2 - 60, 20, 60, color);

	posArr = new double[3]{ GAME_WIDTH/2, GAME_HEIGHT/2 - 72, PI/2 };
	constructionData = GenericFactoryConstructionData(3, posArr);
	HazardManager::pushCircleHazard("vanilla", "mother_turret", constructionData);
	posArr = new double[3]{ GAME_WIDTH/2, GAME_HEIGHT/2 + 72, -PI/2 };
	constructionData = GenericFactoryConstructionData(3, posArr);
	HazardManager::pushCircleHazard("vanilla", "mother_turret", constructionData);

	LevelHelper::pushSymmetricPowerups_Corners(GAME_WIDTH/2, GAME_HEIGHT/2, GAME_WIDTH/2 - (40+20) - 20, GAME_HEIGHT/2 - 20, "vanilla", "speed");

	LevelHelper::pushSymmetricPowerups_UD(GAME_WIDTH/2, GAME_HEIGHT/2, GAME_HEIGHT/2 - 20, "vanilla", "big");
	PowerupManager::pushPowerup(new PowerSquare(GAME_WIDTH/2, GAME_HEIGHT/2, "vanilla", "godmode"));
}

Level* UnnamedLevel12::factory() {
	return new UnnamedLevel12();
}

UnnamedLevel12::UnnamedLevel12() { return; }
