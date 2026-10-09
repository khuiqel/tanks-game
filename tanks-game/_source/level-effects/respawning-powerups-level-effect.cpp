#include "respawning-powerups-level-effect.h"

#include "../powerup-manager.h"

#include <iostream>

std::unordered_map<std::string, float> RespawningPowerupsLevelEffect::getWeights() const {
	std::unordered_map<std::string, float> weights;
	weights.insert({ "vanilla", 2.0f });
	weights.insert({ "random-vanilla", 1.0f }); //powerups need to be manually watched when not watching everything
	weights.insert({ "random", 1.0f });
	return weights;
}

void RespawningPowerupsLevelEffect::PowerSquareWatcher::tick() {
	if (this->isGone) {
		tickCount++;
		if (tickCount >= maxTickCount) {
			//new PowerSquare needed
			PowerSquare* p = new PowerSquare(*powerupCopy); //copy by reference (copy constructor)
			PowerupManager::pushPowerup(p);
			reset(p);
		}
	} else {
		const PowerSquare* p = PowerupManager::getPowerupByID(this->powerupID);
		if (p == nullptr) {
			setIsGone();
		}
	}
}

void RespawningPowerupsLevelEffect::PowerSquareWatcher::reset(const PowerSquare* p) {
	this->powerupID = p->getGameID();
	this->tickCount = 0;
	this->isGone = false;
}

RespawningPowerupsLevelEffect::PowerSquareWatcher::PowerSquareWatcher(const PowerSquare* p, double count) {
	this->powerupID = p->getGameID();
	this->tickCount = 0;
	this->maxTickCount = count;
	this->isGone = false;
	this->powerupCopy = new PowerSquare(*p); //copy by reference (copy constructor)
}

void RespawningPowerupsLevelEffect::PowerSquareWatcher::draw() const {
	if (isGone) {
		powerupCopy->ghostDraw(tickCount / maxTickCount); //TODO: drawing layer things?
	}
}

void RespawningPowerupsLevelEffect::PowerSquareWatcher::ghostDraw(float alpha) const {
	powerupCopy->ghostDraw(alpha); //TODO: drawing layer things?
}

RespawningPowerupsLevelEffect::PowerSquareWatcher::~PowerSquareWatcher() {
	delete powerupCopy;
}

void RespawningPowerupsLevelEffect::watchPowerSquare(const PowerSquare* p) {
	watchPowerSquare(p, default_maxTick);
}

void RespawningPowerupsLevelEffect::watchPowerSquare(const PowerSquare* p, double timeToRespawn) {
	bool found = false;
	for (int i = 0; i < watching.size(); i++) {
		if (watching[i]->getPowerupID() == p->getGameID()) [[unlikely]] {
			found = true;
			std::cerr << "WARNING: RespawningPowerupsLevelEffect is already watching PowerSquare ID#" << p->getGameID() << std::endl;
			break;
		}
	}
	if (!found) [[likely]] {
		watching.push_back(new PowerSquareWatcher(p, timeToRespawn));
	}
}

void RespawningPowerupsLevelEffect::watchLastPowerSquaresPushed(int count) {
	watchLastPowerSquaresPushed(count, default_maxTick);
}

void RespawningPowerupsLevelEffect::watchLastPowerSquaresPushed(int count, double timeToRespwan) {
	for (int i = count; i > 0; i--) {
		watchPowerSquare(PowerupManager::getPowerup(PowerupManager::getNumPowerups()-i), timeToRespwan);
	}
}

void RespawningPowerupsLevelEffect::unwatchPowerSquare(Game_ID powerupID) {
	for (int i = 0; i < watching.size(); i++) {
		if (watching[i]->getPowerupID() == powerupID) {
			watching.erase(watching.begin() + i);
			break;
		}
	}
}

void RespawningPowerupsLevelEffect::apply() {
	if (watchAllPowerups) {
		for (int i = 0; i < PowerupManager::getNumPowerups(); i++) {
			watchPowerSquare(PowerupManager::getPowerup(i));
		}
	}
}

void RespawningPowerupsLevelEffect::tick(const Level* parent) {
	//an option would be to check all powerups, and if there are new ones, watch them
	//this isn't done because of dev menu inserting

	for (int i = 0; i < watching.size(); i++) {
		watching[i]->tick();
	}
}

void RespawningPowerupsLevelEffect::draw() const {
	for (int i = 0; i < watching.size(); i++) {
		watching[i]->draw();
	}
}

void RespawningPowerupsLevelEffect::draw(DrawingLayers layer) const {
	switch (layer) {
		case DrawingLayers::under:
			//nothing
			break;

		case DrawingLayers::normal:
			//nothing
			break;

		default:
			std::cerr << "WARNING: unknown DrawingLayer for " + getName() + " draw!\n";
			[[fallthrough]];
		case DrawingLayers::effects:
			draw();
			break;

		case DrawingLayers::top:
			//nothing
			break;

		case DrawingLayers::debug:
			//later
			break;
	}
}

void RespawningPowerupsLevelEffect::poseDraw() const {
	ghostDraw(.5f);
}

void RespawningPowerupsLevelEffect::poseDraw(DrawingLayers layer) const {
	switch (layer) {
		case DrawingLayers::under:
			//nothing
			break;

		case DrawingLayers::normal:
			//nothing
			break;

		default:
			std::cerr << "WARNING: unknown DrawingLayer for " + getName() + " poseDraw!\n";
			[[fallthrough]];
		case DrawingLayers::effects:
			poseDraw();
			break;

		case DrawingLayers::top:
			//nothing
			break;

		case DrawingLayers::debug:
			//later
			break;
	}
}

void RespawningPowerupsLevelEffect::ghostDraw(float alpha) const {
	for (int i = 0; i < watching.size(); i++) {
		watching[i]->ghostDraw(alpha);
	}
}

void RespawningPowerupsLevelEffect::ghostDraw(DrawingLayers layer, float alpha) const {
	switch (layer) {
		case DrawingLayers::under:
			//nothing
			break;

		case DrawingLayers::normal:
			//nothing
			break;

		default:
			std::cerr << "WARNING: unknown DrawingLayer for " + getName() + " ghostDraw!\n";
			[[fallthrough]];
		case DrawingLayers::effects:
			ghostDraw(alpha);
			break;

		case DrawingLayers::top:
			//nothing
			break;

		case DrawingLayers::debug:
			//later
			break;
	}
}

RespawningPowerupsLevelEffect::RespawningPowerupsLevelEffect(bool watchEverything, double maxTick) {
	watchAllPowerups = watchEverything;
	default_maxTick = maxTick;
}

RespawningPowerupsLevelEffect::RespawningPowerupsLevelEffect(bool watchEverything) : RespawningPowerupsLevelEffect(watchEverything, 500) {}

RespawningPowerupsLevelEffect::RespawningPowerupsLevelEffect() : RespawningPowerupsLevelEffect(true) {}

RespawningPowerupsLevelEffect::~RespawningPowerupsLevelEffect() {
	for (int i = 0; i < watching.size(); i++) {
		delete watching[i];
	}
	//watching.clear(); //not necessary
}

LevelEffect* RespawningPowerupsLevelEffect::factory(const GenericFactoryConstructionData& args) {
	if (args.getDataCount() >= 1) [[likely]] {
		int count = args.getDataPortionLength(0);

		if (count >= 1) [[likely]] {
			//don't push the powerups to watch at the start because level construction happens before level initialization
			const bool* arr = static_cast<const bool*>(args.getDataPortion(0).get());
			bool watch = arr[0];

			if (args.getDataCount() >= 2) {
				int count_data = args.getDataPortionLength(1);
				if (count_data >= 1) {
					const double* arr_data = static_cast<const double*>(args.getDataPortion(1).get());
					double maxTick = arr_data[0];
					return new RespawningPowerupsLevelEffect(watch, maxTick);
				}
			}
			return new RespawningPowerupsLevelEffect(watch);
		}
	}

	return new RespawningPowerupsLevelEffect();
}
