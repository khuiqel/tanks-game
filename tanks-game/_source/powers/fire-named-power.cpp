#include "fire-named-power.h"
#include "../game-manager.h" //settings

#include "../constants.h"

const float FireNamedPower::bulletAngleDeviation = PI/4;
const int FireNamedPower::bulletAmount = 4;

const float  FireNamedPower::maxBulletAcceleration = 3/32.0;
const float  FireNamedPower::minBulletAcceleration = 1/32.0;
const float  FireNamedPower::degradeAmount = .5;
const double FireNamedPower::growAmount = 1.5/32.0; //JS: [1/32, 5/32] //TODO: need way to pass parameters into bulletpower constructor

std::unordered_map<std::string, float> FireNamedPower::getWeights() const {
	std::unordered_map<std::string, float> weights;
	weights.insert({ "vanilla", 1.0f });
	weights.insert({ "random-vanilla", 1.0f });
	weights.insert({ "old", 1.0f });
	weights.insert({ "random-old", 1.0f });
	weights.insert({ "supermix", 1.0f });
	weights.insert({ "supermix-vanilla", 1.0f });
	weights.insert({ "random", 1.0f });
	return weights;
}

TankPower* FireNamedPower::makeTankPower() const {
	return new FireNamedTankPower();
}

BulletPower* FireNamedPower::makeBulletPower() const {
	return new FireNamedBulletPower();
}

Power* FireNamedPower::factory() {
	return new FireNamedPower();
}

FireNamedPower::FireNamedPower() {
	return;
}



#include "../rng.h"

void FireNamedTankPower::additionalShooting(Tank* t, const TankCannonPoint& c, const TankExtraCannonPoint& c2) {
	for (int i = 0; i < FireNamedPower::bulletAmount; i++) {
		float tempAngle = (GameRNG::randFuncf()+GameRNG::randFuncf() - 1) * FireNamedPower::bulletAngleDeviation; //[-1,1) * deviation
		t->defaultMakeBullet(t->velocity.getAngle() + c.angleFromCenter + c2.angleFromCenter + tempAngle, c2.angleFromEdge);
	}
}

BulletPower* FireNamedTankPower::makeBulletPower() const {
	return new FireNamedBulletPower();
}

FireNamedTankPower::FireNamedTankPower() {
	const GameSettings& game_settings = GameManager::get_settings();
	maxTime = game_settings.PowerupDurationBaseTime;
	timeLeft = game_settings.PowerupDurationBaseTime;
	//JS: maxTime = 1000

	modifiesAdditionalShooting = true;
	overridesAdditionalShooting = true;
}



#include "../rng.h"

InteractionUpdateHolder<BulletUpdateStruct, WallUpdateStruct> FireNamedBulletPower::modifiedCollisionWithWall(const Bullet* b, const Wall* w) {
	if (b->velocity.getMagnitude() == 0) {
		return { false, false, new BulletUpdateStruct(0,0,0,0, -FireNamedPower::degradeAmount), nullptr };
	} else {
		if (b->acceleration < 0) {
			return { false, false, new BulletUpdateStruct(0,0, b->acceleration, 0,0), nullptr };
		}
		return { false, false, nullptr, nullptr };
	}
}

BulletPower* FireNamedBulletPower::makeDuplicate() const {
	return new FireNamedBulletPower(this->accelerationAmount);
}

TankPower* FireNamedBulletPower::makeTankPower() const {
	return new FireNamedTankPower();
}

FireNamedBulletPower::FireNamedBulletPower()
: FireNamedBulletPower(-1 * ((GameRNG::randFuncf()+GameRNG::randFuncf())/2 * (FireNamedPower::maxBulletAcceleration - FireNamedPower::minBulletAcceleration) + FireNamedPower::minBulletAcceleration)) {}
//acceleration: [0,1) * accDiff + min

FireNamedBulletPower::FireNamedBulletPower(float acceleration) {
	accelerationAmount = acceleration;

	modifiesCollisionWithWall = true;

	bulletRadiusGrowMultiplies_Moving = false;
}
