#pragma once
#include "constants.h"

enum class DrawingLayers {
	under,
	normal,
	effects,
	top, //tank death
	debug
};

class GameThing {
	friend class GameManager;
	friend struct GameThingComparator;

private:
	Game_ID gameID;
	Team_ID teamID;
	ObjectType objectType;

	double xStart;
	double xEnd;

public:
	Game_ID getGameID() const noexcept { return this->gameID; }
	Team_ID getTeamID() const noexcept { return this->teamID; }
	ObjectType getObjectType() const noexcept { return this->objectType; }

	double get_xStart() const noexcept { return this->xStart; }
	double get_xEnd() const noexcept { return this->xEnd; }

protected:
	void updateAABB() noexcept;
	void changeTeamID(Team_ID t_id) { this->teamID = t_id; }
	GameThing(Team_ID t_id, ObjectType type);

private:
	GameThing() = delete;
	GameThing(const GameThing&) = delete;

public:
	virtual void draw() const = 0; //draw normally
	virtual void draw(DrawingLayers) const = 0;
	virtual void poseDraw() const = 0; //"pose for the camera"
	virtual void poseDraw(DrawingLayers) const = 0;
	virtual void ghostDraw(float alpha) const = 0; //draw something not quite ready for this world
	virtual void ghostDraw(DrawingLayers, float alpha) const = 0;
};

struct GameThingComparator {
	//allows something like an std::map to sort
	constexpr bool operator()(const GameThing& lhs, const GameThing& rhs) const noexcept {
		return lhs.gameID < rhs.gameID;
	}
};
