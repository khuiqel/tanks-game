#include "banana-power.h"
#include "../game-manager.h" //settings

const int BananaPower::bananaCount = 8; //TODO: change with INI
const float BananaPower::maxNewBulletVelocityMultiplier = 5.0/4.0;
const float BananaPower::minNewBulletVelocityMultiplier = 1.0/2.0; //JS range: [0,1]

std::unordered_map<std::string, float> BananaPower::getWeights() const {
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

TankPower* BananaPower::makeTankPower() const {
	return new BananaTankPower();
}

BulletPower* BananaPower::makeBulletPower() const {
	return new BananaBulletPower();
}

Power* BananaPower::factory() {
	return new BananaPower();
}

BananaPower::BananaPower() {
	return;
}



BulletPower* BananaTankPower::makeBulletPower() const {
	return new BananaBulletPower();
}

BananaTankPower::BananaTankPower() {
	const GameSettings& game_settings = GameManager::get_settings();
	maxTime = game_settings.PowerupDurationBaseTime;
	timeLeft = game_settings.PowerupDurationBaseTime;
}



#include "../constants.h"
#include <vector>
#include "../rng.h"

#include "../bullet-manager.h"
//#include <tracy_include.h>

void BananaBulletPower::bananaExplode(const Bullet* b) {
	//ZoneScoped;
	for (int i = 0; i < BananaPower::bananaCount; i++) {
		std::vector<BulletPower*>* bp = new std::vector<BulletPower*>;
		bp->reserve(b->bulletPowers.size()-1);
		for (int i = 0; i < b->bulletPowers.size(); i++) {
			if (b->bulletPowers[i] != this) [[likely]] {
				bp->push_back(b->bulletPowers[i]->makeDuplicate());
			}
		}
		float newVelocity = b->getInitialVelocity();
		if (newVelocity == 0) {
			newVelocity = Tank::default_maxSpeed*BULLET_TO_TANK_SPEED_RATIO; //if bullet's initial speed is zero, it should still explode (TODO: what should the initial speed be?)
		}
		newVelocity = newVelocity * ((GameRNG::randFuncf()+GameRNG::randFuncf())/2 * (BananaPower::maxNewBulletVelocityMultiplier - BananaPower::minNewBulletVelocityMultiplier) + BananaPower::minNewBulletVelocityMultiplier);
		BulletManager::pushBullet(new Bullet(b->x, b->y, b->r/2, GameRNG::randFuncf() * float(2*PI), newVelocity, b->getTeamID(), b->getParentIDType(), b->getParentID(), bp, true));
		delete bp;
	}
}

InteractionBoolHolder BananaBulletPower::modifiedMovement(Bullet* b) {
	if (wasStationary && b->velocity.getMagnitude() == 0) [[unlikely]] {
		bananaExplode(b);
		return { true };
	}
	if (b->velocity.getMagnitude() == 0) [[unlikely]] {
		wasStationary = true;
	} else {
		wasStationary = false; //something else changed the bullet's speed
	}
	return { false };
}

TankPower* BananaBulletPower::makeTankPower() const {
	return new BananaTankPower();
}

BananaBulletPower::BananaBulletPower() {
	modifiesMovement = true;

	wasStationary = false;
}
