#include "wall.h"

#include "constants.h"
#include <iostream>

#include "renderer.h"
#include "color-mixer.h"
#include "background-rect.h"

Wall::Wall(double x_, double y_, double w_, double h_, const ColorValueHolder& c) : GameThing(DEFAULT_TEAM, ObjectType::Wall) {
	this->x = x_;
	this->y = y_;
	this->w = w_;
	this->h = h_;
	this->color = c;
}

Wall::~Wall() {
	//nothing
}

void Wall::update(const WallUpdateStruct* up) {
	this->x += up->x;
	this->y += up->y;
}

void Wall::draw() const {
	ghostDraw(1.0f);
}

void Wall::draw(DrawingLayers layer) const {
	switch (layer) {
		case DrawingLayers::under:
			//nothing
			break;

		default:
			std::cerr << "WARNING: unknown DrawingLayer for Wall::draw!\n";
			[[fallthrough]];
		case DrawingLayers::normal:
			draw();
			break;

		case DrawingLayers::effects:
			//nothing
			break;

		case DrawingLayers::top:
			//nothing
			break;

		case DrawingLayers::debug:
			//later
			break;
	}
}

void Wall::poseDraw() const {
	draw();
}

void Wall::poseDraw(DrawingLayers layer) const {
	switch (layer) {
		case DrawingLayers::under:
			//nothing
			break;

		default:
			std::cerr << "WARNING: unknown DrawingLayer for Wall::poseDraw!\n";
			[[fallthrough]];
		case DrawingLayers::normal:
			poseDraw();
			break;

		case DrawingLayers::effects:
			//nothing
			break;

		case DrawingLayers::top:
			//nothing
			break;

		case DrawingLayers::debug:
			//later
			break;
	}
}

void Wall::ghostDraw(float alpha) const {
	alpha = alpha * alpha;
	ColorValueHolder c = this->color;
	c = ColorMixer::mix(BackgroundRect::getBackColor(), c, alpha);

	float coordsAndColor[] = {
		x,   y,     c.getRf(), c.getGf(), c.getBf(), c.getAf(),
		x+w, y,     c.getRf(), c.getGf(), c.getBf(), c.getAf(),
		x+w, y+h,   c.getRf(), c.getGf(), c.getBf(), c.getAf(),
		x,   y+h,   c.getRf(), c.getGf(), c.getBf(), c.getAf()
	};
	unsigned int indices[] = {
		0, 1, 2,
		2, 3, 0
	};

	Renderer::SubmitBatchedDraw(coordsAndColor, 4 * (2+4), indices, 2 * 3);
}

void Wall::ghostDraw(DrawingLayers layer, float alpha) const {
	switch (layer) {
		case DrawingLayers::under:
			//nothing
			break;

		default:
			std::cerr << "WARNING: unknown DrawingLayer for Wall::ghostDraw!\n";
			[[fallthrough]];
		case DrawingLayers::normal:
			ghostDraw(alpha);
			break;

		case DrawingLayers::effects:
			//nothing
			break;

		case DrawingLayers::top:
			//nothing
			break;

		case DrawingLayers::debug:
			//later
			break;
	}
}

WallUpdateStruct::WallUpdateStruct(double x, double y) {
	this->x = x;
	this->y = y;
}

void WallUpdateStruct::add(const WallUpdateStruct& other) {
	this->x += other.x;
	this->y += other.y;
}
