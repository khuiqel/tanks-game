#include "game-main-loop.h"

//STL stuff:
#include <stdexcept>
#include <string>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <iostream>

//some stuff:
#include "renderer.h"
#include "reset-things.h"
#include "keypress-manager.h"
//other:
#include "background-rect.h"
#include "frame-time-graph.h"
#include "color-cache.h"

//managers:
#include "game-manager.h"
#include "tank-manager.h"
#include "bullet-manager.h"
#include "powerup-manager.h"
#include "wall-manager.h"
#include "level-manager.h"
#include "hazard-manager.h"

//classes with important handling functions:
#include "collision-handler.h"
#include "end-game-handler.h"
#include "physics-handler.h"

#include <tracy_include.h>

void doThing() {
	return;
}

TankInputChar::TankInputChar(std::string key_input) {
	key = key_input;
}
TankInputChar::TankInputChar() {
	key = "`";
}

bool TankInputChar::getKeyState() const {
	return KeypressManager::getKeyState(key);
}

GameMainLoop::GameMainLoop() : GameScene() {
	waitCount = 0;
	maxWaitCount = 100;
}

void GameMainLoop::Tick() {
	ZoneScoped;
	if (EndGameHandler::shouldGameEnd()) {
		waitCount++;
		if (waitCount >= maxWaitCount) {
			waitCount = 0;
			ResetThings::reset();
		}
		FrameTimeGraph::pushGraphTime("tick", 0); //goes after ResetThings::reset() because a draw call will still happen after this
		return;
	}

	auto start = FrameTimeGraph::getTime();
	doThing();

	levelTick();

	moveTanks();
	tankToPowerup();
	tickHazards();

	moveBullets();

	tankShoot();
	tankPowerTickAndCalculate();
	bulletPowerTick();

	tankToEdge();
	bulletToEdge(); //TODO: it would be better to somehow put this in everythingToEverything

	GameManager::updateEveryAABB();
	everythingToEverything();
	//TODO: it's now impossible for a tank to shoot a bullet the exact frame an enemy bullet hits it and lives; worth the effort to adjust?
	//maybe bullet-tank collision pairs could be pushed to a list to check later; update everything, then check bullet-tank, then update again

	//finish up by incrementing the tick count
	GameManager::Tick();

	auto end = FrameTimeGraph::getTime();
	FrameTimeGraph::pushGraphTime("tick", FrameTimeGraph::getDiff(start, end));

	//std::cout << BulletManager::getNumBullets() << std::endl;
	//std::cout << "tick: " << FrameTimeGraph::getDiff(start, end) << "ms" << std::endl;
}

void GameMainLoop::everythingToEverything() {
	ZoneScoped;

	FrameMarkStart("broad phase");
	std::vector<std::vector<std::pair<int, int>>*> collisionLists = PhysicsHandler::sweepAndPrune(GameManager::getObjectCollisionList());
	FrameMarkEnd("broad phase");

	std::vector<Game_ID> bulletDeletionList;
	std::vector<Game_ID> wallDeletionList;
	std::vector<Game_ID> circleHazardDeletionList;
	std::vector<Game_ID> rectHazardDeletionList;

	std::unordered_map<Game_ID, TankUpdateStruct> tankUpdates;
	std::unordered_map<Game_ID, BulletUpdateStruct> bulletUpdates;
	std::unordered_map<Game_ID, WallUpdateStruct> wallUpdates;
	std::unordered_map<Game_ID, CircleHazardUpdateStruct> circleHazardUpdates;
	std::unordered_map<Game_ID, RectHazardUpdateStruct> rectHazardUpdates;

	//for multithreaded collision: probably have to add a "try to die" flag to the update structs, as killing is currently very sequential
	//what about a barrier powerup that also gives invincibility?... that's complicated, worry about it later
	//or how about a barrier that changes the tank's size?! that's even worse, because now the size change will change the resulting position
	//multithreaded RNG: currently thinking loop every object, give it a pointer to the start of its requested RNG (there's a list of like 10K RNG calls stored), increment that pointer by the number of RNG calls requested by the object
	//bulletpowers are still sequential, so that approach needs a mutex for each object, but even then it's still non-deterministic
	//as that is very complicated and needs a sizeable rewrite, that is a future problem to solve

	//TODO: things can get killed in these functions, thus the results can change depending on the number of threads and how the work is distributed
	//while I strive to make things as deterministic as possible, uh, oh well on that
	//not impossible, but it means splitting the kill event from the collision functions, which means a lot of code moving (not really rewriting)
	//that is a later problem, because it would need a re-architecture of the killing system, and may as well put that off until fully multithreaded collision
	//as a temporary solution, maybe make a "kill events" list, handle those afterwards
	//(note: technically this same problem could happen in previous versions, including the 2-phase system in v0.2.5 and the naïve way for everything before)

	FrameMarkStart("narrow phase");
	for (std::vector<std::pair<int, int>>* collisionList : collisionLists) {
		for (int i = 0; i < collisionList->size(); i++) {
			int collisionPairFirst = collisionList->data()[i].first;
			int collisionPairSecond = collisionList->data()[i].second;

			switch (GameManager::getObject(collisionPairFirst)->getObjectType()) {
				default: [[fallthrough]];
				case ObjectType::None:
					break;

				case ObjectType::Tank:
					switch (GameManager::getObject(collisionPairSecond)->getObjectType()) {
						default: [[fallthrough]];
						case ObjectType::None:
							break;
						case ObjectType::Tank:     everythingToEverything_tank_tank(collisionPairFirst, collisionPairSecond, tankUpdates);
							break;
						case ObjectType::Bullet:   everythingToEverything_bullet_tank(collisionPairSecond, collisionPairFirst, bulletDeletionList, bulletUpdates, tankUpdates);
							break;
						case ObjectType::Wall:     everythingToEverything_tank_wall(collisionPairFirst, collisionPairSecond, tankUpdates, wallDeletionList, wallUpdates);
							break;
						case ObjectType::Powerup:  //handled earlier
							break;
						case ObjectType::Hazard_C: everythingToEverything_tank_circlehazard(collisionPairFirst, collisionPairSecond, tankUpdates, circleHazardDeletionList, circleHazardUpdates);
							break;
						case ObjectType::Hazard_R: everythingToEverything_tank_recthazard(collisionPairFirst, collisionPairSecond, tankUpdates, rectHazardDeletionList, rectHazardUpdates);
							break;
					}
					break;

				case ObjectType::Bullet:
					switch (GameManager::getObject(collisionPairSecond)->getObjectType()) {
						default: [[fallthrough]];
						case ObjectType::None:
							break;
						case ObjectType::Tank:     everythingToEverything_bullet_tank(collisionPairFirst, collisionPairSecond, bulletDeletionList, bulletUpdates, tankUpdates);
							break;
						case ObjectType::Bullet:   everythingToEverything_bullet_bullet(collisionPairFirst, collisionPairSecond, bulletDeletionList, bulletUpdates);
							break;
						case ObjectType::Wall:     everythingToEverything_bullet_wall(collisionPairFirst, collisionPairSecond, bulletDeletionList, bulletUpdates, wallDeletionList, wallUpdates);
							break;
						case ObjectType::Powerup:
							break;
						case ObjectType::Hazard_C: everythingToEverything_bullet_circlehazard(collisionPairFirst, collisionPairSecond, bulletDeletionList, bulletUpdates, circleHazardDeletionList, circleHazardUpdates);
							break;
						case ObjectType::Hazard_R: everythingToEverything_bullet_recthazard(collisionPairFirst, collisionPairSecond, bulletDeletionList, bulletUpdates, rectHazardDeletionList, rectHazardUpdates);
							break;
					}
					break;

				case ObjectType::Wall:
					switch (GameManager::getObject(collisionPairSecond)->getObjectType()) {
						default: [[fallthrough]];
						case ObjectType::None:
							break;
						case ObjectType::Tank:     everythingToEverything_tank_wall(collisionPairSecond, collisionPairFirst, tankUpdates, wallDeletionList, wallUpdates);
							break;
						case ObjectType::Bullet:   everythingToEverything_bullet_wall(collisionPairSecond, collisionPairFirst, bulletDeletionList, bulletUpdates, wallDeletionList, wallUpdates);
							break;
						case ObjectType::Wall:     break;
						case ObjectType::Powerup:  break;
						case ObjectType::Hazard_C: break;
						case ObjectType::Hazard_R: break;
					}
					break;

				case ObjectType::Powerup:
					//nothing
					break;

				case ObjectType::Hazard_C:
					switch (GameManager::getObject(collisionPairSecond)->getObjectType()) {
						default: [[fallthrough]];
						case ObjectType::None:
							break;
						case ObjectType::Tank:     everythingToEverything_tank_circlehazard(collisionPairSecond, collisionPairFirst, tankUpdates, circleHazardDeletionList, circleHazardUpdates);
							break;
						case ObjectType::Bullet:   everythingToEverything_bullet_circlehazard(collisionPairSecond, collisionPairFirst, bulletDeletionList, bulletUpdates, circleHazardDeletionList, circleHazardUpdates);
							break;
						case ObjectType::Wall:     break;
						case ObjectType::Powerup:  break;
						case ObjectType::Hazard_C: break;
						case ObjectType::Hazard_R: break;
					}
					break;

				case ObjectType::Hazard_R:
					switch (GameManager::getObject(collisionPairSecond)->getObjectType()) {
						default: [[fallthrough]];
						case ObjectType::None:
							break;
						case ObjectType::Tank:     everythingToEverything_tank_recthazard(collisionPairSecond, collisionPairFirst, tankUpdates, rectHazardDeletionList, rectHazardUpdates);
							break;
						case ObjectType::Bullet:   everythingToEverything_bullet_recthazard(collisionPairSecond, collisionPairFirst, bulletDeletionList, bulletUpdates, rectHazardDeletionList, rectHazardUpdates);
							break;
						case ObjectType::Wall:     break;
						case ObjectType::Powerup:  break;
						case ObjectType::Hazard_C: break;
						case ObjectType::Hazard_R: break;
					}
					break;
			}
		}
	}
	FrameMarkEnd("narrow phase");

	//resolve the rest of collision

	//FrameMarkStart("resolve updates");
	for (auto i = tankUpdates.begin(); i != tankUpdates.end(); i++) {
		Tank* t = TankManager::getTankByID(i->first);
		t->update(&(i->second));
	}
	for (auto i = bulletUpdates.begin(); i != bulletUpdates.end(); i++) {
		Bullet* b = BulletManager::getBulletByID(i->first);
		b->update(&(i->second));
	}
	for (auto i = wallUpdates.begin(); i != wallUpdates.end(); i++) {
		Wall* w = WallManager::getWallByID(i->first);
		w->update(&(i->second));
	}
	for (auto i = circleHazardUpdates.begin(); i != circleHazardUpdates.end(); i++) {
		CircleHazard* ch = HazardManager::getCircleHazardByID(i->first);
		ch->update(&(i->second));
	}
	for (auto i = rectHazardUpdates.begin(); i != rectHazardUpdates.end(); i++) {
		RectHazard* rh = HazardManager::getRectHazardByID(i->first);
		rh->update(&(i->second));
	}
	//FrameMarkEnd("resolve updates");

	//FrameMarkStart("resolve deletions");
	for (int i = bulletDeletionList.size() - 1; i >= 0; i--) {
		BulletManager::deleteBulletByID(bulletDeletionList[i]);
	}
	for (int i = wallDeletionList.size() - 1; i >= 0; i--) {
		WallManager::deleteWallByID(wallDeletionList[i]);
	}
	for (int i = circleHazardDeletionList.size() - 1; i >= 0; i--) {
		HazardManager::deleteCircleHazardByID(circleHazardDeletionList[i]);
	}
	for (int i = rectHazardDeletionList.size() - 1; i >= 0; i--) {
		HazardManager::deleteRectHazardByID(rectHazardDeletionList[i]);
	}
	//FrameMarkEnd("resolve deletions");
}

void GameMainLoop::everythingToEverything_tank_tank(int i, int j, std::unordered_map<Game_ID, TankUpdateStruct>& tankUpdates) {
	Tank* t_first = static_cast<Tank*>(GameManager::getObject(i));
	Tank* t_second = static_cast<Tank*>(GameManager::getObject(j));
	bool t_firstShouldDie = false;
	bool t_secondShouldDie = false;
	bool overridedTankCollision = false;

	if (CollisionHandler::partiallyCollided(t_first, t_second)) {
		for (int k = 0; k < t_first->tankPowers.size(); k++) {
			if (t_first->tankPowers[k]->modifiesCollisionWithTank) {
				if (t_first->tankPowers[k]->overridesCollisionWithTank) {
					overridedTankCollision = true;
				}

				InteractionUpdateHolder<TankUpdateStruct, TankUpdateStruct> check_temp = t_first->tankPowers[k]->modifiedCollisionWithTank(t_first, t_second);
				if (t_first->getTeamID() != t_second->getTeamID()) {
					if (check_temp.deaths.firstShouldDie) {
						t_firstShouldDie = true;
					}
					if (check_temp.deaths.secondShouldDie) {
						t_secondShouldDie = true;
					}
				}

				if (check_temp.firstUpdate != nullptr) {
					if (tankUpdates.find(t_first->getGameID()) == tankUpdates.end()) {
						tankUpdates.insert({ t_first->getGameID(), TankUpdateStruct(*check_temp.firstUpdate) });
					} else {
						tankUpdates[t_first->getGameID()].add(TankUpdateStruct(*check_temp.firstUpdate));
					}
				}
				if (check_temp.secondUpdate != nullptr) {
					if (tankUpdates.find(t_second->getGameID()) == tankUpdates.end()) {
						tankUpdates.insert({ t_second->getGameID(), TankUpdateStruct(*check_temp.secondUpdate) });
					} else {
						tankUpdates[t_second->getGameID()].add(TankUpdateStruct(*check_temp.secondUpdate));
					}
				}

				if (!t_first->tankPowers[k]->modifiedCollisionWithTankCanWorkWithOthers) {
					break;
				}
			}
		}

		for (int k = 0; k < t_second->tankPowers.size(); k++) {
			if (t_second->tankPowers[k]->modifiesCollisionWithTank) {
				if (t_second->tankPowers[k]->overridesCollisionWithTank) {
					overridedTankCollision = true;
				}

				InteractionUpdateHolder<TankUpdateStruct, TankUpdateStruct> check_temp = t_second->tankPowers[k]->modifiedCollisionWithTank(t_second, t_first);
				if (t_first->getTeamID() != t_second->getTeamID()) {
					if (check_temp.deaths.firstShouldDie) {
						t_secondShouldDie = true;
					}
					if (check_temp.deaths.secondShouldDie) {
						t_firstShouldDie = true;
					}
				}

				if (check_temp.firstUpdate != nullptr) {
					if (tankUpdates.find(t_second->getGameID()) == tankUpdates.end()) {
						tankUpdates.insert({ t_second->getGameID(), TankUpdateStruct(*check_temp.firstUpdate) });
					} else {
						tankUpdates[t_second->getGameID()].add(TankUpdateStruct(*check_temp.firstUpdate));
					}
				}
				if (check_temp.secondUpdate != nullptr) {
					if (tankUpdates.find(t_first->getGameID()) == tankUpdates.end()) {
						tankUpdates.insert({ t_first->getGameID(), TankUpdateStruct(*check_temp.secondUpdate) });
					} else {
						tankUpdates[t_first->getGameID()].add(TankUpdateStruct(*check_temp.secondUpdate));
					}
				}

				if (!t_second->tankPowers[k]->modifiedCollisionWithTankCanWorkWithOthers) {
					break;
				}
			}
		}

		if (!overridedTankCollision) {
			InteractionUpdateHolder<TankUpdateStruct, TankUpdateStruct> result = EndGameHandler::determineWinner(t_first, t_second, t_firstShouldDie, t_secondShouldDie);
			t_firstShouldDie = result.deaths.firstShouldDie;
			t_secondShouldDie = result.deaths.secondShouldDie;

			if (result.firstUpdate != nullptr) {
				if (tankUpdates.find(t_first->getGameID()) == tankUpdates.end()) {
					tankUpdates.insert({ t_first->getGameID(), TankUpdateStruct(*result.firstUpdate) });
				} else {
					tankUpdates[t_first->getGameID()].add(TankUpdateStruct(*result.firstUpdate));
				}
			}
			if (result.secondUpdate != nullptr) {
				if (tankUpdates.find(t_second->getGameID()) == tankUpdates.end()) {
					tankUpdates.insert({ t_second->getGameID(), TankUpdateStruct(*result.secondUpdate) });
				} else {
					tankUpdates[t_second->getGameID()].add(TankUpdateStruct(*result.secondUpdate));
				}
			}
		}

		if (t_firstShouldDie) {
			//TODO: proper implementation?
		}
		if (t_secondShouldDie) {
			//TODO: proper implementation?
		}
	}
}

void GameMainLoop::everythingToEverything_tank_wall(int i, int j, std::unordered_map<Game_ID, TankUpdateStruct>& tankUpdates, std::vector<Game_ID>& wallDeletionList, std::unordered_map<Game_ID, WallUpdateStruct>& wallUpdates) {
	Tank* t = static_cast<Tank*>(GameManager::getObject(i));
	Wall* w = static_cast<Wall*>(GameManager::getObject(j));
	bool killTank = false; //unlikely to be set
	bool killWall = false;
	bool overridedWallCollision = false;

	if (CollisionHandler::partiallyCollided(t, w)) {
		for (int k = 0; k < t->tankPowers.size(); k++) {
			if (t->tankPowers[k]->modifiesCollisionWithWall) {
				if (t->tankPowers[k]->overridesCollisionWithWall) {
					overridedWallCollision = true;
				}

				InteractionUpdateHolder<TankUpdateStruct, WallUpdateStruct> check_temp = t->tankPowers[k]->modifiedCollisionWithWall(t, w);
				if (check_temp.deaths.firstShouldDie) {
					killTank = true;
				}
				if (check_temp.deaths.secondShouldDie) {
					killWall = true;
				}

				if (check_temp.firstUpdate != nullptr) {
					if (tankUpdates.find(t->getGameID()) == tankUpdates.end()) {
						tankUpdates.insert({ t->getGameID(), TankUpdateStruct(*check_temp.firstUpdate) });
					} else {
						tankUpdates[t->getGameID()].add(TankUpdateStruct(*check_temp.firstUpdate));
					}
				}
				if (check_temp.secondUpdate != nullptr) {
					if (wallUpdates.find(w->getGameID()) == wallUpdates.end()) {
						wallUpdates.insert({ w->getGameID(), WallUpdateStruct(*check_temp.secondUpdate) });
					} else {
						wallUpdates[w->getGameID()].add(WallUpdateStruct(*check_temp.secondUpdate));
					}
				}

				if (!t->tankPowers[k]->modifiedEdgeCollisionCanWorkWithOthers) {
					break;
				}
			}
		}

		if (!overridedWallCollision) {
			InteractionUpdateHolder<TankUpdateStruct, WallUpdateStruct> result = EndGameHandler::determineWinner(t, w, killTank, killWall);
			if (result.deaths.firstShouldDie) {
				killTank = true;
			}
			if (result.deaths.secondShouldDie) {
				killWall = true;
			}

			if (result.firstUpdate != nullptr) {
				if (tankUpdates.find(t->getGameID()) == tankUpdates.end()) {
					tankUpdates.insert({ t->getGameID(), TankUpdateStruct(*result.firstUpdate) });
				} else {
					tankUpdates[t->getGameID()].add(TankUpdateStruct(*result.firstUpdate));
				}
			}
			if (result.secondUpdate != nullptr) {
				if (wallUpdates.find(w->getGameID()) == wallUpdates.end()) {
					wallUpdates.insert({ w->getGameID(), WallUpdateStruct(*result.secondUpdate) });
				} else {
					wallUpdates[w->getGameID()].add(WallUpdateStruct(*result.secondUpdate));
				}
			}
		}

		if (killTank) {
			//TODO: proper implementation?
		}
		if (killWall) {
			wallDeletionList.push_back(w->getGameID());
		}
	}
}

void GameMainLoop::everythingToEverything_tank_circlehazard(int i, int j, std::unordered_map<Game_ID, TankUpdateStruct>& tankUpdates, std::vector<Game_ID>& circleHazardDeletionList, std::unordered_map<Game_ID, CircleHazardUpdateStruct>& circleHazardUpdates) {
	Tank* t = static_cast<Tank*>(GameManager::getObject(i));
	CircleHazard* ch = static_cast<CircleHazard*>(GameManager::getObject(j));
	bool killTank = false;
	bool killCircleHazard = false;
	bool overridedCircleHazardCollision = false;

	if (CollisionHandler::partiallyCollided(t, ch)) {
		for (int k = 0; k < t->tankPowers.size(); k++) {
			if (t->tankPowers[k]->getModifiesCollisionWithCircleHazard(ch)) {
				if (t->tankPowers[k]->overridesCollisionWithCircleHazard) {
					overridedCircleHazardCollision = true;
				}

				InteractionUpdateHolder<TankUpdateStruct, CircleHazardUpdateStruct> check_temp = t->tankPowers[k]->modifiedCollisionWithCircleHazard(t, ch);
				if (check_temp.deaths.firstShouldDie) {
					killTank = true;
				}
				if (check_temp.deaths.secondShouldDie) {
					killCircleHazard = true;
				}

				if (check_temp.firstUpdate != nullptr) {
					if (tankUpdates.find(t->getGameID()) == tankUpdates.end()) {
						tankUpdates.insert({ t->getGameID(), TankUpdateStruct(*check_temp.firstUpdate) });
					} else {
						tankUpdates[t->getGameID()].add(TankUpdateStruct(*check_temp.firstUpdate));
					}
				}
				if (check_temp.secondUpdate != nullptr) {
					if (circleHazardUpdates.find(ch->getGameID()) == circleHazardUpdates.end()) {
						circleHazardUpdates.insert({ ch->getGameID(), CircleHazardUpdateStruct(*check_temp.secondUpdate) });
					} else {
						circleHazardUpdates[ch->getGameID()].add(CircleHazardUpdateStruct(*check_temp.secondUpdate));
					}
				}

				if (!t->tankPowers[k]->modifiedCollisionWithCircleHazardCanWorkWithOthers) {
					break;
				}
			}
		}

		if (!overridedCircleHazardCollision) {
			if (ch->actuallyCollided(t)) {
				InteractionUpdateHolder<TankUpdateStruct, CircleHazardUpdateStruct> result = EndGameHandler::determineWinner(t, ch, killTank, killCircleHazard);
				if (result.deaths.firstShouldDie) {
					killTank = true;
				}
				if (result.deaths.secondShouldDie) {
					killCircleHazard = true;
				}

				if (result.firstUpdate != nullptr) {
					if (tankUpdates.find(t->getGameID()) == tankUpdates.end()) {
						tankUpdates.insert({ t->getGameID(), TankUpdateStruct(*result.firstUpdate) });
					} else {
						tankUpdates[t->getGameID()].add(TankUpdateStruct(*result.firstUpdate));
					}
				}
				if (result.secondUpdate != nullptr) {
					if (circleHazardUpdates.find(ch->getGameID()) == circleHazardUpdates.end()) {
						circleHazardUpdates.insert({ ch->getGameID(), CircleHazardUpdateStruct(*result.secondUpdate) });
					} else {
						circleHazardUpdates[ch->getGameID()].add(CircleHazardUpdateStruct(*result.secondUpdate));
					}
				}
			}
		}

		if (killTank) {
			//TODO: proper implementation?
		}
		if (killCircleHazard) {
			circleHazardDeletionList.push_back(ch->getGameID());
		}
	}
}

void GameMainLoop::everythingToEverything_tank_recthazard(int i, int j, std::unordered_map<Game_ID, TankUpdateStruct>& tankUpdates, std::vector<Game_ID>& rectHazardDeletionList, std::unordered_map<Game_ID, RectHazardUpdateStruct>& rectHazardUpdates) {
	Tank* t = static_cast<Tank*>(GameManager::getObject(i));
	RectHazard* rh = static_cast<RectHazard*>(GameManager::getObject(j));
	bool killTank = false;
	bool killRectHazard = false;
	bool overridedRectHazardCollision = false;

	if (CollisionHandler::partiallyCollided(t, rh)) {
		for (int k = 0; k < t->tankPowers.size(); k++) {
			if (t->tankPowers[k]->getModifiesCollisionWithRectHazard(rh)) {
				if (t->tankPowers[k]->overridesCollisionWithRectHazard) {
					overridedRectHazardCollision = true;
				}

				InteractionUpdateHolder<TankUpdateStruct, RectHazardUpdateStruct> check_temp = t->tankPowers[k]->modifiedCollisionWithRectHazard(t, rh);
				if (check_temp.deaths.firstShouldDie) {
					killTank = true;
				}
				if (check_temp.deaths.secondShouldDie) {
					killRectHazard = true;
				}

				if (check_temp.firstUpdate != nullptr) {
					if (tankUpdates.find(t->getGameID()) == tankUpdates.end()) {
						tankUpdates.insert({ t->getGameID(), TankUpdateStruct(*check_temp.firstUpdate) });
					} else {
						tankUpdates[t->getGameID()].add(TankUpdateStruct(*check_temp.firstUpdate));
					}
				}
				if (check_temp.secondUpdate != nullptr) {
					if (rectHazardUpdates.find(rh->getGameID()) == rectHazardUpdates.end()) {
						rectHazardUpdates.insert({ rh->getGameID(), RectHazardUpdateStruct(*check_temp.secondUpdate) });
					} else {
						rectHazardUpdates[rh->getGameID()].add(RectHazardUpdateStruct(*check_temp.secondUpdate));
					}
				}

				if (!t->tankPowers[k]->modifiedCollisionWithRectHazardCanWorkWithOthers) {
					break;
				}
			}
		}

		if (!overridedRectHazardCollision) {
			if (rh->actuallyCollided(t)) {
				InteractionUpdateHolder<TankUpdateStruct, RectHazardUpdateStruct> result = EndGameHandler::determineWinner(t, rh, killTank, killRectHazard);
				if (result.deaths.firstShouldDie) {
					killTank = true;
				}
				if (result.deaths.secondShouldDie) {
					killRectHazard = true;
				}

				if (result.firstUpdate != nullptr) {
					if (tankUpdates.find(t->getGameID()) == tankUpdates.end()) {
						tankUpdates.insert({ t->getGameID(), TankUpdateStruct(*result.firstUpdate) });
					} else {
						tankUpdates[t->getGameID()].add(TankUpdateStruct(*result.firstUpdate));
					}
				}
				if (result.secondUpdate != nullptr) {
					if (rectHazardUpdates.find(rh->getGameID()) == rectHazardUpdates.end()) {
						rectHazardUpdates.insert({ rh->getGameID(), RectHazardUpdateStruct(*result.secondUpdate) });
					} else {
						rectHazardUpdates[rh->getGameID()].add(RectHazardUpdateStruct(*result.secondUpdate));
					}
				}
			}
		}

		if (killTank) {
			//TODO: proper implementation?
		}
		if (killRectHazard) {
			rectHazardDeletionList.push_back(rh->getGameID());
		}
	}
}

void GameMainLoop::everythingToEverything_bullet_tank(int i, int j, std::vector<Game_ID>& bulletDeletionList, std::unordered_map<Game_ID, BulletUpdateStruct>& bulletUpdates, std::unordered_map<Game_ID, TankUpdateStruct>& tankUpdates) {
	//bullets can have custom collision with tanks but not vice versa; does it really matter? probably not
	Bullet* b = static_cast<Bullet*>(GameManager::getObject(i));
	Tank* t = static_cast<Tank*>(GameManager::getObject(j));
	bool killBullet = false;
	bool killTank = false;
	bool overridedTankCollision = false;

	if (!b->canCollideWith(t)) {
		return;
	}
	if (CollisionHandler::partiallyCollided(b, t)) {
		for (int k = 0; k < b->bulletPowers.size(); k++) {
			if (b->bulletPowers[k]->modifiesCollisionWithTank) {
				if (b->bulletPowers[k]->overridesCollisionWithTank) {
					overridedTankCollision = true;
				}

				InteractionUpdateHolder<BulletUpdateStruct, TankUpdateStruct> check_temp = b->bulletPowers[k]->modifiedCollisionWithTank(b, t);
				if (check_temp.deaths.firstShouldDie) {
					killBullet = true;
				}
				if (check_temp.deaths.secondShouldDie) {
					killTank = true;
				}

				if (check_temp.firstUpdate != nullptr) {
					if (bulletUpdates.find(b->getGameID()) == bulletUpdates.end()) {
						bulletUpdates.insert({ b->getGameID(), BulletUpdateStruct(*check_temp.firstUpdate) });
					} else {
						bulletUpdates[b->getGameID()].add(BulletUpdateStruct(*check_temp.firstUpdate));
					}
				}
				if (check_temp.secondUpdate != nullptr) {
					if (tankUpdates.find(t->getGameID()) == tankUpdates.end()) {
						tankUpdates.insert({ t->getGameID(), TankUpdateStruct(*check_temp.secondUpdate) });
					} else {
						tankUpdates[t->getGameID()].add(TankUpdateStruct(*check_temp.secondUpdate));
					}
				}

				if (!b->bulletPowers[k]->modifiedCollisionWithTankCanWorkWithOthers) {
					break;
				}
			}
		}

		if (!overridedTankCollision) {
			InteractionBoolHolder result = EndGameHandler::determineWinner(t, b, killTank, killBullet);
			//note: the tank and bullet inputs are switched (compared to above)
			if (result.firstShouldDie) {
				killTank = true;
			}
			if (result.secondShouldDie) {
				killBullet = true;
			}
		}

		if (killBullet) {
			bulletDeletionList.push_back(b->getGameID());
		}
		if (killTank) {
			//TODO: proper implementation?
		}
	}
}

void GameMainLoop::everythingToEverything_bullet_bullet(int i, int j, std::vector<Game_ID>& bulletDeletionList, std::unordered_map<Game_ID, BulletUpdateStruct>& bulletUpdates) {
	//bullets do not get custom collision with other bullets
	Bullet* b_first = static_cast<Bullet*>(GameManager::getObject(i));
	Bullet* b_second = static_cast<Bullet*>(GameManager::getObject(j));
	bool b_firstShouldDie = false;
	bool b_secondShouldDie = false;

	if (!b_first->canCollideWith(b_second)) {
		//TODO: doing this check in the narrow phase actually kinda improves performance, but memory bandwidth is still the limiting factor; keep the check here for simplicity
		return;
	}
	if (CollisionHandler::partiallyCollided(b_first, b_second)) {
		InteractionBoolHolder result = EndGameHandler::determineWinner(b_first, b_second, b_firstShouldDie, b_secondShouldDie);
		b_firstShouldDie = result.firstShouldDie;
		b_secondShouldDie = result.secondShouldDie;

		if (b_firstShouldDie) {
			bulletDeletionList.push_back(b_first->getGameID());
		}
		if (b_secondShouldDie) {
			bulletDeletionList.push_back(b_second->getGameID());
		}
	}
}

void GameMainLoop::everythingToEverything_bullet_wall(int i, int j, std::vector<Game_ID>& bulletDeletionList, std::unordered_map<Game_ID, BulletUpdateStruct>& bulletUpdates, std::vector<Game_ID>& wallDeletionList, std::unordered_map<Game_ID, WallUpdateStruct>& wallUpdates) {
	Bullet* b = static_cast<Bullet*>(GameManager::getObject(i));
	Wall* w = static_cast<Wall*>(GameManager::getObject(j));
	bool killBullet = false;
	bool killWall = false;
	bool overridedWallCollision = false;

	//TODO: ignore edge? bounce is basically the only thing that might care, for blast it's just unnecessary computation
	//idea: bulletpower can set wall collision mode (and hazard)
	if (CollisionHandler::partiallyCollided(b, w)) {
		for (int k = 0; k < b->bulletPowers.size(); k++) {
			if (b->bulletPowers[k]->modifiesCollisionWithWall) {
				if (b->bulletPowers[k]->overridesCollisionWithWall) {
					overridedWallCollision = true;
				}

				InteractionUpdateHolder<BulletUpdateStruct, WallUpdateStruct> check_temp = b->bulletPowers[k]->modifiedCollisionWithWall(b, w);
				if (check_temp.deaths.firstShouldDie) {
					killBullet = true;
				}
				if (check_temp.deaths.secondShouldDie) {
					killWall = true;
				}

				if (check_temp.firstUpdate != nullptr) {
					if (bulletUpdates.find(b->getGameID()) == bulletUpdates.end()) {
						bulletUpdates.insert({ b->getGameID(), BulletUpdateStruct(*check_temp.firstUpdate) });
					} else {
						bulletUpdates[b->getGameID()].add(BulletUpdateStruct(*check_temp.firstUpdate));
					}
				}
				if (check_temp.secondUpdate != nullptr) {
					if (wallUpdates.find(w->getGameID()) == wallUpdates.end()) {
						wallUpdates.insert({ w->getGameID(), WallUpdateStruct(*check_temp.secondUpdate) });
					} else {
						wallUpdates[w->getGameID()].add(WallUpdateStruct(*check_temp.secondUpdate));
					}
				}

				if (!b->bulletPowers[k]->modifiedCollisionWithWallCanWorkWithOthers) {
					break;
				}
			}
		}

		if (!overridedWallCollision) {
			if (CollisionHandler::partiallyCollided(b, w)) {
				killBullet = true;
			}
		}

		if (killBullet) {
			bulletDeletionList.push_back(b->getGameID());
		}
		if (killWall) {
			wallDeletionList.push_back(w->getGameID());
		}
	}
}

void GameMainLoop::everythingToEverything_bullet_circlehazard(int i, int j, std::vector<Game_ID>& bulletDeletionList, std::unordered_map<Game_ID, BulletUpdateStruct>& bulletUpdates, std::vector<Game_ID>& circleHazardDeletionList, std::unordered_map<Game_ID, CircleHazardUpdateStruct>& circleHazardUpdates) {
	Bullet* b = static_cast<Bullet*>(GameManager::getObject(i));
	CircleHazard* ch = static_cast<CircleHazard*>(GameManager::getObject(j));
	bool killBullet = false;
	bool killCircleHazard = false;
	bool overridedCircleHazardCollision = false;

	if (!b->canCollideWith(ch)) {
		return;
	}
	if (CollisionHandler::partiallyCollided(b, ch)) {
		for (int k = 0; k < b->bulletPowers.size(); k++) {
			if (b->bulletPowers[k]->getModifiesCollisionWithCircleHazard(ch)) {
				if (b->bulletPowers[k]->overridesCollisionWithCircleHazard) {
					overridedCircleHazardCollision = true;
				}

				InteractionUpdateHolder<BulletUpdateStruct, CircleHazardUpdateStruct> check_temp = b->bulletPowers[k]->modifiedCollisionWithCircleHazard(b, ch);
				if (check_temp.deaths.firstShouldDie) {
					killBullet = true;
				}
				if (check_temp.deaths.secondShouldDie) {
					killCircleHazard = true;
				}

				if (check_temp.firstUpdate != nullptr) {
					if (bulletUpdates.find(b->getGameID()) == bulletUpdates.end()) {
						bulletUpdates.insert({ b->getGameID(), BulletUpdateStruct(*check_temp.firstUpdate) });
					} else {
						bulletUpdates[b->getGameID()].add(BulletUpdateStruct(*check_temp.firstUpdate));
					}
				}
				if (check_temp.secondUpdate != nullptr) {
					if (circleHazardUpdates.find(ch->getGameID()) == circleHazardUpdates.end()) {
						circleHazardUpdates.insert({ ch->getGameID(), CircleHazardUpdateStruct(*check_temp.secondUpdate) });
					} else {
						circleHazardUpdates[ch->getGameID()].add(CircleHazardUpdateStruct(*check_temp.secondUpdate));
					}
				}

				if (!b->bulletPowers[k]->modifiedCollisionWithCircleHazardCanWorkWithOthers) {
					break;
				}
			}
		}

		if (!overridedCircleHazardCollision) {
			if (ch->actuallyCollided(b)) {
				InteractionUpdateHolder<BulletUpdateStruct, CircleHazardUpdateStruct> result = EndGameHandler::determineWinner(b, ch, killBullet, killCircleHazard);
				if (result.deaths.firstShouldDie) {
					killBullet = true;
				}
				if (result.deaths.secondShouldDie) {
					killCircleHazard = true;
				}

				if (result.firstUpdate != nullptr) {
					if (bulletUpdates.find(b->getGameID()) == bulletUpdates.end()) {
						bulletUpdates.insert({ b->getGameID(), BulletUpdateStruct(*result.firstUpdate) });
					} else {
						bulletUpdates[b->getGameID()].add(BulletUpdateStruct(*result.firstUpdate));
					}
				}
				if (result.secondUpdate != nullptr) {
					if (circleHazardUpdates.find(ch->getGameID()) == circleHazardUpdates.end()) {
						circleHazardUpdates.insert({ ch->getGameID(), CircleHazardUpdateStruct(*result.secondUpdate) });
					} else {
						circleHazardUpdates[ch->getGameID()].add(CircleHazardUpdateStruct(*result.secondUpdate));
					}
				}
			}
		}

		if (killBullet) {
			bulletDeletionList.push_back(b->getGameID());
		}
		if (killCircleHazard) {
			circleHazardDeletionList.push_back(ch->getGameID());
		}
	}
}

void GameMainLoop::everythingToEverything_bullet_recthazard(int i, int j, std::vector<Game_ID>& bulletDeletionList, std::unordered_map<Game_ID, BulletUpdateStruct>& bulletUpdates, std::vector<Game_ID>& rectHazardDeletionList, std::unordered_map<Game_ID, RectHazardUpdateStruct>& rectHazardUpdates) {
	Bullet* b = static_cast<Bullet*>(GameManager::getObject(i));
	RectHazard* rh = static_cast<RectHazard*>(GameManager::getObject(j));
	bool killBullet = false;
	bool killRectHazard = false;
	bool overridedRectHazardCollision = false;

	if (!b->canCollideWith(rh)) {
		return;
	}
	if (CollisionHandler::partiallyCollided(b, rh)) {
		for (int k = 0; k < b->bulletPowers.size(); k++) {
			if (b->bulletPowers[k]->getModifiesCollisionWithRectHazard(rh)) {
				if (b->bulletPowers[k]->overridesCollisionWithRectHazard) {
					overridedRectHazardCollision = true;
				}

				InteractionUpdateHolder<BulletUpdateStruct, RectHazardUpdateStruct> check_temp = b->bulletPowers[k]->modifiedCollisionWithRectHazard(b, rh);
				if (check_temp.deaths.firstShouldDie) {
					killBullet = true;
				}
				if (check_temp.deaths.secondShouldDie) {
					killRectHazard = true;
				}

				if (check_temp.firstUpdate != nullptr) {
					if (bulletUpdates.find(b->getGameID()) == bulletUpdates.end()) {
						bulletUpdates.insert({ b->getGameID(), BulletUpdateStruct(*check_temp.firstUpdate) });
					} else {
						bulletUpdates[b->getGameID()].add(BulletUpdateStruct(*check_temp.firstUpdate));
					}
				}
				if (check_temp.secondUpdate != nullptr) {
					if (rectHazardUpdates.find(rh->getGameID()) == rectHazardUpdates.end()) {
						rectHazardUpdates.insert({ rh->getGameID(), RectHazardUpdateStruct(*check_temp.secondUpdate) });
					} else {
						rectHazardUpdates[rh->getGameID()].add(RectHazardUpdateStruct(*check_temp.secondUpdate));
					}
				}

				if (!b->bulletPowers[k]->modifiedCollisionWithRectHazardCanWorkWithOthers) {
					break;
				}
			}
		}

		if (!overridedRectHazardCollision) {
			if (rh->actuallyCollided(b)) {
				InteractionUpdateHolder<BulletUpdateStruct, RectHazardUpdateStruct> result = EndGameHandler::determineWinner(b, rh, killBullet, killRectHazard);
				if (result.deaths.firstShouldDie) {
					killBullet = true;
				}
				if (result.deaths.secondShouldDie) {
					killRectHazard = true;
				}

				if (result.firstUpdate != nullptr) {
					if (bulletUpdates.find(b->getGameID()) == bulletUpdates.end()) {
						bulletUpdates.insert({ b->getGameID(), BulletUpdateStruct(*result.firstUpdate) });
					} else {
						bulletUpdates[b->getGameID()].add(BulletUpdateStruct(*result.firstUpdate));
					}
				}
				if (result.secondUpdate != nullptr) {
					if (rectHazardUpdates.find(rh->getGameID()) == rectHazardUpdates.end()) {
						rectHazardUpdates.insert({ rh->getGameID(), RectHazardUpdateStruct(*result.secondUpdate) });
					} else {
						rectHazardUpdates[rh->getGameID()].add(RectHazardUpdateStruct(*result.secondUpdate));
					}
				}
			}
		}

		if (killBullet) {
			bulletDeletionList.push_back(b->getGameID());
		}
		if (killRectHazard) {
			rectHazardDeletionList.push_back(rh->getGameID());
		}
	}
}

void GameMainLoop::levelTick() {
	for (int i = 0; i < LevelManager::getNumLevels(); i++) {
		LevelManager::getLevel(i)->tickLevelEffects();
	}
	for (int i = 0; i < LevelManager::getNumLevels(); i++) {
		LevelManager::getLevel(i)->doLevelEffects();
		LevelManager::getLevel(i)->tick();
	}
}

void GameMainLoop::moveTanks() {
	/*
	for (int i = 0; i < TankManager::getNumTanks(); i++) {
		Tank* t = TankManager::getTank(i);
		bool shouldBeKilled = t->move();
		if (shouldBeKilled) {
			EndGameHandler::killTank(t);
		}
	}
	*/

	//TODO: make this a loop (not done right now 'cuz I don't want to do more arrays than I absolutely have to)
	Tank* t = TankManager::getTank(0);
	bool shouldBeKilled = t->move(tank1Inputs[0].getKeyState(), tank1Inputs[1].getKeyState(), tank1Inputs[2].getKeyState(), tank1Inputs[4].getKeyState());
	if (shouldBeKilled) {
		EndGameHandler::killTank(t);
	}
	t = TankManager::getTank(1);
	shouldBeKilled = t->move(tank2Inputs[0].getKeyState(), tank2Inputs[1].getKeyState(), tank2Inputs[2].getKeyState(), tank2Inputs[4].getKeyState());
	if (shouldBeKilled) {
		EndGameHandler::killTank(t);
	}
}

void GameMainLoop::tankToPowerup() {
	//don't separate into broad/narrow phases, it's not necessary
	//if there were more than two tanks, then yes it might be necessary (which would kinda require another update AABBs and S&P for the rest of collision)

	std::vector<int> powerupDeletionList;
	for (int i = 0; i < TankManager::getNumTanks(); i++) {
		Tank* t = TankManager::getTank(i);
		for (int j = 0; j < PowerupManager::getNumPowerups(); j++) {
			PowerSquare* p = PowerupManager::getPowerup(j);
			if (CollisionHandler::partiallyCollided(p, t)) {
				p->givePower(t);
				powerupDeletionList.push_back(j);
			}
		}
	}

	//clear deaths
	for (int i = powerupDeletionList.size() - 1; i >= 0; i--) {
		PowerupManager::deletePowerup(powerupDeletionList[i]);
	}
}

void GameMainLoop::tickHazards() {
	for (int i = 0; i < HazardManager::getNumCircleHazards(); i++) {
		HazardManager::getCircleHazard(i)->tick();
	}
	for (int i = 0; i < HazardManager::getNumRectHazards(); i++) {
		HazardManager::getRectHazard(i)->tick();
	}
}

void GameMainLoop::moveBullets() {
	ZoneScoped;
	for (int i = BulletManager::getNumBullets() - 1; i >= 0; i--) {
		Bullet* b = BulletManager::getBullet(i);
		bool shouldBeKilled = b->move();
		if (shouldBeKilled) [[unlikely]] {
			EndGameHandler::killBullet(b);
			BulletManager::deleteBullet(i);
			continue;
			//TODO: should all the actual deletions go at the end of everything?
		}
	}

	BulletManager::forceLimitBullets();
}

void GameMainLoop::tankPowerTickAndCalculate() {
	for (int i = 0; i < TankManager::getNumTanks(); i++) {
		TankManager::getTank(i)->powerTickAndCalculate();
	}
}

void GameMainLoop::bulletPowerTick() {
	ZoneScoped;
	for (int i = BulletManager::getNumBullets() - 1; i >= 0; i--) {
		Bullet* b = BulletManager::getBullet(i);
		b->powerTick();
		if (b->isDead()) [[unlikely]] {
			//what power would do this?
			BulletManager::deleteBullet(i);
			continue;
		}
	}
}

void GameMainLoop::tankShoot() {
	const GameSettings& game_settings = GameManager::get_settings();

	/*
	for (int i = 0; i < TankManager::getNumTanks(); i++) {
		TankManager::getTank(i)->shoot();
	}
	*/

	//TODO: loop
	TankManager::getTank(0)->shoot(game_settings.AlwaysShootingMode || tank1Inputs[3].getKeyState());
	TankManager::getTank(1)->shoot(game_settings.AlwaysShootingMode || tank2Inputs[3].getKeyState());
}

void GameMainLoop::tankToEdge() {
	for (int i = 0; i < TankManager::getNumTanks(); i++) {
		Tank* t = TankManager::getTank(i);
		bool shouldBeKilled = false;
		bool overridedEdgeCollision = false;

		if (CollisionHandler::partiallyOutOfBounds(t)) {
			for (int k = 0; k < t->tankPowers.size(); k++) {
				if (t->tankPowers[k]->modifiesEdgeCollision) {
					if (t->tankPowers[k]->overridesEdgeCollision) {
						overridedEdgeCollision = true;
					}

					InteractionBoolHolder check_temp = t->tankPowers[k]->modifiedEdgeCollision(t);
					if (check_temp.firstShouldDie) {
						shouldBeKilled = true;
					}

					if (!t->tankPowers[k]->modifiedEdgeCollisionCanWorkWithOthers) {
						break;
					}
				}
			}

			if (!overridedEdgeCollision) {
				CollisionHandler::edgeConstrain(t);
			}

			if (shouldBeKilled) {
				//TODO: proper implementation?
			}
		}
	}
}

void GameMainLoop::bulletToEdge() {
	ZoneScoped;
	for (int i = BulletManager::getNumBullets() - 1; i >= 0; i--) {
		Bullet* b = BulletManager::getBullet(i);
		bool shouldBeKilled = false;
		bool overridedEdgeCollision = false;

		if (CollisionHandler::partiallyOutOfBounds(b)) {
			for (int k = 0; k < b->bulletPowers.size(); k++) {
				if (b->bulletPowers[k]->modifiesEdgeCollision) {
					if (b->bulletPowers[k]->overridesEdgeCollision) {
						overridedEdgeCollision = true;
					}

					InteractionBoolHolder check_temp = b->bulletPowers[k]->modifiedEdgeCollision(b);
					if (check_temp.firstShouldDie) {
						shouldBeKilled = true;
					}

					if (!b->bulletPowers[k]->modifiedEdgeCollisionCanWorkWithOthers) {
						break;
					}
				}
			}

			if (!overridedEdgeCollision) {
				if (CollisionHandler::fullyOutOfBounds(b)) {
					shouldBeKilled = true;
				}
			}

			if (shouldBeKilled) {
				BulletManager::deleteBullet(i);
				continue;
			}
		}
	}
}

void GameMainLoop::drawMain() const {
	ZoneScoped;
	auto start = FrameTimeGraph::getTime();
	Renderer::BeginScene("draw");

	//background rect
	BackgroundRect::draw();

	//hazards under
	for (int i = 0; i < HazardManager::getNumCircleHazards(); i++) {
		HazardManager::getCircleHazard(i)->draw(DrawingLayers::under);
	}
	for (int i = 0; i < HazardManager::getNumRectHazards(); i++) {
		HazardManager::getRectHazard(i)->draw(DrawingLayers::under);
	}

	//level under
	for (int i = 0; i < LevelManager::getNumLevels(); i++) {
		LevelManager::getLevel(i)->draw(DrawingLayers::under);
	}
	for (int i = 0; i < LevelManager::getNumLevels(); i++) {
		LevelManager::getLevel(i)->drawLevelEffects(DrawingLayers::under);
	}

	//powerups
	for (int i = 0; i < PowerupManager::getNumPowerups(); i++) {
		PowerupManager::getPowerup(i)->draw(DrawingLayers::normal);
	}

	//hazards
	for (int i = 0; i < HazardManager::getNumCircleHazards(); i++) {
		HazardManager::getCircleHazard(i)->draw(DrawingLayers::normal);
	}
	for (int i = 0; i < HazardManager::getNumRectHazards(); i++) {
		HazardManager::getRectHazard(i)->draw(DrawingLayers::normal);
	}

	//walls
	for (int i = 0; i < WallManager::getNumWalls(); i++) {
		WallManager::getWall(i)->draw(DrawingLayers::normal);
	}

	//bullets
	FrameMarkStart("bullet draw");
	ColorCacheBullet::invalidateCachedColors();
	for (int i = 0; i < BulletManager::getNumBullets(); i++) {
		BulletManager::getBullet(i)->draw(DrawingLayers::normal);
	}
	FrameMarkEnd("bullet draw");

	//drawing text on the GPU will need a library, so names don't get drawn anymore //TODO: https://github.com/nothings/stb
	/*
	for (int i = 0; i < tanks.size(); i++) {
		tanks[i]->drawName();
	}
	*/

	//tanks
	for (int i = 0; i < TankManager::getNumTanks(); i++) {
		TankManager::getTank(i)->draw(DrawingLayers::normal);
	}

	//hazards effects
	for (int i = 0; i < HazardManager::getNumCircleHazards(); i++) {
		HazardManager::getCircleHazard(i)->draw(DrawingLayers::effects);
	}
	for (int i = 0; i < HazardManager::getNumRectHazards(); i++) {
		HazardManager::getRectHazard(i)->draw(DrawingLayers::effects);
	}

	//level effects
	for (int i = 0; i < LevelManager::getNumLevels(); i++) {
		LevelManager::getLevel(i)->draw(DrawingLayers::effects);
	}
	for (int i = 0; i < LevelManager::getNumLevels(); i++) {
		LevelManager::getLevel(i)->drawLevelEffects(DrawingLayers::effects);
	}

	//tanks dead
	for (int i = 0; i < TankManager::getNumTanks(); i++) {
		TankManager::getTank(i)->draw(DrawingLayers::top);
	}

	auto end = FrameTimeGraph::getTime();
	FrameTimeGraph::pushGraphTime("upload", FrameTimeGraph::getDiff(start, end));
	Renderer::EndScene();

	const GameSettings& game_settings = GameManager::get_settings();
	if (game_settings.EnableDebugDrawing) [[unlikely]] {
		drawLayer(DrawingLayers::debug);
		//TODO: levels should be able to debug draw their starting tank positions (color: white?)
	}

	//end = FrameTimeGraph::getTime();

	//std::cout << "draw all: " << FrameTimeGraph::getDiff(start, end) << "ms" << std::endl << std::endl;
}

void GameMainLoop::drawLayer(DrawingLayers layer) const {
	Renderer::BeginScene("drawLayer" + std::to_string((int)layer));

	//powerups
	for (int i = 0; i < PowerupManager::getNumPowerups(); i++) {
		PowerupManager::getPowerup(i)->draw(layer);
	}

	//hazards
	for (int i = 0; i < HazardManager::getNumCircleHazards(); i++) {
		HazardManager::getCircleHazard(i)->draw(layer);
	}
	for (int i = 0; i < HazardManager::getNumRectHazards(); i++) {
		HazardManager::getRectHazard(i)->draw(layer);
	}

	//walls
	for (int i = 0; i < WallManager::getNumWalls(); i++) {
		WallManager::getWall(i)->draw(layer);
	}

	//bullets
	for (int i = 0; i < BulletManager::getNumBullets(); i++) {
		BulletManager::getBullet(i)->draw(layer);
	}

	//tanks
	for (int i = 0; i < TankManager::getNumTanks(); i++) {
		TankManager::getTank(i)->draw(layer);
	}

	//level
	for (int i = 0; i < LevelManager::getNumLevels(); i++) {
		LevelManager::getLevel(i)->draw(layer);
	}
	for (int i = 0; i < LevelManager::getNumLevels(); i++) {
		LevelManager::getLevel(i)->drawLevelEffects(layer);
	}

	Renderer::EndScene();
}

void GameMainLoop::drawAllLayers() const {
	drawLayer(DrawingLayers::under);
	drawLayer(DrawingLayers::normal);
	drawLayer(DrawingLayers::effects);
	drawLayer(DrawingLayers::top);
	//drawLayer(DrawingLayers::debug);
}
