#pragma once
#include "../power.h"

class MinesPower : public Power {
	//this should be renamed (but probably won't...)
public: //tank stuff
	static const double bulletDistance; //percentage from center (beginning of cannon) to end of cannon

public:
	virtual std::vector<std::string> getPowerTypes() const override {
		std::vector<std::string> types = std::vector<std::string>{ "vanilla-extra", "old" };
		//no random; must be intentional
		//JS: did have supermix (because power mixing was really bad, so it would only drop a single mine down without interfering with anything else)
		return types;
	}
	virtual std::unordered_map<std::string, float> getWeights() const override;
	virtual std::vector<std::string> getPowerAttributes() const override {
		//somewhat conflicted on mixing because it just doesn't mix well
		std::vector<std::string> attributes = std::vector<std::string>{ "mix" };
		return attributes;
	}

	virtual std::string getName() const override { return MinesPower::getClassName(); }
	static std::string getClassName() { return "mines"; }
	virtual ColorValueHolder getColor() const override { return MinesPower::getClassColor(); }
	static ColorValueHolder getClassColor() { return ColorValueHolder(0.0f, 0.0f, 0.0f); } //black, so it's hard to see where the tank is pointed
	virtual float getColorImportance() const override { return MinesPower::getClassColorImportance(); }
	static float getClassColorImportance() { return .5; } //godmode can overpower this

	virtual TankPower* makeTankPower() const override;
	virtual BulletPower* makeBulletPower() const override;

	MinesPower();
	//TODO: virtual destructor for OldMinesPower?
	static Power* factory();
};



class MinesTankPower : public TankPower {
public:
	virtual void tick(Tank* t) override; //for updating modifiesAdditionalShooting based on whether there are any other powers that modify additionalShooting
	//might need a secondary tick for this; one tick to do stuff, another tick to update interaction bools

	virtual ColorValueHolder getColor() const override { return MinesPower::getClassColor(); }
	virtual float getColorImportance() const override { return MinesPower::getClassColorImportance(); }

	virtual TankPower* makeDuplicate() const override { return new MinesTankPower(); }
	virtual BulletPower* makeBulletPower() const override;

	//bool modifiesAdditionalShooting = true;
	virtual void additionalShooting(Tank* parent, const TankCannonPoint&, const TankExtraCannonPoint&) override;
	//bool overridesAdditionalShooting = true;

	virtual double getTankFiringRateMultiplier() const override { return .25; } //.5?

	MinesTankPower();
};



class MinesBulletPower : public BulletPower {
public:
	virtual ColorValueHolder getColor() const override { return MinesPower::getClassColor(); }
	virtual float getColorImportance() const override { return MinesPower::getClassColorImportance(); }
	virtual std::string getColorIdentifier() const override { return MinesPower::getClassName(); }

	virtual BulletPower* makeDuplicate() const override { return new MinesBulletPower(); }
	virtual TankPower* makeTankPower() const override;

	virtual float getBulletSpeedMultiplier() const override { return 0; }
	//bool bulletSpeedStacks = false; //true?
	virtual float getBulletAcceleration() const override { return 0; }
	virtual float getBulletAccelerationImportance() const override { return .5; }

	MinesBulletPower();
};
