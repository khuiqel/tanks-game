#pragma once
class Tank;
struct TankUpdateStruct;

#include <string>
#include <vector>

#include "game-thing.h"
#include "circle.h"
#include "tank-power.h"

#include "color-value-holder.h"
#include "simple-vector-2d.h"
#include "cannon-point.h"

class Tank : public GameThing, public Circle {
	friend class ResetThings;
	friend class PowerFunctionHelper;
	friend class EndGameHandler; //calls this->kill()

public:
	SimpleVector2D velocity;
	float maxSpeed; // = 1;
	float acceleration; // = 1.0/16; //intentional acceleration, not total
	float turningIncrement; // = 64;
	bool dead = false; //only kill() should modify this
	std::vector<CannonPoint> shootingPoints;
	std::vector<ExtraCannonPoint> extraShootingPoints;
	std::vector<TankPower*> tankPowers;
	double shootCount;
	double maxShootCount;
	//std::string name; //note: this is for drawing a name above a tank, but that doesn't exist anymore, so this does nothing

public:
	float getOffenseTier() const;
	float getDefenseTier() const;

protected:
	float getHighestOffenseImportance() const;
	float getHighestOffenseTier(float importance) const;
	float getHighestDefenseImportance() const;
	float getHighestDefenseTier(float importance) const;

public:
	void determineShootingAngles();
	void updateAllValues(); //this is supposed to update all values that can get affected by powers, such as maxSpeed and acceleration
	void updateMaxSpeed();
	void updateAcceleration();
	void updateRadius();
	void updateTurningIncrement();

	double getFiringRateMultiplier() const;

protected:
	void makeBulletCommon(double x, double y, double radius, float angle, float speed);
public:
	void makeBullet(double x, double y, double radius, float angle, float speed, float acc); //does not use makeBulletCommon (avoid using)
	void defaultMakeBullet(float angle); //simple shoot: bullet points away from tank center at a given angle
	void defaultMakeBullet(float angle, float edgeAngleOffset); //same but control over the firing angle
	void preciseMakeBullet(double x_offset, double y_offset, float angle); //make bullet x and y dist from tank, moving with angle; basically just for mines

protected:
	ColorValueHolder defaultColor; //not static because of shiny tanks
	//static const ColorValueHolder defaultNameFill = ColorValueHolder(1.0f, 1.0f, 1.0f);
	//static const ColorValueHolder defaultNameStroke = ColorValueHolder(0.0f, 0.0f, 0.0f);

	bool kill(); //allows for custom death (a.k.a. something saving the tank from death)
	void kill_hard(); //kills without accounting for extra lives
	inline void terminalVelocity(bool forward);
	inline void move_base(bool forward, bool turnL, bool turnR);
	inline void determineShootingAngles_helper(std::vector<float>* newCannonPoints);
	inline float getEvaluatedCannonAngle(unsigned int index) const;
	inline float getEvaluatedCannonAngle(unsigned int indexRegular, unsigned int indexExtra) const;
	inline float getEvaluatedCannonAngleWithEdge(unsigned int indexRegular, unsigned int indexExtra) const;

public:
	//helper stuff:
	ColorValueHolder getBodyColor() const;
	//std::string getName() const { return name; }

	static const float default_maxSpeed;
	static const float default_acceleration;
	static const float default_turningIncrement;

public:
	bool move(bool forward, bool turnL, bool turnR, bool specialKey);
	void shoot(bool shooting);
	void powerTickAndCalculate();
	void removePower(int index);
	void powerReset();
	void update(const TankUpdateStruct*);

	void draw() const override;
	void draw(DrawingLayers) const override;
	void poseDraw() const override;
	void poseDraw(DrawingLayers) const override;
	void ghostDraw(float alpha) const override;
	void ghostDraw(DrawingLayers, float alpha) const override;

protected:
	void drawBody(float alpha = 1.0f) const;
	void drawBodyDead(float alpha = 1.0f) const; //probably doesn't need alpha
	void drawOutline(float alpha = 1.0f) const;
	void drawShootingCooldown(float alpha = 1.0f) const;
	void drawPowerCooldown(float alpha = 1.0f) const;
	void drawMainBarrel(float alpha = 1.0f) const;
	void drawExtraBarrels(float alpha = 1.0f) const;
	void drawExtraExtraBarrels(float alpha = 1.0f) const;

private:
	static SimpleVector2D body_vertices[Circle::NumOfSides+1];
	static unsigned int body_indices[Circle::NumOfSides*3];
	static unsigned int outline_indices[Circle::NumOfSides*2*3];
	static bool initialized_vertices;

	static bool initializeVertices();

public:
	Tank(double x, double y, float angle, Team_ID id, std::string name, double shootCooldown);
	~Tank();
};

struct TankUpdateStruct {
	//deltas:
	double x;
	double y;
	float speed;
	float angle;

	void add(const TankUpdateStruct& other);

	TankUpdateStruct(double x, double y, float speed, float angle);
	TankUpdateStruct() : TankUpdateStruct(0, 0, 0, 0) {}
};
