#pragma once
#include "../circle-hazard.h"
#include "../generalized-lava.h"

#include "../constants.h"
#include "../simple-vector-2d.h"

class CircularLavaHazard : public CircleHazard, public GeneralizedLava {
protected:
	virtual void tryToPushNewBubble(float bubbleRadius) override;

public:
	virtual std::vector<std::string> getHazardTypes() const override {
		std::vector<std::string> types = std::vector<std::string>{ "vanilla", "random-vanilla", "old", "random-old", "random" };
		//it's not really an old hazard (lava was rectangular only)
		return types;
	}
	virtual std::unordered_map<std::string, float> getWeights() const override;

	virtual CircleHazardCollisionType getCollisionType() const override { return CircleHazardCollisionType::under; }

	//virtual bool actuallyCollided(const Tank*) const override { return true; }
	virtual InteractionUpdateHolder<TankUpdateStruct, CircleHazardUpdateStruct> modifiedTankCollision(const Tank*) const override { return { false, false, nullptr, nullptr }; }

	virtual bool actuallyCollided(const Bullet* b) const override { return (b->velocity.getMagnitude() == 0); }
	virtual InteractionUpdateHolder<BulletUpdateStruct, CircleHazardUpdateStruct> modifiedBulletCollision(const Bullet*) const override { return { false, false, nullptr, nullptr }; }

	virtual float getOffenseTier() const override { return .5; }
	virtual float getDefenseTier() const override { return HIGH_TIER; }

public:
	//virtual bool validLocation() const override { return true; }
	virtual bool reasonableLocation() const override;

	virtual std::string getName() const override { return getClassName(); }
	static std::string getClassName() { return "lava"; }

	virtual void tick() override { GeneralizedLava::tick(); }
	virtual void draw() const override;
	virtual void draw(DrawingLayers) const override;
	virtual void poseDraw() const override;
	virtual void poseDraw(DrawingLayers) const override;
	virtual void ghostDraw(float alpha) const override;
	virtual void ghostDraw(DrawingLayers, float alpha) const override;

protected:
	virtual void drawBackground(bool pose, float alpha = 1.0f) const;
	virtual void drawBubbles(bool pose, float alpha = 1.0f) const;

protected:
	//NOTE: bubbles (and the lava) have half the normal circle vertices
	static constexpr unsigned int BubbleSideCount = Circle::NumOfSides / 2;
	static SimpleVector2D bubble_vertices[BubbleSideCount + 1];
	static unsigned int bubble_indices[BubbleSideCount * 3];
	static unsigned int outline_indices[BubbleSideCount * 2*3];
	static bool initialized_vertices;

	static bool initializeVertices();

public:
	CircularLavaHazard(double xpos, double ypos, double radius);
	~CircularLavaHazard();
	static CircleHazard* factory(const GenericFactoryConstructionData&);
	static CircleHazard* randomizingFactory(double x_start, double y_start, double area_width, double area_height, const GenericFactoryConstructionData&);
	virtual int getFactoryArgumentCount() const override { return 3; }
	virtual CircleHazardConstructionTypes getConstructionType() const override { return CircleHazardConstructionTypes::radiusRequired; }
	virtual CircleFactoryInformation getFactoryInformation() const override { return { false, false, false, false, false }; }
};
