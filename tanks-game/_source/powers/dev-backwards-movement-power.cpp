#include "dev-backwards-movement-power.h"
#include "../game-manager.h" //settings

std::unordered_map<std::string, float> DevBackwardsMovementPower::getWeights() const {
	std::unordered_map<std::string, float> weights;
	weights.insert({ "dev", 1.0f });
	weights.insert({ "random-dev", 0.25f }); //TODO
	return weights;
}

TankPower* DevBackwardsMovementPower::makeTankPower() const {
	return new DevBackwardsMovementTankPower();
}

BulletPower* DevBackwardsMovementPower::makeBulletPower() const {
	return new DevBackwardsMovementBulletPower();
}

Power* DevBackwardsMovementPower::factory() {
	return new DevBackwardsMovementPower();
}

DevBackwardsMovementPower::DevBackwardsMovementPower() {
	return;
}



#include <cmath>

InteractionBoolHolder DevBackwardsMovementTankPower::modifiedMovement(Tank* t, bool forward, bool turnL, bool turnR, bool specialKey) {
	if (specialKey) {
		//TODO: I don't know how this should be implemented (it should do something with acceleration; use the tank's or store a vector?)
		t->cir.x -= t->maxSpeed * std::cos(t->velocity.getAngle());
		t->cir.y -= t->maxSpeed * std::sin(t->velocity.getAngle());
	}
	return { false };
}

BulletPower* DevBackwardsMovementTankPower::makeBulletPower() const {
	return new DevBackwardsMovementBulletPower();
}

DevBackwardsMovementTankPower::DevBackwardsMovementTankPower() {
	const GameSettings& game_settings = GameManager::get_settings();
	maxTime = game_settings.PowerupDurationBaseTime;
	timeLeft = game_settings.PowerupDurationBaseTime;

	modifiesMovement = true;
}



TankPower* DevBackwardsMovementBulletPower::makeTankPower() const {
	return new DevBackwardsMovementTankPower();
}

DevBackwardsMovementBulletPower::DevBackwardsMovementBulletPower() {
	//nothing
}
