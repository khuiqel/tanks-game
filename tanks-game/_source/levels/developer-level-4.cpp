#include "developer-level-4.h"

#include "../constants.h"

#include "../reset-things.h"
#include "../level-helper.h"
#include "../powerup-manager.h"
#include "../hazard-manager.h"

ColorValueHolder DeveloperLevel4::getDefaultColor() const {
	//there are no walls in this level...
	return ColorValueHolder(0.25f, 0.25f, 0.25f);
}

std::unordered_map<std::string, float> DeveloperLevel4::getWeights() const {
	std::unordered_map<std::string, float> weights;
	weights.insert({ "dev", 1.0f });
	weights.insert({ "random-dev", 1.0f });
	return weights;
}

void DeveloperLevel4::initialize() {
	//primary purpose: an empty-ish level, just to have a lot of free space
	ResetThings::tankPositionReset(ResetThings::default_tankToEdgeDist, GAME_HEIGHT/2);

	PositionHolder pos;
	GenericFactoryConstructionData constructionData;
	double* posArr;
	std::string* names;

	const double tempWidth = 16;
	posArr = new double[3]{ GAME_WIDTH/2 - tempWidth/2, GAME_HEIGHT/2 - tempWidth/2, tempWidth };
	constructionData = GenericFactoryConstructionData(3, posArr);
	HazardManager::pushRectHazard("vanilla-extra", "spiral_lava", constructionData);

	for (int i = 0; i < 4; i++) {
		pos = LevelHelper::getSymmetricWallPositions_Corners(i, GAME_WIDTH/2, GAME_HEIGHT/2, 60, 60, 30, 30);
		posArr = new double[4]{ pos.first, pos.second, 30, 30 };
		constructionData = GenericFactoryConstructionData(4, posArr);
		HazardManager::pushRectHazard("dev", "reflecktor", constructionData);
	}

	posArr = new double[3]{ GAME_WIDTH/2 - (60+40/2), GAME_HEIGHT/2, 20 };
	double* speedArr = new double[2]{ 1.0, -1.0/100 };
	std::vector<std::vector<std::string>>* powerList = new std::vector<std::vector<std::string>>{ {"vanilla", "speed"}, {"vanilla", "speed", "vanilla", "speed"} };
	//technically this ^^^ will call delete[] on a new but the destructor throws an error at the delete[] if it's a new[1]
	constructionData = GenericFactoryConstructionData(3, posArr);
	constructionData.pushData(2, speedArr);
	constructionData.pushData(1, powerList);
	HazardManager::pushCircleHazard("dev", "cloud", constructionData);

	posArr = new double[3]{ GAME_WIDTH/2 + (60+40/2), GAME_HEIGHT/2, 20 };
	speedArr = new double[2]{ 1.0, -1.0/100 };
	powerList = new std::vector<std::vector<std::string>>{ {"dev", "inversion"}, {"dev", "ring_shooter", "dev", "ultrabounce"}, {"old", "old_bounce", "vanilla-extra", "shield", "dev", "annoying"} };
	constructionData = GenericFactoryConstructionData(3, posArr);
	constructionData.pushData(2, speedArr);
	constructionData.pushData(1, powerList);
	HazardManager::pushCircleHazard("dev", "cloud", constructionData);

	posArr = new double[4]{ GAME_WIDTH/2, GAME_HEIGHT/2 - 80, 20, 60 };
	constructionData = GenericFactoryConstructionData(4, posArr);
	HazardManager::pushCircleHazard("vanilla-extra", "gravity_well", constructionData);

	posArr = new double[4]{ GAME_WIDTH/2, GAME_HEIGHT/2 + 80, 10, 40 };
	double* strengthArr = new double[2]{ -.25, -.95 };
	constructionData = GenericFactoryConstructionData(4, posArr);
	constructionData.pushData(2, strengthArr);
	HazardManager::pushCircleHazard("vanilla-extra", "gravity_well", constructionData);

	//assumption: TANK_RADIUS=16 (why it would ever be changed is beyond me)
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
	PowerupManager::pushPowerup(new PowerSquare(GAME_WIDTH-120, 20, "vanilla-extra", "swarm"));

	PowerupManager::pushPowerup(new PowerSquare(20, GAME_HEIGHT-20, "dev", "inversion"));
	PowerupManager::pushPowerup(new PowerSquare(40, GAME_HEIGHT-20, "dev", "annoying"));
	PowerupManager::pushPowerup(new PowerSquare(60, GAME_HEIGHT-20, "dev", "ultrabounce"));
	PowerupManager::pushPowerup(new PowerSquare(80, GAME_HEIGHT-20, "dev", "wall_sparks"));
	PowerupManager::pushPowerup(new PowerSquare(100, GAME_HEIGHT-20, "dev", "edge_sparks"));
	PowerupManager::pushPowerup(new PowerSquare(120, GAME_HEIGHT-20, "dev", "triple_spread"));
	PowerupManager::pushPowerup(new PowerSquare(140, GAME_HEIGHT-20, "dev", "ring_shooter"));
	PowerupManager::pushPowerup(new PowerSquare(160, GAME_HEIGHT-20, "dev", "banana_split"));
	PowerupManager::pushPowerup(new PowerSquare(180, GAME_HEIGHT-20, "dev", "trickster_circle"));
	PowerupManager::pushPowerup(new PowerSquare(200, GAME_HEIGHT-20, "dev", "trickster_snake"));
	PowerupManager::pushPowerup(new PowerSquare(220, GAME_HEIGHT-20, "dev", "pusher"));
	PowerupManager::pushPowerup(new PowerSquare(240, GAME_HEIGHT-20, "dev", "movement_restarter"));
	PowerupManager::pushPowerup(new PowerSquare(260, GAME_HEIGHT-20, "dev", "mine_layer"));

	names = new std::string[2]{ "multishot", "multishot" };
	PowerupManager::pushPowerup(new PowerSquare(GAME_WIDTH-20, GAME_HEIGHT-20, "vanilla", names, 2));
	names[0] = "speed", names[1] = "wallhack";
	PowerupManager::pushPowerup(new PowerSquare(GAME_WIDTH-40, GAME_HEIGHT-20, "vanilla", names, 2));
	names[0] = "bounce", names[1] = "homing";
	PowerupManager::pushPowerup(new PowerSquare(GAME_WIDTH-60, GAME_HEIGHT-20, "vanilla", names, 2));
	names[0] = "bounce", names[1] = "big";
	PowerupManager::pushPowerup(new PowerSquare(GAME_WIDTH-80, GAME_HEIGHT-20, "vanilla", names, 2));
	names[0] = "wallhack", names[1] = "grenade";
	PowerupManager::pushPowerup(new PowerSquare(GAME_WIDTH-100, GAME_HEIGHT-20, "vanilla", names, 2));
	names[0] = "banana", names[1] = "banana";
	PowerupManager::pushPowerup(new PowerSquare(GAME_WIDTH-120, GAME_HEIGHT-20, "vanilla", names, 2));
	delete[] names;
}

Level* DeveloperLevel4::factory() {
	return new DeveloperLevel4();
}

DeveloperLevel4::DeveloperLevel4() { return; }
