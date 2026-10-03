#pragma once
#include "../power.h"

class BlastPower : public Power {
	//called mega-blast in JS Tanks
public: //tank stuff
	static const float bulletAngleDeviation;
	static const int bulletAmount;

public: //bullet stuff
	static const float maxBulletAcceleration;
	static const float minBulletAcceleration;
	static const float degradeAmount;

public:
	virtual std::vector<std::string> getPowerTypes() const override {
		std::vector<std::string> types = std::vector<std::string>{ "vanilla", "random-vanilla", "old", "random-old", "random" };
		//TODO: move to vanilla-extra?
		//JS: did have supermix (replaced by bounce)
		return types;
	}
	virtual std::unordered_map<std::string, float> getWeights() const override;
	virtual std::vector<std::string> getPowerAttributes() const override {
		std::vector<std::string> attributes = std::vector<std::string>{ "mix" };
		return attributes;
	}

	virtual std::string getName() const override { return BlastPower::getClassName(); }
	static std::string getClassName() { return "blast"; }
	virtual ColorValueHolder getColor() const override { return BlastPower::getClassColor(); }
	static ColorValueHolder getClassColor() { return ColorValueHolder(0x44/255.0, 0x66/255.0, 0x88/255.0); } //dark blue

	virtual TankPower* makeTankPower() const override;
	virtual BulletPower* makeBulletPower() const override;

	BlastPower();
	static Power* factory();
};



class BlastTankPower : public TankPower {
public:
	virtual ColorValueHolder getColor() const override { return BlastPower::getClassColor(); }

	virtual TankPower* makeDuplicate() const override { return new BlastTankPower(); }
	virtual BulletPower* makeBulletPower() const override;

	//bool modifiesAdditionalShooting = true;
	virtual void additionalShooting(Tank* parent, const TankCannonPoint&, const TankExtraCannonPoint&) override;
	//bool overridesAdditionalShooting = true;

	virtual float  getTankMaxSpeedMultiplier() const override { return .5; }
	virtual float  getTankAccelerationMultiplier() const override { return .5; }
	//virtual double getTankFiringRateMultiplier() const override { return .5; } //JS

	BlastTankPower();
};



class BlastBulletPower : public BulletPower {
protected:
	float accelerationAmount;

public:
	virtual ColorValueHolder getColor() const override { return BlastPower::getClassColor(); }
	virtual std::string getColorIdentifier() const override { return BlastPower::getClassName(); }

	virtual BulletPower* makeDuplicate() const override;
	virtual TankPower* makeTankPower() const override;

	//bool modifiesCollisionWithWall = true;
	virtual InteractionUpdateHolder<BulletUpdateStruct, WallUpdateStruct> modifiedCollisionWithWall(const Bullet*, const Wall*) override;

	virtual bool getModifiesCollisionWithCircleHazard(const CircleHazard*) const override;
	virtual InteractionUpdateHolder<BulletUpdateStruct, CircleHazardUpdateStruct> modifiedCollisionWithCircleHazard(const Bullet*, const CircleHazard*) override;

	virtual bool getModifiesCollisionWithRectHazard(const RectHazard*) const override;
	virtual InteractionUpdateHolder<BulletUpdateStruct, RectHazardUpdateStruct> modifiedCollisionWithRectHazard(const Bullet*, const RectHazard*) override;

	virtual double getBulletRadiusMultiplier() const override { return .25; } //JS: .5
	virtual float  getBulletAcceleration() const override { return accelerationAmount; }
	virtual float  getBulletDegradeAmount() const override { return BlastPower::degradeAmount; }

	BlastBulletPower();
	BlastBulletPower(float acceleration); //protected?
};
