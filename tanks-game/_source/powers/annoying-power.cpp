#include "annoying-power.h"
#include "../game-manager.h" //settings

std::unordered_map<std::string, float> AnnoyingPower::getWeights() const {
	std::unordered_map<std::string, float> weights;
	weights.insert({ "dev", 1.0f }); //common since its purpose is to show off game underworkings
	weights.insert({ "random-dev", 0.5f }); //... it's annoying, don't overuse it
	return weights;
}

TankPower* AnnoyingPower::makeTankPower() const {
	return new AnnoyingTankPower();
}

BulletPower* AnnoyingPower::makeBulletPower() const {
	return new AnnoyingBulletPower();
}

Power* AnnoyingPower::factory() {
	return new AnnoyingPower();
}

AnnoyingPower::AnnoyingPower() {
	return;
}



#include "../collision-handler.h"

void AnnoyingTankPower::initialize(Tank* parent) {
	parent->velocity.setMagnitude(0);
	//it's annoying
}

void AnnoyingTankPower::removeEffects(Tank* parent) {
	//nothing
}

BulletPower* AnnoyingTankPower::makeBulletPower() const {
	return new AnnoyingBulletPower();
}

InteractionBoolHolder AnnoyingTankPower::modifiedEdgeCollision(Tank* parent) {
	//this isn't annoying, I just wanted to do it
	//keep the tank in-bounds, but move "in-bounds" one tank diameter away
	CollisionHandler::edgeConstrain(parent->cir, parent->cir.getR() * -2);

	return { false };
}

float AnnoyingTankPower::getTankAccelerationMultiplier() const {
	//might change this
	return .5;
}

AnnoyingTankPower::AnnoyingTankPower() {
	const GameSettings& game_settings = GameManager::get_settings();
	maxTime = game_settings.PowerupDurationBaseTime;
	timeLeft = game_settings.PowerupDurationBaseTime;

	modifiesEdgeCollision = true;
}



#include "../collision-handler.h"

TankPower* AnnoyingBulletPower::makeTankPower() const {
	return new AnnoyingTankPower();
}

InteractionUpdateHolder<BulletUpdateStruct, TankUpdateStruct> AnnoyingBulletPower::modifiedCollisionWithTank(const Bullet* parent, const Tank* t) {
	std::pair<std::pair<double, double>, std::pair<double, double>> vecs = CollisionHandler::pushMovableAwayFromMovable_vecOnly(parent->cir, t->cir);
	return { false, false, new BulletUpdateStruct(vecs.first.first, vecs.first.second, 0,0,0), new TankUpdateStruct(vecs.second.first, vecs.second.second, 0,0) };
}

//has very low offense, very high defense; can't kill, can't be killed
//^^^ cancelled but might reinvestigate in the future (TODO)

float AnnoyingBulletPower::getOffenseImportance() const {
	return .5;
}

float AnnoyingBulletPower::getOffenseTier(const Bullet*) const {
	return 0; //-1? //not sure
}

float AnnoyingBulletPower::getDefenseImportance() const {
	return 0; //.5?
}

float AnnoyingBulletPower::getDefenseTier(const Bullet*) const {
	return 0; //HIGH_TIER? //should be <= 0 so other bullets can kill it... right?
}

AnnoyingBulletPower::AnnoyingBulletPower() {
	modifiesCollisionWithTank = true;
	//modifiedCollisionWithTankCanWorkWithOthers = false; //TODO: should this be set?
}
