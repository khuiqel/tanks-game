#include "mines-power.h"
#include "../game-manager.h" //settings

const double MinesPower::bulletDistance = 1.0/8;

std::unordered_map<std::string, float> MinesPower::getWeights() const {
	std::unordered_map<std::string, float> weights;
	weights.insert({ "vanilla-extra", 0.5f });
	weights.insert({ "old", 0.25f });
	return weights;
}

TankPower* MinesPower::makeTankPower() const {
	return new MinesTankPower();
}

BulletPower* MinesPower::makeBulletPower() const {
	return new MinesBulletPower();
}

Power* MinesPower::factory() {
	return new MinesPower();
}

MinesPower::MinesPower() {
	return;
}



#include <cmath>

void MinesTankPower::tick(Tank* t) {
	for (int i = 0; i < t->tankPowers.size(); i++) {
		if (t->tankPowers[i] != this) [[likely]] {
			if (t->tankPowers[i]->modifiesAdditionalShooting) {
				//might also want to check addsShootingPoints and/or addsExtraShootingPoints
				this->modifiesAdditionalShooting = false;
				this->overridesAdditionalShooting = false;
				return;
			}
		}
	}
	this->modifiesAdditionalShooting = true;
	this->overridesAdditionalShooting = true;
}

void MinesTankPower::additionalShooting(Tank* t, const TankCannonPoint& c, const TankExtraCannonPoint& c2) {
	t->preciseMakeBullet((t->cir.r * MinesPower::bulletDistance) * std::cos(t->velocity.getAngle() + c.angleFromCenter + c2.angleFromCenter),
	                     (t->cir.r * MinesPower::bulletDistance) * std::sin(t->velocity.getAngle() + c.angleFromCenter + c2.angleFromCenter),
	                     t->velocity.getAngle() + c.angleFromCenter + c2.angleFromCenter + c2.angleFromEdge);
}

BulletPower* MinesTankPower::makeBulletPower() const {
	return new MinesBulletPower();
}

MinesTankPower::MinesTankPower() {
	const GameSettings& game_settings = GameManager::get_settings();
	maxTime = game_settings.PowerupDurationBaseTime;
	timeLeft = game_settings.PowerupDurationBaseTime;
	//in JS, the tank's shooting cooldown was reset

	modifiesAdditionalShooting = true;
	overridesAdditionalShooting = true;
}



TankPower* MinesBulletPower::makeTankPower() const {
	return new MinesTankPower();
}

MinesBulletPower::MinesBulletPower() {
	//nothing
}
