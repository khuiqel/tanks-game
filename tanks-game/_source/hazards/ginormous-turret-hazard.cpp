#include "ginormous-turret-hazard.h"

#include "../constants.h"
#include <cmath>
//#include <algorithm> //nothing
#include <iostream>
#include "../rng.h"

#include "../renderer.h"
#include "../color-mixer.h"
#include "../background-rect.h"

#include "../tank.h"
#include "../tank-manager.h"
#include "../bullet-manager.h"
#include "../wall-manager.h"
#include "../hazard-manager.h"

std::unordered_map<std::string, float> GinormousTurretHazard::MinionTurret::getWeights() const {
	std::unordered_map<std::string, float> weights;
	weights.insert({ "vanilla", 0.0f });
	weights.insert({ "dev", 0.0f });
	return weights;
}

GinormousTurretHazard::MinionTurret::MinionTurret(double xpos, double ypos, double angle, Game_ID parentID) : TargetingTurretHazard(xpos, ypos, angle) {
	this->parentID = parentID;
}

GinormousTurretHazard::MinionTurret::~MinionTurret() {
	//nothing
}

CircleHazard* GinormousTurretHazard::MinionTurret::factory(const GenericFactoryConstructionData& args) {
	if (args.getDataCount() >= 2) [[likely]] {
		int count = args.getDataPortionLength(0);
		int count_id = args.getDataPortionLength(1);

		if (count >= 3 && count_id >= 1) [[likely]] {
			const double* arr = static_cast<const double*>(args.getDataPortion(0).get());
			const Game_ID* arr_id = static_cast<const Game_ID*>(args.getDataPortion(1).get());
			double x = arr[0];
			double y = arr[1];
			double a = arr[2];
			Game_ID parent = arr_id[0];
			return new MinionTurret(x, y, a, parent);
		}
	}
	return new MinionTurret(0, 0, 0, 0);
}

void GinormousTurretHazard::MinionTurret::setTarget(Game_ID parentVerification, Game_ID target) {
	if (this->parentID == parentVerification) [[likely]] {
		this->trackingID = target;
		this->targeting = true;
	}
}

void GinormousTurretHazard::MinionTurret::unsetTarget(Game_ID parentVerification) {
	if (this->parentID == parentVerification) [[likely]] {
		this->trackingID = this->getGameID();
		this->targeting = false;
	}
}

void GinormousTurretHazard::MinionTurret::tick() {
	if (currentState == 0) { //either tracking a tank or doing nothing
		if (targeting) { //tracking tank
			tick_continueTracking();
		} else { //no orders
			targetingCount = 0;
			return;
		}
	}

	if (currentState == 1) { //charging up to shoot (and shooting)
		tick_chargeUp();
	}

	if (currentState == 2) { //cooling down
		tick_cooldown();
	}
}

bool GinormousTurretHazard::MinionTurret::canSeeTank(const Tank* t) const {
	return (this->trackingID == t->getGameID());
}

void GinormousTurretHazard::MinionTurret::drawReticule(float alpha) const {
	if (currentState == 2) {
		//this is an absolute hack
		return;
	}
	TargetingTurretHazard::drawReticule(alpha);
}



std::unordered_map<std::string, float> GinormousTurretHazard::getWeights() const {
	std::unordered_map<std::string, float> weights;
	weights.insert({ "vanilla", 0.25f });
	weights.insert({ "random-vanilla", 0.25f });
	weights.insert({ "random", 0.125f });
	return weights;
}

GinormousTurretHazard::GinormousTurretHazard(double xpos, double ypos, double angle, int maxChildren, int startChildren, double childDistMultiplier) : MotherTurretHazard(xpos, ypos, angle, maxChildren, startChildren, childDistMultiplier) {
	cir.r = TANK_RADIUS * 4; // MotherTurretHazard radius * 2 //my vision for this turret was twice as large as this value, but that can barely fit on the screen
	//future note: my idea for the "god turret" is r = TANK_RADIUS * 64 or 32 (32 might be better for gameplay, 64 is more what I wanted), which is nowhere near being able to fit on the screen; since it's so big it doesn't even recognize the little tanks; alternatively, it can immobilize and move tanks around because it's that strong, but maybe this should go to a "demi-god turret" instead
	//lore idea (this is really stupid): since there are turret sizes smaller and larger than the tanks, but not of the same size, does that imply the tanks are turrets (that gained sentience)? what a plot twist!; doesn't explain why other turrets can handle lightning fine but tanks can't though, but whatever
	childDistFromMother = cir.r * childDistMultiplier; //reset because the radius was changed

	stateMultiplier[1] /= 2;
	//turningIncrement /= 2;

	//canAcceptPowers = false; //not sure
}

GinormousTurretHazard::GinormousTurretHazard(double xpos, double ypos, double angle, int maxChildren, int startChildren) : GinormousTurretHazard(xpos, ypos, angle, maxChildren, startChildren, .25) {}

GinormousTurretHazard::GinormousTurretHazard(double xpos, double ypos, double angle) : GinormousTurretHazard(xpos, ypos, angle, 16, 8) {}

GinormousTurretHazard::~GinormousTurretHazard() {
	//nothing
}

CircleHazard* GinormousTurretHazard::factory(const GenericFactoryConstructionData& args) {
	if (args.getDataCount() >= 1) [[likely]] {
		int count = args.getDataPortionLength(0);

		if (count >= 3) [[likely]] {
			const double* arr = static_cast<const double*>(args.getDataPortion(0).get());
			double x = arr[0];
			double y = arr[1];
			double a = arr[2];

			if (args.getDataCount() >= 2) {
				int count_num = args.getDataPortionLength(1);

				if (count_num >= 1) {
					const int* arr_num = static_cast<const int*>(args.getDataPortion(1).get());

					int child_max = arr_num[0];
					int child_start = ((count_num >= 2) ? arr_num[1] : 0);

					if ((args.getDataCount() >= 3) && (args.getDataPortionLength(2) >= 1)) {
						const double* arr_dist = static_cast<const double*>(args.getDataPortion(2).get());
						return new GinormousTurretHazard(x, y, a, child_max, child_start, arr_dist[0]);
					}

					return new GinormousTurretHazard(x, y, a, child_max, child_start);
				}
			}

			return new GinormousTurretHazard(x, y, a);
		}
	}

	return new GinormousTurretHazard(0, 0, 0);
}

void GinormousTurretHazard::turnTowardsTank(const Tank* t) {
	TargetingTurretHazard::turnTowardsTank(t);
	TargetingTurretHazard::turnTowardsTank(t);
	//this is hacky
}

CircleHazard* GinormousTurretHazard::makeTurret(int turretNum) const {
	GenericFactoryConstructionData constructionData;
	const double angle = getChildTurretAngle(turretNum);
	double* posArr = new double[3]{ this->cir.x + (this->cir.r+childDistFromMother) * std::cos(angle), this->cir.y + (this->cir.r+childDistFromMother) * std::sin(angle), angle };
	constructionData = GenericFactoryConstructionData(3, posArr);
	Game_ID* id = new Game_ID[1]{ this->getGameID() };
	constructionData.pushData(1, id);
	CircleHazard* childTurret = MinionTurret::factory(constructionData);
	return childTurret;
}

void GinormousTurretHazard::tick() {
	updateChildCount();
	if (getChildCount() < maxChildTurrets/4) {
		//behave like a MotherTurret
		//tick_stopChildren(); //probably fine for the children to keep targeting, they just won't receive new orders
		MotherTurretHazard::tick();
	} else {
		//behave like a MotherTurret when there's no target
		//otherwise behave sorta like a TargetingTurret

		if (currentState == 0) {
			if (targeting) { //tracking tank
				tick_continueTracking();
				targetingCount = 0;
			}
			if (!targeting) { //looking for a tank
				tick_lookForNewTarget();
			}

			if (targeting) {
				tick_notifyChildren(trackingID);
			} else {
				tick_stopChildren();
				MotherTurretHazard::tick();
			}
		}

		if (currentState == 1) { //charging up
			tick_chargeUp();
		}

		if (currentState == 2) { //cooling down
			tick_cooldown();
		}
	}
}

void GinormousTurretHazard::tick_notifyChildren(Game_ID id) {
	for (int i = 0; i < childTurretIDs.size(); i++) {
		if (!childTurretAlive[i]) {
			continue;
		}
		CircleHazard* ch = HazardManager::getCircleHazardByID(childTurretIDs[i]);
		if (ch != nullptr) [[likely]] { //sanity check
			MinionTurret* mt = static_cast<MinionTurret*>(ch);
			mt->setTarget(this->getGameID(), id);
		}
	}
}

void GinormousTurretHazard::tick_stopChildren() {
	for (int i = 0; i < childTurretIDs.size(); i++) {
		if (!childTurretAlive[i]) {
			continue;
		}
		CircleHazard* ch = HazardManager::getCircleHazardByID(childTurretIDs[i]);
		if (ch != nullptr) [[likely]] { //sanity check
			MinionTurret* mt = static_cast<MinionTurret*>(ch);
			mt->unsetTarget(this->getGameID());
		}
	}
}

void GinormousTurretHazard::drawOutline(float alpha) const {
	StationaryTurretHazard::drawOutline(alpha, 2.0f);
}

void GinormousTurretHazard::drawBarrel(float alpha) const {
	StationaryTurretHazard::drawBarrel(alpha, 3.0f);
}

CircleHazard* GinormousTurretHazard::randomizingFactory(double x_start, double y_start, double area_width, double area_height, const GenericFactoryConstructionData& args) {
	int attempts = 0;
	CircleHazard* randomized = nullptr;
	double xpos, ypos, angle;

	int count = 0;
	if (args.getDataCount() >= 1) {
		count = args.getDataPortionLength(0);
	}
	if (count >= 1) {
		const double* arr = static_cast<const double*>(args.getDataPortion(0).get());
		angle = arr[0];
	} else {
		angle = LevelRNG::randFunc() * (2*PI);
	}

	const double realRadius = (TANK_RADIUS*4) * 1.25 + (TANK_RADIUS/2);

	do {
		xpos = LevelRNG::randNumInRange(x_start + realRadius, x_start + area_width - realRadius);
		ypos = LevelRNG::randNumInRange(y_start + realRadius, y_start + area_height - realRadius);
		CircleHazard* testGinormousTurret = new GinormousTurretHazard(xpos, ypos, angle);
		if (testGinormousTurret->reasonableLocation()) {
			randomized = testGinormousTurret;
			break;
		} else {
			delete testGinormousTurret;
		}
		attempts++;
	} while (attempts < 128);

	return randomized;
}
