#include "custom-power-interpreter.h"

#include <stdexcept>
#include <algorithm> //std::find, std::sort, std::remove
#include <filesystem> //only for catching ModProcessor's exceptions
#include <fstream>
#include <iostream>
#include <vector>
#include "rng.h"

#include "mod-processor.h"
#include "powerup-data-governor.h" //for adding custom powers to the game
#include "basic-ini-parser.h" //for processing custom power files
#include "game-manager.h" //INI file, settings

#include "collision-handler.h"
#include "power-function-helper.h"

const std::string CustomPowerInterpreter::ActionStartPhrase_TankPower = "[TANKPOWER_START]";
const std::string CustomPowerInterpreter::ActionStartPhrase_BulletPower = "[BULLETPOWER_START]";

CustomPower::CustomPowerAction_Tank::CustomPowerAction_Tank(CustomPower::CustomPowerCommands_TankPower c) {
	this->command = c;
	this->data = GenericFactoryConstructionData();
}

CustomPower::CustomPowerAction_Bullet::CustomPowerAction_Bullet(CustomPower::CustomPowerCommands_BulletPower c) {
	this->command = c;
	this->data = GenericFactoryConstructionData();
}

CustomPower::CustomPower(std::string name,
float colorR, float colorG, float colorB,
double tankDuration,
const std::vector<std::string>& types,
const std::vector<std::string>& attributes,
const std::unordered_map<std::string, float>& weights,
std::vector<CustomPowerAction_Tank*>* actions_tank,
std::vector<CustomPowerAction_Bullet*>* actions_bullet) {

	this->name = name;
	this->color = ColorValueHolder(colorR, colorG, colorB);
	this->colorImportance = 0;
	this->powerTypes = std::vector<std::string>(types);
	this->powerWeights = std::unordered_map<std::string, float>(weights);

	this->tankDuration = tankDuration;
	this->powerAttributes = std::vector<std::string>(attributes);

	this->initializationActions_tank = std::shared_ptr<std::vector<CustomPowerAction_Tank*>>(actions_tank, [](std::vector<CustomPowerAction_Tank*>* p) {
		for (int i = 0; i < p->size(); i++) {
			delete p->data()[i];
		}
	});
	this->initializationActions_bullet = std::shared_ptr<std::vector<CustomPowerAction_Bullet*>>(actions_bullet, [](std::vector<CustomPowerAction_Bullet*>* p) {
		for (int i = 0; i < p->size(); i++) {
			delete p->data()[i];
		}
	});
}

CustomPower::CustomPower(std::string name,
float colorR, float colorG, float colorB,
double tankDuration,
const std::vector<std::string>& types,
const std::vector<std::string>& attributes,
const std::unordered_map<std::string, float>& weights,
std::shared_ptr<std::vector<CustomPowerAction_Tank*>> actions_tank,
std::shared_ptr<std::vector<CustomPowerAction_Bullet*>> actions_bullet) {

	this->name = name;
	this->color = ColorValueHolder(colorR, colorG, colorB);
	this->colorImportance = 0;
	this->powerTypes = std::vector<std::string>(types);
	this->powerWeights = std::unordered_map<std::string, float>(weights);

	this->tankDuration = tankDuration;
	this->powerAttributes = std::vector<std::string>(attributes);

	this->initializationActions_tank = actions_tank;
	this->initializationActions_bullet = actions_bullet;
}

CustomPower::~CustomPower() {
	//nothing
}

CustomTankPower::CustomTankPower(std::string name, const ColorValueHolder& color, float colorImportance, double tankDuration,
std::shared_ptr<std::vector<CustomPower::CustomPowerAction_Tank*>> initializationActions_tank,
std::shared_ptr<std::vector<CustomPower::CustomPowerAction_Bullet*>> initializationActions_bullet) {

	this->name = name;
	this->color = color;
	this->colorImportance = colorImportance;
	this->tankDuration = tankDuration;
	this->initializationActions_tank = initializationActions_tank;
	this->initializationActions_bullet = initializationActions_bullet;

	timeLeft = tankDuration;
	maxTime = tankDuration;

	this->tankMaxSpeedMultiplier = TankPower::getTankMaxSpeedMultiplier();
	this->tankAccelerationMultiplier = TankPower::getTankAccelerationMultiplier();
	this->tankRadiusMultiplier = TankPower::getTankRadiusMultiplier();
	this->tankFiringRateMultiplier = TankPower::getTankFiringRateMultiplier();
	this->tankTurningIncrementMultiplier = TankPower::getTankTurningIncrementMultiplier();

	this->offenseImportance = TankPower::getOffenseImportance();
	this->offenseTier = TankPower::getOffenseTier(nullptr);
	this->defenseImportance = TankPower::getDefenseImportance();
	this->defenseTier = TankPower::getDefenseTier(nullptr);

	this->additionalShooting_AngleRelativeToTank = 0;
	this->additionalShooting_AngleRelativeToCannon = 0;
	this->additionalShooting_BulletCount = 1;
	this->modifiedCollisionWithEdge_AdditionalBoundaryAmount = 0;
	this->modifiedDeathHandling_DurationSubtractPercent = 0;

	for (int i = 0; i < initializationActions_tank->size(); i++) {
		GenericFactoryConstructionData& data = initializationActions_tank->data()[i]->data;

		switch (initializationActions_tank->data()[i]->command) {
			case CustomPower::CustomPowerCommands_TankPower::additionalShooting_Enable:
				initialization_additionalShooting_Enable(data);
				break;
			case CustomPower::CustomPowerCommands_TankPower::additionalShooting_AngleRelativeToTank:
				initialization_additionalShooting_AngleRelativeToTank(data);
				break;
			case CustomPower::CustomPowerCommands_TankPower::additionalShooting_AngleRelativeToCannon:
				initialization_additionalShooting_AngleRelativeToCannon(data);
				break;
			case CustomPower::CustomPowerCommands_TankPower::additionalShooting_BulletCount:
				initialization_additionalShooting_BulletCount(data);
				break;

			case CustomPower::CustomPowerCommands_TankPower::addShootingPoints_Enable:
				initialization_addShootingPoints_Enable(data);
				break;
			case CustomPower::CustomPowerCommands_TankPower::addShootingPoints_EquallySpaced:
				initialization_addShootingPoints_EquallySpaced(data);
				break;
			case CustomPower::CustomPowerCommands_TankPower::addShootingPoints_Raw:
				initialization_addShootingPoints_Raw(data);
				break;

			case CustomPower::CustomPowerCommands_TankPower::addExtraShootingPoints_Enable:
				initialization_addExtraShootingPoints_Enable(data);
				break;
			case CustomPower::CustomPowerCommands_TankPower::addExtraShootingPoints_Raw:
				initialization_addExtraShootingPoints_Raw(data);
				break;

			case CustomPower::CustomPowerCommands_TankPower::modifiedCollisionWithEdge_Enable:
				initialization_modifiedCollisionWithEdge_Enable_tank(data);
				break;
			case CustomPower::CustomPowerCommands_TankPower::modifiedCollisionWithEdge_AdditionalBoundary:
				initialization_modifiedCollisionWithEdge_AdditionalBoundary(data);
				break;

			case CustomPower::CustomPowerCommands_TankPower::modifiedDeathHandling_Enable:
				initialization_modifiedDeathHandling_Enable_tank(data);
				break;
			case CustomPower::CustomPowerCommands_TankPower::modifiedDeathHandling_DurationSubtractPercent:
				initialization_modifiedDeathHandling_DurationSubtractPercent_tank(data);
				break;

			case CustomPower::CustomPowerCommands_TankPower::tankMaxSpeedMultiplier:
				initialization_tankMaxSpeedMultiplier(data);
				break;
			case CustomPower::CustomPowerCommands_TankPower::tankMaxSpeedStacks:
				initialization_tankMaxSpeedStacks(data);
				break;
			case CustomPower::CustomPowerCommands_TankPower::tankAccelerationMultiplier:
				initialization_tankAccelerationMultiplier(data);
				break;
			case CustomPower::CustomPowerCommands_TankPower::tankAccelerationStacks:
				initialization_tankAccelerationStacks(data);
				break;
			case CustomPower::CustomPowerCommands_TankPower::tankRadiusMultiplier:
				initialization_tankRadiusMultiplier(data);
				break;
			case CustomPower::CustomPowerCommands_TankPower::tankRadiusStacks:
				initialization_tankRadiusStacks(data);
				break;
			case CustomPower::CustomPowerCommands_TankPower::tankFiringRateMultiplier:
				initialization_tankFiringRateMultiplier(data);
				break;
			case CustomPower::CustomPowerCommands_TankPower::tankFiringRateStacks:
				initialization_tankFiringRateStacks(data);
				break;
			case CustomPower::CustomPowerCommands_TankPower::tankTurningIncrementMultiplier:
				initialization_tankTurningIncrementMultiplier(data);
				break;
			case CustomPower::CustomPowerCommands_TankPower::tankTurningIncrementStacks:
				initialization_tankTurningIncrementStacks(data);
				break;

			case CustomPower::CustomPowerCommands_TankPower::offenseImportance:
				initialization_offenseImportance_tank(data);
				break;
			case CustomPower::CustomPowerCommands_TankPower::offenseTier:
				initialization_offenseTier_tank(data);
				break;
			case CustomPower::CustomPowerCommands_TankPower::defenseImportance:
				initialization_defenseImportance_tank(data);
				break;
			case CustomPower::CustomPowerCommands_TankPower::defenseTier:
				initialization_defenseTier_tank(data);
				break;

			default:
				std::cerr << "Unknown CustomPowerCommand_Tank! (this shouldn't happen because only valid commands should be in this CustomTankPower's list) " + this->name << std::endl;
				break;
		}
	}
}

CustomTankPower::CustomTankPower(std::string name, const ColorValueHolder& color, float colorImportance, double tankDuration,
std::shared_ptr<std::vector<CustomPower::CustomPowerAction_Tank*>> initializationActions_tank,
std::shared_ptr<std::vector<CustomPower::CustomPowerAction_Bullet*>> initializationActions_bullet,
double old_timeLeft)
: CustomTankPower(name, color, colorImportance, tankDuration, initializationActions_tank, initializationActions_bullet) {

	//see CustomBulletPower's other constructor for some notes

	timeLeft = old_timeLeft;
}

CustomTankPower::~CustomTankPower() {
	//nothing
}

CustomBulletPower::CustomBulletPower(std::string name, const ColorValueHolder& color, float colorImportance, double tankDuration,
std::shared_ptr<std::vector<CustomPower::CustomPowerAction_Tank*>> initializationActions_tank,
std::shared_ptr<std::vector<CustomPower::CustomPowerAction_Bullet*>> initializationActions_bullet) {

	this->name = name;
	this->color = color;
	this->colorImportance = colorImportance;
	this->tankDuration = tankDuration;
	this->initializationActions_tank = initializationActions_tank;
	this->initializationActions_bullet = initializationActions_bullet;

	this->bulletSpeedMultiplier = BulletPower::getBulletSpeedMultiplier();
	this->bulletRadiusMultiplier = BulletPower::getBulletRadiusMultiplier();
	this->bulletAcceleration = BulletPower::getBulletAcceleration();
	this->bulletAccelerationImportance = BulletPower::getBulletAccelerationImportance();
	this->bulletDeceleration_DegradeAmount = BulletPower::getBulletDegradeAmount();

	this->offenseImportance = BulletPower::getOffenseImportance();
	this->offenseTier = BulletPower::getOffenseTier(nullptr);
	this->defenseImportance = BulletPower::getDefenseImportance();
	this->defenseTier = BulletPower::getDefenseTier(nullptr);

	this->modifiedCollision_BouncesLeft = 0;
	this->modifiedCollisionWithWall_EnableBounce = false;
	this->modifiedCollisionWithWall_DestroyWallEnable = false;

	for (int i = 0; i < initializationActions_bullet->size(); i++) {
		GenericFactoryConstructionData& data = initializationActions_bullet->data()[i]->data;

		switch (initializationActions_bullet->data()[i]->command) {
			case CustomPower::CustomPowerCommands_BulletPower::modifiedCollision_BounceCount:
				initialization_modifiedCollision_BounceCount(data);
				break;

			case CustomPower::CustomPowerCommands_BulletPower::modifiedCollisionWithEdge_EnableBounce:
				initialization_modifiedCollisionWithEdge_EnableBounce_bullet(data);
				break;

			case CustomPower::CustomPowerCommands_BulletPower::modifiedCollisionWithWall_EnableBounce:
				initialization_modifiedCollisionWithWall_EnableBounce_bullet(data);
				break;
			case CustomPower::CustomPowerCommands_BulletPower::modifiedCollisionWithWall_DestroyWall:
				initialization_modifiedCollisionWithWall_DestroyWall(data);
				break;

			case CustomPower::CustomPowerCommands_BulletPower::modifiedDeathHandling_Enable:
				initialization_modifiedDeathHandling_Enable_bullet(data);
				break;

			case CustomPower::CustomPowerCommands_BulletPower::bulletSpeedMultiplier:
				initialization_bulletSpeedMultiplier(data);
				break;
			case CustomPower::CustomPowerCommands_BulletPower::bulletSpeedStacks:
				initialization_bulletSpeedStacks(data);
				break;
			case CustomPower::CustomPowerCommands_BulletPower::bulletRadiusMultiplier:
				initialization_bulletRadiusMultiplier(data);
				break;
			case CustomPower::CustomPowerCommands_BulletPower::bulletRadiusStacks:
				initialization_bulletRadiusStacks(data);
				break;
			case CustomPower::CustomPowerCommands_BulletPower::bulletDeceleration:
				initialization_bulletDeceleration(data);
				break;
			case CustomPower::CustomPowerCommands_BulletPower::bulletAccelerationImportance:
				initialization_bulletAccelerationImportance(data);
				break;
			case CustomPower::CustomPowerCommands_BulletPower::bulletDeceleration_DegradeAmount:
				initialization_bulletDeceleration_DegradeAmount(data);
				break;

			case CustomPower::CustomPowerCommands_BulletPower::offenseImportance:
				initialization_offenseImportance_bullet(data);
				break;
			case CustomPower::CustomPowerCommands_BulletPower::offenseTier:
				initialization_offenseTier_bullet(data);
				break;
			case CustomPower::CustomPowerCommands_BulletPower::defenseImportance:
				initialization_defenseImportance_bullet(data);
				break;
			case CustomPower::CustomPowerCommands_BulletPower::defenseTier:
				initialization_defenseTier_bullet(data);
				break;

			default:
				std::cerr << "Unknown CustomPowerCommand_Bullet! (this shouldn't happen because only valid commands should be in this CustomBulletPower's list) " + this->name << std::endl;
				break;
		}
	}
}

CustomBulletPower::CustomBulletPower(std::string name, const ColorValueHolder& color, float colorImportance, double tankDuration,
std::shared_ptr<std::vector<CustomPower::CustomPowerAction_Tank*>> initializationActions_tank,
std::shared_ptr<std::vector<CustomPower::CustomPowerAction_Bullet*>> initializationActions_bullet,
double radiusMultiplier, float speedMultiplier, float acceleration, int bouncesLeft)
: CustomBulletPower(name, color, colorImportance, tankDuration, initializationActions_tank, initializationActions_bullet) {

	//the correct thing to do is copy-paste the old constructor and change the bulletSpeedMultiplier case and stuff to skip
	//doing it that way causes RNG to not called; however, this way takes less effort

	this->bulletSpeedMultiplier = speedMultiplier;
	this->bulletRadiusMultiplier = radiusMultiplier;
	this->bulletAcceleration = acceleration;

	this->modifiedCollision_BouncesLeft = bouncesLeft;
	if (bouncesLeft <= 0) {
		this->modifiedCollisionWithWall_EnableBounce = false;
		updateModifiesCollisionWithWall();
		this->modifiesEdgeCollision = false;
	}
}

CustomBulletPower::~CustomBulletPower() {
	//nothing
}

CustomPower::CustomPowerCommands_TankPower CustomPower::strToCommand_tank(const std::string& str) noexcept {
	CustomPower::CustomPowerCommands_TankPower powerCommand_tank;

	if (str == "additionalShooting_Enable") {
		powerCommand_tank = CustomPower::CustomPowerCommands_TankPower::additionalShooting_Enable;
	} else if (str == "additionalShooting_AngleRelativeToTank") {
		powerCommand_tank = CustomPower::CustomPowerCommands_TankPower::additionalShooting_AngleRelativeToTank;
	} else if (str == "additionalShooting_AngleRelativeToCannon") {
		powerCommand_tank = CustomPower::CustomPowerCommands_TankPower::additionalShooting_AngleRelativeToCannon;
	} else if (str == "additionalShooting_BulletCount") {
		powerCommand_tank = CustomPower::CustomPowerCommands_TankPower::additionalShooting_BulletCount;
	}
	else if (str == "addShootingPoints_Enable") {
		powerCommand_tank = CustomPower::CustomPowerCommands_TankPower::addShootingPoints_Enable;
	} else if (str == "addShootingPoints_EquallySpaced") {
		powerCommand_tank = CustomPower::CustomPowerCommands_TankPower::addShootingPoints_EquallySpaced;
	} else if (str == "addShootingPoints_Raw") {
		powerCommand_tank = CustomPower::CustomPowerCommands_TankPower::addShootingPoints_Raw;
	}
	else if (str == "addExtraShootingPoints_Enable") {
		powerCommand_tank = CustomPower::CustomPowerCommands_TankPower::addExtraShootingPoints_Enable;
	} else if (str == "addExtraShootingPoints_Raw") {
		powerCommand_tank = CustomPower::CustomPowerCommands_TankPower::addExtraShootingPoints_Raw;
	}
	else if (str == "modifiedCollisionWithEdge_Enable") {
		powerCommand_tank = CustomPower::CustomPowerCommands_TankPower::modifiedCollisionWithEdge_Enable;
	} else if (str == "modifiedCollisionWithEdge_AdditionalBoundary") {
		powerCommand_tank = CustomPower::CustomPowerCommands_TankPower::modifiedCollisionWithEdge_AdditionalBoundary;
	}
	else if (str == "modifiedDeathHandling_Enable") {
		powerCommand_tank = CustomPower::CustomPowerCommands_TankPower::modifiedDeathHandling_Enable;
	} else if (str == "modifiedDeathHandling_DurationSubtractPercent") {
		powerCommand_tank = CustomPower::CustomPowerCommands_TankPower::modifiedDeathHandling_DurationSubtractPercent;
	}
	else if (str == "tankMaxSpeedMultiplier") {
		powerCommand_tank = CustomPower::CustomPowerCommands_TankPower::tankMaxSpeedMultiplier;
	} else if (str == "tankMaxSpeedStacks") {
		powerCommand_tank = CustomPower::CustomPowerCommands_TankPower::tankMaxSpeedStacks;
	} else if (str == "tankAccelerationMultiplier") {
		powerCommand_tank = CustomPower::CustomPowerCommands_TankPower::tankAccelerationMultiplier;
	} else if (str == "tankRadiusMultiplier") {
		powerCommand_tank = CustomPower::CustomPowerCommands_TankPower::tankRadiusMultiplier;
	} else if (str == "tankRadiusStacks") {
		powerCommand_tank = CustomPower::CustomPowerCommands_TankPower::tankRadiusStacks;
	} else if (str == "tankFiringRateMultiplier") {
		powerCommand_tank = CustomPower::CustomPowerCommands_TankPower::tankFiringRateMultiplier;
	} else if (str == "tankFiringRateStacks") {
		powerCommand_tank = CustomPower::CustomPowerCommands_TankPower::tankFiringRateStacks;
	} else if (str == "tankTurningIncrementMultiplier") {
		powerCommand_tank = CustomPower::CustomPowerCommands_TankPower::tankTurningIncrementMultiplier;
	} else if (str == "tankTurningIncrementStacks") {
		powerCommand_tank = CustomPower::CustomPowerCommands_TankPower::tankTurningIncrementStacks;
	}
	else if (str == "offenseImportance") {
		powerCommand_tank = CustomPower::CustomPowerCommands_TankPower::offenseImportance;
	} else if (str == "offenseTier") {
		powerCommand_tank = CustomPower::CustomPowerCommands_TankPower::offenseTier;
	} else if (str == "defenseImportance") {
		powerCommand_tank = CustomPower::CustomPowerCommands_TankPower::defenseImportance;
	} else if (str == "defenseTier") {
		powerCommand_tank = CustomPower::CustomPowerCommands_TankPower::defenseTier;
	}
	else {
		powerCommand_tank = CustomPower::CustomPowerCommands_TankPower::UNKNOWN;
	}

	return powerCommand_tank;
}

CustomPower::CustomPowerCommands_BulletPower CustomPower::strToCommand_bullet(const std::string& str) noexcept {
	CustomPower::CustomPowerCommands_BulletPower powerCommand_bullet;

	if (str == "modifiedCollision_BounceCount") {
		powerCommand_bullet = CustomPower::CustomPowerCommands_BulletPower::modifiedCollision_BounceCount;
	}
	else if (str == "modifiedCollisionWithEdge_EnableBounce") {
		powerCommand_bullet = CustomPower::CustomPowerCommands_BulletPower::modifiedCollisionWithEdge_EnableBounce;
	}
	else if (str == "modifiedCollisionWithWall_EnableBounce") {
		powerCommand_bullet = CustomPower::CustomPowerCommands_BulletPower::modifiedCollisionWithWall_EnableBounce;
	} else if (str == "modifiedCollisionWithWall_DestroyWall") {
		powerCommand_bullet = CustomPower::CustomPowerCommands_BulletPower::modifiedCollisionWithWall_DestroyWall;
	}
	else if (str == "modifiedDeathHandling_Enable") {
		powerCommand_bullet = CustomPower::CustomPowerCommands_BulletPower::modifiedDeathHandling_Enable;
	}
	else if (str == "bulletSpeedMultiplier") {
		powerCommand_bullet = CustomPower::CustomPowerCommands_BulletPower::bulletSpeedMultiplier;
	} else if (str == "bulletSpeedStacks") {
		powerCommand_bullet = CustomPower::CustomPowerCommands_BulletPower::bulletSpeedStacks;
	} else if (str == "bulletRadiusMultiplier") {
		powerCommand_bullet = CustomPower::CustomPowerCommands_BulletPower::bulletRadiusMultiplier;
	} else if (str == "bulletRadiusStacks") {
		powerCommand_bullet = CustomPower::CustomPowerCommands_BulletPower::bulletRadiusStacks;
	} else if (str == "bulletDeceleration") {
		powerCommand_bullet = CustomPower::CustomPowerCommands_BulletPower::bulletDeceleration;
	} else if (str == "bulletAccelerationImportance") {
		powerCommand_bullet = CustomPower::CustomPowerCommands_BulletPower::bulletAccelerationImportance;
	} else if (str == "bulletDeceleration_DegradeAmount") {
		powerCommand_bullet = CustomPower::CustomPowerCommands_BulletPower::bulletDeceleration_DegradeAmount;
	}
	else if (str == "offenseImportance") {
		powerCommand_bullet = CustomPower::CustomPowerCommands_BulletPower::offenseImportance;
	} else if (str == "offenseTier") {
		powerCommand_bullet = CustomPower::CustomPowerCommands_BulletPower::offenseTier;
	} else if (str == "defenseImportance") {
		powerCommand_bullet = CustomPower::CustomPowerCommands_BulletPower::defenseImportance;
	} else if (str == "defenseTier") {
		powerCommand_bullet = CustomPower::CustomPowerCommands_BulletPower::defenseTier;
	}
	else {
		powerCommand_bullet = CustomPower::CustomPowerCommands_BulletPower::UNKNOWN;
	}

	return powerCommand_bullet;
}

inline void CustomTankPower::initialization_helper_Enable(bool& thingToUpdate, const GenericFactoryConstructionData& data) noexcept {
	if ((data.getDataCount() > 0) && (data.getDataPortionLength(0) > 0)) {
		thingToUpdate = static_cast<const bool*>(data.getDataPortion(0).get())[0];
	} else {
		thingToUpdate = true;
	}
}

inline void CustomTankPower::initialization_additionalShooting_Enable(const GenericFactoryConstructionData& data) noexcept {
	initialization_helper_Enable(modifiesAdditionalShooting, data);
	overridesAdditionalShooting = modifiesAdditionalShooting;
}
inline void CustomTankPower::initialization_additionalShooting_AngleRelativeToTank(const GenericFactoryConstructionData& data) noexcept {
	additionalShooting_AngleRelativeToTank = float(PI/180) * static_cast<const float*>(data.getDataPortion(0).get())[0];
}
inline void CustomTankPower::initialization_additionalShooting_AngleRelativeToCannon(const GenericFactoryConstructionData& data) noexcept {
	additionalShooting_AngleRelativeToCannon = float(PI/180) * static_cast<const float*>(data.getDataPortion(0).get())[0];
}
inline void CustomTankPower::initialization_additionalShooting_BulletCount(const GenericFactoryConstructionData& data) noexcept {
	additionalShooting_BulletCount = static_cast<const int*>(data.getDataPortion(0).get())[0];
}

inline void CustomTankPower::initialization_addShootingPoints_Enable(const GenericFactoryConstructionData& data) noexcept {
	initialization_helper_Enable(addsShootingPoints, data);
}
inline void CustomTankPower::initialization_addShootingPoints_EquallySpaced(const GenericFactoryConstructionData& data) noexcept {
	const int amount = static_cast<const int*>(data.getDataPortion(0).get())[0];
	std::vector<float>* list = PowerFunctionHelper::equallySpacedCannonPoints(amount);
	addShootingPoints_AngleList.insert(addShootingPoints_AngleList.end(), list->begin(), list->end());
	delete list;
	std::sort(addShootingPoints_AngleList.begin(), addShootingPoints_AngleList.end());
}
inline void CustomTankPower::initialization_addShootingPoints_Raw(const GenericFactoryConstructionData& data) noexcept {
	const float* list = static_cast<const float*>(data.getDataPortion(0).get());
	for (int i = 0; i < data.getDataPortionLength(0); i++) {
		addShootingPoints_AngleList.push_back(list[i]);
	}
	std::sort(addShootingPoints_AngleList.begin(), addShootingPoints_AngleList.end());
}

inline void CustomTankPower::initialization_addExtraShootingPoints_Enable(const GenericFactoryConstructionData& data) noexcept {
	initialization_helper_Enable(addsExtraShootingPoints, data);
}
inline void CustomTankPower::initialization_addExtraShootingPoints_Raw(const GenericFactoryConstructionData& data) noexcept {
	const float* list = static_cast<const float*>(data.getDataPortion(0).get());
	for (int i = 0; i < data.getDataPortionLength(0)/2; i++) {
		addExtraShootingPoints_AngleList.push_back({ list[i*2] * float(2*PI), list[i*2+1] * float(2*PI) });
	}
	//std::sort(addExtraShootingPoints_AngleList.begin(), addExtraShootingPoints_AngleList.end());
}

inline void CustomTankPower::initialization_modifiedCollisionWithEdge_Enable_tank(const GenericFactoryConstructionData& data) noexcept {
	initialization_helper_Enable(modifiesEdgeCollision, data);
}
inline void CustomTankPower::initialization_modifiedCollisionWithEdge_AdditionalBoundary(const GenericFactoryConstructionData& data) noexcept {
	modifiedCollisionWithEdge_AdditionalBoundaryAmount = -1 * static_cast<const double*>(data.getDataPortion(0).get())[0];
}

inline void CustomTankPower::initialization_modifiedDeathHandling_Enable_tank(const GenericFactoryConstructionData& data) noexcept {
	initialization_helper_Enable(modifiesDeathHandling, data);
}
inline void CustomTankPower::initialization_modifiedDeathHandling_DurationSubtractPercent_tank(const GenericFactoryConstructionData& data) noexcept {
	modifiedDeathHandling_DurationSubtractPercent = static_cast<const double*>(data.getDataPortion(0).get())[0];
}

inline void CustomTankPower::initialization_tankMaxSpeedMultiplier(const GenericFactoryConstructionData& data) noexcept {
	tankMaxSpeedMultiplier = static_cast<const float*>(data.getDataPortion(0).get())[0];
}
inline void CustomTankPower::initialization_tankMaxSpeedStacks(const GenericFactoryConstructionData& data) noexcept {
	initialization_helper_Enable(tankMaxSpeedStacks, data);
}
inline void CustomTankPower::initialization_tankAccelerationMultiplier(const GenericFactoryConstructionData& data) noexcept {
	tankAccelerationMultiplier = static_cast<const float*>(data.getDataPortion(0).get())[0];
}
inline void CustomTankPower::initialization_tankAccelerationStacks(const GenericFactoryConstructionData& data) noexcept {
	initialization_helper_Enable(tankAccelerationStacks, data);
}
inline void CustomTankPower::initialization_tankRadiusMultiplier(const GenericFactoryConstructionData& data) noexcept {
	tankRadiusMultiplier = static_cast<const double*>(data.getDataPortion(0).get())[0];
}
inline void CustomTankPower::initialization_tankRadiusStacks(const GenericFactoryConstructionData& data) noexcept {
	initialization_helper_Enable(tankRadiusStacks, data);
}
inline void CustomTankPower::initialization_tankFiringRateMultiplier(const GenericFactoryConstructionData& data) noexcept {
	tankFiringRateMultiplier = static_cast<const double*>(data.getDataPortion(0).get())[0];
}
inline void CustomTankPower::initialization_tankFiringRateStacks(const GenericFactoryConstructionData& data) noexcept {
	initialization_helper_Enable(tankFiringRateStacks, data);
}
inline void CustomTankPower::initialization_tankTurningIncrementMultiplier(const GenericFactoryConstructionData& data) noexcept {
	tankTurningIncrementMultiplier = static_cast<const float*>(data.getDataPortion(0).get())[0];
}
inline void CustomTankPower::initialization_tankTurningIncrementStacks(const GenericFactoryConstructionData& data) noexcept {
	initialization_helper_Enable(tankTurningIncrementStacks, data);
}

inline void CustomTankPower::initialization_offenseImportance_tank(const GenericFactoryConstructionData& data) noexcept {
	offenseImportance = static_cast<const float*>(data.getDataPortion(0).get())[0];
}
inline void CustomTankPower::initialization_offenseTier_tank(const GenericFactoryConstructionData& data) noexcept {
	offenseTier = static_cast<const float*>(data.getDataPortion(0).get())[0];
}
inline void CustomTankPower::initialization_defenseImportance_tank(const GenericFactoryConstructionData& data) noexcept {
	defenseImportance = static_cast<const float*>(data.getDataPortion(0).get())[0];
}
inline void CustomTankPower::initialization_defenseTier_tank(const GenericFactoryConstructionData& data) noexcept {
	defenseTier = static_cast<const float*>(data.getDataPortion(0).get())[0];
}

inline void CustomBulletPower::initialization_helper_Enable(bool& thingToUpdate, const GenericFactoryConstructionData& data) noexcept {
	//exactly the same as CustomTankPower::initialization_helper_Enable()
	if ((data.getDataCount() > 0) && (data.getDataPortionLength(0) > 0)) {
		thingToUpdate = static_cast<const bool*>(data.getDataPortion(0).get())[0];
	} else {
		thingToUpdate = true;
	}
}

inline void CustomBulletPower::initialization_modifiedCollision_BounceCount(const GenericFactoryConstructionData& data) noexcept {
	modifiedCollision_BouncesLeft = static_cast<const int*>(data.getDataPortion(0).get())[0];
}

inline void CustomBulletPower::initialization_modifiedCollisionWithEdge_EnableBounce_bullet(const GenericFactoryConstructionData& data) noexcept {
	initialization_helper_Enable(modifiesEdgeCollision, data);
}

inline void CustomBulletPower::initialization_modifiedCollisionWithWall_EnableBounce_bullet(const GenericFactoryConstructionData& data) noexcept {
	initialization_helper_Enable(modifiedCollisionWithWall_EnableBounce, data);
	updateModifiesCollisionWithWall();
}
inline void CustomBulletPower::initialization_modifiedCollisionWithWall_DestroyWall(const GenericFactoryConstructionData& data) noexcept {
	initialization_helper_Enable(modifiedCollisionWithWall_DestroyWallEnable, data);
	updateModifiesCollisionWithWall();
}
inline void CustomBulletPower::updateModifiesCollisionWithWall() noexcept {
	modifiesCollisionWithWall = (modifiedCollisionWithWall_DestroyWallEnable || modifiedCollisionWithWall_EnableBounce);
}

inline void CustomBulletPower::initialization_modifiedDeathHandling_Enable_bullet(const GenericFactoryConstructionData& data) noexcept {
	initialization_helper_Enable(modifiesDeathHandling, data);
}

inline void CustomBulletPower::initialization_bulletSpeedMultiplier(const GenericFactoryConstructionData& data) noexcept {
	float bulletSpeedMultiplierMin = static_cast<const float*>(data.getDataPortion(0).get())[0];
	float bulletSpeedMultiplierMax = static_cast<const float*>(data.getDataPortion(0).get())[1];
	if (bulletSpeedMultiplierMin == bulletSpeedMultiplierMax) {
		bulletSpeedMultiplier = bulletSpeedMultiplierMin;
	} else {
		bulletSpeedMultiplier = (((GameRNG::randFuncf()+GameRNG::randFuncf())/2) * (bulletSpeedMultiplierMax - bulletSpeedMultiplierMin) + bulletSpeedMultiplierMin);
	}
}
inline void CustomBulletPower::initialization_bulletSpeedStacks(const GenericFactoryConstructionData& data) noexcept {
	initialization_helper_Enable(bulletSpeedStacks, data);
}
inline void CustomBulletPower::initialization_bulletRadiusMultiplier(const GenericFactoryConstructionData& data) noexcept {
	double bulletRadiusMultiplierMin = static_cast<const double*>(data.getDataPortion(0).get())[0];
	double bulletRadiusMultiplierMax = static_cast<const double*>(data.getDataPortion(0).get())[1];
	if (bulletRadiusMultiplierMin == bulletRadiusMultiplierMax) {
		bulletRadiusMultiplier = bulletRadiusMultiplierMin;
	} else {
		bulletRadiusMultiplier = (((GameRNG::randFunc()+GameRNG::randFunc())/2) * (bulletRadiusMultiplierMax - bulletRadiusMultiplierMin) + bulletRadiusMultiplierMin);
	}
}
inline void CustomBulletPower::initialization_bulletRadiusStacks(const GenericFactoryConstructionData& data) noexcept {
	initialization_helper_Enable(bulletRadiusStacks, data);
}
inline void CustomBulletPower::initialization_bulletDeceleration(const GenericFactoryConstructionData& data) noexcept {
	float bulletDecelerationMin = static_cast<const float*>(data.getDataPortion(0).get())[0];
	float bulletDecelerationMax = static_cast<const float*>(data.getDataPortion(0).get())[1];
	if (bulletDecelerationMin == bulletDecelerationMax) {
		bulletAcceleration = -1 * bulletDecelerationMin;
	} else {
		bulletAcceleration = -1 * (((GameRNG::randFuncf()+GameRNG::randFuncf())/2) * (bulletDecelerationMax - bulletDecelerationMin) + bulletDecelerationMin);
	}
}
inline void CustomBulletPower::initialization_bulletAccelerationImportance(const GenericFactoryConstructionData& data) noexcept {
	bulletAccelerationImportance = static_cast<const float*>(data.getDataPortion(0).get())[0];
}
inline void CustomBulletPower::initialization_bulletDeceleration_DegradeAmount(const GenericFactoryConstructionData& data) noexcept {
	bulletDeceleration_DegradeAmount = static_cast<const float*>(data.getDataPortion(0).get())[0];
}

inline void CustomBulletPower::initialization_offenseImportance_bullet(const GenericFactoryConstructionData& data) noexcept {
	offenseImportance = static_cast<const float*>(data.getDataPortion(0).get())[0];
}
inline void CustomBulletPower::initialization_offenseTier_bullet(const GenericFactoryConstructionData& data) noexcept {
	offenseTier = static_cast<const float*>(data.getDataPortion(0).get())[0];
}
inline void CustomBulletPower::initialization_defenseImportance_bullet(const GenericFactoryConstructionData& data) noexcept {
	defenseImportance = static_cast<const float*>(data.getDataPortion(0).get())[0];
}
inline void CustomBulletPower::initialization_defenseTier_bullet(const GenericFactoryConstructionData& data) noexcept {
	defenseTier = static_cast<const float*>(data.getDataPortion(0).get())[0];
}

InteractionBoolHolder CustomTankPower::modifiedEdgeCollision(Tank* t) {
	CollisionHandler::edgeConstrain(t, modifiedCollisionWithEdge_AdditionalBoundaryAmount);
	return { false };
}

InteractionBoolHolder CustomTankPower::modifiedDeathHandling(const Tank* parent) {
	if (this->maxTime < 0) {
		return { false, false };
	}
	if (this->timeLeft <= modifiedDeathHandling_DurationSubtractPercent * GameManager::get_settings().PowerupDurationBaseTime) {
		return { false, true };
	}
	this->timeLeft -= modifiedDeathHandling_DurationSubtractPercent * GameManager::get_settings().PowerupDurationBaseTime;
	return { false, false };
}

void CustomTankPower::additionalShooting(Tank* t, const TankCannonPoint& c, const TankExtraCannonPoint& c2) {
	for (int i = 0; i < additionalShooting_BulletCount; i++) {
		float tempAngle_fromTank = (GameRNG::randFuncf()*2 - 1) * additionalShooting_AngleRelativeToTank;
		float tempAngle_fromCannon = (GameRNG::randFuncf()*2 - 1) * additionalShooting_AngleRelativeToCannon;
		t->defaultMakeBullet(t->velocity.getAngle() + c.angleFromCenter + c2.angleFromCenter + tempAngle_fromTank,
		                     c2.angleFromEdge + tempAngle_fromCannon);
	}
}

std::vector<float>* CustomTankPower::addShootingPoints() const {
	return new std::vector<float>(this->addShootingPoints_AngleList);
}

std::vector<std::pair<float, float>>* CustomTankPower::addExtraShootingPoints() const {
	return new std::vector<std::pair<float, float>>(this->addExtraShootingPoints_AngleList);
}

InteractionBoolHolder CustomBulletPower::modifiedEdgeCollision(Bullet* b) {
	//see BounceBulletPower::modifiedEdgeCollision

	bool bouncedY = false;
	//bool bouncedX = false;
	BulletUpdateStruct b_update;

	if (CollisionHandler::partiallyOutOfBounds(b)) {
		auto result = PowerFunctionHelper::bounceEdgeGenericY(b);
		if (result.first) {
			modifiedCollision_BouncesLeft--;
			bouncedY = true;
		}
		//TODO: update modifiedEdgeCollision to also use update structs
		b->update(&result.second);
		if (modifiedCollision_BouncesLeft <= 0) {
			modifiedCollisionWithWall_EnableBounce = false;
			updateModifiesCollisionWithWall();
			modifiesEdgeCollision = false;
			return { CollisionHandler::fullyOutOfBounds(b) };
		}
	}

	if (CollisionHandler::partiallyOutOfBounds(b)) {
		auto result = PowerFunctionHelper::bounceEdgeGenericX(b);
		if (result.first) {
			modifiedCollision_BouncesLeft--;
			//bouncedX = true;
		}
		//TODO: update modifiedEdgeCollision to also use update structs
		b->update(&result.second);
		if (modifiedCollision_BouncesLeft <= 0) {
			modifiedCollisionWithWall_EnableBounce = false;
			updateModifiesCollisionWithWall();
			modifiesEdgeCollision = false;
			return { CollisionHandler::fullyOutOfBounds(b) };
		}
	}

	if (!bouncedY && CollisionHandler::partiallyOutOfBounds(b)) {
		auto result = PowerFunctionHelper::bounceEdgeGenericY(b);
		if (result.first) {
			modifiedCollision_BouncesLeft--;
			//bouncedY = true;
		}
		//TODO: update modifiedEdgeCollision to also use update structs
		b->update(&result.second);
		if (modifiedCollision_BouncesLeft <= 0) {
			modifiedCollisionWithWall_EnableBounce = false;
			updateModifiesCollisionWithWall();
			modifiesEdgeCollision = false;
			//return { CollisionHandler::fullyOutOfBounds(b) };
		}
	}

	return { CollisionHandler::fullyOutOfBounds(b) };
}

InteractionUpdateHolder<BulletUpdateStruct, WallUpdateStruct> CustomBulletPower::modifiedCollisionWithWall(const Bullet* b, const Wall* w) {
	//see BounceBulletPower::modifiedCollisionWithWall

	if (!modifiedCollisionWithWall_EnableBounce) {
		//from BigNamedBulletPower::modifiedCollisionWithWall
		return { false, true, nullptr, nullptr };
	}

	std::shared_ptr<BulletUpdateStruct> b_update;
	std::shared_ptr<WallUpdateStruct> w_update;

	if (b->velocity.getMagnitude() * float(Bullet::default_radius/b->r) <= .5f) {
		auto result = PowerFunctionHelper::bounceGenericWithCorners(b, w);
		b_update = result.second.firstUpdate;
		w_update = result.second.secondUpdate;
		if (result.first) {
			modifiedCollision_BouncesLeft--;
		}
	} else {
		auto result = PowerFunctionHelper::bounceGeneric(b, w);
		b_update = result.second.firstUpdate;
		w_update = result.second.secondUpdate;
		if (result.first) {
			modifiedCollision_BouncesLeft--;
		}
	}

	if (modifiedCollision_BouncesLeft <= 0) {
		modifiedCollisionWithWall_EnableBounce = false;
		updateModifiesCollisionWithWall();
		modifiesEdgeCollision = false;
	}

	return { (modifiedCollision_BouncesLeft < 0), this->modifiedCollisionWithWall_DestroyWallEnable, b_update, w_update };
}

InteractionBoolHolder CustomBulletPower::modifiedDeathHandling(const Bullet* parent) {
	return { false, true };
}

TankPower* CustomPower::makeTankPower() const {
	return new CustomTankPower(name, color, colorImportance, tankDuration,
		initializationActions_tank, initializationActions_bullet);
}

BulletPower* CustomPower::makeBulletPower() const {
	return new CustomBulletPower(name, color, colorImportance, tankDuration,
		initializationActions_tank, initializationActions_bullet);
}

TankPower* CustomTankPower::makeDuplicate() const {
	return new CustomTankPower(name, color, colorImportance, tankDuration,
		initializationActions_tank, initializationActions_bullet,
		timeLeft);
}

BulletPower* CustomTankPower::makeBulletPower() const {
	return new CustomBulletPower(name, color, colorImportance, tankDuration,
		initializationActions_tank, initializationActions_bullet);
}

BulletPower* CustomBulletPower::makeDuplicate() const {
	return new CustomBulletPower(name, color, colorImportance, tankDuration,
		initializationActions_tank, initializationActions_bullet,
		bulletRadiusMultiplier, bulletSpeedMultiplier, bulletAcceleration, modifiedCollision_BouncesLeft);
}

TankPower* CustomBulletPower::makeTankPower() const {
	return new CustomTankPower(name, color, colorImportance, tankDuration,
		initializationActions_tank, initializationActions_bullet);
}

CustomPower* CustomPower::factory() const {
	return new CustomPower(name,
		color.getRf(), color.getGf(), color.getBf(),
		tankDuration,
		powerTypes, powerAttributes, powerWeights,
		initializationActions_tank,
		initializationActions_bullet);
}



void CustomPowerInterpreter::ProcessCustomPowers() noexcept {
	//get list of mods, interpret them, then add factories to PowerupDataGovernor
	//should syntax version be a thing? likely features will only be added, but who knows

	auto modOrder_temp = ModProcessor::getListOfMods();
	if (!modOrder_temp.has_value()) {
		return;
	}
	std::vector<std::string>& modOrder_list = modOrder_temp.value();

	for (int i = 0; i < modOrder_list.size(); i++) {
		std::vector<std::string> powerOrder_list;
		try {
			powerOrder_list = ModProcessor::getListOfFiles("mods/" + modOrder_list[i] + "/powers");
		}
		catch (const std::filesystem::filesystem_error& e) {
			std::cerr << "ModProcessor::getListOfFiles(" + ("mods/" + modOrder_list[i] + "/powers") + ") filesystem_error: " << e.what() << std::endl;
			continue; //hope things work out on the next mod
		}

		for (int j = 0; j < powerOrder_list.size(); j++) {
			//std::cout << "modOrder_list[" << i << "]: " << "powerOrder_list[" << j << "]: mods/" + modOrder_list[i] + "/powers/" << powerOrder_list[j] << std::endl;
			std::string powerPath = "mods/" + modOrder_list[i] + "/powers/" + powerOrder_list[j];
			CustomPower* power;
			try {
				power = processCustomPower(powerPath);
			}
			catch (const std::runtime_error& e) {
				std::cerr << "ERROR: Unable to process " + powerPath + ", reason: " << e.what() << std::endl;
				continue;
			}

			if (power == nullptr) [[unlikely]] {
				//std::cerr << "WARNING: Bad path \"" + powerPath + "\"" << std::endl;
				continue;
			} else {
				try {
					addCustomPower(power);
				}
				catch (const std::runtime_error& e) {
					std::cerr << "ERROR: Unable to materialize " + powerPath + ", reason: " << e.what() << std::endl;
					delete power;
					continue;
				}
			}

			std::cout << "Processed custom power: " + powerPath << std::endl;
		}
	}

	std::cout << std::endl;
}

CustomPower* CustomPowerInterpreter::processCustomPower(std::string path) {
	if (path == "") [[unlikely]] {
		return nullptr;
	}

	std::string name;
	float colorR, colorG, colorB;
	double tankDurationMultiplier = 1;
	std::vector<std::string> types;
	std::vector<std::string> attributes = std::vector<std::string>{ "stack", "mix" };
	std::unordered_map<std::string, float> weights;
	std::vector<CustomPower::CustomPowerAction_Tank*>* actions_tank;
	std::vector<CustomPower::CustomPowerAction_Bullet*>* actions_bullet;

	bool name_set = false;
	bool color_set = false;
	bool types_set = false;
	//bool attributes_set = false;
	bool weights_set = false;

	std::vector<std::string> temp_weights; //set when reading the file
	int temp_weights_lineNum;

	bool powerAction_tankStarted = false;
	bool powerAction_bulletStarted = false;
	std::vector<std::pair<std::string, int>> powerAction_tankList;
	std::vector<std::pair<std::string, int>> powerAction_bulletList;

	std::ifstream power_file;
	power_file.open(path);

	if (power_file.is_open()) {
		std::string line;
		int lineNum = 0;
		std::string error_string = ""; //when an exception is generated, this is set and the loop below is broken out of (so the file can get closed)

		while (std::getline(power_file, line)) {
			line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
			line.erase(std::remove(line.begin(), line.end(), '\n'), line.end());
			lineNum++;
			BasicINIParser::removeLeftWhitespace(line);
			BasicINIParser::removeComments(line);
			BasicINIParser::removeRightWhitespace(line);
			if (line.size() == 0) {
				continue;
			}

			if (line == ActionStartPhrase_TankPower) [[unlikely]] {
				powerAction_tankStarted = true;
				powerAction_bulletStarted = false;
				continue;
			} else if (line == ActionStartPhrase_BulletPower) [[unlikely]] {
				powerAction_tankStarted = false;
				powerAction_bulletStarted = true;
				continue;
			}

			if (!powerAction_tankStarted && !powerAction_bulletStarted) { //assignment
				//read the same way as an INI file: split on the '=', left is name, right is values
				//values get split on spaces (just like INI files)
				//code copied from BasicINIParser

				int separator_index = BasicINIParser::findSeparatorIndex(line);
				if (separator_index == -1) [[unlikely]] {
					error_string = "Syntax error on line " + std::to_string(lineNum) + ": no '='";
					break;
				} else {
					std::string assignmentName = line.substr(0, separator_index);
					std::string assignmentValues = line.substr(separator_index+1);
					BasicINIParser::removeRightWhitespace(assignmentName);
					BasicINIParser::processEscapeSequences_most(assignmentValues);

					std::vector<std::string> assignmentValues_list;
					while (assignmentValues.size() != 0) {
						BasicINIParser::removeLeftWhitespace(assignmentValues);
						int substr_end = BasicINIParser::findSubstringEndIndex(assignmentValues);
						if (assignmentValues[0] == '\"') {
							assignmentValues_list.push_back(assignmentValues.substr(1, substr_end-1));
							assignmentValues.erase(0, substr_end+1);
						} else {
							assignmentValues_list.push_back(assignmentValues.substr(0, substr_end));
							assignmentValues.erase(0, substr_end);
						}
						BasicINIParser::processEscapeSequences_all(assignmentValues_list[assignmentValues_list.size()-1]);
					}

					if (assignmentValues_list.size() == 0) [[unlikely]] {
						error_string = "Syntax error on line " + std::to_string(lineNum) + ": no data given for \"" + assignmentName + "\"";
						break;
					}

					if (assignmentName == "Name") {
						name = assignmentValues_list[0];
						name_set = true;
					}
					else if (assignmentName == "Color") {
						if (assignmentValues_list.size() != 3) [[unlikely]] {
							error_string = "Syntax error on line " + std::to_string(lineNum) + ": expected 3 data items for \"Color\" but got " + std::to_string(assignmentValues_list.size());
							break;
						}
						try {
							colorR = std::stof(assignmentValues_list[0]);
							colorG = std::stof(assignmentValues_list[1]);
							colorB = std::stof(assignmentValues_list[2]);
						}
						catch (const std::exception&) {
							//std::invalid_argument (and std::out_of_range)
							error_string = "Syntax error on line " + std::to_string(lineNum) + ": unable to parse \"" + assignmentName + "\" values";
							break;
						}
						color_set = true;
					}
					else if (assignmentName == "PowerTypes") {
						types = assignmentValues_list;
						types_set = true;
					}
					else if (assignmentName == "PowerWeights") {
						temp_weights = assignmentValues_list;
						temp_weights_lineNum = lineNum;
						weights_set = true;
					}
					else if (assignmentName == "PowerAttributes") {
						attributes = assignmentValues_list;
						//attributes_set = true;
					}
					else if (assignmentName == "PowerTankDurationMultiplier") {
						if (assignmentValues_list.size() != 1) [[unlikely]] {
							error_string = "Syntax error on line " + std::to_string(lineNum) + ": expected 1 data item for \"PowerTankDurationMultiplier\" but got " + std::to_string(assignmentValues_list.size());
							break;
						}
						try {
							tankDurationMultiplier = std::stod(assignmentValues_list[0]);
						}
						catch (const std::exception&) {
							error_string = "Syntax error on line " + std::to_string(lineNum) + ": unable to parse \"" + assignmentName + "\" values";
							break;
						}
					}
					else {
						//error_string = "Syntax error on line " + std::to_string(lineNum) + ": unknown assignment \"" + assignmentName + "\"";
						//break;
						std::cerr << "Warning parsing power " + path + ": line " + std::to_string(lineNum) + ": unknown assignment \"" + assignmentName + "\"" << std::endl;
					}
				}

			} else { //command
				//just push the line to a list to be processed later
				if (powerAction_tankStarted) {
					powerAction_tankList.push_back({ line, lineNum });
				} else { //powerAction_bulletStarted
					powerAction_bulletList.push_back({ line, lineNum });
				}
			}
		}

		power_file.close();
		if (error_string != "") {
			throw std::runtime_error(error_string);
		}
	} else {
		throw std::runtime_error("Could not read file \"" + path + "\"");
	}

	if (!name_set) {
		throw std::runtime_error("Error parsing power: power name not set");
	}
	if (!color_set) {
		throw std::runtime_error("Error parsing power: power color not set");
	}
	if (!types_set) {
		throw std::runtime_error("Error parsing power: power types not set");
	}
	if (!weights_set) {
		throw std::runtime_error("Error parsing power: power weights not set");
	}

	if (types.size() == 0) {
		throw std::runtime_error("Error parsing power: no power types stated");
	}
	if (types.size() < temp_weights.size()) {
		throw std::runtime_error("Error parsing power: more power types than power weights");
	}
	if (types.size() != temp_weights.size()) {
		throw std::runtime_error("Error parsing power: more power weights than power types");
	}
	for (int i = 0; i < types.size(); i++) {
		float weight;
		try {
			weight = std::stof(temp_weights[i]);
		}
		catch (const std::exception&) {
			throw std::runtime_error("Syntax error on line " + std::to_string(temp_weights_lineNum) + ": unable to parse \"PowerWeights\" value[" + std::to_string(i) + "]: \"" + temp_weights[i] + "\"");
		}
		weights.insert({ types[i], weight });
	}

	actions_tank = new std::vector<CustomPower::CustomPowerAction_Tank*>;
	for (int i = 0; i < powerAction_tankList.size(); i++) {
		try {
			actions_tank->push_back(stringToAction_tank(powerAction_tankList[i]));
		}
		catch (const std::runtime_error& e) {
			delete actions_tank;
			throw std::runtime_error("Error parsing power_tank: " + std::string(e.what()));
		}
	}

	actions_bullet = new std::vector<CustomPower::CustomPowerAction_Bullet*>;
	for (int i = 0; i < powerAction_bulletList.size(); i++) {
		try {
			actions_bullet->push_back(stringToAction_bullet(powerAction_bulletList[i]));
		}
		catch (const std::runtime_error& e) {
			delete actions_tank;
			delete actions_bullet;
			throw std::runtime_error("Error parsing power_bullet: " + std::string(e.what()));
		}
	}

	return new CustomPower(name,
		colorR, colorG, colorB,
		tankDurationMultiplier * GameManager::get_settings().PowerupDurationBaseTime,
		types, attributes, weights,
		actions_tank, actions_bullet);
}

inline CustomPower::CustomPowerAction_Tank* CustomPowerInterpreter::stringToAction_tank(const std::pair<std::string, int>& command) {
	std::string line = command.first;
	std::vector<std::string> words;
	while (line.size() != 0) {
		BasicINIParser::removeLeftWhitespace(line);
		int substr_end = BasicINIParser::findSubstringEndIndex(line);
		if (line[0] == '\"') {
			words.push_back(line.substr(1, substr_end-1));
			line.erase(0, substr_end+1);
		} else {
			words.push_back(line.substr(0, substr_end));
			line.erase(0, substr_end);
		}
		//BasicINIParser::processEscapeSequences_all(words[words.size()-1]);
	}

	CustomPower::CustomPowerCommands_TankPower powerCommand_tank = CustomPower::strToCommand_tank(words[0]);
	std::string powerCommand_tank_strBackup = words[0]; //for printing error messages
	words.erase(words.begin()); //remove the command type from the parameter list

	CustomPower::CustomPowerAction_Tank* powerAction_tank = new CustomPower::CustomPowerAction_Tank(powerCommand_tank);
	GenericFactoryConstructionData& constructionData = powerAction_tank->data;

	try {
		switch (powerCommand_tank) {
			case CustomPower::CustomPowerCommands_TankPower::additionalShooting_Enable:
				stringToAction_genericEnable(words, constructionData);
				break;
			case CustomPower::CustomPowerCommands_TankPower::additionalShooting_AngleRelativeToTank:
				stringToAction_genericSingleValue_f(words, constructionData); //note: angle in degrees
				break;
			case CustomPower::CustomPowerCommands_TankPower::additionalShooting_AngleRelativeToCannon:
				stringToAction_genericSingleValue_f(words, constructionData); //note: angle in degrees
				break;
			case CustomPower::CustomPowerCommands_TankPower::additionalShooting_BulletCount:
				stringToAction_genericSingleValue_i(words, constructionData);
				break;

			case CustomPower::CustomPowerCommands_TankPower::addShootingPoints_Enable:
				stringToAction_genericEnable(words, constructionData);
				break;
			case CustomPower::CustomPowerCommands_TankPower::addShootingPoints_EquallySpaced:
				stringToAction_genericSingleValue_i(words, constructionData);
				break;
			case CustomPower::CustomPowerCommands_TankPower::addShootingPoints_Raw:
				stringToAction_addShootingPoints_Raw(words, constructionData);
				break;

			case CustomPower::CustomPowerCommands_TankPower::addExtraShootingPoints_Enable:
				stringToAction_genericEnable(words, constructionData);
				break;
			case CustomPower::CustomPowerCommands_TankPower::addExtraShootingPoints_Raw:
				stringToAction_addExtraShootingPoints_Raw(words, constructionData);
				break;

			case CustomPower::CustomPowerCommands_TankPower::modifiedCollisionWithEdge_Enable:
				stringToAction_genericEnable(words, constructionData);
				break;
			case CustomPower::CustomPowerCommands_TankPower::modifiedCollisionWithEdge_AdditionalBoundary:
				stringToAction_genericSingleValue_d(words, constructionData);
				break;

			case CustomPower::CustomPowerCommands_TankPower::modifiedDeathHandling_Enable:
				stringToAction_genericEnable(words, constructionData);
				break;
			case CustomPower::CustomPowerCommands_TankPower::modifiedDeathHandling_DurationSubtractPercent:
				stringToAction_genericSingleValue_d(words, constructionData);
				break;

			case CustomPower::CustomPowerCommands_TankPower::tankMaxSpeedMultiplier:
				stringToAction_genericSingleValue_f(words, constructionData);
				break;
			case CustomPower::CustomPowerCommands_TankPower::tankMaxSpeedStacks:
				stringToAction_genericEnable(words, constructionData);
				break;
			case CustomPower::CustomPowerCommands_TankPower::tankAccelerationMultiplier:
				stringToAction_genericSingleValue_f(words, constructionData);
				break;
			case CustomPower::CustomPowerCommands_TankPower::tankAccelerationStacks:
				stringToAction_genericEnable(words, constructionData);
				break;
			case CustomPower::CustomPowerCommands_TankPower::tankRadiusMultiplier:
				stringToAction_genericSingleValue_d(words, constructionData);
				break;
			case CustomPower::CustomPowerCommands_TankPower::tankRadiusStacks:
				stringToAction_genericEnable(words, constructionData);
				break;
			case CustomPower::CustomPowerCommands_TankPower::tankFiringRateMultiplier:
				stringToAction_genericSingleValue_d(words, constructionData);
				break;
			case CustomPower::CustomPowerCommands_TankPower::tankFiringRateStacks:
				stringToAction_genericEnable(words, constructionData);
				break;
			case CustomPower::CustomPowerCommands_TankPower::tankTurningIncrementMultiplier:
				stringToAction_genericSingleValue_f(words, constructionData);
				break;
			case CustomPower::CustomPowerCommands_TankPower::tankTurningIncrementStacks:
				stringToAction_genericEnable(words, constructionData);
				break;

			case CustomPower::CustomPowerCommands_TankPower::offenseImportance:
				stringToAction_genericSingleValue_f(words, constructionData);
				break;
			case CustomPower::CustomPowerCommands_TankPower::offenseTier:
				stringToAction_genericSingleValue_f(words, constructionData);
				break;
			case CustomPower::CustomPowerCommands_TankPower::defenseImportance:
				stringToAction_genericSingleValue_f(words, constructionData);
				break;
			case CustomPower::CustomPowerCommands_TankPower::defenseTier:
				stringToAction_genericSingleValue_f(words, constructionData);
				break;

			default:
				throw std::runtime_error("Unknown power_tank command!");
				//break;
		}
	}
	catch (const std::runtime_error& e) {
		delete powerAction_tank;
		throw std::runtime_error("line " + std::to_string(command.second) + " command \"" + powerCommand_tank_strBackup + "\": " + e.what());
	}

	return powerAction_tank;
}

inline CustomPower::CustomPowerAction_Bullet* CustomPowerInterpreter::stringToAction_bullet(const std::pair<std::string, int>& command) {
	std::string line = command.first;
	std::vector<std::string> words;
	while (line.size() != 0) {
		BasicINIParser::removeLeftWhitespace(line);
		int substr_end = BasicINIParser::findSubstringEndIndex(line);
		if (line[0] == '\"') {
			words.push_back(line.substr(1, substr_end-1));
			line.erase(0, substr_end+1);
		} else {
			words.push_back(line.substr(0, substr_end));
			line.erase(0, substr_end);
		}
		//BasicINIParser::processEscapeSequences_all(words[words.size()-1]);
	}

	CustomPower::CustomPowerCommands_BulletPower powerCommand_bullet = CustomPower::strToCommand_bullet(words[0]);
	std::string powerCommand_bullet_strBackup = words[0]; //for printing error messages
	words.erase(words.begin()); //remove the command type from the parameter list

	CustomPower::CustomPowerAction_Bullet* powerAction_bullet = new CustomPower::CustomPowerAction_Bullet(powerCommand_bullet);
	GenericFactoryConstructionData& constructionData = powerAction_bullet->data;

	try {
		switch (powerCommand_bullet) {
			case CustomPower::CustomPowerCommands_BulletPower::modifiedCollision_BounceCount:
				stringToAction_genericSingleValue_i(words, constructionData);
				break;

			case CustomPower::CustomPowerCommands_BulletPower::modifiedCollisionWithEdge_EnableBounce:
				stringToAction_genericEnable(words, constructionData);
				break;

			case CustomPower::CustomPowerCommands_BulletPower::modifiedCollisionWithWall_EnableBounce:
				stringToAction_genericEnable(words, constructionData);
				break;
			case CustomPower::CustomPowerCommands_BulletPower::modifiedCollisionWithWall_DestroyWall:
				stringToAction_genericEnable(words, constructionData);
				break;

			case CustomPower::CustomPowerCommands_BulletPower::modifiedDeathHandling_Enable:
				stringToAction_genericEnable(words, constructionData);
				break;

			case CustomPower::CustomPowerCommands_BulletPower::bulletSpeedMultiplier:
				stringToAction_bulletSpeedMultiplier(words, constructionData);
				break;
			case CustomPower::CustomPowerCommands_BulletPower::bulletSpeedStacks:
				stringToAction_genericEnable(words, constructionData);
				break;
			case CustomPower::CustomPowerCommands_BulletPower::bulletRadiusMultiplier:
				stringToAction_bulletRadiusMultiplier(words, constructionData);
				break;
			case CustomPower::CustomPowerCommands_BulletPower::bulletRadiusStacks:
				stringToAction_genericEnable(words, constructionData);
				break;
			case CustomPower::CustomPowerCommands_BulletPower::bulletDeceleration:
				stringToAction_bulletDeceleration(words, constructionData);
				break;
			case CustomPower::CustomPowerCommands_BulletPower::bulletAccelerationImportance:
				stringToAction_genericSingleValue_f(words, constructionData);
				break;
			case CustomPower::CustomPowerCommands_BulletPower::bulletDeceleration_DegradeAmount:
				stringToAction_genericSingleValue_f(words, constructionData);
				break;

			case CustomPower::CustomPowerCommands_BulletPower::offenseImportance:
				stringToAction_genericSingleValue_f(words, constructionData);
				break;
			case CustomPower::CustomPowerCommands_BulletPower::offenseTier:
				stringToAction_genericSingleValue_f(words, constructionData);
				break;
			case CustomPower::CustomPowerCommands_BulletPower::defenseImportance:
				stringToAction_genericSingleValue_f(words, constructionData);
				break;
			case CustomPower::CustomPowerCommands_BulletPower::defenseTier:
				stringToAction_genericSingleValue_f(words, constructionData);
				break;

			default:
				throw std::runtime_error("Unknown power_bullet command!");
				//break;
		}
	}
	catch (const std::runtime_error& e) {
		delete powerAction_bullet;
		throw std::runtime_error("line " + std::to_string(command.second) + " command \"" + powerCommand_bullet_strBackup + "\": " + e.what());
	}

	return powerAction_bullet;
}

void CustomPowerInterpreter::addCustomPower(CustomPower* power) {
	const BasicINIParser::BasicINIData& ini_data = GameManager::get_INI();

	std::vector<std::string> types = power->getPowerTypes();
	if (ini_data.exists("MODS", "ModSafetyChecks") && std::stoi(ini_data.get("MODS", "ModSafetyChecks"))) {
		std::string result = PowerupDataGovernor::checkCustomPowerTypesAgainstProtectedTypes(types);
		if (result != "") {
			throw std::runtime_error("Error pushing power " + power->getName() + ": power includes \"" + result + "\" type, which is not allowed");
		}
	}

	PowerupDataGovernor::addCustomPower(power->getName(), types, power);
}

inline void CustomPowerInterpreter::stringToAction_genericEnable(const std::vector<std::string>& words, GenericFactoryConstructionData& constructionData) {
	if (words.size() < 1) {
		//constructionData.pushData(1, new bool[1]{ true }); //CustomPower assumes true is the default
		return;
	}

	bool enable;
	try {
		enable = std::stoi(words[0]);
	}
	catch (const std::exception&) {
		throw std::runtime_error("unable to parse values");
		//plural but whatever, makes copy-pasting elsewhere easier
	}

	constructionData.pushData(1, new bool[1]{ enable });
}

inline void CustomPowerInterpreter::stringToAction_genericSingleValue_i(const std::vector<std::string>& words, GenericFactoryConstructionData& constructionData) {
	if (words.size() < 1) {
		throw std::runtime_error("expected 1 data item");
	}

	int value;
	try {
		value = std::stoi(words[0]);
	}
	catch (const std::exception&) {
		throw std::runtime_error("unable to parse values");
	}

	constructionData.pushData(1, new int[1]{ value });
}

inline void CustomPowerInterpreter::stringToAction_genericSingleValue_f(const std::vector<std::string>& words, GenericFactoryConstructionData& constructionData) {
	if (words.size() < 1) {
		throw std::runtime_error("expected 1 data item");
	}

	float value;
	try {
		value = std::stof(words[0]);
	}
	catch (const std::exception&) {
		throw std::runtime_error("unable to parse values");
	}

	constructionData.pushData(1, new float[1]{ value });
}

inline void CustomPowerInterpreter::stringToAction_genericSingleValue_d(const std::vector<std::string>& words, GenericFactoryConstructionData& constructionData) {
	if (words.size() < 1) {
		throw std::runtime_error("expected 1 data item");
	}

	double value;
	try {
		value = std::stod(words[0]);
	}
	catch (const std::exception&) {
		throw std::runtime_error("unable to parse values");
	}

	constructionData.pushData(1, new double[1]{ value });
}

inline void CustomPowerInterpreter::stringToAction_addShootingPoints_Raw(const std::vector<std::string>& words, GenericFactoryConstructionData& constructionData) {
	if (words.size() < 1) {
		throw std::runtime_error("expected at least 1 data item");
	}

	float* values = new float[words.size()];
	try {
		for (int i = 0; i < words.size(); i++) {
			values[i] = std::stof(words[i]) / 360;
		}
	}
	catch (const std::exception&) {
		delete[] values;
		throw std::runtime_error("unable to parse values");
	}

	constructionData.pushData(words.size(), values);
}

inline void CustomPowerInterpreter::stringToAction_addExtraShootingPoints_Raw(const std::vector<std::string>& words, GenericFactoryConstructionData& constructionData) {
	if (words.size() < 1) {
		throw std::runtime_error("expected at least 1 data item");
	}
	if (words.size() % 2 == 1) {
		throw std::runtime_error("expected an even number of data items");
	}

	float* values = new float[words.size()];
	try {
		for (int i = 0; i < words.size(); i++) {
			values[i] = std::stof(words[i]) / 360;
		}
	}
	catch (const std::exception&) {
		delete[] values;
		throw std::runtime_error("unable to parse values");
	}

	constructionData.pushData(words.size(), values);
}

inline void CustomPowerInterpreter::stringToAction_bulletSpeedMultiplier(const std::vector<std::string>& words, GenericFactoryConstructionData& constructionData) {
	if (words.size() < 1) {
		throw std::runtime_error("expected 1 or 2 data items but got " + std::to_string(words.size()));
	}

	float velMin, velMax;
	try {
		velMin = std::stof(words[0]);
		if (words.size() > 1) {
			velMax = std::stof(words[1]);
		} else {
			velMax = velMin;
		}
	}
	catch (const std::exception&) {
		throw std::runtime_error("unable to parse values");
	}

	constructionData.pushData(2, new float[2]{ velMin, velMax });
}

inline void CustomPowerInterpreter::stringToAction_bulletRadiusMultiplier(const std::vector<std::string>& words, GenericFactoryConstructionData& constructionData) {
	if (words.size() < 1) {
		throw std::runtime_error("expected 1 or 2 data items but got " + std::to_string(words.size()));
	}

	double radMin, radMax;
	try {
		radMin = std::stod(words[0]);
		if (words.size() > 1) {
			radMax = std::stod(words[1]);
		} else {
			radMax = radMin;
		}
	}
	catch (const std::exception&) {
		throw std::runtime_error("unable to parse values");
	}

	constructionData.pushData(2, new double[2]{ radMin, radMax });
}

inline void CustomPowerInterpreter::stringToAction_bulletDeceleration(const std::vector<std::string>& words, GenericFactoryConstructionData& constructionData) {
	if (words.size() < 1) {
		throw std::runtime_error("expected 1 or 2 data items but got " + std::to_string(words.size()));
	}

	float decMin, decMax;
	try {
		decMin = std::stof(words[0]);
		if (words.size() > 1) {
			decMax = std::stof(words[1]);
		} else {
			decMax = decMin;
		}
	}
	catch (const std::exception&) {
		throw std::runtime_error("unable to parse values");
	}

	constructionData.pushData(2, new float[2]{ decMin, decMax });
}
