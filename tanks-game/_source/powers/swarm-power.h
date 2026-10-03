#pragma once
#include "../power.h"

class SwarmPower : public Power {
public: //tank stuff
	static const float initialAngleDiff;

public: //bullet stuff
	static const float homingStrength;

public:
	virtual std::vector<std::string> getPowerTypes() const override {
		std::vector<std::string> types = std::vector<std::string>{ "vanilla-extra", "random-vanilla", "random" };
		//TODO: move to vanilla?
		return types;
	}
	virtual std::unordered_map<std::string, float> getWeights() const override;
	virtual std::vector<std::string> getPowerAttributes() const override {
		std::vector<std::string> attributes = std::vector<std::string>{ "mix" };
		return attributes;
	}

	virtual std::string getName() const override { return SwarmPower::getClassName(); }
	static std::string getClassName() { return "swarm"; }
	virtual ColorValueHolder getColor() const override { return SwarmPower::getClassColor(); }
	static ColorValueHolder getClassColor() { return ColorValueHolder(0xE6/255.0, 0xC5/255.0, 0x22/255.0); } //maybe it should be a bit brighter

	virtual TankPower* makeTankPower() const override;
	virtual BulletPower* makeBulletPower() const override;

	SwarmPower();
	static Power* factory();
};



class SwarmTankPower : public TankPower {
protected:
	bool leftRightToggle;

public:
	virtual ColorValueHolder getColor() const override { return SwarmPower::getClassColor(); }

	virtual TankPower* makeDuplicate() const override { return new SwarmTankPower(); }
	virtual BulletPower* makeBulletPower() const override;

	//bool modifiesAdditionalShooting = true;
	virtual void additionalShooting(Tank* parent, const TankCannonPoint&, const TankExtraCannonPoint&) override;
	//bool overridesAdditionalShooting = true;

	virtual double getTankFiringRateMultiplier() const override { return .5; }

	SwarmTankPower();
};



class SwarmBulletPower : public BulletPower {
public:
	virtual ColorValueHolder getColor() const override { return SwarmPower::getClassColor(); }
	virtual std::string getColorIdentifier() const override { return SwarmPower::getClassName(); }

	virtual BulletPower* makeDuplicate() const override { return new SwarmBulletPower(); }
	virtual TankPower* makeTankPower() const override;

	//bool modifiesMovement = true;
	virtual InteractionBoolHolder modifiedMovement(Bullet*) override;

	//bool modifiesEdgeCollision = true;
	virtual InteractionBoolHolder modifiedEdgeCollision(Bullet*) override;

	virtual float getBulletSpeedMultiplier() const override { return .5; }

	SwarmBulletPower();
};
