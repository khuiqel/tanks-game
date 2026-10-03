#include "developer-level-3.h"

#include "../constants.h"

#include "../reset-things.h"
#include "../level-helper.h"
#include "../powerup-manager.h"
#include "../wall-manager.h"
#include "../hazard-manager.h"

ColorValueHolder DeveloperLevel3::getDefaultColor() const {
	return ColorValueHolder(0.25f, 0.25f, 0.25f);
}

std::unordered_map<std::string, float> DeveloperLevel3::getWeights() const {
	std::unordered_map<std::string, float> weights;
	weights.insert({ "dev", 1.0f });
	weights.insert({ "random-dev", 1.0f });
	return weights;
}

void DeveloperLevel3::initialize() {
	//primary purpose: stupid powerup showcase with some hazards
	ResetThings::tankPositionReset(ResetThings::default_tankToEdgeDist, GAME_HEIGHT/2);

	ColorValueHolder color = getDefaultColor();
	PositionHolder pos;
	GenericFactoryConstructionData constructionData;
	double* posArr;
	std::string* names;

	//from dev1
	pos = LevelHelper::getSymmetricWallPositions_LR(0, GAME_WIDTH/2, GAME_HEIGHT/2, 60, 20, 80);
	WallManager::pushWall(new Wall(pos.first, pos.second, 20, 80, color));
	pos = LevelHelper::getSymmetricWallPositions_LR(1, GAME_WIDTH/2, GAME_HEIGHT/2, 60, 20, 80);
	WallManager::pushWall(new Wall(pos.first, pos.second, 20, 80, color));

	posArr = new double[4]{ GAME_WIDTH/2 - 80 + 20, GAME_HEIGHT/2 + 10, 60*2, 20*2 - 10 };
	constructionData = GenericFactoryConstructionData(4, posArr);
	HazardManager::pushRectHazard("vanilla", "lava", constructionData);
	posArr = new double[3]{ GAME_WIDTH/2, GAME_HEIGHT/2 + 100, 40 };
	constructionData = GenericFactoryConstructionData(3, posArr);
	HazardManager::pushCircleHazard("vanilla", "lightning", constructionData);
	posArr = new double[3]{ GAME_WIDTH/2, GAME_HEIGHT/2, -PI/2 };
	constructionData = GenericFactoryConstructionData(3, posArr);
	HazardManager::pushCircleHazard("vanilla", "mother_turret", constructionData);

	//not from dev1
	LevelHelper::pushSymmetricPowerups_LR(GAME_WIDTH/2, GAME_HEIGHT/2, 60+20+40+10, "dev", "backwards_movement");

	posArr = new double[3]{ GAME_WIDTH/2, GAME_HEIGHT/2 + 10+30/2, PI/2 };
	int* patrolCount = new int[1]{ 4 };
	double* patrolRoute = new double[4*2]{
		GAME_WIDTH/2 - 40, GAME_HEIGHT/2 + 10+30/2,
		GAME_WIDTH/2 - 40, GAME_HEIGHT/2 - 10-30/2,
		GAME_WIDTH/2 + 40, GAME_HEIGHT/2 - 10-30/2,
		GAME_WIDTH/2 + 40, GAME_HEIGHT/2 + 10+30/2
	};
	constructionData = GenericFactoryConstructionData(3, posArr);
	constructionData.pushData(1, patrolCount);
	constructionData.pushData(4*2, patrolRoute);
	HazardManager::pushCircleHazard("vanilla", "patrolling_turret", constructionData);

	pos = LevelHelper::getSymmetricWallPositions_LR(0, GAME_WIDTH/2, GAME_HEIGHT/2, 60, 20, 80);
	posArr = new double[4]{ pos.first - 40, pos.second, 20, 80 };
	constructionData = GenericFactoryConstructionData(4, posArr);
	HazardManager::pushRectHazard("dev", "wall", constructionData);
	pos = LevelHelper::getSymmetricWallPositions_LR(1, GAME_WIDTH/2, GAME_HEIGHT/2, 60, 20, 80);
	posArr = new double[4]{ pos.first + 40, pos.second, 20, 80 };
	constructionData = GenericFactoryConstructionData(4, posArr);
	HazardManager::pushRectHazard("dev", "wall", constructionData);

	//from dev2
	PowerupManager::pushPowerup(new PowerSquare(20, 20, "vanilla", "speed"));
	PowerupManager::pushPowerup(new PowerSquare(40, 20, "vanilla", "wallhack"));
	PowerupManager::pushPowerup(new PowerSquare(60, 20, "vanilla", "bounce"));
	PowerupManager::pushPowerup(new PowerSquare(80, 20, "vanilla", "multishot"));
	PowerupManager::pushPowerup(new PowerSquare(100, 20, "vanilla", "triple"));
	PowerupManager::pushPowerup(new PowerSquare(120, 20, "vanilla", "homing"));
	PowerupManager::pushPowerup(new PowerSquare(140, 20, "vanilla", "invincible"));
	PowerupManager::pushPowerup(new PowerSquare(160, 20, "vanilla", "grenade"));
	PowerupManager::pushPowerup(new PowerSquare(180, 20, "vanilla", "fire"));
	PowerupManager::pushPowerup(new PowerSquare(200, 20, "vanilla", "blast"));
	PowerupManager::pushPowerup(new PowerSquare(220, 20, "vanilla", "banana"));
	PowerupManager::pushPowerup(new PowerSquare(240, 20, "vanilla", "big"));
	PowerupManager::pushPowerup(new PowerSquare(260, 20, "vanilla", "megadeath"));

	PowerupManager::pushPowerup(new PowerSquare(GAME_WIDTH-20, 20, "vanilla-extra", "mines"));
	PowerupManager::pushPowerup(new PowerSquare(GAME_WIDTH-40, 20, "vanilla-extra", "shotgun"));
	PowerupManager::pushPowerup(new PowerSquare(GAME_WIDTH-60, 20, "vanilla-extra", "tracking"));
	PowerupManager::pushPowerup(new PowerSquare(GAME_WIDTH-80, 20, "vanilla-extra", "barrier"));
	PowerupManager::pushPowerup(new PowerSquare(GAME_WIDTH-100, 20, "vanilla-extra", "shield"));

	//PowerupManager::pushPowerup(new PowerSquare(GAME_WIDTH/2, GAME_HEIGHT/2, "vanilla", "godmode"));

	PowerupManager::pushPowerup(new PowerSquare(20, GAME_HEIGHT-20, "vanilla", "speed"));
	PowerupManager::pushPowerup(new PowerSquare(40, GAME_HEIGHT-20, "vanilla", "speed"));
	PowerupManager::pushPowerup(new PowerSquare(60, GAME_HEIGHT-20, "vanilla", "speed"));
	PowerupManager::pushPowerup(new PowerSquare(100, GAME_HEIGHT-20, "dev", "inversion"));
	PowerupManager::pushPowerup(new PowerSquare(120, GAME_HEIGHT-20, "dev", "annoying"));
	PowerupManager::pushPowerup(new PowerSquare(140, GAME_HEIGHT-20, "dev", "other_stuff_is_poison"));

	names = new std::string[2]{ "multishot", "multishot" };
	PowerupManager::pushPowerup(new PowerSquare(GAME_WIDTH-20, GAME_HEIGHT-20, "vanilla", names, 2));
	names[0] = "bounce", names[1] = "homing";
	PowerupManager::pushPowerup(new PowerSquare(GAME_WIDTH-60, GAME_HEIGHT-20, "vanilla", names, 2));
	names[0] = "banana", names[1] = "banana";
	PowerupManager::pushPowerup(new PowerSquare(GAME_WIDTH-120, GAME_HEIGHT-20, "vanilla", names, 2));
	delete[] names;
}

Level* DeveloperLevel3::factory() {
	return new DeveloperLevel3();
}

DeveloperLevel3::DeveloperLevel3() { return; }
