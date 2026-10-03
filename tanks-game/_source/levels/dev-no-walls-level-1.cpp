#include "dev-no-walls-level-1.h"

#include "../constants.h"

#include "../reset-things.h"
#include "../level-helper.h"
#include "../powerup-manager.h"
#include "../hazard-manager.h"
#include "../level-manager.h"

ColorValueHolder DevNoWallsLevel1::getDefaultColor() const {
	//no walls...
	return ColorValueHolder(0.0f, 0.0f, 0.0f);
}

std::unordered_map<std::string, float> DevNoWallsLevel1::getWeights() const {
	std::unordered_map<std::string, float> weights;
	weights.insert({ "dev", 0.5f });
	weights.insert({ "random-dev", 0.5f });
	return weights;
}

void DevNoWallsLevel1::initialize() {
	ResetThings::tankPositionReset(40);

	PositionHolder pos;
	GenericFactoryConstructionData constructionData;
	double* posArr;

	const PositionHolder* wallArray = LevelHelper::getClassicWallPositions();
	for (int i = 0; i < 4; i++) {
		//classic JS walls, except NoBulletZones
		posArr = new double[4]{ wallArray[i].first, wallArray[i].second, 32, 128 };
		constructionData = GenericFactoryConstructionData(4, posArr);
		HazardManager::pushRectHazard("vanilla", "no_bullet_zone", constructionData);
	}

	const double distFromCorner = 40;
	LevelHelper::pushSymmetricPowerups_Corners(GAME_WIDTH/2, GAME_HEIGHT/2, GAME_WIDTH/2 - ((wallArray[0].first+32) + distFromCorner), GAME_HEIGHT/2 - distFromCorner, "vanilla-extra", "shield");

	PowerupManager::pushPowerup(new PowerSquare(GAME_WIDTH/2, GAME_HEIGHT/2, "dev", "inversion")); //this is evil (allowed because it's a dev level)
	PowerupManager::pushPowerup(new PowerSquare(GAME_WIDTH/2, GAME_HEIGHT/2, "vanilla", "wallhack")); //heh

	//other potential powers:
	//triple, grenade, fire, mines, tracking?
	//other potential hazards:
	//none

	delete[] wallArray;
}

Level* DevNoWallsLevel1::factory() {
	return new DevNoWallsLevel1();
}

DevNoWallsLevel1::DevNoWallsLevel1() {
	GenericFactoryConstructionData constructionData;
	effects.push_back(LevelManager::makeLevelEffect("vanilla", "respawning_powerups", constructionData));
	//should probably only affect the shield powerups, not everything, but whatever
}
