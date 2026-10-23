#include "trickster-snake-power.h"
#include "../game-manager.h" //settings

const double TricksterSnakePower::timePeriod = 50;
const double TricksterSnakePower::movementAmplitude = 16.0/timePeriod;

std::unordered_map<std::string, float> TricksterSnakePower::getWeights() const {
	std::unordered_map<std::string, float> weights;
	weights.insert({ "dev", 1.0f });
	weights.insert({ "random-dev", 1.0f });
	return weights;
}

TankPower* TricksterSnakePower::makeTankPower() const {
	return new TricksterSnakeTankPower();
}

BulletPower* TricksterSnakePower::makeBulletPower() const {
	return new TricksterSnakeBulletPower();
}

Power* TricksterSnakePower::factory() {
	return new TricksterSnakePower();
}

TricksterSnakePower::TricksterSnakePower() {
	return;
}



BulletPower* TricksterSnakeTankPower::makeBulletPower() const {
	return new TricksterSnakeBulletPower();
}

TricksterSnakeTankPower::TricksterSnakeTankPower() {
	const GameSettings& game_settings = GameManager::get_settings();
	maxTime = game_settings.PowerupDurationBaseTime;
	timeLeft = game_settings.PowerupDurationBaseTime;
}



#include "../constants.h"
#include <cmath>

InteractionBoolHolder TricksterSnakeBulletPower::modifiedMovement(Bullet* b) {
	//ypos = TricksterSnakePower::movementAmplitude * std::sin((2*PI) * (tickCount / TricksterSnakePower::timePeriod))
	//ydiff = A * (2*PI) * std::cos((2*PI) * tick)

	const float moveDiff = (TricksterSnakePower::movementAmplitude * (2*PI)) * std::cos((2*PI) * (tickCount / TricksterSnakePower::timePeriod));
	const SimpleVector2D perpendicularMoveDiff = SimpleVector2D(b->velocity.getAngle() + float(PI/2), moveDiff, true);

	b->cir.x += perpendicularMoveDiff.getXComp();
	b->cir.y += perpendicularMoveDiff.getYComp();

	tickCount++; //not sure whether this should go at the start or end
	return { false };
}

TankPower* TricksterSnakeBulletPower::makeTankPower() const {
	return new TricksterSnakeTankPower();
}

TricksterSnakeBulletPower::TricksterSnakeBulletPower() {
	modifiesMovement = true;

	tickCount = 0;
}
