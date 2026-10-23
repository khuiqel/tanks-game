#include "bounce-power.h"
#include "../game-manager.h" //settings

const int BouncePower::maxBounces = 16;

std::unordered_map<std::string, float> BouncePower::getWeights() const {
	std::unordered_map<std::string, float> weights;
	weights.insert({ "vanilla", 1.0f });
	weights.insert({ "random-vanilla", 1.0f });
	weights.insert({ "old", 1.0f });
	weights.insert({ "supermix", 1.0f });
	weights.insert({ "supermix-vanilla", 1.0f });
	weights.insert({ "random", 1.0f });
	return weights;
}

TankPower* BouncePower::makeTankPower() const {
	return new BounceTankPower();
}

BulletPower* BouncePower::makeBulletPower() const {
	return new BounceBulletPower();
}

Power* BouncePower::factory() {
	return new BouncePower();
}

BouncePower::BouncePower() {
	return;
}



BulletPower* BounceTankPower::makeBulletPower() const {
	return new BounceBulletPower();
}

BounceTankPower::BounceTankPower() {
	const GameSettings& game_settings = GameManager::get_settings();
	maxTime = game_settings.PowerupDurationBaseTime;
	timeLeft = game_settings.PowerupDurationBaseTime;
}



#include "../collision-handler.h"
#include "../power-function-helper.h"

InteractionBoolHolder BounceBulletPower::modifiedEdgeCollision(Bullet* b) {
	//the bullet can bounce off of edges twice in a single tick
	//therefore, it can lose 2 bounces at once
	//shouldn't ever have negative bounces, so need to check Y, then X, then Y if Y wasn't already checked, and check bouncesLeft after each edge bounce
	//I could have different checks for partiallyOutOfBounds() to only require one check each but whatever

	//TODO: this system will only bounce off one edge if it's on the corner and has one bounce left; should be changed to actually bounce off the corner, may as well count it as one bounce
	//TODO: should store "max bounces" to implement that

	//TODO: only bounce when moving?

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
		b->update(&result.second);
		if (bouncesLeft <= 0) {
			modifiesCollisionWithWall = false;
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
		b->update(&result.second);
		if (bouncesLeft <= 0) {
			modifiesCollisionWithWall = false;
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
		b->update(&result.second);
		if (bouncesLeft <= 0) {
			modifiesCollisionWithWall = false;
			modifiesEdgeCollision = false;
			//return { CollisionHandler::fullyOutOfBounds(b->cir) };
		}
	}

	return { CollisionHandler::fullyOutOfBounds(b->cir) };
}

InteractionUpdateHolder<BulletUpdateStruct, WallUpdateStruct> BounceBulletPower::modifiedCollisionWithWall(const Bullet* b, const Wall* w) {
	//TODO: only bounce when moving?

	std::shared_ptr<BulletUpdateStruct> b_update;
	std::shared_ptr<WallUpdateStruct> w_update;

	if (b->velocity.getMagnitude() * float(Bullet::default_radius/b->cir.r) <= .5f) {
		auto result = PowerFunctionHelper::bounceGenericWithCorners(b, w);
		b_update = result.second.firstUpdate;
		w_update = result.second.secondUpdate;
		if (result.first) {
			bouncesLeft--;
		}
	} else {
		auto result = PowerFunctionHelper::bounceGeneric(b, w);
		b_update = result.second.firstUpdate;
		w_update = result.second.secondUpdate;
		if (result.first) {
			bouncesLeft--;
		}
	}

	if (bouncesLeft <= 0) {
		modifiesCollisionWithWall = false;
		modifiesEdgeCollision = false;
	}

	return { (bouncesLeft < 0), false, b_update, w_update };
}

BulletPower* BounceBulletPower::makeDuplicate() const {
	return new BounceBulletPower(this->bouncesLeft);
}

TankPower* BounceBulletPower::makeTankPower() const {
	return new BounceTankPower();
}

BounceBulletPower::BounceBulletPower() : BounceBulletPower(BouncePower::maxBounces) {}

BounceBulletPower::BounceBulletPower(int bounces) {
	this->bouncesLeft = bounces;
	if (bounces > 0) [[likely]] {
		modifiesCollisionWithWall = true;
		modifiesEdgeCollision = true;
	}
}
