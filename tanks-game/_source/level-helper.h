#pragma once
#include <string>

#include "wall.h"

using PositionHolder = std::pair<double, double>;

class LevelHelper { //has stuff for randomizing stuff in levels
public:
	//simple helper stuff (use this):
	static void pushRandomWalls(int count, double x_beginning, double y_beginning, double width_ofArea, double height_ofArea, const ColorValueHolder& c, double minW, double maxW, double minH, double maxH);
	static inline void pushRandomWalls(int count, double x_beginning, double y_beginning, double width_ofArea, double height_ofArea, const ColorValueHolder& c) {
		LevelHelper::pushRandomWalls(count, x_beginning, y_beginning, width_ofArea, height_ofArea, c, 12, 64, 8, 96);
	}
	static void pushClassicWalls(const ColorValueHolder& c);

	static void pushSymmetricPowerups_LR(double x_center, double y_center, double x_offset, std::string type, std::string name);
	static void pushSymmetricPowerups_LR(double x_center, double y_center, double x_offset, const std::string* types, const std::string* names, int count);
	static void pushSymmetricPowerups_LR_Alternate(double x_center, double y_center, double x_offset,
		std::string type1, std::string name1, std::string type2, std::string name2);
	static void pushSymmetricPowerups_LR_Alternate(double x_center, double y_center, double x_offset,
		const std::string* types1, const std::string* names1, int count1, const std::string* types2, const std::string* names2, int count2);

	static void pushSymmetricPowerups_UD(double x_center, double y_center, double y_offset, std::string type, std::string name);
	static void pushSymmetricPowerups_UD(double x_center, double y_center, double y_offset, const std::string* types, const std::string* names, int count);
	static void pushSymmetricPowerups_UD_Alternate(double x_center, double y_center, double y_offset,
		std::string type1, std::string name1, std::string type2, std::string name2);
	static void pushSymmetricPowerups_UD_Alternate(double x_center, double y_center, double y_offset,
		const std::string* types1, const std::string* names1, int count1, const std::string* types2, const std::string* names2, int count2);

	static void pushSymmetricPowerups_Corners(double x_center, double y_center, double x_offset, double y_offset, std::string type, std::string name);
	static void pushSymmetricPowerups_Corners(double x_center, double y_center, double x_offset, double y_offset, const std::string* types, const std::string* names, int count);
	static void pushSymmetricPowerups_Corners_Alternate(double x_center, double y_center, double x_offset, double y_offset,
		std::string type1, std::string name1, std::string type2, std::string name2);
	static void pushSymmetricPowerups_Corners_Alternate(double x_center, double y_center, double x_offset, double y_offset,
		const std::string* types1, const std::string* names1, int count1, const std::string* types2, const std::string* names2, int count2);

	static void pushSymmetricPowerups_DiagForwardSlash(double x_center, double y_center, double x_offset, double y_offset, std::string type, std::string name);
	static void pushSymmetricPowerups_DiagForwardSlash(double x_center, double y_center, double x_offset, double y_offset, const std::string* types, const std::string* names, int count);
	static void pushSymmetricPowerups_DiagForwardSlash_Alternate(double x_center, double y_center, double x_offset, double y_offset,
		std::string type1, std::string name1, std::string type2, std::string name2);
	static void pushSymmetricPowerups_DiagForwardSlash_Alternate(double x_center, double y_center, double x_offset, double y_offset,
		const std::string* types1, const std::string* names1, int count1, const std::string* types2, const std::string* names2, int count2);

	static void pushSymmetricPowerups_DiagBackwardSlash(double x_center, double y_center, double x_offset, double y_offset, std::string type, std::string name);
	static void pushSymmetricPowerups_DiagBackwardSlash(double x_center, double y_center, double x_offset, double y_offset, const std::string* types, const std::string* names, int count);
	static void pushSymmetricPowerups_DiagBackwardSlash_Alternate(double x_center, double y_center, double x_offset, double y_offset,
		std::string type1, std::string name1, std::string type2, std::string name2);
	static void pushSymmetricPowerups_DiagBackwardSlash_Alternate(double x_center, double y_center, double x_offset, double y_offset,
		const std::string* types1, const std::string* names1, int count1, const std::string* types2, const std::string* names2, int count2);

	//TODO: powerup push but from a list of random powers

	static void pushSymmetricWalls_LR(double x_center, double y_center, double x_offset, double wallWidth, double wallHeight, const ColorValueHolder& c);
	static void pushSymmetricWalls_UD(double x_center, double y_center, double y_offset, double wallWidth, double wallHeight, const ColorValueHolder& c);
	static void pushSymmetricWalls_Corners(double x_center, double y_center, double x_offset, double y_offset, double wallWidth, double wallHeight, const ColorValueHolder& c);
	static void pushSymmetricWalls_DiagForwardSlash(double x_center, double y_center, double x_offset, double y_offset, double wallWidth, double wallHeight, const ColorValueHolder& c);
	static void pushSymmetricWalls_DiagBackwardSlash(double x_center, double y_center, double x_offset, double y_offset, double wallWidth, double wallHeight, const ColorValueHolder& c);



	//manual helper functions (mainly for hazards):
	static Wall* makeNewRandomWall(double x_beginning, double y_beginning, double width_ofArea, double height_ofArea, const ColorValueHolder& c, double minW, double maxW, double minH, double maxH);
	static inline Wall* makeNewRandomWall(double x_beginning, double y_beginning, double width_ofArea, double height_ofArea, const ColorValueHolder& c) {
		return LevelHelper::makeNewRandomWall(x_beginning, y_beginning, width_ofArea, height_ofArea, c, 12, 64, 8, 96);
	}
	[[nodiscard]] static PositionHolder* getClassicWallPositions();

	static std::string* getRandomPowers(int count, std::string type); //handles stacking and weights for a whole group (use this)
	static std::string* getRandomPowers(int count, std::string type, const std::string* names, int nameCount); //no option for multiple types (it's effort)
	static std::string* getRandomPowersOld(int count, bool replacement, const std::string* names, int nameCount); //equal weight, replacement is an option
	static std::string* getRandomPowers(int count, const bool* powersCanStack, const std::string* names, int nameCount); //equal weight, replaces the non-stacking names
	static std::string* getRandomPowers(int count, const bool* powersCanStack, const std::string* names, const float* weights, int powerCount); //nonequal weight, replaces the non-stacking names
	static std::string powerAlternate(int position, int rand, std::string p1, std::string p2); //position: {0,1,2,3}, rand: {0,1}
	static std::string simplePowerAlternate(int position, int rand, std::string p1, std::string p2); //position: {0,1}, rand: {0,1}
	static int powerAlternateNum(int position, int rand); //position: {0,1,2,3}, rand: {0,1}
	static int simplePowerAlternateNum(int position, int rand); //position: {0,1}, rand: {0,1}

	static PositionHolder getSymmetricPowerupPositions_LR(int position, double x_center, double y_center, double x_offset); //position: {0,1}
	static PositionHolder getSymmetricPowerupPositions_UD(int position, double x_center, double y_center, double y_offset); //position: {0,1}
	static PositionHolder getSymmetricPowerupPositions_Corners(int position, double x_center, double y_center, double x_offset, double y_offset); //position: {0,1,2,3}
	static PositionHolder getSymmetricPowerupPositions_DiagForwardSlash(int position, double x_center, double y_center, double x_offset, double y_offset); //position: {0,1}
	static PositionHolder getSymmetricPowerupPositions_DiagBackwardSlash(int position, double x_center, double y_center, double x_offset, double y_offset); //position: {0,1}
	//about _DiagForwardSlash and _DiagBackwardSlash: it's half of _Corners; uses the diagonal two that go in the direction of a forward slash (/) or backslash (\)

	static PositionHolder getSymmetricWallPositions_LR(int position, double x_center, double y_center, double x_offset, double wallWidth, double wallHeight); //position: {0,1}
	static PositionHolder getSymmetricWallPositions_UD(int position, double x_center, double y_center, double y_offset, double wallWidth, double wallHeight); //position: {0,1}
	static PositionHolder getSymmetricWallPositions_Corners(int position, double x_center, double y_center, double x_offset, double y_offset, double wallWidth, double wallHeight); //position: {0,1,2,3}
	static PositionHolder getSymmetricWallPositions_DiagForwardSlash(int position, double x_center, double y_center, double x_offset, double y_offset, double wallWidth, double wallHeight); //position: {0,1}
	static PositionHolder getSymmetricWallPositions_DiagBackwardSlash(int position, double x_center, double y_center, double x_offset, double y_offset, double wallWidth, double wallHeight); //position: {0,1}

private:
	LevelHelper() = delete;
	LevelHelper(const LevelHelper&) = delete;
};
