#include "minefield-level-effect.h"

#include "../constants.h"
#include <cmath>
#include "../rng.h"

#include "../bullet-manager.h"
#include "../powers/mines-power.h"

std::unordered_map<std::string, float> MinefieldLevelEffect::getWeights() const {
	std::unordered_map<std::string, float> weights;
	weights.insert({ "vanilla", 1.0f });
	weights.insert({ "random-vanilla", 1.0f });
	weights.insert({ "old", 1.0f });
	weights.insert({ "random-old", 1.0f });
	return weights;
}

Bullet* MinefieldLevelEffect::genMine() const {
	std::vector<BulletPower*> bp = { static_cast<BulletPower*>(new MinesBulletPower()) };
	const double r = TANK_RADIUS * BULLET_TO_TANK_RADIUS_RATIO;
	double x = GameRNG::randNumInRange(minefield_startX + r, minefield_startX + minefield_areaWidth - r);
	double y = GameRNG::randNumInRange(minefield_startY + r, minefield_startY + minefield_areaHeight - r);
	Bullet* mine = new Bullet(x, y, r, GameRNG::randFuncf() * float(2*PI), 0, HAZARD_TEAM, BulletParentType::team, NO_PARENT, &bp);
	return mine;
}

void MinefieldLevelEffect::pushRandomMine() {
	Bullet* b = genMine();
	BulletManager::pushBullet(b);
	aliveMinesPushed.push_back(b->getGameID());
}

inline void MinefieldLevelEffect::pushMine() {
	BulletManager::pushBullet(ghostMine);
	aliveMinesPushed.push_back(ghostMine->getGameID());
}

void MinefieldLevelEffect::apply() {
	for (int i = 0; i < initialMineCount; i++) {
		pushRandomMine();
	}
}

void MinefieldLevelEffect::tick(const Level* parent) {
	updateMineCount();
	if (aliveMinesPushed.size() >= maxNumOfMines) {
		tickCount = 0;
		return;
	}

	tickCount++;
	if (tickCount >= tickCycle) {
		tickCount -= tickCycle;
		pushMine();
		ghostMine = genMine();
	}

	//might make sense to have the ghost bullet inserting in doEffects()
}

void MinefieldLevelEffect::updateMineCount() {
	for (int i = aliveMinesPushed.size()-1; i >= 0; i--) {
		const Bullet* b = BulletManager::getBulletByID(aliveMinesPushed[i]);
		if (b == nullptr) { //not found
			aliveMinesPushed.erase(aliveMinesPushed.begin() + i);
		}
	}
}

void MinefieldLevelEffect::draw() const {
	if (aliveMinesPushed.size() < maxNumOfMines) {
		float alpha = float(tickCount) / float(tickCycle);
		ghostMine->ghostDraw(alpha * alpha);
	}
}

void MinefieldLevelEffect::draw(DrawingLayers layer) const {
	switch (layer) {
		default:
			std::cerr << "WARNING: unknown DrawingLayer for " + getName() + " draw!\n";
			[[fallthrough]];
		case DrawingLayers::under:
			draw(); //should it be in effects? JS had it there
			break;

		case DrawingLayers::normal:
			//nothing
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

void MinefieldLevelEffect::poseDraw() const {
	//TODO: I have no idea what this should be
	draw();
}

void MinefieldLevelEffect::poseDraw(DrawingLayers layer) const {
	switch (layer) {
		default:
			std::cerr << "WARNING: unknown DrawingLayer for " + getName() + " poseDraw!\n";
			[[fallthrough]];
		case DrawingLayers::under:
			poseDraw();
			break;

		case DrawingLayers::normal:
			//nothing
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

void MinefieldLevelEffect::ghostDraw(float alpha) const {
	//TODO: this fine?
	ghostMine->ghostDraw(alpha);
}

void MinefieldLevelEffect::ghostDraw(DrawingLayers layer, float alpha) const {
	switch (layer) {
		default:
			std::cerr << "WARNING: unknown DrawingLayer for " + getName() + " ghostDraw!\n";
			[[fallthrough]];
		case DrawingLayers::under:
			ghostDraw(alpha);
			break;

		case DrawingLayers::normal:
			//nothing
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

MinefieldLevelEffect::MinefieldLevelEffect(double x_start, double y_start, double area_width, double area_height, int initialMineCount, int maxMineCount) {
	tickCount = 0;
	tickCycle = 400; //in JS, 4*powerSpeed

	minefield_startX = x_start;
	minefield_startY = y_start;
	minefield_areaWidth = area_width;
	minefield_areaHeight = area_height;
	this->initialMineCount = initialMineCount;

	maxNumOfMines = maxMineCount;
	aliveMinesPushed.reserve(maxMineCount);
	ghostMine = genMine();
}

MinefieldLevelEffect::MinefieldLevelEffect(double x_start, double y_start, double area_width, double area_height, int initialMineCount) :
	MinefieldLevelEffect(x_start, y_start, area_width, area_height, initialMineCount, 128) {}

MinefieldLevelEffect::MinefieldLevelEffect(double x_start, double y_start, double area_width, double area_height) :
	MinefieldLevelEffect(x_start, y_start, area_width, area_height, 16) {}

MinefieldLevelEffect::MinefieldLevelEffect() : MinefieldLevelEffect(0, 0, GAME_WIDTH, GAME_HEIGHT) {}

MinefieldLevelEffect::~MinefieldLevelEffect() {
	delete ghostMine;
}

LevelEffect* MinefieldLevelEffect::factory(const GenericFactoryConstructionData& args) {
	if (args.getDataCount() >= 1) [[likely]] {
		int count = args.getDataPortionLength(0);

		if (count >= 4) [[likely]] {
			const double* arr = static_cast<const double*>(args.getDataPortion(0).get());
			double x_start = arr[0];
			double y_start = arr[1];
			double area_width = arr[2];
			double area_height = arr[3];

			if ((args.getDataCount() >= 2) && (args.getDataPortionLength(1) >= 1)) {
				const int* arr_mine = static_cast<const int*>(args.getDataPortion(1).get());
				int mineCount = arr_mine[0];

				if (args.getDataPortionLength(1) >= 2) {
					int maxMines = arr_mine[1];
					return new MinefieldLevelEffect(x_start, y_start, area_width, area_height, mineCount, maxMines);
				} else {
					return new MinefieldLevelEffect(x_start, y_start, area_width, area_height, mineCount);
				}
			} else {
				return new MinefieldLevelEffect(x_start, y_start, area_width, area_height);
			}
		}
	}

	return new MinefieldLevelEffect();
}
