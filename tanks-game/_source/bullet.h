#pragma once
class Bullet;
struct BulletUpdateStruct;

#include "constants.h"
#include <vector>

#include "game-thing.h"
#include "circle.h"
#include "bullet-power.h"

#include "color-value-holder.h"
#include "simple-vector-2d.h"

enum class BulletParentType {
	team, //tanks
	individual, //hazards
	name, //certain hazards?
	none //anything?
};

class Bullet : public GameThing, public Circle {
	friend class ResetThings;
	friend class PowerFunctionHelper;
	friend class EndGameHandler;
	friend class Renderer; //only to access the vertices

public: //hopefully temporary
	SimpleVector2D velocity;
	float initial_velocity;
	float acceleration;
	Game_ID parentID; //may not be used, depends on parentType
	BulletParentType parentType;
	std::vector<BulletPower*> bulletPowers; //change eventually?
	float lifeValue; //[0,100], controls transparency (0=nothing, 100=opaque)
	Color_ID m_colorIdentifier;

public:
	float getOffenseTier() const;
	float getDefenseTier() const;

protected:
	float getHighestOffenseImportance() const;
	float getHighestOffenseTier(float importance) const;
	float getHighestDefenseImportance() const;
	float getHighestDefenseTier(float importance) const;

public:
	float  getBulletSpeedMultiplier() const;
	double getBulletRadiusMultiplier() const;
	float  getBulletAcceleration() const;
	float  getBulletDegradeAmount() const;
	std::pair<double, double> getBulletRadiusGrowNumber_Stationary() const; //{additive, multiplier}
	std::pair<double, double> getBulletRadiusGrowNumber_Moving() const;     //{additive, multiplier}

protected:
	bool kill(); //allows for custom death (a.k.a. something saving the bullet from death)
	inline void move_base();
	inline void degradeHandle();
	inline void growHandle();

public:
	//helper functions:
	const ColorValueHolder& getColor() const;
	inline float getInitialVelocity() const { return initial_velocity; }
	Game_ID getParentID() const { return parentID; }
	BulletParentType getParentIDType() const { return parentType; }
	bool isDead() const { return (lifeValue <= 0); }

	static const ColorValueHolder default_color;
	static const double default_radius;

public:
	bool move();
	void powerTick();
	void removePower(int index);
	void updateColorIdentifier();

	bool canCollideWith(const GameThing*) const;
	bool canCollideWith(const Bullet*) const;
	void update(const BulletUpdateStruct*);

	void draw() const override;
	void draw(DrawingLayers) const override;
	void poseDraw() const override;
	void poseDraw(DrawingLayers) const override;
	void ghostDraw(float alpha) const override;
	void ghostDraw(DrawingLayers, float alpha) const override;

private:
	//NOTE: bullets have half the normal circle vertices
	static constexpr unsigned int BulletSideCount = Circle::NumOfSides / 2;
	static float instanced_vertices[2 * ((BulletSideCount + 2) + (BulletSideCount + 1) + (BulletSideCount * 2))];
	static unsigned int instanced_indices[BulletSideCount * 3 * (1+1+2)];
	static bool initialized_vertices;
public:
	static bool initializeVertices(); //initialized in main()

private:
	Bullet(double x, double y, float angle, Team_ID teamID, BulletParentType parentType, Game_ID parentID); //every bullet uses this
	Bullet(double x, double y, float angle, Team_ID teamID, BulletParentType parentType, Game_ID parentID, const std::vector<BulletPower*>* bp); //most bullets use this
public:
	Bullet(double x, double y, double r, float angle, float vel, Team_ID teamID, BulletParentType parentType, Game_ID parentID, const std::vector<BulletPower*>* bp, bool lessOverriding); //basically just for banana
	Bullet(double x, double y, double r, float angle, float vel, float acc, Team_ID teamID, BulletParentType parentType, Game_ID parentID, const std::vector<BulletPower*>* bp, bool manualAcceleration); //avoid using
public:
	Bullet(double x, double y, double r, float angle, float vel, Team_ID teamID, BulletParentType parentType, Game_ID parentID);
	Bullet(double x, double y, double r, float angle, float vel, Team_ID teamID, BulletParentType parentType, Game_ID parentID, const std::vector<BulletPower*>* bp);
	~Bullet();
};

struct BulletUpdateStruct {
	//deltas:
	double x;
	double y;
	float speed;
	float angle;
	float alpha;
	//float acceleration;
	//others:
	//std::vector<Bullet*> newBullets; //not implemented, don't use

	void add(const BulletUpdateStruct& other);

	//BulletUpdateStruct(double x, double y, float speed, float angle, float alpha, const std::vector<Bullet*>& newBullets);
	BulletUpdateStruct(double x, double y, float speed, float angle, float alpha);
	BulletUpdateStruct() : BulletUpdateStruct(0, 0, 0, 0, 0) {}
};
