#include "mega-death-power.h"
#include "../game-manager.h" //settings

#include "../constants.h"
#include <cmath> //floor

const float MegaDeathPower::destroyWallTier = DESTRUCTION_TIER;
const double MegaDeathPower::bulletSizeMultiplierPerTick = 65.0/64.0;

std::unordered_map<std::string, float> MegaDeathPower::getWeights() const {
	std::unordered_map<std::string, float> weights;
	weights.insert({ "vanilla", 0.5f });
	weights.insert({ "random-vanilla", 0.25f });
	weights.insert({ "random", 0.125f });
	return weights;
}

TankPower* MegaDeathPower::makeTankPower() const {
	return new MegaDeathTankPower();
}

BulletPower* MegaDeathPower::makeBulletPower() const {
	return new MegaDeathBulletPower();
}

Power* MegaDeathPower::factory() {
	return new MegaDeathPower();
}

MegaDeathPower::MegaDeathPower() {
	return;
}



BulletPower* MegaDeathTankPower::makeBulletPower() const {
	return new MegaDeathBulletPower();
}

MegaDeathTankPower::MegaDeathTankPower() {
	const GameSettings& game_settings = GameManager::get_settings();
	maxTime = game_settings.PowerupDurationBaseTime;
	timeLeft = game_settings.PowerupDurationBaseTime;
}



void MegaDeathBulletPower::tick(Bullet* b) {
	if (getOffenseTier(b) >= MegaDeathPower::destroyWallTier) {
		modifiesCollisionWithWall = true;
		modifiedCollisionWithWallCanWorkWithOthers = false;
	} else { //the size could have decreased
		modifiesCollisionWithWall = false;
		modifiedCollisionWithWallCanWorkWithOthers = true;
	}
}

InteractionUpdateHolder<BulletUpdateStruct, WallUpdateStruct> MegaDeathBulletPower::modifiedCollisionWithWall(const Bullet* b, const Wall* w) {
	//tick() happens before this, so modifiesCollisionWithWall will only be set to true if the bullet can destroy walls
	return { false, true, nullptr, nullptr };
}

float MegaDeathBulletPower::getOffenseTier(const Bullet* b) const {
	float value = static_cast<float>(b->cir.r / (Bullet::default_radius*4)) * MegaDeathPower::destroyWallTier;
	//return (value >= MegaDeathPower::destroyWallTier ? std::floor(value) : 0); //this is what I originally wanted in JS Tanks, I think, but in practice isn't preferable
	return value;
}

float MegaDeathBulletPower::getDefenseTier(const Bullet* b) const {
	float value = static_cast<float>(b->cir.r / (Bullet::default_radius*4)) * MegaDeathPower::destroyWallTier;
	//return (value >= MegaDeathPower::destroyWallTier ? std::floor(value) : 0);
	return value;
}

TankPower* MegaDeathBulletPower::makeTankPower() const {
	return new MegaDeathTankPower();
}

MegaDeathBulletPower::MegaDeathBulletPower() {
	//modifiesCollisionWithWall = true;
	//modifiedCollisionWithWallCanWorkWithOthers = false;

	bulletRadiusGrowMultiplies_Stationary = true;
	bulletRadiusGrowMultiplies_Moving = true;
}
