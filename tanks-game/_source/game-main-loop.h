#pragma once
#include <string>
#include <vector>
#include <unordered_map>

#include "game-scene.h"
#include "game-thing.h" //for DrawingLayers

#include "tank.h"
#include "bullet.h"
#include "wall.h"
#include "circle-hazard.h"
#include "rect-hazard.h"

struct TankInputChar {
protected:
	std::string key;
public:
	std::string getKey() const { return key; }
	bool getKeyState() const;
	TankInputChar(std::string);
	TankInputChar();
};

class GameMainLoop : public GameScene {
	friend class DeveloperManager;
	friend class GameManager; //needed?

protected:
	int waitCount;
	int maxWaitCount;

public: //only for ResetThings
	//forward, turnL, turnR, shooting, specialKey
	TankInputChar tank1Inputs[5];
	TankInputChar tank2Inputs[5];

public:
	GameMainLoop();
	void Tick() override;
	void Draw() const override { drawMain(); }

	//tick stuff:
	void levelTick();
	void moveTanks();
	void tankToPowerup();
	void tickHazards();
	void moveBullets();
	void tankShoot();
	void tankPowerTickAndCalculate();
	void bulletPowerTick();
	void tankToEdge();
	void bulletToEdge();

	void everythingToEverything();
	void everythingToEverything_tank_tank(int i, int j, std::unordered_map<Game_ID, TankUpdateStruct>& tankUpdates);
	void everythingToEverything_tank_wall(int i, int j, std::unordered_map<Game_ID, TankUpdateStruct>& tankUpdates, std::vector<Game_ID>& wallDeletionList, std::unordered_map<Game_ID, WallUpdateStruct>& wallUpdates);
	void everythingToEverything_tank_circlehazard(int i, int j, std::unordered_map<Game_ID, TankUpdateStruct>& tankUpdates, std::vector<Game_ID>& circleHazardDeletionList, std::unordered_map<Game_ID, CircleHazardUpdateStruct>& circleHazardUpdates);
	void everythingToEverything_tank_recthazard(int i, int j, std::unordered_map<Game_ID, TankUpdateStruct>& tankUpdates, std::vector<Game_ID>& rectHazardDeletionList, std::unordered_map<Game_ID, RectHazardUpdateStruct>& rectHazardUpdates);
	void everythingToEverything_bullet_tank(int i, int j, std::vector<Game_ID>& bulletDeletionList, std::unordered_map<Game_ID, BulletUpdateStruct>& bulletUpdates, std::unordered_map<Game_ID, TankUpdateStruct>& tankUpdates);
	void everythingToEverything_bullet_bullet(int i, int j, std::vector<Game_ID>& bulletDeletionList, std::unordered_map<Game_ID, BulletUpdateStruct>& bulletUpdates);
	void everythingToEverything_bullet_wall(int i, int j, std::vector<Game_ID>& bulletDeletionList, std::unordered_map<Game_ID, BulletUpdateStruct>& bulletUpdates, std::vector<Game_ID>& wallDeletionList, std::unordered_map<Game_ID, WallUpdateStruct>& wallUpdates);
	void everythingToEverything_bullet_circlehazard(int i, int j, std::vector<Game_ID>& bulletDeletionList, std::unordered_map<Game_ID, BulletUpdateStruct>& bulletUpdates, std::vector<Game_ID>& circleHazardDeletionList, std::unordered_map<Game_ID, CircleHazardUpdateStruct>& circleHazardUpdates);
	void everythingToEverything_bullet_recthazard(int i, int j, std::vector<Game_ID>& bulletDeletionList, std::unordered_map<Game_ID, BulletUpdateStruct>& bulletUpdates, std::vector<Game_ID>& rectHazardDeletionList, std::unordered_map<Game_ID, RectHazardUpdateStruct>& rectHazardUpdates);

	void drawMain() const; //doesn't draw all layers (since not everything uses everything)
	void drawAllLayers() const;
	void drawLayer(DrawingLayers) const;

private:
	GameMainLoop(const GameMainLoop&) = delete;
};
