#pragma once
#include <cstdint>
#include <numbers>

typedef int32_t Game_ID;
typedef int8_t Team_ID;

enum class ObjectType : uint8_t {
	None       = 0, //error
	Tank       = 1,
	Bullet     = 2,
	Wall       = 3,
	Powerup    = 4,
	Hazard_C   = 5,
	Hazard_R   = 6,
};

typedef int32_t Color_ID;

constexpr double PI = std::numbers::pi;
constexpr double GAME_WIDTH = 640;
constexpr double GAME_HEIGHT = 320;
constexpr double TANK_RADIUS = 16;
constexpr double SHOOT_COOLDOWN = 100;
constexpr double POWER_COOLDOWN = 500;

constexpr double BULLET_TO_TANK_RADIUS_RATIO = .25;
constexpr float  BULLET_TO_TANK_SPEED_RATIO = 4; //2 isn't that bad

constexpr float DESTRUCTION_TIER = 2; //offense tier for destroying walls and turrets & stuff
constexpr float LOW_TIER = -999;
constexpr float HIGH_TIER = 999;
constexpr float LOW_IMPORTANCE = -1;
constexpr float HIGH_IMPORTANCE = 999;

constexpr Team_ID DEFAULT_TEAM = 0; //walls and friends are this team
constexpr Team_ID HAZARD_TEAM = -1; //default team of hazards
constexpr Game_ID NO_PARENT = -1;   //bullets that don't have a parent for parentID

//typedef void(*voidFunction)(void); //this isn't used
//using voidFunction = void(*)(void); //C++11 version

//tank team rules:
//0  = no team or default team
//-1 = hazard team (hazards can be split up into multiple teams, but by default there's only one)
//1  = default teamID of tank #1 ("WASD")
//2  = default teamID of tank #2 ("Arrow Keys")
//tank teams always > 0
