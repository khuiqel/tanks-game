#include "power-function-helper.h"

#include "constants.h"
#include <cmath>
#include <iostream>

#include "collision-handler.h"
#include "tank-manager.h"

std::pair<bool, InteractionUpdateHolder<BulletUpdateStruct, WallUpdateStruct>> PowerFunctionHelper::superbounceGeneric(const Bullet* b, const Wall* w, double strength) {
	double b_xDelta, b_yDelta;
	float  b_angleDelta;
	double w_xDelta, w_yDelta;

	if (b->cir.y - w->rec.y <= (w->rec.h / w->rec.w) * (b->cir.x - w->rec.x)) { //I think this is bottom right
		if (b->cir.y - (w->rec.y + w->rec.h) <= (-w->rec.h / w->rec.w) * (b->cir.x - w->rec.x)) { //bottom
			b_yDelta = -1 * ((b->cir.y + b->cir.r - w->rec.y) * 2); //b->y = w->y - b->r
			b_angleDelta = -2 * b->velocity.getAngle(); //b->velocity.setAngle(b->velocity.getAngle() * -1);
			w_yDelta = strength;
			b_xDelta = 0, w_xDelta = 0;
		} else { //right
			b_xDelta = (w->rec.x + w->rec.w - (b->cir.x - b->cir.r)) * 2; //b->x = w->x + w->w + b->r
			b_angleDelta = float(PI) - 2*b->velocity.getAngle(); //b->velocity.setAngle(PI - b->velocity.getAngle());
			w_xDelta = -1 * strength;
			b_yDelta = 0, w_yDelta = 0;
		}
	} else { //top left?
		if (b->cir.y - (w->rec.y + w->rec.h) <= (-w->rec.h / w->rec.w) * (b->cir.x - w->rec.x)) { //left
			b_xDelta = -1 * ((b->cir.x + b->cir.r - w->rec.x) * 2); //b->x = w->x - b->r
			b_angleDelta = float(PI) - 2*b->velocity.getAngle(); //b->velocity.setAngle(PI - b->velocity.getAngle());
			w_xDelta = strength;
			b_yDelta = 0, w_yDelta = 0;
		} else { //top
			b_yDelta = (w->rec.y + w->rec.h - (b->cir.y - b->cir.r)) * 2; //b->y = w->y + w->h + b->r
			b_angleDelta = -2 * b->velocity.getAngle(); //b->velocity.setAngle(b->velocity.getAngle() * -1);
			w_yDelta = -1 * strength;
			b_xDelta = 0, w_xDelta = 0;
		}
	}

	return { true, { false, false, new BulletUpdateStruct(b_xDelta, b_yDelta, 0, b_angleDelta, 0), new WallUpdateStruct(w_xDelta, w_yDelta) } };
}

std::pair<bool, InteractionUpdateHolder<BulletUpdateStruct, WallUpdateStruct>> PowerFunctionHelper::superbounceGenericWithCorners(const Bullet* b, const Wall* w, double strength) { //not the default because bullets move too quickly on average
	if ((b->cir.x < w->rec.x) && (b->cir.y < w->rec.y)) { //circle in bottom left
		return PowerFunctionHelper::superbounceGenericCornerHandler(b, w, w->rec.x, w->rec.y, strength);
	}
	//can't do elses because more than one corner could be in the circle (TODO: really?)
	if ((b->cir.x > (w->rec.x + w->rec.w)) && (b->cir.y < w->rec.y)) { //circle in bottom right
		return PowerFunctionHelper::superbounceGenericCornerHandler(b, w, (w->rec.x + w->rec.w), w->rec.y, strength);
	}
	if ((b->cir.x < w->rec.x) && (b->cir.y > (w->rec.y + w->rec.h))) { //circle in top left
		return PowerFunctionHelper::superbounceGenericCornerHandler(b, w, w->rec.x, (w->rec.y + w->rec.h), strength);
	}
	if ((b->cir.x > (w->rec.x + w->rec.w)) && (b->cir.y > (w->rec.y + w->rec.h))) { //circle in top right
		return PowerFunctionHelper::superbounceGenericCornerHandler(b, w, (w->rec.x + w->rec.w), (w->rec.y + w->rec.h), strength);
	}

	return PowerFunctionHelper::superbounceGeneric(b, w, strength);
}

std::pair<bool, InteractionUpdateHolder<BulletUpdateStruct, WallUpdateStruct>> PowerFunctionHelper::superbounceGenericCornerHandler(const Bullet* b, const Wall* w, double x, double y, double strength) {
	//I have no idea if this is correct but it behaves exactly as I want it to, I think
	//it's not exact because intersection points aren't calculated but it's close enough

	double b_xDelta = 0, b_yDelta = 0;
	float  b_angleDelta;
	double w_xDelta, w_yDelta;

	double d = std::sqrt((x - b->cir.x)*(x - b->cir.x) + (y - b->cir.y)*(y - b->cir.y));
	float angle = std::atan2(float(y - b->cir.y), float(x - b->cir.x));
	b_yDelta -= std::sin(angle) * (b->cir.r - d);
	b_xDelta -= std::cos(angle) * (b->cir.r - d);

	//so a rectangle has an area of influence against a circle: outer edges + radius, and corners are radius (picture a rounded rectangle)
	//when a bullet's center enters the area, it is inside the rectangle, and therefore needs to reflect
	//in effect, rounded rectangle against a point is the same as rectangle against circle
	//the bullet's angle needs to reflect off of the perpendicular to the tangent, and the tangent goes through the intersection between the bullet's path and the area of influence

	float newAngle = 2*angle - (b->velocity.getAngle() - float(PI));
	b_yDelta += std::sin(newAngle) * (b->cir.r - d);
	b_xDelta += std::cos(newAngle) * (b->cir.r - d);
	b_angleDelta = (newAngle - b->velocity.getAngle()); //b->velocity.setAngle(newAngle);

	w_yDelta = std::sin(angle) * strength;
	w_xDelta = std::cos(angle) * strength;

	return { true, { false, false, new BulletUpdateStruct(b_xDelta, b_yDelta, 0, b_angleDelta, 0), new WallUpdateStruct(w_xDelta, w_yDelta) } };
}

std::pair<bool, BulletUpdateStruct> PowerFunctionHelper::bounceEdgeGenericX(const Bullet* b) {
	double b_xDelta;
	float  b_angleDelta;
	bool bounced = false;

	if (b->cir.x + b->cir.r > GAME_WIDTH) { //right edge
		b_xDelta = -1 * (((b->cir.x + b->cir.r) - GAME_WIDTH) * 2);
		b_angleDelta = float(PI) - 2*b->velocity.getAngle(); //b->velocity.setAngle(PI - b->velocity.getAngle());
		bounced = true;
	} else if (b->cir.x - b->cir.r < 0) { //left edge
		b_xDelta = -(b->cir.x - b->cir.r) * 2;
		b_angleDelta = float(PI) - 2*b->velocity.getAngle(); //b->velocity.setAngle(PI - b->velocity.getAngle());
		bounced = true;
	} else {
		b_xDelta = 0;
		b_angleDelta = 0;
	}

	return { bounced, BulletUpdateStruct(b_xDelta, 0, 0, b_angleDelta, 0) };
}

std::pair<bool, BulletUpdateStruct> PowerFunctionHelper::bounceEdgeGenericY(const Bullet* b) {
	double b_yDelta;
	float  b_angleDelta;
	bool bounced = false;

	if (b->cir.y + b->cir.r > GAME_HEIGHT) { //top edge
		b_yDelta = -1 * (((b->cir.y + b->cir.r) - GAME_HEIGHT) * 2);
		b_angleDelta = -2 * b->velocity.getAngle(); //b->velocity.setAngle(b->velocity.getAngle() * -1);
		bounced = true;
	} else if (b->cir.y - b->cir.r < 0) { //bottom edge
		b_yDelta = -(b->cir.y - b->cir.r) * 2;
		b_angleDelta = -2 * b->velocity.getAngle(); //b->velocity.setAngle(b->velocity.getAngle() * -1);
		bounced = true;
	} else {
		b_yDelta = 0;
		b_angleDelta = 0;
	}

	return { bounced, BulletUpdateStruct(0, b_yDelta, 0, b_angleDelta, 0) };
}

std::vector<float>* PowerFunctionHelper::equallySpacedCannonPoints(int count) {
	std::vector<float>* newCannonPoints = new std::vector<float>;
	newCannonPoints->reserve(count-1);
	for (int i = 1; i < count; i++) {
		newCannonPoints->push_back(i / float(count));
	}
	return newCannonPoints;
}

template <bool targetUsingAngleDiff>
Game_ID PowerFunctionHelper::homingGenericTarget(const Bullet* b) {
	int targetTankIndex = -1; //only targets tanks for now
	//idea for targeting multiple tanks: generate a single RNG value per frame, use that for tie-breaking stuff
	//games typically use port priority for tie-breaking, so I guess it will depend on how lazy I want to be

	if constexpr (targetUsingAngleDiff) {
		float minAngleDiff = (2*PI) * 2; //is way more than enough
		for (int i = 0; i < TankManager::getNumTanks(); i++) {
			const Tank* t = TankManager::getTank(i);
			if (t->getTeamID() != b->getTeamID()) [[likely]] {
				float a = std::abs(std::atan2(static_cast<float>(b->cir.y - t->cir.y), static_cast<float>(b->cir.x - t->cir.x)));
				if (a < minAngleDiff) {
					minAngleDiff = a;
					targetTankIndex = i;
				}
			}
		}
	} else { //targetUsingDistance
		double minDistSquaredDiff = (GAME_WIDTH*2 + GAME_HEIGHT*2) * (GAME_WIDTH*2 + GAME_HEIGHT*2); //should be enough
		for (int i = 0; i < TankManager::getNumTanks(); i++) {
			const Tank* t = TankManager::getTank(i);
			if (t->getTeamID() != b->getTeamID()) [[likely]] {
				double dSquared = (b->cir.x - t->cir.x)*(b->cir.x - t->cir.x) + (b->cir.y - t->cir.y)*(b->cir.y - t->cir.y);
				if (dSquared < minDistSquaredDiff) {
					minDistSquaredDiff = dSquared;
					targetTankIndex = i;
				}
			}
		}
	}

	if (targetTankIndex == -1) [[unlikely]] {
		return -1; //means there was nothing to target; realistically, shouldn't be happening, unless 1-tank mode is a thing
	}
	return TankManager::getTank(targetTankIndex)->getGameID();
}
template Game_ID PowerFunctionHelper::homingGenericTarget<true>(const Bullet* b);
template Game_ID PowerFunctionHelper::homingGenericTarget<false>(const Bullet* b);

void PowerFunctionHelper::homingGenericMove(Bullet* b, Game_ID targetID, float maxAngleChange) {
	const Tank* t = TankManager::getTankByID(targetID);

	const SimpleVector2D distToTank = SimpleVector2D(t->cir.getX() - b->cir.x, t->cir.getY() - b->cir.y);
	float theta = SimpleVector2D::angleBetween(distToTank, b->velocity);
	if (std::abs(theta) <= maxAngleChange) {
		//small angle adjustment needed
		b->velocity.setAngle(distToTank.getAngle());
	} else {
		//large angle adjustment needed
		if (theta < 0) {
			b->velocity.changeAngle(maxAngleChange);
		} else {
			b->velocity.changeAngle(-maxAngleChange);
		}
	}
}
