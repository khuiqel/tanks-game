#include "level-helper.h"

#include "constants.h"
#include <cmath> //ceil
#include <algorithm> //std::find
#include "rng.h"
#include "mylib.h" //isInArray, weightedSelect

#include "powerup-data-governor.h"
#include "powerup-manager.h"
#include "wall-manager.h"

void LevelHelper::pushRandomWalls(int count, double x_beginning, double y_beginning, double width_ofArea, double height_ofArea, const ColorValueHolder& c, double minW, double maxW, double minH, double maxH) {
	for (int i = 0; i < count; i++) {
		WallManager::pushWall(LevelHelper::makeNewRandomWall(x_beginning, y_beginning, width_ofArea, height_ofArea, c, minW, maxW, minH, maxH));
	}
}

void LevelHelper::pushClassicWalls(const ColorValueHolder& c) {
	for (int i = 0; i < 4; i++) {
		//classic JS walls
		PositionHolder pos = LevelHelper::getSymmetricWallPositions_Corners(i, GAME_WIDTH/2, GAME_HEIGHT/2, GAME_WIDTH/2-40*2-32, GAME_HEIGHT/2-128, 32, 128);
		WallManager::pushWall(new Wall(pos.first, pos.second, 32, 128, c));
	}

	//compiler warning C4506: can't inline a function if it's not defined right away, unless extern is used
	//that's why this function wasn't declared as inline
}

void LevelHelper::pushSymmetricPowerups_LR(double x_center, double y_center, double x_offset, std::string type, std::string name) {
	PositionHolder pos = LevelHelper::getSymmetricPowerupPositions_LR(0, x_center, y_center, x_offset);
	PowerupManager::pushPowerup(new PowerSquare(pos.first, pos.second, type, name));
	pos = LevelHelper::getSymmetricPowerupPositions_LR(1, x_center, y_center, x_offset);
	PowerupManager::pushPowerup(new PowerSquare(pos.first, pos.second, type, name));
}

void LevelHelper::pushSymmetricPowerups_LR(double x_center, double y_center, double x_offset, const std::string* types, const std::string* names, int count) {
	PositionHolder pos = LevelHelper::getSymmetricPowerupPositions_LR(0, x_center, y_center, x_offset);
	PowerupManager::pushPowerup(new PowerSquare(pos.first, pos.second, types, names, count));
	pos = LevelHelper::getSymmetricPowerupPositions_LR(1, x_center, y_center, x_offset);
	PowerupManager::pushPowerup(new PowerSquare(pos.first, pos.second, types, names, count));
}

void LevelHelper::pushSymmetricPowerups_LR_Alternate(double x_center, double y_center, double x_offset, std::string type1, std::string name1, std::string type2, std::string name2) {
	int tempRand = LevelRNG::randFunc() * 2;
	PositionHolder pos = LevelHelper::getSymmetricPowerupPositions_LR(0, x_center, y_center, x_offset);
	PowerupManager::pushPowerup(new PowerSquare(pos.first, pos.second, LevelHelper::simplePowerAlternate(0, tempRand, type1, type2), LevelHelper::simplePowerAlternate(0, tempRand, name1, name2)));
	pos = LevelHelper::getSymmetricPowerupPositions_LR(1, x_center, y_center, x_offset);
	PowerupManager::pushPowerup(new PowerSquare(pos.first, pos.second, LevelHelper::simplePowerAlternate(1, tempRand, type1, type2), LevelHelper::simplePowerAlternate(1, tempRand, name1, name2)));
	//is the double simplePowerAlternate() call ideal? probably not, but save that refactor for later (use std::pair?)
}

void LevelHelper::pushSymmetricPowerups_LR_Alternate(double x_center, double y_center, double x_offset, const std::string* types1, const std::string* names1, int count1, const std::string* types2, const std::string* names2, int count2) {
	PositionHolder pos;
	int tempRand = LevelRNG::randFunc() * 2;
	if (tempRand == 0) {
		pos = LevelHelper::getSymmetricPowerupPositions_LR(0, x_center, y_center, x_offset);
		PowerupManager::pushPowerup(new PowerSquare(pos.first, pos.second, types1, names1, count1));
		pos = LevelHelper::getSymmetricPowerupPositions_LR(1, x_center, y_center, x_offset);
		PowerupManager::pushPowerup(new PowerSquare(pos.first, pos.second, types2, names2, count2));
	} else {
		pos = LevelHelper::getSymmetricPowerupPositions_LR(0, x_center, y_center, x_offset);
		PowerupManager::pushPowerup(new PowerSquare(pos.first, pos.second, types2, names2, count2));
		pos = LevelHelper::getSymmetricPowerupPositions_LR(1, x_center, y_center, x_offset);
		PowerupManager::pushPowerup(new PowerSquare(pos.first, pos.second, types1, names1, count1));
	}
	//is this branch ideal? probably, at least it's clean and easy
}

void LevelHelper::pushSymmetricPowerups_UD(double x_center, double y_center, double y_offset, std::string type, std::string name) {
	PositionHolder pos = LevelHelper::getSymmetricPowerupPositions_UD(0, x_center, y_center, y_offset);
	PowerupManager::pushPowerup(new PowerSquare(pos.first, pos.second, type, name));
	pos = LevelHelper::getSymmetricPowerupPositions_UD(1, x_center, y_center, y_offset);
	PowerupManager::pushPowerup(new PowerSquare(pos.first, pos.second, type, name));
}

void LevelHelper::pushSymmetricPowerups_UD(double x_center, double y_center, double y_offset, const std::string* types, const std::string* names, int count) {
	PositionHolder pos = LevelHelper::getSymmetricPowerupPositions_UD(0, x_center, y_center, y_offset);
	PowerupManager::pushPowerup(new PowerSquare(pos.first, pos.second, types, names, count));
	pos = LevelHelper::getSymmetricPowerupPositions_UD(1, x_center, y_center, y_offset);
	PowerupManager::pushPowerup(new PowerSquare(pos.first, pos.second, types, names, count));
}

void LevelHelper::pushSymmetricPowerups_UD_Alternate(double x_center, double y_center, double y_offset, std::string type1, std::string name1, std::string type2, std::string name2) {
	int tempRand = LevelRNG::randFunc() * 2;
	PositionHolder pos = LevelHelper::getSymmetricPowerupPositions_UD(0, x_center, y_center, y_offset);
	PowerupManager::pushPowerup(new PowerSquare(pos.first, pos.second, LevelHelper::simplePowerAlternate(0, tempRand, type1, type2), LevelHelper::simplePowerAlternate(0, tempRand, name1, name2)));
	pos = LevelHelper::getSymmetricPowerupPositions_UD(1, x_center, y_center, y_offset);
	PowerupManager::pushPowerup(new PowerSquare(pos.first, pos.second, LevelHelper::simplePowerAlternate(1, tempRand, type1, type2), LevelHelper::simplePowerAlternate(1, tempRand, name1, name2)));
}

void LevelHelper::pushSymmetricPowerups_UD_Alternate(double x_center, double y_center, double y_offset, const std::string* types1, const std::string* names1, int count1, const std::string* types2, const std::string* names2, int count2) {
	PositionHolder pos;
	int tempRand = LevelRNG::randFunc() * 2;
	if (tempRand == 0) {
		pos = LevelHelper::getSymmetricPowerupPositions_UD(0, x_center, y_center, y_offset);
		PowerupManager::pushPowerup(new PowerSquare(pos.first, pos.second, types1, names1, count1));
		pos = LevelHelper::getSymmetricPowerupPositions_UD(1, x_center, y_center, y_offset);
		PowerupManager::pushPowerup(new PowerSquare(pos.first, pos.second, types2, names2, count2));
	} else {
		pos = LevelHelper::getSymmetricPowerupPositions_UD(0, x_center, y_center, y_offset);
		PowerupManager::pushPowerup(new PowerSquare(pos.first, pos.second, types2, names2, count2));
		pos = LevelHelper::getSymmetricPowerupPositions_UD(1, x_center, y_center, y_offset);
		PowerupManager::pushPowerup(new PowerSquare(pos.first, pos.second, types1, names1, count1));
	}
}

void LevelHelper::pushSymmetricPowerups_Corners(double x_center, double y_center, double x_offset, double y_offset, std::string type, std::string name) {
	for (int i = 0; i < 4; i++) {
		PositionHolder pos = LevelHelper::getSymmetricPowerupPositions_Corners(i, x_center, y_center, x_offset, y_offset);
		PowerupManager::pushPowerup(new PowerSquare(pos.first, pos.second, type, name));
	}
}

void LevelHelper::pushSymmetricPowerups_Corners(double x_center, double y_center, double x_offset, double y_offset, const std::string* types, const std::string* names, int count) {
	for (int i = 0; i < 4; i++) {
		PositionHolder pos = LevelHelper::getSymmetricPowerupPositions_Corners(i, x_center, y_center, x_offset, y_offset);
		PowerupManager::pushPowerup(new PowerSquare(pos.first, pos.second, types, names, count));
	}
}

void LevelHelper::pushSymmetricPowerups_Corners_Alternate(double x_center, double y_center, double x_offset, double y_offset, std::string type1, std::string name1, std::string type2, std::string name2) {
	int tempRand = LevelRNG::randFunc() * 2;
	for (int i = 0; i < 4; i++) {
		PositionHolder pos = LevelHelper::getSymmetricPowerupPositions_Corners(i, x_center, y_center, x_offset, y_offset);
		PowerupManager::pushPowerup(new PowerSquare(pos.first, pos.second, LevelHelper::powerAlternate(i, tempRand, type1, type2), LevelHelper::powerAlternate(i, tempRand, name1, name2)));
	}
}

void LevelHelper::pushSymmetricPowerups_Corners_Alternate(double x_center, double y_center, double x_offset, double y_offset, const std::string* types1, const std::string* names1, int count1, const std::string* types2, const std::string* names2, int count2) {
	int tempRand = LevelRNG::randFunc() * 2;
	for (int i = 0; i < 4; i++) {
		PositionHolder pos = LevelHelper::getSymmetricPowerupPositions_Corners(i, x_center, y_center, x_offset, y_offset);
		if (LevelHelper::powerAlternateNum(i, tempRand) == 0) {
			PowerupManager::pushPowerup(new PowerSquare(pos.first, pos.second, types1, names1, count1));
		} else {
			PowerupManager::pushPowerup(new PowerSquare(pos.first, pos.second, types2, names2, count2));
		}
	}
}

void LevelHelper::pushSymmetricPowerups_DiagForwardSlash(double x_center, double y_center, double x_offset, double y_offset, std::string type, std::string name) {
	PositionHolder pos = LevelHelper::getSymmetricPowerupPositions_DiagForwardSlash(0, x_center, y_center, x_offset, y_offset);
	PowerupManager::pushPowerup(new PowerSquare(pos.first, pos.second, type, name));
	pos = LevelHelper::getSymmetricPowerupPositions_DiagForwardSlash(1, x_center, y_center, x_offset, y_offset);
	PowerupManager::pushPowerup(new PowerSquare(pos.first, pos.second, type, name));
}

void LevelHelper::pushSymmetricPowerups_DiagForwardSlash(double x_center, double y_center, double x_offset, double y_offset, const std::string* types, const std::string* names, int count) {
	PositionHolder pos = LevelHelper::getSymmetricPowerupPositions_DiagForwardSlash(0, x_center, y_center, x_offset, y_offset);
	PowerupManager::pushPowerup(new PowerSquare(pos.first, pos.second, types, names, count));
	pos = LevelHelper::getSymmetricPowerupPositions_DiagForwardSlash(1, x_center, y_center, x_offset, y_offset);
	PowerupManager::pushPowerup(new PowerSquare(pos.first, pos.second, types, names, count));
}

void LevelHelper::pushSymmetricPowerups_DiagForwardSlash_Alternate(double x_center, double y_center, double x_offset, double y_offset, std::string type1, std::string name1, std::string type2, std::string name2) {
	int tempRand = LevelRNG::randFunc() * 2;
	PositionHolder pos = LevelHelper::getSymmetricPowerupPositions_DiagForwardSlash(0, x_center, y_center, x_offset, y_offset);
	PowerupManager::pushPowerup(new PowerSquare(pos.first, pos.second, LevelHelper::simplePowerAlternate(0, tempRand, type1, type2), LevelHelper::simplePowerAlternate(0, tempRand, name1, name2)));
	pos = LevelHelper::getSymmetricPowerupPositions_DiagForwardSlash(1, x_center, y_center, x_offset, y_offset);
	PowerupManager::pushPowerup(new PowerSquare(pos.first, pos.second, LevelHelper::simplePowerAlternate(1, tempRand, type1, type2), LevelHelper::simplePowerAlternate(1, tempRand, name1, name2)));
}

void LevelHelper::pushSymmetricPowerups_DiagForwardSlash_Alternate(double x_center, double y_center, double x_offset, double y_offset, const std::string* types1, const std::string* names1, int count1, const std::string* types2, const std::string* names2, int count2) {
	PositionHolder pos;
	int tempRand = LevelRNG::randFunc() * 2;
	if (tempRand == 0) {
		pos = LevelHelper::getSymmetricPowerupPositions_DiagForwardSlash(0, x_center, y_center, x_offset, y_offset);
		PowerupManager::pushPowerup(new PowerSquare(pos.first, pos.second, types1, names1, count1));
		pos = LevelHelper::getSymmetricPowerupPositions_DiagForwardSlash(1, x_center, y_center, x_offset, y_offset);
		PowerupManager::pushPowerup(new PowerSquare(pos.first, pos.second, types2, names2, count2));
	} else {
		pos = LevelHelper::getSymmetricPowerupPositions_DiagForwardSlash(0, x_center, y_center, x_offset, y_offset);
		PowerupManager::pushPowerup(new PowerSquare(pos.first, pos.second, types2, names2, count2));
		pos = LevelHelper::getSymmetricPowerupPositions_DiagForwardSlash(1, x_center, y_center, x_offset, y_offset);
		PowerupManager::pushPowerup(new PowerSquare(pos.first, pos.second, types1, names1, count1));
	}
}

void LevelHelper::pushSymmetricPowerups_DiagBackwardSlash(double x_center, double y_center, double x_offset, double y_offset, std::string type, std::string name) {
	PositionHolder pos = LevelHelper::getSymmetricPowerupPositions_DiagBackwardSlash(0, x_center, y_center, x_offset, y_offset);
	PowerupManager::pushPowerup(new PowerSquare(pos.first, pos.second, type, name));
	pos = LevelHelper::getSymmetricPowerupPositions_DiagBackwardSlash(1, x_center, y_center, x_offset, y_offset);
	PowerupManager::pushPowerup(new PowerSquare(pos.first, pos.second, type, name));
}

void LevelHelper::pushSymmetricPowerups_DiagBackwardSlash(double x_center, double y_center, double x_offset, double y_offset, const std::string* types, const std::string* names, int count) {
	PositionHolder pos = LevelHelper::getSymmetricPowerupPositions_DiagBackwardSlash(0, x_center, y_center, x_offset, y_offset);
	PowerupManager::pushPowerup(new PowerSquare(pos.first, pos.second, types, names, count));
	pos = LevelHelper::getSymmetricPowerupPositions_DiagBackwardSlash(1, x_center, y_center, x_offset, y_offset);
	PowerupManager::pushPowerup(new PowerSquare(pos.first, pos.second, types, names, count));
}

void LevelHelper::pushSymmetricPowerups_DiagBackwardSlash_Alternate(double x_center, double y_center, double x_offset, double y_offset, std::string type1, std::string name1, std::string type2, std::string name2) {
	int tempRand = LevelRNG::randFunc() * 2;
	PositionHolder pos = LevelHelper::getSymmetricPowerupPositions_DiagBackwardSlash(0, x_center, y_center, x_offset, y_offset);
	PowerupManager::pushPowerup(new PowerSquare(pos.first, pos.second, LevelHelper::simplePowerAlternate(0, tempRand, type1, type2), LevelHelper::simplePowerAlternate(0, tempRand, name1, name2)));
	pos = LevelHelper::getSymmetricPowerupPositions_DiagBackwardSlash(1, x_center, y_center, x_offset, y_offset);
	PowerupManager::pushPowerup(new PowerSquare(pos.first, pos.second, LevelHelper::simplePowerAlternate(1, tempRand, type1, type2), LevelHelper::simplePowerAlternate(1, tempRand, name1, name2)));
}

void LevelHelper::pushSymmetricPowerups_DiagBackwardSlash_Alternate(double x_center, double y_center, double x_offset, double y_offset, const std::string* types1, const std::string* names1, int count1, const std::string* types2, const std::string* names2, int count2) {
	PositionHolder pos;
	int tempRand = LevelRNG::randFunc() * 2;
	if (tempRand == 0) {
		pos = LevelHelper::getSymmetricPowerupPositions_DiagBackwardSlash(0, x_center, y_center, x_offset, y_offset);
		PowerupManager::pushPowerup(new PowerSquare(pos.first, pos.second, types1, names1, count1));
		pos = LevelHelper::getSymmetricPowerupPositions_DiagBackwardSlash(1, x_center, y_center, x_offset, y_offset);
		PowerupManager::pushPowerup(new PowerSquare(pos.first, pos.second, types2, names2, count2));
	} else {
		pos = LevelHelper::getSymmetricPowerupPositions_DiagBackwardSlash(0, x_center, y_center, x_offset, y_offset);
		PowerupManager::pushPowerup(new PowerSquare(pos.first, pos.second, types2, names2, count2));
		pos = LevelHelper::getSymmetricPowerupPositions_DiagBackwardSlash(1, x_center, y_center, x_offset, y_offset);
		PowerupManager::pushPowerup(new PowerSquare(pos.first, pos.second, types1, names1, count1));
	}
}

void LevelHelper::pushSymmetricWalls_LR(double x_center, double y_center, double x_offset, double wallWidth, double wallHeight, const ColorValueHolder& c) {
	PositionHolder pos = LevelHelper::getSymmetricWallPositions_LR(0, x_center, y_center, x_offset, wallWidth, wallHeight);
	WallManager::pushWall(new Wall(pos.first, pos.second, wallWidth, wallHeight, c));
	pos = LevelHelper::getSymmetricWallPositions_LR(1, x_center, y_center, x_offset, wallWidth, wallHeight);
	WallManager::pushWall(new Wall(pos.first, pos.second, wallWidth, wallHeight, c));
}

void LevelHelper::pushSymmetricWalls_UD(double x_center, double y_center, double y_offset, double wallWidth, double wallHeight, const ColorValueHolder& c) {
	PositionHolder pos = LevelHelper::getSymmetricWallPositions_UD(0, x_center, y_center, y_offset, wallWidth, wallHeight);
	WallManager::pushWall(new Wall(pos.first, pos.second, wallWidth, wallHeight, c));
	pos = LevelHelper::getSymmetricWallPositions_UD(1, x_center, y_center, y_offset, wallWidth, wallHeight);
	WallManager::pushWall(new Wall(pos.first, pos.second, wallWidth, wallHeight, c));
}

void LevelHelper::pushSymmetricWalls_Corners(double x_center, double y_center, double x_offset, double y_offset, double wallWidth, double wallHeight, const ColorValueHolder& c) {
	for (int i = 0; i < 4; i++) {
		PositionHolder pos = LevelHelper::getSymmetricWallPositions_Corners(i, x_center, y_center, x_offset, y_offset, wallWidth, wallHeight);
		WallManager::pushWall(new Wall(pos.first, pos.second, wallWidth, wallHeight, c));
	}
}

void LevelHelper::pushSymmetricWalls_DiagForwardSlash(double x_center, double y_center, double x_offset, double y_offset, double wallWidth, double wallHeight, const ColorValueHolder& c) {
	PositionHolder pos = LevelHelper::getSymmetricWallPositions_DiagForwardSlash(0, x_center, y_center, x_offset, y_offset, wallWidth, wallHeight);
	WallManager::pushWall(new Wall(pos.first, pos.second, wallWidth, wallHeight, c));
	pos = LevelHelper::getSymmetricWallPositions_DiagForwardSlash(1, x_center, y_center, x_offset, y_offset, wallWidth, wallHeight);
	WallManager::pushWall(new Wall(pos.first, pos.second, wallWidth, wallHeight, c));
}

void LevelHelper::pushSymmetricWalls_DiagBackwardSlash(double x_center, double y_center, double x_offset, double y_offset, double wallWidth, double wallHeight, const ColorValueHolder& c) {
	PositionHolder pos = LevelHelper::getSymmetricWallPositions_DiagBackwardSlash(0, x_center, y_center, x_offset, y_offset, wallWidth, wallHeight);
	WallManager::pushWall(new Wall(pos.first, pos.second, wallWidth, wallHeight, c));
	pos = LevelHelper::getSymmetricWallPositions_DiagBackwardSlash(1, x_center, y_center, x_offset, y_offset, wallWidth, wallHeight);
	WallManager::pushWall(new Wall(pos.first, pos.second, wallWidth, wallHeight, c));
}



PositionHolder* LevelHelper::getClassicWallPositions() {
	PositionHolder* wallArray = new PositionHolder[4];
	for (int i = 0; i < 4; i++) {
		//classic JS walls
		wallArray[i] = LevelHelper::getSymmetricWallPositions_Corners(i, GAME_WIDTH/2, GAME_HEIGHT/2, GAME_WIDTH/2-40*2-32, GAME_HEIGHT/2-128, 32, 128);
	}
	return wallArray;
}

Wall* LevelHelper::makeNewRandomWall(double x_beginning, double y_beginning, double width_ofArea, double height_ofArea, const ColorValueHolder& c, double minW, double maxW, double minH, double maxH) {
	double w = LevelRNG::randNumInRange(minW, maxW);
	double h = LevelRNG::randNumInRange(minH, maxH);
	double x = LevelRNG::randNumInRange(x_beginning, x_beginning + width_ofArea - w);
	double y = LevelRNG::randNumInRange(y_beginning, y_beginning + height_ofArea - h);

	return new Wall(x, y, w, h, c);
}

std::string* LevelHelper::getRandomPowers(int count, std::string type) {
	//get list of powers
	int powerNum = PowerupDataGovernor::getNumPowerTypes(type);
	std::string* possiblePowers = new std::string[powerNum];
	for (int i = 0; i < powerNum; i++) {
		possiblePowers[i] = PowerupDataGovernor::getPowerName(type, i);
	}

	//get stacking status and weight of the powers
	bool* canStack = new bool[powerNum];
	float* powerWeights = new float[powerNum];
	for (int i = 0; i < powerNum; i++) {
		Power* p = PowerupDataGovernor::getPower(type, possiblePowers[i]);
		std::vector<std::string> attributes = p->getPowerAttributes();
		canStack[i] = (std::find(attributes.begin(), attributes.end(), "stack") != attributes.end());
		powerWeights[i] = p->getWeights()[type];
		delete p;
	}

	//get the randomized powers
	std::string* n = LevelHelper::getRandomPowers(count, canStack, possiblePowers, powerWeights, powerNum);

	delete[] possiblePowers;
	delete[] canStack;
	delete[] powerWeights;

	return n;
}

//this can be condensed with the previous function but whatever
std::string* LevelHelper::getRandomPowers(int count, std::string type, const std::string* names, int nameCount) {
	//get stacking status and weight of the powers
	bool* canStack = new bool[nameCount];
	float* powerWeights = new float[nameCount];
	for (int i = 0; i < nameCount; i++) {
		Power* p = PowerupDataGovernor::getPower(type, names[i]);
		std::vector<std::string> attributes = p->getPowerAttributes();
		canStack[i] = (std::find(attributes.begin(), attributes.end(), "stack") != attributes.end());
		powerWeights[i] = p->getWeights()[type];
		delete p;
	}

	//get the randomized powers
	std::string* n = LevelHelper::getRandomPowers(count, canStack, names, powerWeights, nameCount);

	delete[] canStack;
	delete[] powerWeights;

	return n;
}

std::string* LevelHelper::getRandomPowersOld(int count, bool replacement, const std::string* names, int nameCount) {
	std::string* n = new std::string[count];
	int* used = new int[count];

	for (int i = 0; i < count; i++) {
		if (!replacement) {
			//first verify if a new name can be chosen:
			bool found = true;
			for (int j = 0; j < i; j++) {
				if (!isInArray<int>(j, used, i+1)) {
					found = false;
					break;
				}
			}
			//then add name based on result
			if (!found) {
				//there are values that can get chosen
				int index;
				do {
					index = LevelRNG::randFunc() * nameCount;
				} while (isInArray<int>(index, used, i+1));
				used[i] = index;
			} else {
				//there aren't values that can get chosen
				replacement = true; //instead of getting stuck or throwing an error, just put duplicates in (change eventually)
			}
		}
		if (replacement) {
			int index = LevelRNG::randFunc() * nameCount;
			used[i] = index;
		}
	}

	for (int i = 0; i < count; i++) {
		n[i] = names[used[i]];
	}

	delete[] used;
	return n;
}

std::string* LevelHelper::getRandomPowers(int count, const bool* powersCanStack, const std::string* names, int nameCount) {
	std::string* n = new std::string[count];
	int* used = new int[count];

	for (int i = 0; i < count; i++) {
		//first verify if a new name can be chosen:
		bool leftoverPowersCanStack = true;
		for (int j = 0; j < i; j++) {
			if (!powersCanStack[used[j]]) {
				leftoverPowersCanStack = false;
				break;
			}
		}
		//then add name based on result
		int index;
		if (!leftoverPowersCanStack) {
			//there are values that can get chosen
			do {
				index = LevelRNG::randFunc() * nameCount;
			} while (!powersCanStack[index] && isInArray<int>(index, used, i+1));
		} else {
			//there aren't values that can get chosen or no stacking problems yet
			index = LevelRNG::randFunc() * nameCount;
		}
		//not sure if the comments are right, but the results are (empirically determined)
		used[i] = index;
	}

	for (int i = 0; i < count; i++) {
		n[i] = names[used[i]];
	}

	delete[] used;
	return n;
}

//this function could become an endless loop if the only stacking power left has a weight of 0
std::string* LevelHelper::getRandomPowers(int count, const bool* powersCanStack, const std::string* names, const float* weights, int powerCount) {
	std::string* n = new std::string[count];
	int* used = new int[count];

	for (int i = 0; i < count; i++) {
		//first verify if a new name can be chosen:
		bool leftoverPowersCanStack = true;
		for (int j = 0; j < i; j++) {
			if (!powersCanStack[used[j]]) {
				leftoverPowersCanStack = false;
				break;
			}
		}
		//then add name based on result
		int index;
		if (!leftoverPowersCanStack) {
			//there are values that can get chosen
			do {
				index = weightedSelect<float>(weights, powerCount);
			} while (!powersCanStack[index] && isInArray<int>(index, used, i+1));
		} else {
			//there aren't values that can get chosen or no stacking problems yet
			index = weightedSelect<float>(weights, powerCount);
		}
		//not sure if the comments are right, but the results are (empirically determined)
		used[i] = index;
	}

	for (int i = 0; i < count; i++) {
		n[i] = names[used[i]];
	}

	delete[] used;
	return n;
}

std::string LevelHelper::powerAlternate(int position, int rand, std::string p1, std::string p2) {
	return (((int)std::ceil( float(position%3) / 3) + rand) % 2 == 0 ? p1 : p2);
	/*
	{0,   1,   2,  3} // start
	{0,   1,   2,  0} // %3, then cast to float
	{0,  .3,  .6,  0} // /3, then add rand
	{0,  .3,  .6,  0} (rand=0) -> ceil -> {0, 1, 1, 0}
	{1, 1.3, 1.6,  1} (rand=1) -> ceil -> {1, 2, 2, 1}
	{0, 1, 1, 0} OR	{1, 0, 0, 1} // %2
	*/
}

std::string LevelHelper::simplePowerAlternate(int position, int rand, std::string p1, std::string p2) {
	return ((position + rand) % 2 == 0 ? p1 : p2);
}

int LevelHelper::powerAlternateNum(int position, int rand) {
	//see powerAlternate()
	return ((int)std::ceil( float(position%3) / 3) + rand) % 2;
}

int LevelHelper::simplePowerAlternateNum(int position, int rand) {
	return (position + rand) % 2;
}

PositionHolder LevelHelper::getSymmetricPowerupPositions_LR(int position, double x_center, double y_center, double x_offset) {
	return { x_center + ((position%2)*2-1) * x_offset, y_center };
}

PositionHolder LevelHelper::getSymmetricPowerupPositions_UD(int position, double x_center, double y_center, double y_offset) {
	return { x_center, y_center + ((position%2)*2-1) * y_offset };
}

PositionHolder LevelHelper::getSymmetricPowerupPositions_Corners(int position, double x_center, double y_center, double x_offset, double y_offset) {
	//return { x_center - (position%2*2-1) * x_offset, y_center - ((position/2)*2-1) * y_offset };
	return { x_center + (position%2*2-1) * x_offset, y_center + ((position/2)*2-1) * y_offset };
	//the above returns give the same results but in opposite orders; the order doesn't matter, so I'll arbitrarily choose the second
	//LD, RD, LU, RU
	/*
	switch (position) {
		case 0: return { x_center - x_offset, y_center - y_offset };
		case 1: return { x_center + x_offset, y_center - y_offset };
		case 2: return { x_center - x_offset, y_center + y_offset };
		case 3: return { x_center + x_offset, y_center + y_offset };
		default: return {0,0};
	}
	*/
}

PositionHolder LevelHelper::getSymmetricPowerupPositions_DiagForwardSlash(int position, double x_center, double y_center, double x_offset, double y_offset) {
	return { x_center + ((position%2)*2-1) * x_offset, y_center + ((position%2)*2-1) * y_offset };
	//note: has not been tested
}

PositionHolder LevelHelper::getSymmetricPowerupPositions_DiagBackwardSlash(int position, double x_center, double y_center, double x_offset, double y_offset) {
	return { x_center + ((position%2)*2-1) * x_offset, y_center - ((position%2)*2-1) * y_offset };
	//note: has not been tested
}

PositionHolder LevelHelper::getSymmetricWallPositions_LR(int position, double x_center, double y_center, double x_offset, double wallWidth, double wallHeight) {
	return { x_center + ((position%2)*2-1) * x_offset - ((position+1)%2) * wallWidth, y_center - wallHeight/2 };
}

PositionHolder LevelHelper::getSymmetricWallPositions_UD(int position, double x_center, double y_center, double y_offset, double wallWidth, double wallHeight) {
	return { x_center - wallWidth/2, y_center + ((position%2)*2-1) * y_offset - ((position+1)%2) * wallHeight };
}

PositionHolder LevelHelper::getSymmetricWallPositions_Corners(int position, double x_center, double y_center, double x_offset, double y_offset, double wallWidth, double wallHeight) {
	return { x_center + ((position%2)*2-1) * x_offset - ((position+1)%2) * wallWidth, y_center + ((position/2)*2-1) * y_offset - ((3-position)/2) * wallHeight };
	/*
	//LD, RD, LU, RU
	switch (position) {
		case 0: return { x_center - x_offset - wallWidth, y_center - y_offset - wallHeight };
		case 1: return { x_center + x_offset, y_center - y_offset - wallHeight };
		case 2: return { x_center - x_offset - wallWidth, y_center + y_offset };
		case 3: return { x_center + x_offset, y_center + y_offset };
		default: return {0,0};
	}
	*/
}

PositionHolder LevelHelper::getSymmetricWallPositions_DiagForwardSlash(int position, double x_center, double y_center, double x_offset, double y_offset, double wallWidth, double wallHeight) {
	return { x_center + ((position%2)*2-1) * x_offset - ((position+1)%2) * wallWidth, y_center + ((position%2)*2-1) * y_offset - ((position+1)%2) * wallHeight };
	//note: has not been tested
}

PositionHolder LevelHelper::getSymmetricWallPositions_DiagBackwardSlash(int position, double x_center, double y_center, double x_offset, double y_offset, double wallWidth, double wallHeight) {
	return { x_center + ((position%2)*2-1) * x_offset - ((position+1)%2) * wallWidth, y_center - ((position%2)*2-1) * y_offset - (position%2) * wallHeight };
	//note: has not been tested
}
