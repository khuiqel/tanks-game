#pragma once
#include <utility>
#include <vector>

//#include "tank.h"
#include "bullet.h"
#include "wall.h"

class PowerFunctionHelper {
protected:
	static inline std::pair<bool, InteractionUpdateHolder<BulletUpdateStruct, WallUpdateStruct>> bounceGenericCornerHandler(const Bullet* b, const Wall* w, double x, double y) {
		return superbounceGenericCornerHandler(b, w, x, y, 0);
	}
	static std::pair<bool, InteractionUpdateHolder<BulletUpdateStruct, WallUpdateStruct>> superbounceGenericCornerHandler(const Bullet*, const Wall*, double x, double y, double strength = 0);

public:
	template <bool targetUsingAngleDiff>
	static Game_ID homingGenericTarget(const Bullet*); //find target for homing
	static void homingGenericMove(Bullet*, Game_ID targetID, float maxAngleChange); //do homing on target

	static inline std::pair<bool, InteractionUpdateHolder<BulletUpdateStruct, WallUpdateStruct>> bounceGeneric(const Bullet* b, const Wall* w) { return superbounceGeneric(b, w, 0); }
	static inline std::pair<bool, InteractionUpdateHolder<BulletUpdateStruct, WallUpdateStruct>> bounceGenericWithCorners(const Bullet* b, const Wall* w) { return superbounceGenericWithCorners(b, w, 0); }
	static std::pair<bool, BulletUpdateStruct> bounceEdgeGenericX(const Bullet* b);
	static std::pair<bool, BulletUpdateStruct> bounceEdgeGenericY(const Bullet* b);
	//superbounce: bounce except push the wall (default parameter to make sure it's used intentionally)
	static std::pair<bool, InteractionUpdateHolder<BulletUpdateStruct, WallUpdateStruct>> superbounceGeneric(const Bullet*, const Wall*, double strength = 0);
	static std::pair<bool, InteractionUpdateHolder<BulletUpdateStruct, WallUpdateStruct>> superbounceGenericWithCorners(const Bullet*, const Wall*, double strength = 0);

	static std::vector<float>* equallySpacedCannonPoints(int count);

private:
	PowerFunctionHelper() = delete;
	PowerFunctionHelper(const PowerFunctionHelper&) = delete;
};
