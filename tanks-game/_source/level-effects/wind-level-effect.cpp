#include "wind-level-effect.h"

#include "../constants.h"
#include <cmath> //sqrt
#include <algorithm> //std::copy
#include <iostream>
#include "../rng.h"

#include "../renderer.h"
#include "../color-mixer.h"
#include "../background-rect.h"

#include "../tank-manager.h"
#include "../bullet-manager.h"

SimpleVector2D WindLevelEffect::vertices_spike[3];
SimpleVector2D WindLevelEffect::vertices_arrow[7];
unsigned int WindLevelEffect::indices_spike[1*3];
unsigned int WindLevelEffect::indices_arrow[3*3];
bool WindLevelEffect::initialized_vertices = false;

std::unordered_map<std::string, float> WindLevelEffect::getWeights() const {
	std::unordered_map<std::string, float> weights;
	weights.insert({ "vanilla", 1.0f });
	weights.insert({ "random-vanilla", 0.5f });
	weights.insert({ "old", 1.0f });
	weights.insert({ "random-old", 1.0f });
	weights.insert({ "random", 0.5f });
	return weights;
}

float WindLevelEffect::getWindStrengthMultiplier() const {
	switch (currentState) {
		case 0: return 0;
		case 1: return tickCount / (tickCycle * stateMultiplier[1]);
		case 2: return 1;
		case 3: return 1 - tickCount / (tickCycle * stateMultiplier[3]);
		default: return 0;
	}
}

void WindLevelEffect::tick(const Level* parent) {
	tickCount++;
	bool changeWind = false;
	//TODO: account for states to be skipped entirely, better
	while (tickCount >= tickCycle * stateMultiplier[currentState]) {
		if (tickCycle * stateMultiplier[currentState] <= 0) {
			tickCount = 0;
			++currentState %= maxState;
			if (currentState == 1) {
				changeWind = true;
			}
			break;
		} else {
			tickCount -= tickCycle * stateMultiplier[currentState];
			++currentState %= maxState;
			if (currentState == 1) {
				changeWind = true;
			}
		}
	}
	if (changeWind) {
		//pushDirection = SimpleVector2D(GameRNG::randFunc() * (2*PI), GameRNG::randFunc(), true); //JS, I think (~90% sure)
		pushDirection = SimpleVector2D(GameRNG::randFuncf() * float(2*PI), GameRNG::randFloatInRange(.125, 1.0), true);
	}
}

void WindLevelEffect::doEffects(Level* parent) const {
	if (getWindStrengthMultiplier() > 0) {
		for (int i = 0; i < TankManager::getNumTanks(); i++) {
			Tank* t = TankManager::getTank(i);
			t->x += pushDirection.getXComp() * getWindStrengthMultiplier();
			t->y += pushDirection.getYComp() * getWindStrengthMultiplier();
		}
		for (int i = 0; i < BulletManager::getNumBullets(); i++) {
			Bullet* b = BulletManager::getBullet(i);
			b->x += pushDirection.getXComp() * getWindStrengthMultiplier();
			b->y += pushDirection.getYComp() * getWindStrengthMultiplier();
		}
	}
}

void WindLevelEffect::draw() const {
	//length = 16 * pushDirection.getMagnitude()
	//7 x 5 arrows

	if (getWindStrengthMultiplier() <= 0) {
		return;
	}

	ColorValueHolder color = ColorMixer::mix(BackgroundRect::getBackColor(), ColorValueHolder(0.0f, 0.0f, 0.0f));

	const float length = 16 * (pushDirection.getMagnitude() * getWindStrengthMultiplier()); //normal
	//const float length = 16 * std::sqrt(pushDirection.getMagnitude() * getWindStrengthMultiplier()); //ez way to make small values more visible
	//sqrt is not a big deal to compute: https://stackoverflow.com/questions/41582376/fast-approximation-of-square-rootx-with-known-0-x-1
	const float x_offset = 64; //JS x offset: (528-112)/7 (it's the distance between the main walls) = 59.42857
	const float y_offset = 64; //JS y offset: (320)/5 = 64
	for (int i = -3; i <= 3; i++) {
		for (int j = -2; j <= 2; j++) {
			if (i == 0 && j == 0) {
				float coordsAndColor_arrow[7*(2+4)];
				for (int k = 0; k < 7; k++) {
					SimpleVector2D vertex = SimpleVector2D(vertices_arrow[k]);
					vertex.scaleAndRotate(length, pushDirection.getAngle());
					coordsAndColor_arrow[k*6]   = (static_cast<float>(GAME_WIDTH/2)  + i*x_offset) + vertex.getXComp();
					coordsAndColor_arrow[k*6+1] = (static_cast<float>(GAME_HEIGHT/2) + j*y_offset) + vertex.getYComp();
					coordsAndColor_arrow[k*6+2] = color.getRf();
					coordsAndColor_arrow[k*6+3] = color.getGf();
					coordsAndColor_arrow[k*6+4] = color.getBf();
					coordsAndColor_arrow[k*6+5] = color.getAf();
				}

				Renderer::SubmitBatchedDraw(coordsAndColor_arrow, 7*(2+4), indices_arrow, 3*3);
			} else {
				float coordsAndColor_spike[3*(2+4)];
				for (int k = 0; k < 3; k++) {
					SimpleVector2D vertex = SimpleVector2D(vertices_spike[k]);
					vertex.scaleAndRotate(length, pushDirection.getAngle());
					coordsAndColor_spike[k*6]   = (static_cast<float>(GAME_WIDTH/2)  + i*x_offset) + vertex.getXComp();
					coordsAndColor_spike[k*6+1] = (static_cast<float>(GAME_HEIGHT/2) + j*y_offset) + vertex.getYComp();
					coordsAndColor_spike[k*6+2] = color.getRf();
					coordsAndColor_spike[k*6+3] = color.getGf();
					coordsAndColor_spike[k*6+4] = color.getBf();
					coordsAndColor_spike[k*6+5] = color.getAf();
				}

				Renderer::SubmitBatchedDraw(coordsAndColor_spike, 3*(2+4), indices_spike, 1*3);
			}
		}
	}
}

void WindLevelEffect::draw(DrawingLayers layer) const {
	switch (layer) {
		default:
			std::cerr << "WARNING: unknown DrawingLayer for " + getName() + " draw!\n";
			[[fallthrough]];
		case DrawingLayers::under:
			draw();
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

void WindLevelEffect::poseDraw() const {
	ghostDraw(1.0f);
}

void WindLevelEffect::poseDraw(DrawingLayers layer) const {
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

void WindLevelEffect::ghostDraw(float alpha) const {
	//from draw() (maybe copy-pasting isn't a good idea)
	//7 x 5 arrows

	ColorValueHolder color = ColorMixer::mix(BackgroundRect::getBackColor(), ColorValueHolder(0.0f, 0.0f, 0.0f));
	color = ColorMixer::mix(BackgroundRect::getBackColor(), color, alpha);

	const float length = 16 * pushDirection.getMagnitude();
	const float x_offset = 64;
	const float y_offset = 64;
	for (int i = -3; i <= 3; i++) {
		for (int j = -2; j <= 2; j++) {
			if (i == -j) {
				continue;
			}

			double angle;
			if (i < -j) {
				//bottom left
				angle = -3*(PI/4);
			} else {
				//top right
				angle = PI/4;
			}

			float coordsAndColor_spike[3*(2+4)];
			for (int k = 0; k < 3; k++) {
				SimpleVector2D vertex = SimpleVector2D(vertices_spike[k]);
				vertex.scaleAndRotate(length, pushDirection.getAngle());
				coordsAndColor_spike[k*6]   = (static_cast<float>(GAME_WIDTH/2)  + i*x_offset) + vertex.getXComp();
				coordsAndColor_spike[k*6+1] = (static_cast<float>(GAME_HEIGHT/2) + j*y_offset) + vertex.getYComp();
				coordsAndColor_spike[k*6+2] = color.getRf();
				coordsAndColor_spike[k*6+3] = color.getGf();
				coordsAndColor_spike[k*6+4] = color.getBf();
				coordsAndColor_spike[k*6+5] = color.getAf();
			}

			Renderer::SubmitBatchedDraw(coordsAndColor_spike, 3*(2+4), indices_spike, 1*3);
		}
	}
}

void WindLevelEffect::ghostDraw(DrawingLayers layer, float alpha) const {
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

bool WindLevelEffect::initializeVertices() {
	if (initialized_vertices) [[likely]] {
		return false;
	}

	//an arrow was originally intended to be the main indicator of direction, but was complex to implement (when doing bad code), so a spike was used

	vertices_spike[0] = SimpleVector2D(-1.0f, -0.125f);
	vertices_spike[1] = SimpleVector2D(-1.0f,  0.125f);
	vertices_spike[2] = SimpleVector2D( 1.0f,  0.0f  );

	unsigned int spike_temp[] = { 0, 1, 2 };
	std::copy(spike_temp, spike_temp + (1*3), indices_spike);

	vertices_arrow[0] = SimpleVector2D(-1.0f, -0.125f); //bottom left
	vertices_arrow[1] = SimpleVector2D(-1.0f,  0.125f); //top left
	vertices_arrow[2] = SimpleVector2D( 0.5f,  0.125f); //top right
	vertices_arrow[3] = SimpleVector2D( 0.5f,  0.5f  );
	vertices_arrow[4] = SimpleVector2D( 1.0f,  0.0f  ); //arrow tip
	vertices_arrow[5] = SimpleVector2D( 0.5f, -0.5f  );
	vertices_arrow[6] = SimpleVector2D( 0.5f, -0.125f); //bottom right

	unsigned int arrow_temp[] = {
		0, 1, 2,
		2, 6, 0,
		3, 4, 5
	};
	std::copy(arrow_temp, arrow_temp + (3*3), indices_arrow);

	initialized_vertices = true;
	return true;
}

WindLevelEffect::WindLevelEffect(bool transitionWind) {
	tickCount = 0;
	tickCycle = 200;
	currentState = 0;
	maxState = 4;
	if (transitionWind) {
		stateMultiplier = new double[4]{ .75, .25, .75, .25 };
	} else {
		stateMultiplier = new double[4]{ 1, 0, 1, 0 };
	}

	initializeVertices();
}

WindLevelEffect::WindLevelEffect() : WindLevelEffect(false) {}

WindLevelEffect::~WindLevelEffect() {
	delete[] stateMultiplier;
}

LevelEffect* WindLevelEffect::factory(const GenericFactoryConstructionData& args) {
	//TODO: modify stateMultiplier
	if (args.getDataCount() >= 1) [[likely]] {
		int count = args.getDataPortionLength(0);

		if (count >= 1) [[likely]] {
			const bool* arr = static_cast<const bool*>(args.getDataPortion(0).get());
			bool t = arr[0];
			return new WindLevelEffect(t);
		}
	}

	return new WindLevelEffect();
}
