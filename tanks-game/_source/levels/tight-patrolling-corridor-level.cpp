#include "tight-patrolling-corridor-level.h"

#include "../constants.h"
#include "../rng.h"

#include "../reset-things.h"
#include "../level-helper.h"
#include "../wall-manager.h"
#include "../powerup-manager.h"
#include "../hazard-manager.h"

ColorValueHolder TightPatrollingCorridorLevel::getDefaultColor() const {
	//return ColorValueHolder(15.0f/16, 15.0f/16, 12.0f/16); //very icky yellow (at least it's unique; it just looks a little disgusting)
	return ColorValueHolder(15.0f/16, 15.0f/16, 14.0f/16); //very white beige
	//the color idea was beige but very white 'cuz that's vaguely like the white hallways in spy movies
	//(is that a valid idea? I'm not really sure how spy movies should look)
}

std::unordered_map<std::string, float> TightPatrollingCorridorLevel::getWeights() const {
	std::unordered_map<std::string, float> weights;
	weights.insert({ "vanilla-extra", 0.5f });
	weights.insert({ "random-vanilla", 0.5f }); //TODO: seems too common
	return weights;
}

void TightPatrollingCorridorLevel::initialize() {
	ResetThings::tankPositionReset((GAME_WIDTH/2 - ((60+20 + 40) + 80 + 20))/2, (2.0/5) * GAME_HEIGHT, 3);
	//starting x determined by outer walls (see below)

	ColorValueHolder color = getDefaultColor();
	//int tempRand;
	PositionHolder pos;
	GenericFactoryConstructionData constructionData;
	double* posArr;
	std::string* names;
	std::shared_ptr<int[]> patrolCount;
	std::shared_ptr<double[]> patrolRoute1;
	std::shared_ptr<double[]> patrolRoute2;

	//LevelHelper::pushClassicWalls(color);

	posArr = new double[3]{ GAME_WIDTH/2 - 40, GAME_HEIGHT/2, 0 };
	patrolCount = std::shared_ptr<int[]>(new int[1]{ 2 }, GenericFactoryConstructionData::deleteArrFunc);
	patrolRoute1 = std::shared_ptr<double[]>(new double[2*2]{
		GAME_WIDTH/2 - 40, GAME_HEIGHT/2 + 40,
		GAME_WIDTH/2 - 40, GAME_HEIGHT/2 - 40
	}, GenericFactoryConstructionData::deleteArrFunc);
	patrolRoute2 = std::shared_ptr<double[]>(new double[2*2]{
		GAME_WIDTH/2 + 40, GAME_HEIGHT/2 - 40,
		GAME_WIDTH/2 + 40, GAME_HEIGHT/2 + 40
	}, GenericFactoryConstructionData::deleteArrFunc);
	constructionData = GenericFactoryConstructionData(3, posArr);
	constructionData.pushData(1, patrolCount);
	constructionData.pushData(2*2, patrolRoute1);
	HazardManager::pushCircleHazard("vanilla", "patrolling_turret", constructionData);
	posArr = new double[3]{ GAME_WIDTH/2 + 40, GAME_HEIGHT/2, PI };
	constructionData = GenericFactoryConstructionData(3, posArr);
	constructionData.pushData(1, patrolCount);
	constructionData.pushData(2*2, patrolRoute2);
	HazardManager::pushCircleHazard("vanilla", "patrolling_turret", constructionData);

	//inner walls
	pos = LevelHelper::getSymmetricWallPositions_LR(0, GAME_WIDTH/2, GAME_HEIGHT/2, 60, 20, (40+40)*2);
	WallManager::pushWall(new Wall(pos.first, pos.second, 20, (40+40)*2, color));
	pos = LevelHelper::getSymmetricWallPositions_LR(1, GAME_WIDTH/2, GAME_HEIGHT/2, 60, 20, (40+40)*2);
	WallManager::pushWall(new Wall(pos.first, pos.second, 20, (40+40)*2, color));

	//good center powerup
	names = new std::string[4]{ "bounce", "homing", "banana", "banana" };
	PowerupManager::pushPowerup(new PowerSquare(GAME_WIDTH/2, GAME_HEIGHT/2, "vanilla", names, 4));
	delete[] names;

	//"inner" powers
	for (int i = 0; i < 4; i++) {
		pos = LevelHelper::getSymmetricPowerupPositions_Corners(i, GAME_WIDTH/2, GAME_HEIGHT/2, 60+20+20/2, 40+40/2);
		//PowerupManager::pushPowerup(new PowerSquare(pos.first, pos.second, "vanilla-extra", "shield"));
		PowerupManager::pushPowerup(new PowerSquare(pos.first, pos.second, "vanilla-extra", "barrier"));
	}

	//next layer
	//pos = LevelHelper::getSymmetricWallPositions_UD(0, GAME_WIDTH/2, GAME_HEIGHT/2, (40+30) + 40, (60+20/2 + 30)*2, 20);
	pos = LevelHelper::getSymmetricWallPositions_UD(0, GAME_WIDTH/2, GAME_HEIGHT/2, GAME_HEIGHT/2 - 20, (60+20 + 40)*2, 20);
	WallManager::pushWall(new Wall(pos.first, pos.second, (60+20 + 40)*2, 20, color));
	pos = LevelHelper::getSymmetricWallPositions_UD(1, GAME_WIDTH/2, GAME_HEIGHT/2, GAME_HEIGHT/2 - 20, (60+20 + 40)*2, 20);
	WallManager::pushWall(new Wall(pos.first, pos.second, (60+20 + 40)*2, 20, color));

	for (int i = 0; i < 4; i++) {
		pos = LevelHelper::getSymmetricWallPositions_Corners(i, GAME_WIDTH/2, GAME_HEIGHT/2, (60+20 + 40) + 20 + 20/2, 40-10, 20/2, 40+10*2);
		WallManager::pushWall(new Wall(pos.first, pos.second, 20/2, 40+10*2, color));
		//alternatively: no bullet zones
	}

	posArr = new double[3]{ GAME_WIDTH/2, GAME_HEIGHT - 35, -PI/2 };
	patrolCount = std::shared_ptr<int[]>(new int[1]{ 2 }, GenericFactoryConstructionData::deleteArrFunc);
	patrolRoute1 = std::shared_ptr<double[]>(new double[2*2]{
		GAME_WIDTH/2 - (60+20/2), GAME_HEIGHT - 35,
		GAME_WIDTH/2 + (60+20/2), GAME_HEIGHT - 35
	}, GenericFactoryConstructionData::deleteArrFunc);
	patrolRoute2 = std::shared_ptr<double[]>(new double[2*2]{
		GAME_WIDTH/2 + (60+20/2), 35,
		GAME_WIDTH/2 - (60+20/2), 35
	}, GenericFactoryConstructionData::deleteArrFunc);
	constructionData = GenericFactoryConstructionData(3, posArr);
	constructionData.pushData(1, patrolCount);
	constructionData.pushData(2*2, patrolRoute1);
	HazardManager::pushCircleHazard("vanilla", "patrolling_turret", constructionData);
	posArr = new double[3]{ GAME_WIDTH/2, 35, PI/2 };
	constructionData = GenericFactoryConstructionData(3, posArr);
	constructionData.pushData(1, patrolCount);
	constructionData.pushData(2*2, patrolRoute2);
	HazardManager::pushCircleHazard("vanilla", "patrolling_turret", constructionData);

	//outer walls
	for (int i = 0; i < 4; i++) {
		pos = LevelHelper::getSymmetricWallPositions_Corners(i, GAME_WIDTH/2, GAME_HEIGHT/2, (60+20 + 40) + 80, 20, 20, GAME_HEIGHT/2-(20+40));
		WallManager::pushWall(new Wall(pos.first, pos.second, 20, GAME_HEIGHT/2-(20+40), color));
	}
	//extra
	for (int i = 0; i < 4; i++) {
		pos = LevelHelper::getSymmetricWallPositions_Corners(i, GAME_WIDTH/2, GAME_HEIGHT/2, (60+20 + 40) + 80 + 20, GAME_HEIGHT/2-(20+40), GAME_WIDTH/2 - ((60+20 + 40) + 80 + 20), 20);
		WallManager::pushWall(new Wall(pos.first, pos.second, GAME_WIDTH/2 - ((60+20 + 40) + 80 + 20), 20, color));
	}

	//extra powers
	for (int i = 0; i < 4; i++) {
		pos = LevelHelper::getSymmetricPowerupPositions_Corners(i, GAME_WIDTH/2, GAME_HEIGHT/2, GAME_WIDTH/2 - 20, GAME_HEIGHT/2 - 20);
		PowerupManager::pushPowerup(new PowerSquare(pos.first, pos.second, "vanilla", "speed"));
	}
	//experimental power positions
	for (int i = 0; i < 4; i++) {
		pos = LevelHelper::getSymmetricPowerupPositions_Corners(i, GAME_WIDTH/2, GAME_HEIGHT/2, GAME_WIDTH/2 - 10, GAME_HEIGHT/2 - (20+20+20+10));
		PowerupManager::pushPowerup(new PowerSquare(pos.first, pos.second, "vanilla", "invincible"));
	}

	//UL and UR turrets
	posArr = new double[3]{ GAME_WIDTH/2 - ((60+20 + 40) + 80 - 20), ((GAME_HEIGHT - 20) + (GAME_HEIGHT/2 + 20+40))/2, 0 };
	patrolCount = std::shared_ptr<int[]>(new int[1]{ 2 }, GenericFactoryConstructionData::deleteArrFunc);
	patrolRoute1 = std::shared_ptr<double[]>(new double[2*2]{
		GAME_WIDTH/2 - ((60+20 + 40) + 80 - 20), GAME_HEIGHT - 20,
		GAME_WIDTH/2 - ((60+20 + 40) + 80 - 20), GAME_HEIGHT/2 + 20+40
	}, GenericFactoryConstructionData::deleteArrFunc);
	patrolRoute2 = std::shared_ptr<double[]>(new double[2*2]{
		GAME_WIDTH/2 + ((60+20 + 40) + 80 - 20), GAME_HEIGHT - 20,
		GAME_WIDTH/2 + ((60+20 + 40) + 80 - 20), GAME_HEIGHT/2 + 20+40
	}, GenericFactoryConstructionData::deleteArrFunc);
	constructionData = GenericFactoryConstructionData(3, posArr);
	constructionData.pushData(1, patrolCount);
	constructionData.pushData(2*2, patrolRoute1);
	HazardManager::pushCircleHazard("vanilla", "patrolling_turret", constructionData);
	posArr = new double[3]{ GAME_WIDTH/2 + ((60+20 + 40) + 80 - 20), ((GAME_HEIGHT - 20) + (GAME_HEIGHT/2 + 20+40))/2, PI };
	constructionData = GenericFactoryConstructionData(3, posArr);
	constructionData.pushData(1, patrolCount);
	constructionData.pushData(2*2, patrolRoute2);
	HazardManager::pushCircleHazard("vanilla", "patrolling_turret", constructionData);

	//DL and DR turrets
	posArr = new double[3]{ GAME_WIDTH/2 - ((60+20 + 40) + 80 - 20), ((20) + (GAME_HEIGHT/2 - (20+40)))/2, 0 };
	patrolCount = std::shared_ptr<int[]>(new int[1]{ 2 }, GenericFactoryConstructionData::deleteArrFunc);
	patrolRoute1 = std::shared_ptr<double[]>(new double[2*2]{
		GAME_WIDTH/2 - ((60+20 + 40) + 80 - 20), 20,
		GAME_WIDTH/2 - ((60+20 + 40) + 80 - 20), GAME_HEIGHT/2 - (20+40)
	}, GenericFactoryConstructionData::deleteArrFunc);
	patrolRoute2 = std::shared_ptr<double[]>(new double[2*2]{
		GAME_WIDTH/2 + ((60+20 + 40) + 80 - 20), 20,
		GAME_WIDTH/2 + ((60+20 + 40) + 80 - 20), GAME_HEIGHT/2 - (20+40)
	}, GenericFactoryConstructionData::deleteArrFunc);
	constructionData = GenericFactoryConstructionData(3, posArr);
	constructionData.pushData(1, patrolCount);
	constructionData.pushData(2*2, patrolRoute1);
	HazardManager::pushCircleHazard("vanilla", "patrolling_turret", constructionData);
	posArr = new double[3]{ GAME_WIDTH/2 + ((60+20 + 40) + 80 - 20), ((20) + (GAME_HEIGHT/2 - (20+40)))/2, PI };
	constructionData = GenericFactoryConstructionData(3, posArr);
	constructionData.pushData(1, patrolCount);
	constructionData.pushData(2*2, patrolRoute2);
	HazardManager::pushCircleHazard("vanilla", "patrolling_turret", constructionData);

	//level idea: N++ X-04-4 (pretty sure there are other levels with similar designs)
}

Level* TightPatrollingCorridorLevel::factory() {
	return new TightPatrollingCorridorLevel();
}

TightPatrollingCorridorLevel::TightPatrollingCorridorLevel() { return; }
