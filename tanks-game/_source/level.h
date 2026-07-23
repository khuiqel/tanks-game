#pragma once
class Level;

#include <string>
#include <vector>
#include <unordered_map>

#include "game-thing.h"
#include "level-effect.h"

#include "color-value-holder.h"
#include "generic-factory-construction-data.h"

//only a GameThing to be drawable
class Level : public GameThing {
	friend class LevelManager; //actually needs this unlike the other managers because the manager has more control

protected:
	std::vector<LevelEffect*> effects; //cleared in Level's destructor

public:
	LevelEffect* getLevelEffect(unsigned int i) const { return effects[i]; }
	unsigned int getNumEffects() const { return effects.size(); }

	virtual std::string getName() const = 0;
	virtual ColorValueHolder getDefaultColor() const = 0;
	virtual std::vector<std::string> getLevelTypes() const = 0;
	virtual std::unordered_map<std::string, float> getWeights() const = 0; //intended range: (0,1]

	virtual void initialize() = 0;
	virtual void tick() { return; }
	void tickLevelEffects();
	void doLevelEffects();
	virtual void draw() const override { return; }
	virtual void draw(DrawingLayers) const override { return; }
	virtual void poseDraw() const override { return; }
	virtual void poseDraw(DrawingLayers) const override { return; }
	virtual void ghostDraw(float) const override { return; } //ignore... probably
	virtual void ghostDraw(DrawingLayers, float) const override { return; } //ignore... probably
	void drawLevelEffects() const;
	void drawLevelEffects(DrawingLayers) const;

	Level() : GameThing(DEFAULT_TEAM, ObjectType::None) {}
	virtual ~Level();
	//static Level* factory(const GenericFactoryConstructionData&); //not necessary
	//static Level* factory();
};
