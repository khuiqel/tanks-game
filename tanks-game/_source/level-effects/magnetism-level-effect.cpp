#include "magnetism-level-effect.h"

#include <cmath>

#include "../tank-manager.h"
#include "../powerup-manager.h"

std::unordered_map<std::string, float> MagnetismLevelEffect::getWeights() const {
	std::unordered_map<std::string, float> weights;
	weights.insert({ "vanilla", 1.0f });
	weights.insert({ "random-vanilla", 0.5f }); //powerups need to be manually watched when not watching everything
	return weights;
}

void MagnetismLevelEffect::watchPowerSquare(Game_ID p) {
	watchPowerSquare(p, default_distToStartMoving, default_maxMoveAmount);
}

void MagnetismLevelEffect::watchPowerSquare(Game_ID p, double distToStartMoving) {
	watchPowerSquare(p, distToStartMoving, default_maxMoveAmount);
}

void MagnetismLevelEffect::watchPowerSquare(Game_ID p, double distToStartMoving, double maxMoveAmount) {
	bool found = false;
	for (int i = 0; i < watching.size(); i++) {
		if (watching[i] == p) [[unlikely]] {
			found = true;
			std::cerr << "WARNING: MagnetismLevelEffect is already watching PowerSquare ID#" << p << std::endl;
			break;
		}
	}
	if (!found) [[likely]] {
		watching.push_back(p);
		watchingData.push_back({ distToStartMoving, maxMoveAmount });
	}
}

void MagnetismLevelEffect::watchLastPowerSquaresPushed(int count) {
	watchLastPowerSquaresPushed(count, default_distToStartMoving, default_maxMoveAmount);
}

void MagnetismLevelEffect::watchLastPowerSquaresPushed(int count, double distToStartMoving) {
	watchLastPowerSquaresPushed(count, distToStartMoving, default_maxMoveAmount);
}

void MagnetismLevelEffect::watchLastPowerSquaresPushed(int count, double distToStartMoving, double maxMoveAmount) {
	for (int i = count; i > 0; i--) {
		watchPowerSquare(PowerupManager::getPowerup(PowerupManager::getNumPowerups()-i)->getGameID());
	}
}

void MagnetismLevelEffect::unwatchPowerSquare(Game_ID powerupID) {
	for (int i = 0; i < watching.size(); i++) {
		if (watching[i] == powerupID) {
			watching.erase(watching.begin() + i);
			watchingData.erase(watchingData.begin() + i);
			break;
		}
	}
}

void MagnetismLevelEffect::apply() {
	if (watchAllPowerups) {
		for (int i = 0; i < PowerupManager::getNumPowerups(); i++) {
			watchPowerSquare(PowerupManager::getPowerup(i)->getGameID());
		}
	}
}

void MagnetismLevelEffect::tick(const Level* parent) {
	for (int i = watching.size() - 1; i >= 0; i--) {
		PowerSquare* p = PowerupManager::getPowerupByID(watching[i]);
		if (p == nullptr) {
			//for now, delete from the list if they can't be found, because this can't refresh the Game_ID of a respawned powerup
			watching.erase(watching.begin() + i);
			watchingData.erase(watchingData.begin() + i);
			continue;
		}
	}
}

void MagnetismLevelEffect::doEffects(Level* parent) const {
	//an option would be to check all powerups, and if there are new ones, watch them
	//note: doesn't work well with respawning powerups because those powersquares get different Game_IDs
	//a fix would probably involve making PowerSquareWatcher a regular element of the game

	for (int i = 0; i < watching.size(); i++) {
		PowerSquare* p = PowerupManager::getPowerupByID(watching[i]);
		if (p != nullptr) [[likely]] {
			const double p_x = p->rec.x + p->rec.w/2;
			const double p_y = p->rec.y + p->rec.h/2;
			double moveDelta[2] = { 0, 0 };
			for (int j = 0; j < TankManager::getNumTanks(); j++) {
				const Tank* t = TankManager::getTank(j);
				const SimpleVector2D distToTank = SimpleVector2D(t->cir.x - p_x, t->cir.y - p_y);
				const double& distToStartMoving = watchingData[i].first;
				const double& maxMoveAmount = watchingData[i].second;
				if (distToTank.getMagnitude() < distToStartMoving) {
					const double maxMovePercentage = 1 - (distToTank.getMagnitude() / distToStartMoving); //linear interpolation
					moveDelta[0] += (maxMoveAmount * maxMovePercentage) * std::cos(distToTank.getAngle());
					moveDelta[1] += (maxMoveAmount * maxMovePercentage) * std::sin(distToTank.getAngle());
				}
			}
			p->rec.x += moveDelta[0];
			p->rec.y += moveDelta[1];
		} else {
			//very unlikely to get here; can only happen if a later level effect's tick() altered powerups after this's tick()
		}
	}
}

MagnetismLevelEffect::MagnetismLevelEffect(bool watchEverything, double distToStartMoving, double maxMagnetismStrength) {
	this->watchAllPowerups = watchEverything;
	this->default_distToStartMoving = distToStartMoving;
	this->default_maxMoveAmount = maxMagnetismStrength;
}

MagnetismLevelEffect::MagnetismLevelEffect(bool watchEverything) : MagnetismLevelEffect(watchEverything, 96, 1) {}

MagnetismLevelEffect::MagnetismLevelEffect() : MagnetismLevelEffect(true) {}

MagnetismLevelEffect::~MagnetismLevelEffect() {
	//nothing
}

LevelEffect* MagnetismLevelEffect::factory(const GenericFactoryConstructionData& args) {
	if (args.getDataCount() >= 1) [[likely]] {
		int count = args.getDataPortionLength(0);

		if (count >= 1) [[likely]] {
			const bool* arr = static_cast<const bool*>(args.getDataPortion(0).get());
			bool watch = arr[0];

			if (args.getDataCount() >= 2) {
				int count_data = args.getDataPortionLength(1);
				if (count_data >= 2) {
					const double* arr_data = static_cast<const double*>(args.getDataPortion(1).get());
					double dist = arr_data[0];
					double strength = arr_data[1];
					return new MagnetismLevelEffect(watch, dist, strength);
				}
			}
			return new MagnetismLevelEffect(watch);
		}
	}

	return new MagnetismLevelEffect();
}
