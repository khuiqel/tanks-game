#include "tracking-power.h"
#include "../game-manager.h" //settings

std::unordered_map<std::string, float> TrackingPower::getWeights() const {
	std::unordered_map<std::string, float> weights;
	weights.insert({ "vanilla-extra", 0.5f });
	weights.insert({ "old", 0.5f });
	weights.insert({ "random-old", 0.25f });
	return weights;
}

TankPower* TrackingPower::makeTankPower() const {
	return new TrackingTankPower();
}

BulletPower* TrackingPower::makeBulletPower() const {
	return new TrackingBulletPower();
}

Power* TrackingPower::factory() {
	return new TrackingPower();
}

TrackingPower::TrackingPower() {
	return;
}



BulletPower* TrackingTankPower::makeBulletPower() const {
	return new TrackingBulletPower();
}

TrackingTankPower::TrackingTankPower() {
	const GameSettings& game_settings = GameManager::get_settings();
	maxTime = game_settings.PowerupDurationBaseTime;
	timeLeft = game_settings.PowerupDurationBaseTime;
}



#include "../constants.h"

#include "../power-function-helper.h"

InteractionBoolHolder TrackingBulletPower::modifiedMovement(Bullet* b) {
	Game_ID targetID = PowerFunctionHelper::homingGenericTarget<true>(b);
	if (targetID != -1) [[likely]] {
		PowerFunctionHelper::homingGenericMove(b, targetID, 2*PI);
	} else {
		//update with HomingBulletPower::modifiedMovement
		//can't just set b->angle to the correct angle because there has to be a targeting round (for multi-tank mode)
	}
	return { false };
}

InteractionUpdateHolder<BulletUpdateStruct, WallUpdateStruct> TrackingBulletPower::modifiedCollisionWithWall(const Bullet* b, const Wall* w) {
	return { false, false, nullptr, nullptr };
}

TankPower* TrackingBulletPower::makeTankPower() const {
	return new TrackingTankPower();
}

TrackingBulletPower::TrackingBulletPower() {
	modifiesMovement = true;
	modifiesCollisionWithWall = true;
	modifiedCollisionWithWallCanWorkWithOthers = false;
}
