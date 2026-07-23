#pragma once
class CircleHazard;
struct CircleHazardUpdateStruct;

#include <string>
#include <vector>
#include <unordered_map>

#include "game-thing.h"
#include "circle.h"

#include "tank.h"
#include "bullet.h"
#include "generic-factory-construction-data.h"

//I strongly doubt anything will use this; the only thing I can think of is like a sentry gun, so I'm probably going to get rid of this
struct CircleFactoryInformation {
	bool requiresWallOnLeft;
	bool requiresWallOnRight;
	bool requiresWallOnTop;
	bool requiresWallOnBottom;
	bool wallRequirementCanBeAnEdge;
	CircleFactoryInformation(bool wallLeft, bool wallRight, bool wallTop, bool wallBottom, bool edgeEqualsWall) {
		requiresWallOnLeft = wallLeft;
		requiresWallOnRight = wallRight;
		requiresWallOnTop = wallTop;
		requiresWallOnBottom = wallBottom;
		wallRequirementCanBeAnEdge = edgeEqualsWall;
	}
	CircleFactoryInformation(bool wallLeft, bool wallRight, bool wallTop, bool wallBottom)
	: CircleFactoryInformation(wallLeft, wallRight, wallTop, wallBottom, true) {}
	CircleFactoryInformation()
	: CircleFactoryInformation(false, false, false, false) {}
};

//this is intended for hazard randomization; if the actual hazard type is known, then the constructor will be known
enum class CircleHazardConstructionTypes /*: unsigned char */ {
	constructionIsTooComplex, //patrolling turret (add a "path required" parameter?)
	simpleConstruction, //only x and y required (no r or angle)
	angleRequired,
	radiusRequired,
	radiusAndAngleRequired,
};

enum class CircleHazardCollisionType {
	solid,
	under
};

class CircleHazard : public GameThing, public Circle {
public:
	virtual std::vector<std::string> getHazardTypes() const = 0;
	virtual std::unordered_map<std::string, float> getWeights() const = 0; //intended range: (0,1]

	virtual CircleHazardCollisionType getCollisionType() const = 0; // { return CircleHazardCollisionType::solid; }

	virtual bool actuallyCollided(const Tank*) const { return true; }
	virtual void specialEffectTankCollision(const Tank*) { return; } //always activated before modifiedTankCollision
	virtual InteractionUpdateHolder<TankUpdateStruct, CircleHazardUpdateStruct> modifiedTankCollision(const Tank*) const;

	virtual bool actuallyCollided(const Bullet*) const { return true; }
	virtual void specialEffectBulletCollision(const Bullet*) { return; } //always activated before modifiedBulletCollision
	virtual InteractionUpdateHolder<BulletUpdateStruct, CircleHazardUpdateStruct> modifiedBulletCollision(const Bullet*) const;

	virtual float getOffenseTier() const = 0;
	virtual float getDefenseTier() const = 0;

public:
	virtual bool validLocation() const { return true; }
	virtual bool reasonableLocation() const = 0;
	virtual void initialize() { return; } //called when circlehazard is pushed for the first time
	//virtual void uninitialize() { return; } //called when circlehazard is destroyed (TODO)
	void update(const CircleHazardUpdateStruct*);

	virtual std::string getName() const = 0;
	//static std::string getClassName();

	virtual void tick() = 0;
	virtual void draw() const override = 0;
	virtual void draw(DrawingLayers) const override = 0;
	virtual void poseDraw() const override = 0;
	virtual void poseDraw(DrawingLayers) const override = 0;
	virtual void ghostDraw(float alpha) const override = 0;
	virtual void ghostDraw(DrawingLayers, float alpha) const override = 0;

	virtual ~CircleHazard() { return; }
	//static CircleHazard* factory(const GenericFactoryConstructionData&);
	//static CircleHazard* randomizingFactory(double x_start, double y_start, double area_width, double area_height, const GenericFactoryConstructionData&);
	virtual int getFactoryArgumentCount() const = 0;
	virtual CircleHazardConstructionTypes getConstructionType() const = 0;
	virtual CircleFactoryInformation getFactoryInformation() const = 0;

protected:
	CircleHazard(Team_ID t_id) : GameThing(t_id, ObjectType::Hazard_C) {}
};

struct CircleHazardUpdateStruct {
	//deltas:
	double x;
	double y;

	void add(const CircleHazardUpdateStruct& other);

	CircleHazardUpdateStruct(double x, double y);
	CircleHazardUpdateStruct() : CircleHazardUpdateStruct(0, 0) {}
};
