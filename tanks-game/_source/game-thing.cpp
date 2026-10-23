#include "game-thing.h"

#include "game-manager.h"

GameThing::GameThing(Team_ID t_id, ObjectType type) {
	this->gameID = GameManager::getNextID();
	this->teamID = t_id;
	this->objectType = type;
	//does not initialize xStart/xEnd
}
