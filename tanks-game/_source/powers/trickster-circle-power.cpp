#include "trickster-circle-power.h"
#include "../game-manager.h" //settings

const double TricksterCirclePower::timePeriod = 50;
const double TricksterCirclePower::movementRadius = 16.0/timePeriod;

std::unordered_map<std::string, float> TricksterCirclePower::getWeights() const {
	std::unordered_map<std::string, float> weights;
	weights.insert({ "dev", 0.5f });
	weights.insert({ "random-dev", 0.5f }); //not as cool as the snake version
	return weights;
}

TankPower* TricksterCirclePower::makeTankPower() const {
	return new TricksterCircleTankPower();
}

BulletPower* TricksterCirclePower::makeBulletPower() const {
	return new TricksterCircleBulletPower();
}

Power* TricksterCirclePower::factory() {
	return new TricksterCirclePower();
}

TricksterCirclePower::TricksterCirclePower() {
	return;
}



BulletPower* TricksterCircleTankPower::makeBulletPower() const {
	return new TricksterCircleBulletPower();
}

TricksterCircleTankPower::TricksterCircleTankPower() {
	const GameSettings& game_settings = GameManager::get_settings();
	maxTime = game_settings.PowerupDurationBaseTime;
	timeLeft = game_settings.PowerupDurationBaseTime;
}



#include "../constants.h"
#include <cmath>

InteractionBoolHolder TricksterCircleBulletPower::modifiedMovement(Bullet* b) {
	//displacement from "center" vector = {TricksterCirclePower::movementRadius * std::cos((2*PI) * (tickCount / TricksterCirclePower::timePeriod)), y value similar}
	//derivative = {(TricksterCirclePower::movementRadius * (2*PI)) * -std::sin((2*PI) * (tickCount / TricksterCirclePower::timePeriod)), y value similar}

	//negative tickCount because it looks cooler
	const float moveXDiff = (TricksterCirclePower::movementRadius * (2*PI)) * -std::sin((2*PI) * (-tickCount / TricksterCirclePower::timePeriod));
	const float moveYDiff = (TricksterCirclePower::movementRadius * (2*PI)) *  std::cos((2*PI) * (-tickCount / TricksterCirclePower::timePeriod));
	const SimpleVector2D vectorMoveDiff = SimpleVector2D(b->velocity.getXComp() + moveXDiff, b->velocity.getYComp() + moveYDiff);

	b->cir.x += vectorMoveDiff.getXComp();
	b->cir.y += vectorMoveDiff.getYComp();

	tickCount++; //not sure whether this should go at the start or end
	return { false };
}

TankPower* TricksterCircleBulletPower::makeTankPower() const {
	return new TricksterCircleTankPower();
}

TricksterCircleBulletPower::TricksterCircleBulletPower() {
	modifiesMovement = true;

	tickCount = 0;
}
