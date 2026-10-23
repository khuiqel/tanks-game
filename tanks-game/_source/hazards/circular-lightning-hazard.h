#pragma once
#include "../circle-hazard.h"
#include "../generalized-lightning.h"

#include "../constants.h"
#include "../simple-vector-2d.h"

class CircularLightningHazard : public CircleHazard, public GeneralizedLightning {
protected:
	//unsigned int maxBolts; // = 1;
	virtual void refreshBolt(LightningBolt*) const override;
	virtual void pushBolt(LightningBolt*) override;
	virtual void pushDefaultBolt(int num, bool randomize) override; //randomize should be true all of the time

	inline Circle getCenterPoint() const {
		//for checks when a bullet/tank collides (needs to be a function in case the lightning changes size or position)
		return Circle(cir.x, cir.y, 0);
	}

public:
	virtual std::vector<std::string> getHazardTypes() const override {
		std::vector<std::string> types = std::vector<std::string>{ "vanilla", "random-vanilla", "old", "random-old", "random" };
		//it's not really an old hazard (lightning was rectangular only)
		return types;
	}
	virtual std::unordered_map<std::string, float> getWeights() const override;

	virtual CircleHazardCollisionType getCollisionType() const override { return CircleHazardCollisionType::under; }

	virtual bool actuallyCollided(const Tank*) const override { return currentlyActive; }
	virtual void specialEffectTankCollision(const Tank*) override;
	virtual InteractionUpdateHolder<TankUpdateStruct, CircleHazardUpdateStruct> modifiedTankCollision(const Tank*) const override { return { false, false, nullptr, nullptr }; }

	virtual bool actuallyCollided(const Bullet*) const override { return currentlyActive; }
	virtual void specialEffectBulletCollision(const Bullet*) override;
	virtual InteractionUpdateHolder<BulletUpdateStruct, CircleHazardUpdateStruct> modifiedBulletCollision(const Bullet*) const override { return { false, false, nullptr, nullptr }; }

	virtual float getOffenseTier() const override { return .5; } //1.5?
	virtual float getDefenseTier() const override { return HIGH_TIER; }

protected:
	virtual void specialEffectCircleCollision(const Circle&); //tanks and bullets are both circles, so calculating the bolt positions would be the same

public:
	virtual bool validLocation() const override;
	virtual bool reasonableLocation() const override;

	virtual std::string getName() const override { return getClassName(); }
	static std::string getClassName() { return "lightning"; }

	virtual void tick() override { GeneralizedLightning::tick(); }
	virtual void draw() const override;
	virtual void draw(DrawingLayers) const override;
	virtual void poseDraw() const override;
	virtual void poseDraw(DrawingLayers) const override;
	virtual void ghostDraw(float alpha) const override;
	virtual void ghostDraw(DrawingLayers, float alpha) const override;

protected:
	virtual void drawBackground(bool pose, float alpha = 1.0f) const;
	virtual void drawBackgroundOutline(float alpha) const; //called by drawBackground()
	virtual void drawBolts(float alpha = 1.0f) const;
	virtual void drawBolts_Pose(float alpha = 1.0f) const;

protected:
	static SimpleVector2D body_vertices[Circle::NumOfSides+1];
	static unsigned int body_indices[Circle::NumOfSides*3];
	static unsigned int outline_indices[Circle::NumOfSides*2*3];
	static bool initialized_vertices;

	static bool initializeVertices();

public:
	CircularLightningHazard(double xpos, double ypos, double radius);
	~CircularLightningHazard();
	static CircleHazard* factory(const GenericFactoryConstructionData&);
	static CircleHazard* randomizingFactory(double x_start, double y_start, double area_width, double area_height, const GenericFactoryConstructionData&);
	virtual int getFactoryArgumentCount() const override { return 3; }
	virtual CircleHazardConstructionTypes getConstructionType() const override { return CircleHazardConstructionTypes::radiusRequired; }
	virtual CircleFactoryInformation getFactoryInformation() const override { return { false, false, false, false, false }; }
};
