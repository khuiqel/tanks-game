#include "dev-tiny-tanks-level-effect.h"

#include "../tank-manager.h"

std::unordered_map<std::string, float> DevTinyTanksLevelEffect::getWeights() const {
	std::unordered_map<std::string, float> weights;
	weights.insert({ "dev", 0.5f });
	weights.insert({ "random-dev", 0.5f });
	return weights;
}

void DevTinyTanksLevelEffect::apply() {
	//TODO: this should not be necessary
	for (int i = 0; i < TankManager::getNumTanks(); i++) {
		TankManager::getTank(i)->cir.r *= .25;
	}
}

DevTinyTanksLevelEffect::DevTinyTanksLevelEffect() { return; }

LevelEffect* DevTinyTanksLevelEffect::factory(const GenericFactoryConstructionData& args) {
	return new DevTinyTanksLevelEffect();
}
