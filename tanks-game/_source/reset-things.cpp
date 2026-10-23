#include "reset-things.h"

#include "constants.h"
#include <algorithm> //std::clamp
#include <iostream>
#include "rng.h"
#include "mylib.h" //weightedSelect

#include "tank-manager.h"
#include "bullet-manager.h"
#include "wall-manager.h"
#include "powerup-manager.h"
#include "hazard-manager.h"
#include "level-manager.h"

#include "game-manager.h"
#include "end-game-handler.h"
#include "frame-time-graph.h"

const int ResetThings::default_tankStartingYCount = 5;
const double ResetThings::default_tankToEdgeDist = 20;
const double ResetThings::default_tankStartingYRange = (4.0/5) * GAME_HEIGHT;

void ResetThings::reset() {
	EndGameHandler::finalizeScores();
	//TODO: this good?
	for (int i = 0; i < EndGameHandler::teamsParticipating.size(); i++) {
		std::cout << EndGameHandler::teamsParticipating[i].teamName << " score: " << EndGameHandler::teamWins[EndGameHandler::teamsParticipating[i].teamID];

		if (i != EndGameHandler::teamsParticipating.size()-1) {
			std::cout << " | ";
		} else {
			std::cout << std::endl;
		}
	}

	BulletManager::clearBullets();
	WallManager::clearWalls();
	PowerupManager::clearPowerups();
	HazardManager::clearCircleHazards();
	HazardManager::clearRectHazards();
	LevelManager::clearLevels();
	TankManager::clearTanks();
	GameManager::clearObjects();

	//TODO: maybe GameMainLoop should hold the shooting cooldown? (but the tank still needs to get initialized with it...)
	const GameSettings& game_settings = GameManager::get_settings();
	double shootCooldown = game_settings.ShootingCooldown;

	TankManager::pushTank(new Tank(default_tankToEdgeDist, GAME_HEIGHT/2, 0, 1, "WASD", shootCooldown));
	TankManager::pushTank(new Tank(GAME_WIDTH-default_tankToEdgeDist, GAME_HEIGHT/2, PI, 2, "Arrow Keys", shootCooldown));

	if (game_settings.GameForceSameLevel) {
		LevelManager::pushLevel(game_settings.GameForceSameLevel_identifier.first, game_settings.GameForceSameLevel_identifier.second);
	} else [[likely]] {
		if (game_settings.CustomLevelPlaylist.empty()) {
			const std::string levelPlaylist = game_settings.GameLevelPlaylist;
			LevelManager::pushLevel(levelPlaylist, LevelManager::levelWeightedSelect(levelPlaylist));
		} else {
			int levelIndex = LevelManager::customLevelWeightedSelect(game_settings.CustomLevelPlaylist);
			const auto& levelIdentifier = game_settings.CustomLevelPlaylist[levelIndex];
			LevelManager::pushLevel(levelIdentifier.first.first, levelIdentifier.first.second);
		}
	}

	//initialize levels from LevelManager level list
	for (int i = 0; i < LevelManager::getNumLevels(); i++) {
		LevelManager::getLevel(i)->initialize();
	}
	//initialize their level effects
	for (int i = 0; i < LevelManager::getNumLevels(); i++) {
		Level* l = LevelManager::getLevel(i);
		for (int j = 0; j < l->getNumEffects(); j++) {
			l->getLevelEffect(j)->apply();
		}
	}

	if (game_settings.ReportCurrentLevel) {
		std::cout << "> Now playing level \"" + LevelManager::getLevel(0)->getName() + "\"..." << std::endl;
		//this doesn't print for the first reset; main reason is laziness, secondary reason is the first level should be known
	}

	GameManager::Reset();
	FrameTimeGraph::clearGraphs();
}

void ResetThings::firstReset() {
	for (int i = 0; i < LevelManager::getNumLevels(); i++) {
		LevelManager::getLevel(i)->initialize();
	}
	for (int i = 0; i < LevelManager::getNumLevels(); i++) {
		Level* l = LevelManager::getLevel(i);
		for (int j = 0; j < l->getNumEffects(); j++) {
			l->getLevelEffect(j)->apply();
		}
	}
}

void ResetThings::firstGameInitialize(std::string tank1TeamName, std::string tank2TeamName, GameMainLoop& gameInstance) {
	const BasicINIParser::BasicINIData& ini_data = GameManager::get_INI();
	const GameSettings& game_settings = GameManager::get_settings();

	std::string tank1Name = "WASD";
	std::string tank2Name = "Arrow Keys";
	if (ini_data.exists("CONTROLS", "Tank1.Name")) {
		tank1Name = ini_data.get("CONTROLS", "Tank1.Name");
	}
	if (ini_data.exists("CONTROLS", "Tank2.Name")) {
		tank2Name = ini_data.get("CONTROLS", "Tank2.Name");
	}
	if (ini_data.exists("CONTROLS", "Tank1.TeamName")) {
		tank1TeamName = ini_data.get("CONTROLS", "Tank1.TeamName");
	}
	if (ini_data.exists("CONTROLS", "Tank2.TeamName")) {
		tank2TeamName = ini_data.get("CONTROLS", "Tank2.TeamName");
	}

	std::string tank1Forward = "w", tank1Left = "a", tank1Right = "d", tank1Shoot = "s", tank1Special = "e";
	if (ini_data.exists("CONTROLS", "Tank1.Forward")) {
		tank1Forward = ini_data.get("CONTROLS", "Tank1.Forward");
	}
	if (ini_data.exists("CONTROLS", "Tank1.Left")) {
		tank1Left = ini_data.get("CONTROLS", "Tank1.Left");
	}
	if (ini_data.exists("CONTROLS", "Tank1.Right")) {
		tank1Right = ini_data.get("CONTROLS", "Tank1.Right");
	}
	if (ini_data.exists("CONTROLS", "Tank1.Shoot")) {
		tank1Shoot = ini_data.get("CONTROLS", "Tank1.Shoot");
	}
	if (ini_data.exists("CONTROLS", "Tank1.Special")) {
		tank1Special = ini_data.get("CONTROLS", "Tank1.Special");
	}
	std::string tank2Forward = "Up", tank2Left = "Left", tank2Right = "Right", tank2Shoot = "Down", tank2Special = "/";
	if (ini_data.exists("CONTROLS", "Tank2.Forward")) {
		tank2Forward = ini_data.get("CONTROLS", "Tank2.Forward");
	}
	if (ini_data.exists("CONTROLS", "Tank2.Left")) {
		tank2Left = ini_data.get("CONTROLS", "Tank2.Left");
	}
	if (ini_data.exists("CONTROLS", "Tank2.Right")) {
		tank2Right = ini_data.get("CONTROLS", "Tank2.Right");
	}
	if (ini_data.exists("CONTROLS", "Tank2.Shoot")) {
		tank2Shoot = ini_data.get("CONTROLS", "Tank2.Shoot");
	}
	if (ini_data.exists("CONTROLS", "Tank2.Special")) {
		tank2Special = ini_data.get("CONTROLS", "Tank2.Special");
	}

	gameInstance.tank1Inputs[0] = TankInputChar(tank1Forward);
	gameInstance.tank1Inputs[1] = TankInputChar(tank1Left);
	gameInstance.tank1Inputs[2] = TankInputChar(tank1Right);
	gameInstance.tank1Inputs[3] = TankInputChar(tank1Shoot);
	gameInstance.tank1Inputs[4] = TankInputChar(tank1Special);
	gameInstance.tank2Inputs[0] = TankInputChar(tank2Forward);
	gameInstance.tank2Inputs[1] = TankInputChar(tank2Left);
	gameInstance.tank2Inputs[2] = TankInputChar(tank2Right);
	gameInstance.tank2Inputs[3] = TankInputChar(tank2Shoot);
	gameInstance.tank2Inputs[4] = TankInputChar(tank2Special);

	double shootCooldown = game_settings.ShootingCooldown;

	TankManager::pushTank(new Tank(20, GAME_HEIGHT/2, 0, 1, tank1Name, shootCooldown));
	TankManager::pushTank(new Tank(GAME_WIDTH-20, GAME_HEIGHT/2, PI, 2, tank2Name, shootCooldown));
	EndGameHandler::addTeamToWatch(1, tank1TeamName);
	EndGameHandler::addTeamToWatch(2, tank2TeamName);
	EndGameHandler::addTeamToWatch(HAZARD_TEAM, "HAZARDS");
}

void ResetThings::firstLevelPush() {
	const BasicINIParser::BasicINIData& ini_data = GameManager::get_INI();

	if (ini_data.exists("GAME_OPTIONS", "GameFirstLevel") && (ini_data.length("GAME_OPTIONS", "GameFirstLevel") >= 2)) {
		LevelManager::pushLevel(ini_data.get("GAME_OPTIONS", "GameFirstLevel", 0), ini_data.get("GAME_OPTIONS", "GameFirstLevel", 1));
	} else [[unlikely]] {
		LevelManager::pushLevel("vanilla", "default_random");
	}
}

//TODO: tankPositionReset accounting for more than two tanks

void ResetThings::tankPositionReset() {
	tankPositionReset(default_tankToEdgeDist);
}

void ResetThings::tankPositionReset(double x) {
	tankPositionReset(x, default_tankStartingYRange, default_tankStartingYCount);
}

void ResetThings::tankPositionReset(double x, double y) {
	Tank* first = TankManager::getTank(0);
	Tank* second = TankManager::getTank(1);

	x = std::clamp<double>(x, 0, GAME_WIDTH); //no trolls here
	y = std::clamp<double>(y, 0, GAME_HEIGHT); //trolls begone

	first->cir.x = x;
	first->cir.y = y;
	second->cir.x = GAME_WIDTH - x;
	second->cir.y = GAME_HEIGHT - y;
}

void ResetThings::tankPositionReset(double x, double yRange, int yCount) {
	Tank* first = TankManager::getTank(0);
	Tank* second = TankManager::getTank(1);

	x = std::clamp<double>(x, 0, GAME_WIDTH);
	yRange = std::clamp<double>(yRange, 0, GAME_HEIGHT);
	yCount = std::max(yCount, 1); //I thought std::mix/max was overloaded, but it's actually a template; neat, learning is cool

	int randNum = LevelRNG::randFunc() * yCount; //stays outside the conditional to ensure RNG is called when resetting the level (unless exact position is used)
	double yVal;
	if (yCount == 1) {
		yVal = GAME_HEIGHT/2;
	} else [[likely]] {
		yVal = randNum * (yRange/(yCount-1)) + ((GAME_HEIGHT - yRange)/2);
	}

	first->cir.x = x;
	first->cir.y = yVal;
	second->cir.x = GAME_WIDTH - x;
	second->cir.y = GAME_HEIGHT - yVal;
}
