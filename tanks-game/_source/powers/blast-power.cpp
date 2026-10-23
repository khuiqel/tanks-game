#include "blast-power.h"
#include "../game-manager.h" //settings

#include "../constants.h"

const float BlastPower::bulletAngleDeviation = PI/3;
const int BlastPower::bulletAmount = 16;

const float BlastPower::maxBulletAcceleration = 3/16.0;
const float BlastPower::minBulletAcceleration = 1/16.0;
const float BlastPower::degradeAmount = .25;

std::unordered_map<std::string, float> BlastPower::getWeights() const {
	std::unordered_map<std::string, float> weights;
	weights.insert({ "vanilla", 1.0f });
	weights.insert({ "random-vanilla", 0.5f });
	weights.insert({ "old", 1.0f });
	weights.insert({ "random-old", 0.5f });
	weights.insert({ "random", 0.25f });
	return weights;
}

TankPower* BlastPower::makeTankPower() const {
	return new BlastTankPower();
}

BulletPower* BlastPower::makeBulletPower() const {
	return new BlastBulletPower();
}

Power* BlastPower::factory() {
	return new BlastPower();
}

BlastPower::BlastPower() {
	return;
}



#include "../rng.h"

void BlastTankPower::additionalShooting(Tank* t, const TankCannonPoint& c, const TankExtraCannonPoint& c2) {
	for (int i = 0; i < BlastPower::bulletAmount; i++) {
		float tempAngle = (GameRNG::randFuncf()*2 - 1) * BlastPower::bulletAngleDeviation; //[-1,1) * deviation
		t->defaultMakeBullet(t->velocity.getAngle() + c.angleFromCenter + c2.angleFromCenter + tempAngle, c2.angleFromEdge);
	}
}

BulletPower* BlastTankPower::makeBulletPower() const {
	return new BlastBulletPower();
}

BlastTankPower::BlastTankPower() {
	const GameSettings& game_settings = GameManager::get_settings();
	maxTime = game_settings.PowerupDurationBaseTime;
	timeLeft = game_settings.PowerupDurationBaseTime;
	//JS: maxTime = 1000

	modifiesAdditionalShooting = true;
	overridesAdditionalShooting = true;
}



#include "../rng.h"

#include "../collision-handler.h"

InteractionUpdateHolder<BulletUpdateStruct, WallUpdateStruct> BlastBulletPower::modifiedCollisionWithWall(const Bullet* b, const Wall* w) {
	std::pair<double, double> vec = CollisionHandler::pushMovableAwayFromImmovable_vecOnly(b->cir, w->rec);
	return { false, false, new BulletUpdateStruct(vec.first, vec.second, -1024*b->velocity.getMagnitude(), 0,0), nullptr };
}

bool BlastBulletPower::getModifiesCollisionWithCircleHazard(const CircleHazard* ch) const {
	return (ch->getCollisionType() == CircleHazardCollisionType::solid);
}

bool BlastBulletPower::getModifiesCollisionWithRectHazard(const RectHazard* rh) const {
	return (rh->getCollisionType() == RectHazardCollisionType::solid);
}

InteractionUpdateHolder<BulletUpdateStruct, CircleHazardUpdateStruct> BlastBulletPower::modifiedCollisionWithCircleHazard(const Bullet* b, const CircleHazard* ch) {
	std::pair<double, double> vec = CollisionHandler::pushMovableAwayFromImmovable_vecOnly(b->cir, ch->cir);
	return { false, false, new BulletUpdateStruct(vec.first, vec.second, -1024*b->velocity.getMagnitude(), 0,0), nullptr };
}

InteractionUpdateHolder<BulletUpdateStruct, RectHazardUpdateStruct> BlastBulletPower::modifiedCollisionWithRectHazard(const Bullet* b, const RectHazard* rh) {
	std::pair<double, double> vec = CollisionHandler::pushMovableAwayFromImmovable_vecOnly(b->cir, rh->rec);
	return { false, false, new BulletUpdateStruct(vec.first, vec.second, -1024*b->velocity.getMagnitude(), 0,0), nullptr };
}

BulletPower* BlastBulletPower::makeDuplicate() const {
	return new BlastBulletPower(this->accelerationAmount);
}

TankPower* BlastBulletPower::makeTankPower() const {
	return new BlastTankPower();
}

BlastBulletPower::BlastBulletPower()
: BlastBulletPower(-1 * ((GameRNG::randFuncf()+GameRNG::randFuncf())/2 * (BlastPower::maxBulletAcceleration - BlastPower::minBulletAcceleration) + BlastPower::minBulletAcceleration)) {}
//acceleration: [0,1) * accDiff + min

BlastBulletPower::BlastBulletPower(float acceleration) {
	accelerationAmount = acceleration;

	modifiesCollisionWithWall = true;
	modifiedCollisionWithWallCanWorkWithOthers = false; //TODO: not sure on this one
}
