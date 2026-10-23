#include "wall-sparks-power.h"
#include "../game-manager.h" //settings

#include "../constants.h"

#include "bounce-power.h"

const int WallSparksPower::extraBulletsCount = 4;
const int WallSparksPower::maxBounces = BouncePower::maxBounces;
const float WallSparksPower::maxNewBulletVelocity = 1.25;
const float WallSparksPower::minNewBulletVelocity =  .75;
const float WallSparksPower::bulletAngleDeviation = PI/8;

std::unordered_map<std::string, float> WallSparksPower::getWeights() const {
	std::unordered_map<std::string, float> weights;
	weights.insert({ "dev", 1.0f });
	weights.insert({ "random-dev", 1.0f });
	return weights;
}

TankPower* WallSparksPower::makeTankPower() const {
	return new WallSparksTankPower();
}

BulletPower* WallSparksPower::makeBulletPower() const {
	return new WallSparksBulletPower();
}

Power* WallSparksPower::factory() {
	return new WallSparksPower();
}

WallSparksPower::WallSparksPower() {
	return;
}



BulletPower* WallSparksTankPower::makeBulletPower() const {
	return new WallSparksBulletPower();
}

WallSparksTankPower::WallSparksTankPower() {
	const GameSettings& game_settings = GameManager::get_settings();
	maxTime = game_settings.PowerupDurationBaseTime;
	timeLeft = game_settings.PowerupDurationBaseTime;
}



#include <vector>
#include "../rng.h"

#include "../collision-handler.h"
#include "../power-function-helper.h"
#include "../bullet-manager.h"

void WallSparksBulletPower::sparkExplode(const Bullet* b, const BulletUpdateStruct* b_update) {
	//see BananaBulletPower::bananaExplode()
	for (int i = 0; i < WallSparksPower::extraBulletsCount; i++) {
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
		newVelocity = newVelocity * ((GameRNG::randFuncf()+GameRNG::randFuncf())/2 * (WallSparksPower::maxNewBulletVelocity - WallSparksPower::minNewBulletVelocity) + WallSparksPower::minNewBulletVelocity);
		float newAngle = (b->velocity.getAngle() + b_update->angle) + (GameRNG::randFuncf()+GameRNG::randFuncf() - 1) * WallSparksPower::bulletAngleDeviation;
		BulletManager::pushBullet(new Bullet(b->cir.x + b_update->x, b->cir.y + b_update->y, b->cir.r, newAngle, newVelocity, b->getTeamID(), b->getParentIDType(), b->getParentID(), bp, true));
		delete bp;
	}
}

InteractionBoolHolder WallSparksBulletPower::modifiedEdgeCollision(Bullet* b) {
	//see BounceBulletPower::modifiedEdgeCollision()

	bool bouncedY = false;
	//bool bouncedX = false;
	BulletUpdateStruct b_update;

	if (CollisionHandler::partiallyOutOfBounds(b->cir)) {
		auto result = PowerFunctionHelper::bounceEdgeGenericY(b);
		if (result.first) {
			bouncesLeft--;
			bouncedY = true;
		}
		//TODO: update modifiedEdgeCollision to also use update structs
		sparkExplode(b, &result.second);
		b->update(&result.second);
		if (bouncesLeft <= 0) {
			modifiesCollisionWithWall = false; //for wall sparks
			modifiesEdgeCollision = false;
			return { CollisionHandler::fullyOutOfBounds(b->cir) };
		}
	}

	if (CollisionHandler::partiallyOutOfBounds(b->cir)) {
		auto result = PowerFunctionHelper::bounceEdgeGenericX(b);
		if (result.first) {
			bouncesLeft--;
			//bouncedX = true;
		}
		//TODO: update modifiedEdgeCollision to also use update structs
		sparkExplode(b, &result.second);
		b->update(&result.second);
		if (bouncesLeft <= 0) {
			modifiesCollisionWithWall = false; //for wall sparks
			modifiesEdgeCollision = false;
			return { CollisionHandler::fullyOutOfBounds(b->cir) };
		}
	}

	if (!bouncedY && CollisionHandler::partiallyOutOfBounds(b->cir)) {
		auto result = PowerFunctionHelper::bounceEdgeGenericY(b);
		if (result.first) {
			bouncesLeft--;
			//bouncedY = true;
		}
		//TODO: update modifiedEdgeCollision to also use update structs
		sparkExplode(b, &result.second);
		b->update(&result.second);
		if (bouncesLeft <= 0) {
			modifiesCollisionWithWall = false; //for wall sparks
			modifiesEdgeCollision = false;
			//return { CollisionHandler::fullyOutOfBounds(b->cir) };
		}
	}

	return { CollisionHandler::fullyOutOfBounds(b->cir) };
}

InteractionUpdateHolder<BulletUpdateStruct, WallUpdateStruct> WallSparksBulletPower::modifiedCollisionWithWall(const Bullet* b, const Wall* w) {
	//see BounceBulletPower::modifiedWallCollision()

	std::shared_ptr<BulletUpdateStruct> b_update;
	std::shared_ptr<WallUpdateStruct> w_update;

	//first, bounce off wall

	auto result = PowerFunctionHelper::bounceGeneric(b, w);
	b_update = result.second.firstUpdate;
	w_update = result.second.secondUpdate;
	if (result.first) {
		bouncesLeft--;
	}

	if (bouncesLeft <= 0) {
		modifiesCollisionWithWall = false;
		modifiesEdgeCollision = false; //for edge sparks
	}

	//second, create more bullets (they lose this power, just like banana)

	sparkExplode(b, b_update.get());

	return { (bouncesLeft < 0), false, b_update, w_update };
	//feels a little overpowered to let the bullet bounce and live...
}

TankPower* WallSparksBulletPower::makeTankPower() const {
	return new WallSparksTankPower();
}

WallSparksBulletPower::WallSparksBulletPower() : WallSparksBulletPower(WallSparksPower::maxBounces) {}

WallSparksBulletPower::WallSparksBulletPower(int bounces) {
	bouncesLeft = bounces;
	if (bounces > 0) [[likely]] {
		modifiesCollisionWithWall = true;
	}
}
