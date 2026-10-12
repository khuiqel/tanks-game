#pragma once
#include "../rect-hazard.h"
#include "../generalized-lava.h"

#include "../constants.h"
#include "../simple-vector-2d.h"

class RectangularLavaHazard : public RectHazard, public GeneralizedLava {
protected:
	virtual void tryToPushNewBubble(float bubbleRadius) override;

public:
	virtual std::vector<std::string> getHazardTypes() const override {
		std::vector<std::string> types = std::vector<std::string>{ "vanilla", "random-vanilla", "old", "random-old", "random" };
		return types;
	}
	virtual std::unordered_map<std::string, float> getWeights() const override;

	virtual RectHazardCollisionType getCollisionType() const override { return RectHazardCollisionType::under; }

	//virtual bool actuallyCollided(const Tank*) const override { return true; }
	virtual InteractionUpdateHolder<TankUpdateStruct, RectHazardUpdateStruct> modifiedTankCollision(const Tank*) const override { return { false, false, nullptr, nullptr }; }

	virtual bool actuallyCollided(const Bullet* b) const override { return (b->velocity.getMagnitude() == 0); }
	virtual InteractionUpdateHolder<BulletUpdateStruct, RectHazardUpdateStruct> modifiedBulletCollision(const Bullet*) const override { return { false, false, nullptr, nullptr }; }

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
	//NOTE: bubbles have half the normal circle vertices
	static constexpr unsigned int BubbleSideCount = Circle::NumOfSides / 2;
	static SimpleVector2D bubble_vertices[BubbleSideCount + 1];
	static unsigned int bubble_indices[BubbleSideCount * 3];
	static unsigned int outline_indices[BubbleSideCount * 2*3];
	static bool initialized_vertices;

	static bool initializeVertices();

public:
	RectangularLavaHazard(double xpos, double ypos, double width, double height);
	~RectangularLavaHazard();
	static RectHazard* factory(const GenericFactoryConstructionData&);
	static RectHazard* randomizingFactory(double x_start, double y_start, double area_width, double area_height, const GenericFactoryConstructionData&);
	virtual int getFactoryArgumentCount() const override { return 4; }
	virtual RectHazardConstructionTypes getConstructionType() const override { return RectHazardConstructionTypes::standardConstruction; }
	virtual RectFactoryInformation getFactoryInformation() const override { return { false, false, false, false, false }; }
};
