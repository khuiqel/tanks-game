#include "timed-reward-level.h"

#include "../constants.h"
#include <cmath> //sin, cos

#include "../reset-things.h"
#include "../level-helper.h"
#include "../wall-manager.h"
#include "../powerup-manager.h"
#include "../hazard-manager.h"
#include "../simple-vector-2d.h"

#include "../game-manager.h" //getTickCount()

ColorValueHolder TimedRewardLevel::getDefaultColor() const {
	//return ColorValueHolder(0x97/255.0, 0x5C/255.0, 0x5C/255.0);
	//return ColorValueHolder(0x9E/255.0, 0x4C/255.0, 0x4C/255.0);
	return ColorValueHolder(0x97/255.0, 0x46/255.0, 0x46/255.0);
}

std::unordered_map<std::string, float> TimedRewardLevel::getWeights() const {
	std::unordered_map<std::string, float> weights;
	weights.insert({ "vanilla-extra", 0.25f });
	weights.insert({ "random-vanilla", 0.25f });
	weights.insert({ "dev", 0.5f });
	weights.insert({ "random-dev", 0.5f });
	//shouldn't really be in a regular group since it's not done... but it's fine
	return weights;
}

void TimedRewardLevel::tick() {
	//the intention of the moving walls was not to move sinusoidally but more linearly (like a triangle wave but square wave tips)
	//I don't really care since this isn't an actual level right now; maybe it'll get updated in the future

	for (int i = 0; i < 4; i++) {
		Wall* tempWall = WallManager::getWallByID(movingWalls[i]);
		if (tempWall != nullptr) {
			//tempWall->rec.y += (10 * std::sin((2*PI) * GameManager::getTickCount() / 200)) * ((i/2 == 0)*2-1);
			tempWall->rec.y += ((2*PI)/200 * 10 * std::cos(2*PI * GameManager::getTickCount() / 200)) * ((i/2 == 0)*2-1); //derivative
			//std::cout << tempWall->rec.y << std::endl;
		}
	}
}

void TimedRewardLevel::initialize() {
	ResetThings::tankPositionReset(40);

	ColorValueHolder color = getDefaultColor();
	PositionHolder pos;
	GenericFactoryConstructionData constructionData;
	double* posArr;

	pos = LevelHelper::getSymmetricWallPositions_UD(0, GAME_WIDTH/2, GAME_HEIGHT/2, 40, 120*2, 40);
	WallManager::pushWall(new Wall(pos.first, pos.second, 120*2, 40, color));
	pos = LevelHelper::getSymmetricWallPositions_UD(1, GAME_WIDTH/2, GAME_HEIGHT/2, 40, 120*2, 40);
	WallManager::pushWall(new Wall(pos.first, pos.second, 120*2, 40, color));

	//moving walls
	for (int i = 0; i < 4; i++) {
		pos = LevelHelper::getSymmetricWallPositions_Corners(i, GAME_WIDTH/2, GAME_HEIGHT/2, 120-20, 20/2, 20, 40+20);
		Wall* tempWall = new Wall(pos.first, pos.second, 20, 40+20, color);
		movingWalls[i] = tempWall->getGameID();
		WallManager::pushWall(tempWall);
	}

	PowerupManager::pushPowerup(new PowerSquare(GAME_WIDTH/2, GAME_HEIGHT/2, "vanilla", "godmode"));

	for (int i = 0; i < 4; i++) {
		pos = LevelHelper::getSymmetricPowerupPositions_Corners(i, GAME_WIDTH/2, GAME_HEIGHT/2, 120 - 20 - 4*2, 40 - 4*2);
		SimpleVector2D distToCenter = SimpleVector2D(GAME_WIDTH/2 - pos.first, GAME_HEIGHT/2 - pos.second);
		posArr = new double[3]{ pos.first, pos.second, distToCenter.getAngle() };
		constructionData = GenericFactoryConstructionData(3, posArr);
		HazardManager::pushCircleHazard("vanilla", "stationary_turret", constructionData);
	}

	//some extra walls
	LevelHelper::pushSymmetricWalls_Corners(GAME_WIDTH/2, GAME_HEIGHT/2, 120+40, 40+20, 40, 20, color);

	LevelHelper::pushSymmetricPowerups_Corners(GAME_WIDTH/2, GAME_HEIGHT/2, 120 + 60, GAME_HEIGHT/2 - (40+40)/2, "vanilla-extra", "shield");

	pos = LevelHelper::getSymmetricWallPositions_UD(0, GAME_WIDTH/2, GAME_HEIGHT/2, 40+40, 20, (GAME_HEIGHT/2 - (40+40)));
	posArr = new double[4]{ pos.first, pos.second, 20, GAME_HEIGHT/2 - (40+40) };
	constructionData = GenericFactoryConstructionData(4, posArr);
	HazardManager::pushRectHazard("vanilla", "vertical_lightning", constructionData);
	pos = LevelHelper::getSymmetricWallPositions_UD(1, GAME_WIDTH/2, GAME_HEIGHT/2, 40+40, 20, (GAME_HEIGHT/2 - (40+40)));
	posArr = new double[4]{ pos.first, pos.second, 20, GAME_HEIGHT/2 - (40+40) };
	constructionData = GenericFactoryConstructionData(4, posArr);
	HazardManager::pushRectHazard("vanilla", "vertical_lightning", constructionData);

	//other potential powers:
	//homing, tracking, big, speed?, triple?, blast, annoying
	//other potential hazards:
	//lava (moving)? targeting turret
}

Level* TimedRewardLevel::factory() {
	return new TimedRewardLevel();
}

TimedRewardLevel::TimedRewardLevel() {
	movingWalls = new Game_ID[4];
}

TimedRewardLevel::~TimedRewardLevel() {
	delete[] movingWalls;
}
