#pragma once
#include <string>
#include <vector>
#include <memory>

#include "power.h"
#include "generic-factory-construction-data.h"

class CustomPower : public Power {
	friend class CustomPowerInterpreter;

public:
	enum class CustomPowerCommands_TankPower {
		additionalShooting_Enable,
		additionalShooting_AngleRelativeToTank,
		additionalShooting_AngleRelativeToCannon,
		additionalShooting_BulletCount,

		addShootingPoints_Enable,
		addShootingPoints_EquallySpaced,
		addShootingPoints_Raw,

		addExtraShootingPoints_Enable,
		addExtraShootingPoints_Raw,

		modifiedCollisionWithEdge_Enable, //not modifiedEdgeCollision for consistency with bulletpowers
		modifiedCollisionWithEdge_AdditionalBoundary,

		//modifiedCollisionWithTank_Enable,

		//modifiedCollisionWithWall_Enable,
		//modifiedCollisionWithWall_DestroyWall,

		//modifiedCollisionWithCircleHazard_Enable,
		//modifiedCollisionWithRectHazard_Enable,

		modifiedDeathHandling_Enable,
		modifiedDeathHandling_DurationSubtractPercent,

		tankMaxSpeedMultiplier,
		tankMaxSpeedStacks,
		tankAccelerationMultiplier,
		tankAccelerationStacks,
		tankRadiusMultiplier,
		tankRadiusStacks,
		tankFiringRateMultiplier,
		tankFiringRateStacks,
		tankTurningIncrementMultiplier,
		tankTurningIncrementStacks,

		offenseImportance,
		offenseTier,
		defenseImportance,
		defenseTier,

		UNKNOWN //error, don't use
	};
	enum class CustomPowerCommands_BulletPower {
		modifiedCollision_BounceCount,

		modifiedCollisionWithEdge_EnableBounce, //not modifiedEdgeCollision for consistency with wall collision

		//modifiedCollisionWithTank_Enable,

		modifiedCollisionWithWall_EnableBounce, //no ultrabouncing
		modifiedCollisionWithWall_DestroyWall,

		//modifiedCollisionWithCircleHazard_Enable,
		//modifiedCollisionWithRectHazard_Enable,

		modifiedDeathHandling_Enable, //TODO: other options?

		bulletSpeedMultiplier,
		bulletSpeedStacks,
		bulletRadiusMultiplier,
		bulletRadiusStacks,
		bulletDeceleration,
		bulletAccelerationImportance,
		bulletDeceleration_DegradeAmount,

		offenseImportance,
		offenseTier,
		defenseImportance,
		defenseTier,

		UNKNOWN //error, don't use
	};

	static CustomPowerCommands_TankPower strToCommand_tank(const std::string&) noexcept; //UNKNOWN when can't parse
	static CustomPowerCommands_BulletPower strToCommand_bullet(const std::string&) noexcept; //UNKNOWN when can't parse

	struct CustomPowerAction_Tank {
		CustomPowerCommands_TankPower command;
		GenericFactoryConstructionData data;
		CustomPowerAction_Tank(CustomPowerCommands_TankPower command);
	};
	struct CustomPowerAction_Bullet {
		CustomPowerCommands_BulletPower command;
		GenericFactoryConstructionData data;
		CustomPowerAction_Bullet(CustomPowerCommands_BulletPower command);
	};

protected:
	//for CustomPowerInterpreter
	CustomPower(std::string name,
		float colorR, float colorG, float colorB,
		double tankDuration,
		const std::vector<std::string>& types,
		const std::vector<std::string>& attributes,
		const std::unordered_map<std::string, float>& weights,
		std::vector<CustomPowerAction_Tank*>* actions_tank,
		std::vector<CustomPowerAction_Bullet*>* actions_bullet);
	//for CustomPower::factory()
	CustomPower(std::string name,
		float colorR, float colorG, float colorB,
		double tankDuration,
		const std::vector<std::string>& types,
		const std::vector<std::string>& attributes,
		const std::unordered_map<std::string, float>& weights,
		std::shared_ptr<std::vector<CustomPowerAction_Tank*>> actions_tank,
		std::shared_ptr<std::vector<CustomPowerAction_Bullet*>> actions_bullet);

protected:
	std::vector<std::string> powerTypes;
	std::unordered_map<std::string, float> powerWeights;
	std::vector<std::string> powerAttributes;
	std::string name;
	ColorValueHolder color;
	float colorImportance;

	double tankDuration;
	std::shared_ptr<std::vector<CustomPowerAction_Tank*>> initializationActions_tank; //remember, the vector's elements need to be deleted
	std::shared_ptr<std::vector<CustomPowerAction_Bullet*>> initializationActions_bullet; //remember, the vector's elements need to be deleted

public:
	//shouldn't be virtual
	std::vector<std::string> getPowerTypes() const override { return this->powerTypes; }
	std::unordered_map<std::string, float> getWeights() const override { return this->powerWeights; }
	std::vector<std::string> getPowerAttributes() const override { return this->powerAttributes; }

	std::string getName() const override { return this->name; }
	ColorValueHolder getColor() const override { return this->color; }
	float getColorImportance() const override { return this->colorImportance; }

	TankPower* makeTankPower() const override;
	BulletPower* makeBulletPower() const override;

	virtual ~CustomPower();
	[[nodiscard]] CustomPower* factory() const; //makes a copy of itself; used by CustomPowerInterpreter to give PowerupDataGovernor an actual power
};



class CustomTankPower : public TankPower {
protected:
	std::string name;
	ColorValueHolder color;
	float colorImportance;
	double tankDuration; //on the off chance maxTime gets changed
	std::shared_ptr<std::vector<CustomPower::CustomPowerAction_Tank*>> initializationActions_tank;
	std::shared_ptr<std::vector<CustomPower::CustomPowerAction_Bullet*>> initializationActions_bullet;

	float  tankMaxSpeedMultiplier;
	float  tankAccelerationMultiplier;
	double tankRadiusMultiplier;
	double tankFiringRateMultiplier;
	float  tankTurningIncrementMultiplier;

	float offenseImportance;
	float offenseTier;
	float defenseImportance;
	float defenseTier;

	float additionalShooting_AngleRelativeToTank;
	float additionalShooting_AngleRelativeToCannon;
	int additionalShooting_BulletCount;

	std::vector<float> addShootingPoints_AngleList;
	std::vector<std::pair<float, float>> addExtraShootingPoints_AngleList;

	double modifiedCollisionWithEdge_AdditionalBoundaryAmount;

	double modifiedDeathHandling_DurationSubtractPercent;

protected:
	inline void initialization_helper_Enable(bool& thingToUpdate, const GenericFactoryConstructionData&) noexcept;

	inline void initialization_additionalShooting_Enable(const GenericFactoryConstructionData&) noexcept;
	inline void initialization_additionalShooting_AngleRelativeToTank(const GenericFactoryConstructionData&) noexcept;
	inline void initialization_additionalShooting_AngleRelativeToCannon(const GenericFactoryConstructionData&) noexcept;
	inline void initialization_additionalShooting_BulletCount(const GenericFactoryConstructionData&) noexcept;

	inline void initialization_addShootingPoints_Enable(const GenericFactoryConstructionData&) noexcept;
	inline void initialization_addShootingPoints_EquallySpaced(const GenericFactoryConstructionData&) noexcept;
	inline void initialization_addShootingPoints_Raw(const GenericFactoryConstructionData&) noexcept;

	inline void initialization_addExtraShootingPoints_Enable(const GenericFactoryConstructionData&) noexcept;
	inline void initialization_addExtraShootingPoints_Raw(const GenericFactoryConstructionData&) noexcept;

	inline void initialization_modifiedCollisionWithEdge_Enable_tank(const GenericFactoryConstructionData&) noexcept;
	inline void initialization_modifiedCollisionWithEdge_AdditionalBoundary(const GenericFactoryConstructionData&) noexcept;

	inline void initialization_modifiedDeathHandling_Enable_tank(const GenericFactoryConstructionData&) noexcept;
	inline void initialization_modifiedDeathHandling_DurationSubtractPercent_tank(const GenericFactoryConstructionData&) noexcept;

	inline void initialization_tankMaxSpeedMultiplier(const GenericFactoryConstructionData&) noexcept;
	inline void initialization_tankMaxSpeedStacks(const GenericFactoryConstructionData&) noexcept;
	inline void initialization_tankAccelerationMultiplier(const GenericFactoryConstructionData&) noexcept;
	inline void initialization_tankAccelerationStacks(const GenericFactoryConstructionData&) noexcept;
	inline void initialization_tankRadiusMultiplier(const GenericFactoryConstructionData&) noexcept;
	inline void initialization_tankRadiusStacks(const GenericFactoryConstructionData&) noexcept;
	inline void initialization_tankFiringRateMultiplier(const GenericFactoryConstructionData&) noexcept;
	inline void initialization_tankFiringRateStacks(const GenericFactoryConstructionData&) noexcept;
	inline void initialization_tankTurningIncrementMultiplier(const GenericFactoryConstructionData&) noexcept;
	inline void initialization_tankTurningIncrementStacks(const GenericFactoryConstructionData&) noexcept;

	inline void initialization_offenseImportance_tank(const GenericFactoryConstructionData&) noexcept;
	inline void initialization_offenseTier_tank(const GenericFactoryConstructionData&) noexcept;
	inline void initialization_defenseImportance_tank(const GenericFactoryConstructionData&) noexcept;
	inline void initialization_defenseTier_tank(const GenericFactoryConstructionData&) noexcept;

public:
	ColorValueHolder getColor() const override { return this->color; }
	float getColorImportance() const override { return this->colorImportance; }

	TankPower* makeDuplicate() const override;
	BulletPower* makeBulletPower() const override;

	InteractionBoolHolder modifiedEdgeCollision(Tank*) override;

	//InteractionUpdateHolder<TankUpdateStruct, TankUpdateStruct> modifiedCollisionWithTank(const Tank* parent, const Tank* other) override;

	//InteractionUpdateHolder<TankUpdateStruct, WallUpdateStruct> modifiedCollisionWithWall(const Tank*, const Wall*) override;

	//bool getModifiesCollisionWithCircleHazard(const CircleHazard*) const override;
	//InteractionUpdateHolder<TankUpdateStruct, CircleHazardUpdateStruct> modifiedCollisionWithCircleHazard(const Tank*, const CircleHazard*) override;

	//bool getModifiesCollisionWithRectHazard(const RectHazard*) const override;
	//InteractionUpdateHolder<TankUpdateStruct, RectHazardUpdateStruct> modifiedCollisionWithRectHazard(const Tank*, const RectHazard*) override;

	InteractionBoolHolder modifiedDeathHandling(const Tank* parent) override;

	void additionalShooting(Tank* parent, const TankCannonPoint&, const TankExtraCannonPoint&) override;

	std::vector<float>* addShootingPoints() const override;

	std::vector<std::pair<float, float>>* addExtraShootingPoints() const override;

	float  getTankMaxSpeedMultiplier() const override { return this->tankMaxSpeedMultiplier; }
	float  getTankAccelerationMultiplier() const override { return this->tankAccelerationMultiplier; }
	double getTankRadiusMultiplier() const override { return this->tankRadiusMultiplier; }
	double getTankFiringRateMultiplier() const override { return this->tankFiringRateMultiplier; }
	float  getTankTurningIncrementMultiplier() const override { return this->tankTurningIncrementMultiplier; }

	float getOffenseImportance() const override { return this->offenseImportance; }
	float getOffenseTier(const Tank*) const override { return this->offenseTier; }
	float getDefenseImportance() const override { return this->defenseImportance; }
	float getDefenseTier(const Tank*) const override { return this->defenseTier; }

	virtual ~CustomTankPower();
	CustomTankPower(std::string name, const ColorValueHolder& color, float colorImportance, double tankDuration,
		std::shared_ptr<std::vector<CustomPower::CustomPowerAction_Tank*>> initializationActions_tank,
		std::shared_ptr<std::vector<CustomPower::CustomPowerAction_Bullet*>> initializationActions_bullet); //normal
	CustomTankPower(std::string name, const ColorValueHolder& color, float colorImportance, double tankDuration,
		std::shared_ptr<std::vector<CustomPower::CustomPowerAction_Tank*>> initializationActions_tank,
		std::shared_ptr<std::vector<CustomPower::CustomPowerAction_Bullet*>> initializationActions_bullet,
		double old_timeLeft); //for makeDuplicate() to pass on timeLeft for modifiedDeathHandling
};



class CustomBulletPower : public BulletPower {
protected:
	std::string name;
	ColorValueHolder color;
	float colorImportance;
	double tankDuration;
	std::shared_ptr<std::vector<CustomPower::CustomPowerAction_Tank*>> initializationActions_tank;
	std::shared_ptr<std::vector<CustomPower::CustomPowerAction_Bullet*>> initializationActions_bullet;

	float  bulletSpeedMultiplier;
	double bulletRadiusMultiplier;
	float  bulletAcceleration;
	float  bulletAccelerationImportance;
	float  bulletDeceleration_DegradeAmount;

	float offenseImportance;
	float offenseTier;
	float defenseImportance;
	float defenseTier;

	int modifiedCollision_BouncesLeft;

	bool modifiedCollisionWithWall_EnableBounce;
	bool modifiedCollisionWithWall_DestroyWallEnable;

protected:
	inline void initialization_helper_Enable(bool& thingToUpdate, const GenericFactoryConstructionData&) noexcept;

	inline void initialization_modifiedCollision_BounceCount(const GenericFactoryConstructionData&) noexcept;

	inline void initialization_modifiedCollisionWithEdge_EnableBounce_bullet(const GenericFactoryConstructionData&) noexcept;

	inline void initialization_modifiedCollisionWithWall_EnableBounce_bullet(const GenericFactoryConstructionData&) noexcept;
	inline void initialization_modifiedCollisionWithWall_DestroyWall(const GenericFactoryConstructionData&) noexcept;
	inline void updateModifiesCollisionWithWall() noexcept;

	inline void initialization_modifiedDeathHandling_Enable_bullet(const GenericFactoryConstructionData&) noexcept;

	inline void initialization_bulletSpeedMultiplier(const GenericFactoryConstructionData&) noexcept;
	inline void initialization_bulletSpeedStacks(const GenericFactoryConstructionData&) noexcept;
	inline void initialization_bulletRadiusMultiplier(const GenericFactoryConstructionData&) noexcept;
	inline void initialization_bulletRadiusStacks(const GenericFactoryConstructionData&) noexcept;
	inline void initialization_bulletDeceleration(const GenericFactoryConstructionData&) noexcept;
	inline void initialization_bulletAccelerationImportance(const GenericFactoryConstructionData&) noexcept;
	inline void initialization_bulletDeceleration_DegradeAmount(const GenericFactoryConstructionData&) noexcept;

	inline void initialization_offenseImportance_bullet(const GenericFactoryConstructionData&) noexcept;
	inline void initialization_offenseTier_bullet(const GenericFactoryConstructionData&) noexcept;
	inline void initialization_defenseImportance_bullet(const GenericFactoryConstructionData&) noexcept;
	inline void initialization_defenseTier_bullet(const GenericFactoryConstructionData&) noexcept;

public:
	ColorValueHolder getColor() const override { return this->color; }
	float getColorImportance() const override { return this->colorImportance; }
	std::string getColorIdentifier() const override { return this->name; }

	BulletPower* makeDuplicate() const override;
	TankPower* makeTankPower() const override;

	//InteractionBoolHolder modifiedMovement(Bullet*) override;

	InteractionBoolHolder modifiedEdgeCollision(Bullet*) override;

	InteractionUpdateHolder<BulletUpdateStruct, WallUpdateStruct> modifiedCollisionWithWall(const Bullet*, const Wall*) override;

	//bool getModifiesCollisionWithCircleHazard(const CircleHazard*) const override;
	//InteractionUpdateHolder<BulletUpdateStruct, CircleHazardUpdateStruct> modifiedCollisionWithCircleHazard(const Bullet*, const CircleHazard*) override;

	//bool getModifiesCollisionWithRectHazard(const RectHazard*) const override;
	//InteractionUpdateHolder<BulletUpdateStruct, RectHazardUpdateStruct> modifiedCollisionWithRectHazard(const Bullet*, const RectHazard*) override;

	InteractionBoolHolder modifiedDeathHandling(const Bullet* parent) override;

	float  getBulletSpeedMultiplier() const override { return this->bulletSpeedMultiplier; }
	double getBulletRadiusMultiplier() const override { return this->bulletRadiusMultiplier; }
	float  getBulletAcceleration() const override { return this->bulletAcceleration; }
	float  getBulletAccelerationImportance() const override { return this->bulletAccelerationImportance; }
	float  getBulletDegradeAmount() const override { return this->bulletDeceleration_DegradeAmount; }

	float getOffenseImportance() const override { return this->offenseImportance; }
	float getOffenseTier(const Bullet*) const override { return this->offenseTier; }
	float getDefenseImportance() const override { return this->defenseImportance; }
	float getDefenseTier(const Bullet*) const override { return this->defenseTier; }

	virtual ~CustomBulletPower();
	CustomBulletPower(std::string name, const ColorValueHolder& color, float colorImportance, double tankDuration,
		std::shared_ptr<std::vector<CustomPower::CustomPowerAction_Tank*>> initializationActions_tank,
		std::shared_ptr<std::vector<CustomPower::CustomPowerAction_Bullet*>> initializationActions_bullet); //normal
	CustomBulletPower(std::string name, const ColorValueHolder& color, float colorImportance, double tankDuration,
		std::shared_ptr<std::vector<CustomPower::CustomPowerAction_Tank*>> initializationActions_tank,
		std::shared_ptr<std::vector<CustomPower::CustomPowerAction_Bullet*>> initializationActions_bullet,
		double radiusMultiplier, float speedMultiplier, float acceleration, int bouncesLeft); //for makeDuplicate() to pass on things that can vary in a range and bounces left
};



class CustomPowerInterpreter {
public:
	static void ProcessCustomPowers() noexcept;

protected:
	static const std::string ActionStartPhrase_TankPower; // = "[TANKPOWER_START]";
	static const std::string ActionStartPhrase_BulletPower; // = "[BULLETPOWER_START]";

protected:
	//actually read through the file and create the custom level
	//throws on syntax error, returns nullptr if bad for some reason (currently just path=="")
	//needs to print warnings on extra parameters for a command
	static CustomPower* processCustomPower(std::string path);
	static inline CustomPower::CustomPowerAction_Tank* stringToAction_tank(const std::pair<std::string, int>& command); //throws when action is ill-formatted
	static inline CustomPower::CustomPowerAction_Bullet* stringToAction_bullet(const std::pair<std::string, int>& command); //throws when action is ill-formatted
	static void addCustomPower(CustomPower* power); //add it to PowerupDataGovernor (with some checks)

protected: //helpers
	static inline void stringToAction_genericEnable(const std::vector<std::string>& words, GenericFactoryConstructionData& constructionData);
	static inline void stringToAction_genericSingleValue_i(const std::vector<std::string>& words, GenericFactoryConstructionData& constructionData);
	static inline void stringToAction_genericSingleValue_f(const std::vector<std::string>& words, GenericFactoryConstructionData& constructionData);
	static inline void stringToAction_genericSingleValue_d(const std::vector<std::string>& words, GenericFactoryConstructionData& constructionData);

protected: //tank
	//TODO: should all the commented-out functions call the helper they need to?

	//static inline void stringToAction_additionalShooting_Enable(const std::vector<std::string>& words, GenericFactoryConstructionData& constructionData);
	//static inline void stringToAction_additionalShooting_AngleRelativeToTank(const std::vector<std::string>& words, GenericFactoryConstructionData& constructionData);
	//static inline void stringToAction_additionalShooting_AngleRelativeToCannon(const std::vector<std::string>& words, GenericFactoryConstructionData& constructionData);
	//static inline void stringToAction_additionalShooting_BulletCount(const std::vector<std::string>& words, GenericFactoryConstructionData& constructionData);

	//static inline void stringToAction_addShootingPoints_Enable(const std::vector<std::string>& words, GenericFactoryConstructionData& constructionData);
	//static inline void stringToAction_addShootingPoints_EquallySpaced(const std::vector<std::string>& words, GenericFactoryConstructionData& constructionData);
	static inline void stringToAction_addShootingPoints_Raw(const std::vector<std::string>& words, GenericFactoryConstructionData& constructionData);

	//static inline void stringToAction_addExtraShootingPoints_Enable(const std::vector<std::string>& words, GenericFactoryConstructionData& constructionData);
	static inline void stringToAction_addExtraShootingPoints_Raw(const std::vector<std::string>& words, GenericFactoryConstructionData& constructionData);

	//static inline void stringToAction_modifiedCollisionWithEdge_Enable(const std::vector<std::string>& words, GenericFactoryConstructionData& constructionData);
	//static inline void stringToAction_modifiedCollisionWithEdge_AdditionalBoundary(const std::vector<std::string>& words, GenericFactoryConstructionData& constructionData);

	//static inline void stringToAction_modifiedDeathHandling_Enable(const std::vector<std::string>& words, GenericFactoryConstructionData& constructionData);
	//static inline void stringToAction_modifiedDeathHandling_DurationSubtractPercent(const std::vector<std::string>& words, GenericFactoryConstructionData& constructionData);

	//static inline void stringToAction_tankMaxSpeedMultiplier(const std::vector<std::string>& words, GenericFactoryConstructionData& constructionData);
	//static inline void stringToAction_tankMaxSpeedStacks(const std::vector<std::string>& words, GenericFactoryConstructionData& constructionData);
	//static inline void stringToAction_tankAccelerationMultiplier(const std::vector<std::string>& words, GenericFactoryConstructionData& constructionData);
	//static inline void stringToAction_tankAccelerationStacks(const std::vector<std::string>& words, GenericFactoryConstructionData& constructionData);
	//static inline void stringToAction_tankRadiusMultiplier(const std::vector<std::string>& words, GenericFactoryConstructionData& constructionData);
	//static inline void stringToAction_tankRadiusStacks(const std::vector<std::string>& words, GenericFactoryConstructionData& constructionData);
	//static inline void stringToAction_tankFiringRateMultiplier(const std::vector<std::string>& words, GenericFactoryConstructionData& constructionData);
	//static inline void stringToAction_tankFiringRateStacks(const std::vector<std::string>& words, GenericFactoryConstructionData& constructionData);
	//static inline void stringToAction_tankTurningIncrementMultiplier(const std::vector<std::string>& words, GenericFactoryConstructionData& constructionData);
	//static inline void stringToAction_tankTurningIncrementStacks(const std::vector<std::string>& words, GenericFactoryConstructionData& constructionData);

	//static inline void stringToAction_offenseImportance(const std::vector<std::string>& words, GenericFactoryConstructionData& constructionData);
	//static inline void stringToAction_offenseTier(const std::vector<std::string>& words, GenericFactoryConstructionData& constructionData);
	//static inline void stringToAction_defenseImportance(const std::vector<std::string>& words, GenericFactoryConstructionData& constructionData);
	//static inline void stringToAction_defenseTier(const std::vector<std::string>& words, GenericFactoryConstructionData& constructionData);

protected: //bullet
	//static inline void stringToAction_modifiedCollision_BounceCount(const std::vector<std::string>& words, GenericFactoryConstructionData& constructionData);

	//static inline void stringToAction_modifiedCollisionWithEdge_EnableBounce(const std::vector<std::string>& words, GenericFactoryConstructionData& constructionData);

	//static inline void stringToAction_modifiedCollisionWithWall_EnableBounce(const std::vector<std::string>& words, GenericFactoryConstructionData& constructionData);
	//static inline void stringToAction_modifiedCollisionWithWall_DestroyWall(const std::vector<std::string>& words, GenericFactoryConstructionData& constructionData);

	//static inline void stringToAction_modifiedDeathHandling_Enable(const std::vector<std::string>& words, GenericFactoryConstructionData& constructionData);

	static inline void stringToAction_bulletSpeedMultiplier(const std::vector<std::string>& words, GenericFactoryConstructionData& constructionData);
	//static inline void stringToAction_bulletSpeedStacks(const std::vector<std::string>& words, GenericFactoryConstructionData& constructionData);
	static inline void stringToAction_bulletRadiusMultiplier(const std::vector<std::string>& words, GenericFactoryConstructionData& constructionData);
	//static inline void stringToAction_bulletRadiusStacks(const std::vector<std::string>& words, GenericFactoryConstructionData& constructionData);
	static inline void stringToAction_bulletDeceleration(const std::vector<std::string>& words, GenericFactoryConstructionData& constructionData);
	//static inline void stringToAction_bulletAccelerationImportance(const std::vector<std::string>& words, GenericFactoryConstructionData& constructionData);
	//static inline void stringToAction_bulletDeceleration_DegradeAmount(const std::vector<std::string>& words, GenericFactoryConstructionData& constructionData);

	//static inline void stringToAction_offenseImportance(const std::vector<std::string>& words, GenericFactoryConstructionData& constructionData);
	//static inline void stringToAction_offenseTier(const std::vector<std::string>& words, GenericFactoryConstructionData& constructionData);
	//static inline void stringToAction_defenseImportance(const std::vector<std::string>& words, GenericFactoryConstructionData& constructionData);
	//static inline void stringToAction_defenseTier(const std::vector<std::string>& words, GenericFactoryConstructionData& constructionData);

private:
	CustomPowerInterpreter() = delete;
	CustomPowerInterpreter(const CustomPowerInterpreter&) = delete;
};
