#pragma once
#include <utility>
#include <optional>

#include "circle.h"
#include "rect.h"

//would've been nice to have a pair and rename its members to x and y, but C++ doesn't allow that
//the closest way to do that requires anonymous structs which aren't a good idea: union PositionHolder { std::pair<double,double> pos; struct { double x; double y; } }
using PositionHolder = std::pair<double, double>;

class CollisionHandler {
protected:
	static bool cornerCollided(const Circle&, double x, double y);
	static bool cornerCollidedIgnoreEdge(const Circle&, double x, double y);

	static void cornerPushMovableAwayFromImmovable(Circle& movable, double x, double y);
	static void cornerPushMovableAwayFromMovable(Circle& movable1, Rect& movable2, double x, double y);
	static void cornerPushMovableAwayFromImmovable(Rect& movable, Circle&, double x, double y);
	static void cornerPushMovableAwayFromMovable(Rect& movable1, Circle& movable2, double x, double y) {
		CollisionHandler::cornerPushMovableAwayFromMovable(movable2, movable1, x, y);
	}

	static std::pair<double, double>
	cornerPushMovableAwayFromImmovable_vecOnly(const Circle& movable, double x, double y);
	static std::pair<std::pair<double, double>, std::pair<double, double>>
	cornerPushMovableAwayFromMovable_vecOnly(const Circle& movable1, const Rect& movable2, double x, double y);
	static std::pair<double, double>
	cornerPushMovableAwayFromImmovable_vecOnly(const Rect& movable, const Circle&, double x, double y);
	static std::pair<std::pair<double, double>, std::pair<double, double>>
	cornerPushMovableAwayFromMovable_vecOnly(const Rect& movable1, const Circle& movable2, double x, double y) {
		std::pair<std::pair<double, double>, std::pair<double, double>> vecs = CollisionHandler::cornerPushMovableAwayFromMovable_vecOnly(movable2, movable1, x, y);
		return { vecs.second, vecs.first };
	}

public: //for tanks and bullets and stuff (things that move around)
	static bool partiallyOutOfBounds(const Rect&);
	static bool partiallyOutOfBoundsIgnoreEdge(const Rect&);
	static bool fullyOutOfBounds(const Rect&);
	static bool fullyOutOfBoundsIgnoreEdge(const Rect&);

	//below: only does simple check, treating the circle like a square
	static bool partiallyOutOfBounds(const Circle&);
	static bool partiallyOutOfBoundsIgnoreEdge(const Circle&);
	static bool fullyOutOfBounds(const Circle&);
	static bool fullyOutOfBoundsIgnoreEdge(const Circle&);

	static void edgeConstrain(Rect&);
	static void edgeConstrain(Circle&);
	static void edgeConstrain(Rect&, double distFromEdge); //why? at the very least, it's better than powers having to do it themselves, I guess
	static void edgeConstrain(Circle&, double distFromEdge);

public: //collision detection and handling (just moving)
	static bool partiallyCollided(const Rect&, const Rect&);
	static bool partiallyCollided(const Rect&, const Circle&);
	static bool partiallyCollided(const Circle& c, const Rect& r) {
		return CollisionHandler::partiallyCollided(r, c);
	}
	static bool partiallyCollided(const Circle&, const Circle&);

	static bool partiallyCollidedIgnoreEdge(const Rect&, const Rect&);
	static bool partiallyCollidedIgnoreEdge(const Rect&, const Circle&);
	static bool partiallyCollidedIgnoreEdge(const Circle& c, const Rect& r) {
		return CollisionHandler::partiallyCollidedIgnoreEdge(r, c);
	}
	static bool partiallyCollidedIgnoreEdge(const Circle&, const Circle&);

	static bool fullyCollided(const Rect&, const Rect&);
	static bool fullyCollided(const Rect&, const Circle&);
	static bool fullyCollided(const Circle&, const Rect&); //different
	static bool fullyCollided(const Circle&, const Circle&);

	static void pushMovableAwayFromImmovable(Rect& movable, Rect& immovable);
	static void pushMovableAwayFromMovable(Rect& movable1, Rect& movable2);

	static void pushMovableAwayFromImmovable(Circle& movable, Rect& immovable);
	static void pushMovableAwayFromMovable(Circle& movable1, Rect& movable2) {
		CollisionHandler::pushMovableAwayFromMovable(movable2, movable1);
	}

	//these two are weird
	static void pushMovableAwayFromImmovable(Rect& movable, Circle& immovable);
	static void pushMovableAwayFromMovable(Rect& movable1, Circle& movable2);

	static void pushMovableAwayFromImmovable(Circle& movable, Circle& immovable);
	static void pushMovableAwayFromMovable(Circle& movable1, Circle& movable2);

public: //displacement-only versions
	static std::pair<double, double>
	pushMovableAwayFromImmovable_vecOnly(const Rect& movable, const Rect& immovable);
	static std::pair<std::pair<double, double>, std::pair<double, double>>
	pushMovableAwayFromMovable_vecOnly(const Rect& movable1, const Rect& movable2);

	static std::pair<double, double>
	pushMovableAwayFromImmovable_vecOnly(const Circle& movable, const Rect& immovable);
	static std::pair<std::pair<double, double>, std::pair<double, double>>
	pushMovableAwayFromMovable_vecOnly(const Circle& movable1, const Rect& movable2) {
		std::pair<std::pair<double, double>, std::pair<double, double>> vecs = CollisionHandler::pushMovableAwayFromMovable_vecOnly(movable2, movable1);
		return { vecs.second, vecs.first };
	}

	static std::pair<double, double>
	pushMovableAwayFromImmovable_vecOnly(const Rect& movable, const Circle& immovable);
	static std::pair<std::pair<double, double>, std::pair<double, double>>
	pushMovableAwayFromMovable_vecOnly(const Rect& movable1, const Circle& movable2);

	static std::pair<double, double>
	pushMovableAwayFromImmovable_vecOnly(const Circle& movable, const Circle& immovable);
	static std::pair<std::pair<double, double>, std::pair<double, double>>
	pushMovableAwayFromMovable_vecOnly(const Circle& movable1, const Circle& movable2);

	/*
	//interesting...
	static void pullMovableIntoImmovable(Rect& movable, Rect& immovable);
	static void pullMovableIntoMovable(Rect& movable1, Rect& movable2);
	static void pullMovableNearImmovable(Rect& movable, Rect& immovable); //like a reverse pull (but not a push because one has to stay inside the other)

	static void pullMovableIntoImmovable(Circle& movable, Rect& immovable);
	static void pullMovableIntoMovable(Circle& movable1, Rect& movable2);
	static void pullMovableNearImmovable(Circle& movable, Rect& immovable); //like a reverse pull (but not a push because one has to stay inside the other)

	static void pullMovableIntoImmovable(Rect& movable, Circle& immovable);
	static void pullMovableIntoMovable(Rect& movable1, Circle& movable2);
	static void pullMovableNearImmovable(Rect& movable, Circle& immovable); //like a reverse pull (but not a push because one has to stay inside the other)

	static void pullMovableIntoImmovable(Circle& movable, Circle& immovable);
	static void pullMovableIntoMovable(Circle& movable1, Circle& movable2);
	static void pullMovableNearImmovable(Circle& movable, Circle& immovable); //like a reverse pull (but not a push because one has to stay inside the other)
	*/

public:
	//check if intersection or tangent happens, and if so where:
	static std::optional<std::pair<PositionHolder, PositionHolder>> circleLineIntersectionCheckAndGet(const Circle&, double lineX1, double lineY1, double lineX2, double lineY2);
	//only check if it happens:
	static bool circleLineIntersectionHappens(const Circle&, double lineX1, double lineY1, double lineX2, double lineY2);
	//only get location of intersection:
	static std::pair<PositionHolder, PositionHolder> circleLineIntersection(const Circle&, double lineX1, double lineY1, double lineX2, double lineY2);

public: //other stuff
	static bool lineLineCollision(double line1X, double line1Y, double line2X, double line2Y, double line3X, double line3Y, double line4X, double line4Y);
	static bool lineRectCollision(double line1X, double line1Y, double line2X, double line2Y, const Rect& r);

private:
	CollisionHandler() = delete;
	CollisionHandler(const CollisionHandler&) = delete;
};
