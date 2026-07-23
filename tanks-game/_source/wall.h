#pragma once
class Wall;
struct WallUpdateStruct;

#include "game-thing.h"
#include "rect.h"

#include "color-value-holder.h"

class Wall : public GameThing, public Rect {
	friend class ResetThings; //not really needed
	friend class PowerFunctionHelper;

public: //TODO: shouldn't be public
	ColorValueHolder color;

public:
	void draw() const override;
	void draw(DrawingLayers) const override;
	void poseDraw() const override;
	void poseDraw(DrawingLayers) const override;
	void ghostDraw(float alpha) const override;
	void ghostDraw(DrawingLayers, float alpha) const override;

	void update(const WallUpdateStruct*);

	Wall(double x, double y, double w, double h, const ColorValueHolder& c);
	~Wall();
};

struct WallUpdateStruct {
	//deltas:
	double x;
	double y;
	//absolutes:
	//ColorValueHolder color;

	void add(const WallUpdateStruct& other);

	WallUpdateStruct(double x, double y);
	WallUpdateStruct() : WallUpdateStruct(0, 0) {}
};
