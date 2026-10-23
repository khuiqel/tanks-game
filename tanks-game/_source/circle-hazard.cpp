#include "circle-hazard.h"

#include "collision-handler.h"

/*
std::vector<std::string> CircleHazard::getHazardTypes() const {
	std::vector<std::string> types = std::vector<std::string>{ "vanilla", "random-vanilla", "random" };
	return types;
}

std::unordered_map<std::string, float> CircleHazard::getWeights() const {
	std::unordered_map<std::string, float> weights;
	weights.insert({ "vanilla", 1.0f });
	weights.insert({ "random-vanilla", 1.0f });
	weights.insert({ "random", 1.0f });
	return weights;
}
*/

void CircleHazard::update(const CircleHazardUpdateStruct* up) {
	this->cir.x += up->x;
	this->cir.y += up->y;
}

InteractionUpdateHolder<TankUpdateStruct, CircleHazardUpdateStruct> CircleHazard::modifiedTankCollision(const Tank* t) const {
	std::pair<double, double> vec = CollisionHandler::pushMovableAwayFromImmovable_vecOnly(t->cir, this->cir);
	return { false, false, new TankUpdateStruct(vec.first, vec.second, 0,0), nullptr };
}

InteractionUpdateHolder<BulletUpdateStruct, CircleHazardUpdateStruct> CircleHazard::modifiedBulletCollision(const Bullet* b) const {
	std::pair<double, double> vec = CollisionHandler::pushMovableAwayFromImmovable_vecOnly(b->cir, this->cir);
	return { false, false, new BulletUpdateStruct(vec.first, vec.second, 0,0,0), nullptr };
}

CircleHazardUpdateStruct::CircleHazardUpdateStruct(double x, double y) {
	this->x = x;
	this->y = y;
}

void CircleHazardUpdateStruct::add(const CircleHazardUpdateStruct& other) {
	this->x += other.x;
	this->y += other.y;
}

//hazard notes:
/*
hazard ideas:
teleport zone
pipes?
magnetic hazard of some kind (when bullet or something is near it, shoots out some electricity (not lightning; there's a difference) and pulls it in)
oil spill
exploding barrel


JS hazard numbers (with names updated):
0 = targeting turret, 1 = horizontal lightning, 2 = lava (rectangular), 3 = stationary turret, 4 = vertical lightning, 5 = rectangular lightning, 6 = no bullet zone, 7 = patrolling turret (unimplemented)
quite the lack of circular hazards
C++ order of creating the JS hazards (probably):
stationary turret, horizontal lightning, lava, rectangular lightning, vertical lightning, no bullet zone, targeting turret, patrolling turret


random fun facts:
in JS, one day I got the idea for a turret, so I set about to implement it, but then got stuck because there was nowhere to add it; many hours later, the targeting turret finally made its first appearance
when I first showed off horizontal lightning to others (in JS), they thought it was made from drawing an image because of its grayer background
surprisingly, I don't think I got the idea for JS to have circular lightning, lava, or no bullet zones; maybe I subconciously blocked the idea because that would've been more work

*/
