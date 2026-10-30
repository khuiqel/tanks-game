#include "ancient-mega-death-power.h"
#include "../game-manager.h" //settings

std::unordered_map<std::string, float> AncientMegaDeathPower::getWeights() const {
	std::unordered_map<std::string, float> weights;
	weights.insert({ "dev", 0.5f });
	weights.insert({ "random-dev", 0.125f });
	return weights;
}

TankPower* AncientMegaDeathPower::makeTankPower() const {
	return new AncientMegaDeathTankPower();
}

BulletPower* AncientMegaDeathPower::makeBulletPower() const {
	return new AncientMegaDeathBulletPower();
}

Power* AncientMegaDeathPower::factory() {
	return new AncientMegaDeathPower();
}

AncientMegaDeathPower::AncientMegaDeathPower() {
	return;
}



BulletPower* AncientMegaDeathTankPower::makeBulletPower() const {
	return new AncientMegaDeathBulletPower();
}

AncientMegaDeathTankPower::AncientMegaDeathTankPower() {
	const GameSettings& game_settings = GameManager::get_settings();
	maxTime = game_settings.PowerupDurationBaseTime;
	timeLeft = game_settings.PowerupDurationBaseTime;
}



#include "../constants.h"

#include "../power-function-helper.h"

InteractionBoolHolder AncientMegaDeathBulletPower::modifiedMovement(Bullet* b) {
	//from TrackingBulletPower::modifiedMovement
	Game_ID targetID = PowerFunctionHelper::homingGenericTarget<true>(b);
	if (targetID != -1) [[likely]] {
		PowerFunctionHelper::homingGenericMove(b, targetID, 2*PI);
	} else {
		//update with HomingBulletPower::modifiedMovement
		//can't just set b->angle to the correct angle because there has to be a targeting round (for multi-tank mode)
	}
	return { false };
}

InteractionUpdateHolder<BulletUpdateStruct, WallUpdateStruct> AncientMegaDeathBulletPower::modifiedCollisionWithWall(const Bullet* b, const Wall* w) {
	return { false, false, nullptr, nullptr };
}

TankPower* AncientMegaDeathBulletPower::makeTankPower() const {
	return new AncientMegaDeathTankPower();
}

AncientMegaDeathBulletPower::AncientMegaDeathBulletPower() {
	modifiesMovement = true;
	modifiesCollisionWithWall = true;
	modifiedCollisionWithWallCanWorkWithOthers = false;
}
