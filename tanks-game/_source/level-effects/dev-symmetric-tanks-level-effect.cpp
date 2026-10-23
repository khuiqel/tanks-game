#include "dev-symmetric-tanks-level-effect.h"

#include "../constants.h"

#include "../tank-manager.h"

std::unordered_map<std::string, float> DevSymmetricTanksLevelEffect::getWeights() const {
	std::unordered_map<std::string, float> weights;
	weights.insert({ "dev", 0.0f });
	//intentional only
	return weights;
}

void DevSymmetricTanksLevelEffect::tick(const Level* parent) {
	TankManager::getTank(1)->cir.x = GAME_WIDTH - TankManager::getTank(0)->cir.x;
	TankManager::getTank(1)->cir.y = GAME_HEIGHT - TankManager::getTank(0)->cir.y;
	if (mirrorAngle) {
		TankManager::getTank(1)->velocity.setAngle(TankManager::getTank(0)->velocity.getAngle() + float(PI));
	}
}

DevSymmetricTanksLevelEffect::DevSymmetricTanksLevelEffect(bool alsoDoAngle) {
	this->mirrorAngle = alsoDoAngle;
}

DevSymmetricTanksLevelEffect::DevSymmetricTanksLevelEffect() : DevSymmetricTanksLevelEffect(false) {}

LevelEffect* DevSymmetricTanksLevelEffect::factory(const GenericFactoryConstructionData& args) {
	if (args.getDataCount() >= 1) [[likely]] {
		int count = args.getDataPortionLength(0);

		if (count >= 1) [[likely]] {
			const bool* arr = static_cast<const bool*>(args.getDataPortion(0).get());
			bool angle = arr[0];
			return new DevSymmetricTanksLevelEffect(angle);
		}
	}

	return new DevSymmetricTanksLevelEffect();
}
