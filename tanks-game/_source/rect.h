#pragma once

struct Rect {
	friend class DeveloperManager;
	friend class CollisionHandler;
	friend class PowerFunctionHelper;

public:
	double x;
	double y;
	double w;
	double h;

	double getX() const noexcept { return x; }
	double getY() const noexcept { return y; }
	double getW() const noexcept { return w; }
	double getH() const noexcept { return h; }

	Rect() = default;
	Rect(double x_, double y_, double w_, double h_) { this->x = x_; this->y = y_; this->w = w_; this->h = h_; }
};
