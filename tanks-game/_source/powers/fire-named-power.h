#pragma once
#include "../power.h"

class FireNamedPower : public Power {
public: //tank stuff
	static const float bulletAngleDeviation;
	static const int bulletAmount;

public: //bullet stuff
	static const float  maxBulletAcceleration;
	static const float  minBulletAcceleration;
	static const float  degradeAmount;
	static const double growAmount;

public:
	virtual std::vector<std::string> getPowerTypes() const override {
		std::vector<std::string> types = std::vector<std::string>{ "vanilla", "random-vanilla", "old", "random-old", "supermix", "supermix-vanilla", "random" };
		return types;
	}
	virtual std::unordered_map<std::string, float> getWeights() const override;
	virtual std::vector<std::string> getPowerAttributes() const override {
		std::vector<std::string> attributes = std::vector<std::string>{ "mix" };
		return attributes;
	}

	virtual std::string getName() const override { return FireNamedPower::getClassName(); }
	static std::string getClassName() { return "fire"; }
	virtual ColorValueHolder getColor() const override { return FireNamedPower::getClassColor(); }
	static ColorValueHolder getClassColor() { return ColorValueHolder(1.0f, 0.5f, 0.0f); } //orange //JS: #FF8800

	virtual TankPower* makeTankPower() const override;
	virtual BulletPower* makeBulletPower() const override;

	FireNamedPower();
	static Power* factory();
};



class FireNamedTankPower : public TankPower {
public:
	virtual ColorValueHolder getColor() const override { return FireNamedPower::getClassColor(); }

	virtual TankPower* makeDuplicate() const override { return new FireNamedTankPower(); }
	virtual BulletPower* makeBulletPower() const override;

	//bool modifiesAdditionalShooting = true;
	virtual void additionalShooting(Tank* parent, const TankCannonPoint&, const TankExtraCannonPoint&) override;
	//bool overridesAdditionalShooting = true;

	//virtual float  getTankMaxSpeedMultiplier() const override { return .75; } //JS
	//virtual float  getTankAccelerationMultiplier() const override { return .75; } //JS
	virtual double getTankFiringRateMultiplier() const override { return .5; } //JS: .25
	virtual float  getTankTurningIncrementMultiplier() const override { return 2; }

	FireNamedTankPower();
};



class FireNamedBulletPower : public BulletPower {
protected:
	float accelerationAmount;

public:
	virtual ColorValueHolder getColor() const override { return FireNamedPower::getClassColor(); }
	virtual std::string getColorIdentifier() const override { return FireNamedPower::getClassName(); }

	virtual BulletPower* makeDuplicate() const override;
	virtual TankPower* makeTankPower() const override;

	//bool modifiesCollisionWithWall = true;
	virtual InteractionUpdateHolder<BulletUpdateStruct, WallUpdateStruct> modifiedCollisionWithWall(const Bullet*, const Wall*) override;

	virtual float  getBulletSpeedMultiplier() const override { return .5; }
	virtual float  getBulletAcceleration() const override { return accelerationAmount; }
	virtual float  getBulletDegradeAmount() const override { return FireNamedPower::degradeAmount; }
	virtual double getBulletRadiusGrowNumber_Moving() const override { return FireNamedPower::growAmount; }
	//bool bulletRadiusGrowMultiplies_Moving = false;

	FireNamedBulletPower();
	FireNamedBulletPower(float acceleration); //protected?
};
