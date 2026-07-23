#pragma once
class LevelEffect;

#include <string>
#include <vector>
#include <unordered_map>

#include "game-thing.h"

#include "level.h"
#include "generic-factory-construction-data.h"

//TODO: level effects basically need to be powers but for levels (needs to be able to override some things in GameMainLoop, I think)
class LevelEffect : public GameThing {
public:
	virtual std::string getName() const = 0;
	virtual std::vector<std::string> getLevelEffectTypes() const = 0;
	virtual std::unordered_map<std::string, float> getWeights() const = 0; //intended range: (0,1]

public:
	virtual void apply() = 0;
	virtual void tick(const Level* parent) = 0; //update variables and stuff
	virtual void doEffects(Level* parent) const = 0; //do what needs to be done

	virtual float  getTankMaxSpeedMultiplier() const { return 1; }
	virtual float  getTankAccelerationMultiplier() const { return 1; }
	virtual double getTankRadiusMultiplier() const { return 1; }
	virtual double getTankFiringRateMultiplier() const { return 1; }
	virtual float  getTankTurningIncrementMultiplier() const { return 1; }
	//they all stack because you shouldn't have too many

	virtual void draw() const override { return; }
	virtual void draw(DrawingLayers) const override { return; }
	virtual void poseDraw() const override { return; }
	virtual void poseDraw(DrawingLayers) const override { return; }
	virtual void ghostDraw(float alpha) const override { return; }
	virtual void ghostDraw(DrawingLayers, float alpha) const override { return; }

	LevelEffect() : GameThing(DEFAULT_TEAM, ObjectType::None) {}
	virtual ~LevelEffect() { return; }
	//static LevelEffect* factory(const GenericFactoryConstructionData&);
};
