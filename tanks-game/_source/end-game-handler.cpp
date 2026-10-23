#include "end-game-handler.h"

#include <iostream>

#include "collision-handler.h"
#include "priority-handler.h"
#include "tank-manager.h"

//TODO?: CircleHazard and RectHazard use CollisionHandler for their modified tank/bullet collision by default

std::unordered_map<Team_ID, int> EndGameHandler::teamWins;
std::vector<EndGameHandler::Team_IDAndString> EndGameHandler::teamsParticipating;
std::vector<EndGameHandler::DoubleTeam_ID> EndGameHandler::killEvents;

void EndGameHandler::addTeamToWatch(Team_ID teamID, std::string teamName) {
	if (teamID == DEFAULT_TEAM) {
		std::cerr << "WARNING: DEFAULT_TEAM is not allowed to have a score\n";
	} else {
		teamsParticipating.push_back({ teamID, teamName });
		teamWins.insert({ teamID, 0 });
	}
}

void EndGameHandler::clearWatchingTeams() {
	for (int i = 0; i < teamsParticipating.size(); i++) {
		teamWins.erase(teamsParticipating[i].teamID);
	}
	teamsParticipating.clear();
	killEvents.clear(); //just in case
}

void EndGameHandler::finalizeScores() {
	//TODO: GameManager holds information about objects and their IDs (and what teamIDs are being used?)
	//TODO: is this done? (will definitely update when multi-tank mode becomes a thing)

	//process kill events
	std::vector<EndGameHandler::DoubleTeam_ID> pastEvents; //in case tank dies from multiple bullets at the same time
	for (int i = 0; i < killEvents.size(); i++) {
		/*
		if (killEvents[i].killer == DEFAULT_TEAM) {
			continue;
		}
		*/
		bool previouslyHappened = false;
		for (int j = 0; j < pastEvents.size(); j++) {
			if ((killEvents[i].killer == pastEvents[j].killer) && (killEvents[i].killee == pastEvents[j].killee)) {
				previouslyHappened = true;
				break;
			}
		}
		if (!previouslyHappened) {
			if (killEvents[i].killer != killEvents[i].killee) { //friendly fire score increase prevention
				teamWins[killEvents[i].killer]++;
				pastEvents.push_back(killEvents[i]);
				//this can increase the score of DEFAULT_TEAM and other non-participating teams, but that's fine since their score isn't shown
			}
			//don't decrease score
		}
	}
	killEvents.clear();

	//TODO: if two tanks kill each other at the same time, then they'll both get a point; might change it so that can't happen, though I think the logic will be complicated when num of tanks > 2
	//other note: if hazard and tank kill other tank at same time, they both get a point; this is the intended behavior
	//quick note: there can be many hazards in the level, so the hazard team can get multiple points in one round (TODO: the logic can't deal with team mode, so fix that)
}

bool EndGameHandler::shouldGameEnd() {
	//TODO: account for two tanks on the same team (probably need to make GameManager keep track of stuff)
	int numOfAliveTanks = 0;
	for (int i = 0; i < TankManager::getNumTanks(); i++) {
		if (!TankManager::getTank(i)->dead) {
			numOfAliveTanks++;
		}
	}
	return (numOfAliveTanks <= 1);
}

void EndGameHandler::killTank(Tank* t) {
	t->kill_hard();
	killEvents.push_back({ DEFAULT_TEAM, t->getTeamID() }); //TODO: which team should get this point?
}

bool EndGameHandler::killTank(Tank* t, const GameThing* thing) {
	bool tankDies = t->kill();
	if (tankDies) {
		killEvents.push_back({ thing->getTeamID(), t->getTeamID() });
	}
	return tankDies;
}

void EndGameHandler::killBullet(Bullet* b) {
	//something?
}

bool EndGameHandler::killBullet(Bullet* b, const GameThing* thing) {
	bool bulletDies = b->kill();
	if (bulletDies) {
		//something?
		//maybe increase in bullet kills from a team, for statistics
	}
	return bulletDies;
}

InteractionBoolHolder EndGameHandler::determineWinner(const Tank* t, const Bullet* b, bool attemptTankDeath, bool attemptBulletDeath) {
	PriorityResult result = PriorityHandler::determinePriority(t, b);
	bool tankDies = attemptTankDeath, bulletDies = attemptBulletDeath;
	switch (result) {
		case PriorityResult::bothDie:
			tankDies = true;
			bulletDies = true;
			break;
		case PriorityResult::neitherDie:
			//nothing
			break;
		case PriorityResult::firstDies:
			tankDies = true;
			break;
		case PriorityResult::secondDies:
			bulletDies = true;
			break;
		default:
			std::cerr << "WARNING: unknown PriorityResult!\n";
			//neither die
			break;
	}
	if (bulletDies && !tankDies) {
		bulletDies = killBullet(const_cast<Bullet*>(b), t);
	}
	if (tankDies) {
		//if the bullet wants to kill the tank, the bullet must die
		bulletDies = true;
		tankDies = killTank(const_cast<Tank*>(t), b);
	}
	return { tankDies, bulletDies };
}

InteractionUpdateHolder<TankUpdateStruct, TankUpdateStruct> EndGameHandler::determineWinner(const Tank* t1, const Tank* t2, bool attemptTank1Death, bool attemptTank2Death) {
	PriorityResult result = PriorityHandler::determinePriority(t1, t2);
	bool firstTankDies = attemptTank1Death, secondTankDies = attemptTank2Death;
	std::pair<std::pair<double, double>, std::pair<double, double>> vecs = { {0,0}, {0,0} };
	switch (result) {
		case PriorityResult::bothDie:
			firstTankDies = true;
			secondTankDies = true;
			break;
		case PriorityResult::neitherDie: //normal result
			vecs = CollisionHandler::pushMovableAwayFromMovable_vecOnly(t1->cir, t2->cir);
			break;
		case PriorityResult::firstDies:
			firstTankDies = true;
			break;
		case PriorityResult::secondDies:
			secondTankDies = true;
			break;
		default:
			std::cerr << "WARNING: unknown PriorityResult!\n";
			//neither die
			break;
	}
	if (firstTankDies) {
		firstTankDies = killTank(const_cast<Tank*>(t1), t2);
	}
	if (secondTankDies) {
		secondTankDies = killTank(const_cast<Tank*>(t2), t1);
	}
	return { firstTankDies, secondTankDies, new TankUpdateStruct(vecs.first.first, vecs.first.second, 0,0), new TankUpdateStruct(vecs.second.first, vecs.second.second, 0,0) };
}

InteractionBoolHolder EndGameHandler::determineWinner(const Bullet* b1, const Bullet* b2, bool attemptBullet1Death, bool attemptBullet2Death) {
	PriorityResult result = PriorityHandler::determinePriority(b1, b2);
	bool firstBulletDies = attemptBullet1Death, secondBulletDies = attemptBullet2Death;
	switch (result) {
		case PriorityResult::bothDie:
			firstBulletDies = true;
			secondBulletDies = true;
			break;
		case PriorityResult::neitherDie:
			//nothing
			break;
		case PriorityResult::firstDies:
			firstBulletDies = true;
			break;
		case PriorityResult::secondDies:
			secondBulletDies = true;
			break;
		default:
			std::cerr << "WARNING: unknown PriorityResult!\n";
			//neither die
			break;
	}
	if (firstBulletDies) {
		firstBulletDies = killBullet(const_cast<Bullet*>(b1), b2);
	}
	if (secondBulletDies) {
		secondBulletDies = killBullet(const_cast<Bullet*>(b2), b1);
	}
	return { firstBulletDies, secondBulletDies };
}

InteractionUpdateHolder<TankUpdateStruct, CircleHazardUpdateStruct> EndGameHandler::determineWinner(const Tank* t, const CircleHazard* ch, bool attemptTankDeath, bool attemptCircleHazardDeath) {
	PriorityResult result = PriorityHandler::determinePriority(t, ch);
	bool tankDies = attemptTankDeath, circleHazardDies = attemptCircleHazardDeath;
	bool usedModifiedCollision = false;
	std::shared_ptr<TankUpdateStruct> update_t = nullptr;
	std::shared_ptr<CircleHazardUpdateStruct> update_ch = nullptr;
	std::pair<double, double> vec_t  = {0,0};
	switch (result) {
		case PriorityResult::bothDie:
			tankDies = true;
			circleHazardDies = true;
			break;
		case PriorityResult::neitherDie: {
			const_cast<CircleHazard*>(ch)->specialEffectTankCollision(t);
			InteractionUpdateHolder<TankUpdateStruct, CircleHazardUpdateStruct> modifiedCollisionResult = ch->modifiedTankCollision(t);
			if (modifiedCollisionResult.deaths.firstShouldDie) {
				tankDies = true;
			}
			if (modifiedCollisionResult.deaths.secondShouldDie) {
				circleHazardDies = true;
			}
			update_t = modifiedCollisionResult.firstUpdate;
			update_ch = modifiedCollisionResult.secondUpdate;
			usedModifiedCollision = true;
			} break;
		case PriorityResult::firstDies:
			const_cast<CircleHazard*>(ch)->specialEffectTankCollision(t);
			tankDies = true;
			break;
		case PriorityResult::secondDies:
			circleHazardDies = true;
			break;
		default:
			std::cerr << "WARNING: unknown PriorityResult!\n";
			//neither die
			break;
	}
	if (tankDies) {
		tankDies = killTank(const_cast<Tank*>(t), ch);
	}
	if (usedModifiedCollision) {
		return { tankDies, circleHazardDies, update_t, update_ch };
	} else {
		return { tankDies, circleHazardDies, new TankUpdateStruct(vec_t.first, vec_t.second, 0,0), nullptr };
	}
}

InteractionUpdateHolder<TankUpdateStruct, RectHazardUpdateStruct> EndGameHandler::determineWinner(const Tank* t, const RectHazard* rh, bool attemptTankDeath, bool attemptRectHazardDeath) {
	PriorityResult result = PriorityHandler::determinePriority(t, rh);
	bool tankDies = attemptTankDeath, rectHazardDies = attemptRectHazardDeath;
	bool usedModifiedCollision = false;
	std::shared_ptr<TankUpdateStruct> update_t = nullptr;
	std::shared_ptr<RectHazardUpdateStruct> update_rh = nullptr;
	std::pair<double, double> vec_t  = {0,0};
	switch (result) {
		case PriorityResult::bothDie:
			tankDies = true;
			rectHazardDies = true;
			break;
		case PriorityResult::neitherDie: {
			const_cast<RectHazard*>(rh)->specialEffectTankCollision(t);
			InteractionUpdateHolder<TankUpdateStruct, RectHazardUpdateStruct> modifiedCollisionResult = rh->modifiedTankCollision(t);
			if (modifiedCollisionResult.deaths.firstShouldDie) {
				tankDies = true;
			}
			if (modifiedCollisionResult.deaths.secondShouldDie) {
				rectHazardDies = true;
			}
			update_t = modifiedCollisionResult.firstUpdate;
			update_rh = modifiedCollisionResult.secondUpdate;
			usedModifiedCollision = true;
			} break;
		case PriorityResult::firstDies:
			const_cast<RectHazard*>(rh)->specialEffectTankCollision(t);
			tankDies = true;
			break;
		case PriorityResult::secondDies:
			rectHazardDies = true;
			break;
		default:
			std::cerr << "WARNING: unknown PriorityResult!\n";
			//neither die
			break;
	}
	if (tankDies) {
		tankDies = killTank(const_cast<Tank*>(t), rh);
	}
	if (usedModifiedCollision) {
		return { tankDies, rectHazardDies, update_t, update_rh };
	} else {
		return { tankDies, rectHazardDies, new TankUpdateStruct(vec_t.first, vec_t.second, 0,0), nullptr };
	}
}

InteractionUpdateHolder<BulletUpdateStruct, CircleHazardUpdateStruct> EndGameHandler::determineWinner(const Bullet* b, const CircleHazard* ch, bool attemptBulletDeath, bool attemptCircleHazardDeath) {
	PriorityResult result = PriorityHandler::determinePriority(b, ch);
	bool bulletDies = attemptBulletDeath, circleHazardDies = attemptCircleHazardDeath;
	bool usedModifiedCollision = false;
	std::shared_ptr<BulletUpdateStruct> update_b = nullptr;
	std::shared_ptr<CircleHazardUpdateStruct> update_ch = nullptr;
	std::pair<double, double> vec_b  = {0,0};
	switch (result) {
		case PriorityResult::bothDie:
			bulletDies = true;
			circleHazardDies = true;
			break;
		case PriorityResult::neitherDie: {
			const_cast<CircleHazard*>(ch)->specialEffectBulletCollision(b);
			InteractionUpdateHolder<BulletUpdateStruct, CircleHazardUpdateStruct> modifiedCollisionResult = ch->modifiedBulletCollision(b);
			if (modifiedCollisionResult.deaths.firstShouldDie) {
				bulletDies = true;
			}
			if (modifiedCollisionResult.deaths.secondShouldDie) {
				circleHazardDies = true;
			}
			update_b = modifiedCollisionResult.firstUpdate;
			update_ch = modifiedCollisionResult.secondUpdate;
			usedModifiedCollision = true;
			} break;
		case PriorityResult::firstDies:
			const_cast<CircleHazard*>(ch)->specialEffectBulletCollision(b);
			bulletDies = true;
			break;
		case PriorityResult::secondDies:
			circleHazardDies = true;
			break;
		default:
			std::cerr << "WARNING: unknown PriorityResult!\n";
			//neither die
			break;
	}
	if (bulletDies && (ch->getCollisionType() != CircleHazardCollisionType::solid)) {
		bulletDies = killBullet(const_cast<Bullet*>(b), ch);
	}
	if (usedModifiedCollision) {
		return { bulletDies, circleHazardDies, update_b, update_ch };
	} else {
		return { bulletDies, circleHazardDies, new BulletUpdateStruct(vec_b.first, vec_b.second, 0,0,0), nullptr };
	}
}

InteractionUpdateHolder<BulletUpdateStruct, RectHazardUpdateStruct> EndGameHandler::determineWinner(const Bullet* b, const RectHazard* rh, bool attemptBulletDeath, bool attemptRectHazardDeath) {
	PriorityResult result = PriorityHandler::determinePriority(b, rh);
	bool bulletDies = attemptBulletDeath, rectHazardDies = attemptRectHazardDeath;
	bool usedModifiedCollision = false;
	std::shared_ptr<BulletUpdateStruct> update_b = nullptr;
	std::shared_ptr<RectHazardUpdateStruct> update_rh = nullptr;
	std::pair<double, double> vec_b  = {0,0};
	switch (result) {
		case PriorityResult::bothDie:
			bulletDies = true;
			rectHazardDies = true;
			break;
		case PriorityResult::neitherDie: {
			const_cast<RectHazard*>(rh)->specialEffectBulletCollision(b);
			InteractionUpdateHolder<BulletUpdateStruct, RectHazardUpdateStruct> modifiedCollisionResult = rh->modifiedBulletCollision(b);
			if (modifiedCollisionResult.deaths.firstShouldDie) {
				bulletDies = true;
			}
			if (modifiedCollisionResult.deaths.secondShouldDie) {
				rectHazardDies = true;
			}
			update_b = modifiedCollisionResult.firstUpdate;
			update_rh = modifiedCollisionResult.secondUpdate;
			usedModifiedCollision = true;
			} break;
		case PriorityResult::firstDies:
			const_cast<RectHazard*>(rh)->specialEffectBulletCollision(b);
			bulletDies = true;
			break;
		case PriorityResult::secondDies:
			rectHazardDies = true;
			break;
		default:
			std::cerr << "WARNING: unknown PriorityResult!\n";
			//neither die
			break;
	}
	if (bulletDies && (rh->getCollisionType() != RectHazardCollisionType::solid)) {
		bulletDies = killBullet(const_cast<Bullet*>(b), rh);
	}
	if (usedModifiedCollision) {
		return { bulletDies, rectHazardDies, update_b, update_rh };
	} else {
		return { bulletDies, rectHazardDies, new BulletUpdateStruct(vec_b.first, vec_b.second, 0,0,0), nullptr };
	}
}

InteractionUpdateHolder<TankUpdateStruct, WallUpdateStruct> EndGameHandler::determineWinner(const Tank* t, const Wall* w, bool attemptTankDeath, bool attemptWallDeath) {
	std::pair<double, double> vec = CollisionHandler::pushMovableAwayFromImmovable_vecOnly(t->cir, w->rec);
	bool tankDies = attemptTankDeath;
	if (tankDies) {
		tankDies = killTank(const_cast<Tank*>(t), w);
	}
	return { tankDies, false, new TankUpdateStruct(vec.first, vec.second, 0,0), nullptr};
}
