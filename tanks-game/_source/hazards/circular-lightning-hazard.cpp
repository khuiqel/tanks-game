#include "circular-lightning-hazard.h"

#include "../constants.h"
#include <cmath>
#include <stdexcept>
#include <algorithm> //std::find, std::copy
#include <iostream>
#include "../rng.h"
#include "../mylib.h" //pointInPolygon

#include "../renderer.h"
#include "../color-mixer.h"
#include "../background-rect.h"

#include "../collision-handler.h"
#include "../wall-manager.h"
#include "../hazard-manager.h"

SimpleVector2D CircularLightningHazard::body_vertices[Circle::NumOfSides+1];
unsigned int CircularLightningHazard::body_indices[Circle::NumOfSides*3];
unsigned int CircularLightningHazard::outline_indices[Circle::NumOfSides*2*3];
bool CircularLightningHazard::initialized_vertices = false;

std::unordered_map<std::string, float> CircularLightningHazard::getWeights() const {
	std::unordered_map<std::string, float> weights;
	weights.insert({ "vanilla", 1.0f });
	weights.insert({ "random-vanilla", 1.0f });
	weights.insert({ "old", 1.0f });
	weights.insert({ "random-old", 1.0f });
	weights.insert({ "random", 1.0f });
	return weights;
}

CircularLightningHazard::CircularLightningHazard(double xpos, double ypos, double radius) : CircleHazard(HAZARD_TEAM) {
	cir.x = xpos;
	cir.y = ypos;
	cir.r = radius;

	tickCount = 0;
	tickCycle = 100; //100 is JS default (because of power speed)
	constexpr double temp[2] = { 2, 2 };
	std::copy(temp, temp+2, stateMultiplier);
	currentlyActive = false;
	//flexible = false;

	maxBolts = 1;
	lengthOfBolt = 4; //JS: 2 (though the circular version didn't exist in JS)
	boltTick = 0;
	boltCycle = 4; //JS: none (though the circular version didn't exist in JS)
	boltsNeeded = false;

	//canAcceptPowers = false;

	initializeVertices();
}

CircularLightningHazard::~CircularLightningHazard() {
	//nothing
}

bool CircularLightningHazard::initializeVertices() {
	if (initialized_vertices) [[likely]] {
		return false;
	}

	body_vertices[0] = SimpleVector2D(0, 0);
	for (int i = 1; i < Circle::NumOfSides+1; i++) {
		body_vertices[i] = SimpleVector2D(std::cos((i-1) * (2*PI / Circle::NumOfSides)), std::sin((i-1) * (2*PI / Circle::NumOfSides)));
	}

	for (int i = 0; i < Circle::NumOfSides; i++) {
		body_indices[i*3]   = 0;
		body_indices[i*3+1] = i+1;
		body_indices[i*3+2] = (i+1) % Circle::NumOfSides + 1;
	}

	for (int i = 0; i < Circle::NumOfSides; i++) {
		outline_indices[i*6]   =  i*2;
		outline_indices[i*6+1] =  i*2+1;
		outline_indices[i*6+2] = (i*2+3) % (Circle::NumOfSides*2);
		outline_indices[i*6+3] = (i*2+3) % (Circle::NumOfSides*2);
		outline_indices[i*6+4] = (i*2+2) % (Circle::NumOfSides*2);
		outline_indices[i*6+5] =  i*2;
	}

	initialized_vertices = true;
	return true;
}

CircleHazard* CircularLightningHazard::factory(const GenericFactoryConstructionData& args) {
	if (args.getDataCount() >= 1) [[likely]] {
		int count = args.getDataPortionLength(0);

		if (count >= 3) [[likely]] {
			const double* arr = static_cast<const double*>(args.getDataPortion(0).get());
			double x = arr[0];
			double y = arr[1];
			double r = arr[2];
			return new CircularLightningHazard(x, y, r);
		}
	}

	return new CircularLightningHazard(0, 0, 0);
}

void CircularLightningHazard::specialEffectCircleCollision(const Circle& other) {
	const Circle centerPoint = getCenterPoint();
	double intersectionX, intersectionY;
	int boltPoints;
	if (CollisionHandler::fullyCollided(centerPoint, other)) {
		intersectionX = other.x;
		intersectionY = other.y;
		boltPoints = 2;
	} else {
		SimpleVector2D distToCircle = SimpleVector2D(other.x - centerPoint.x, other.y - centerPoint.y);
		distToCircle.changeMagnitude(-1 * other.r);
		intersectionX = centerPoint.x + distToCircle.getXComp();
		intersectionY = centerPoint.y + distToCircle.getYComp();
		boltPoints = getDefaultNumBoltPoints(distToCircle.getMagnitude());
	}
	pushBolt(new LightningBolt(0, 0, intersectionX - cir.x, intersectionY - cir.y, boltPoints));
}

void CircularLightningHazard::specialEffectTankCollision(const Tank* t) {
	//if bolts are being randomized, clear them, and mark that they've been cleared
	if (!boltsNeeded) {
		clearBolts();
	}
	boltsNeeded = true;

	if (std::find(targetedObjects.begin(), targetedObjects.end(), t->getGameID()) == targetedObjects.end()) {
		targetedObjects.push_back(t->getGameID());
	} else {
		return;
	}

	specialEffectCircleCollision(t->cir);
}

void CircularLightningHazard::specialEffectBulletCollision(const Bullet* b) {
	if (!boltsNeeded) {
		clearBolts();
	}
	boltsNeeded = true;

	if (std::find(targetedObjects.begin(), targetedObjects.end(), b->getGameID()) == targetedObjects.end()) {
		targetedObjects.push_back(b->getGameID());
	} else {
		return;
	}

	specialEffectCircleCollision(b->cir);
}

void CircularLightningHazard::pushBolt(LightningBolt* l) {
	bolts.push_back(l);
	refreshBolt(l);
}

void CircularLightningHazard::pushDefaultBolt(int num, bool randomize) {
	//the default bolt is from center to a random point
	float randR = float(cir.r)*VisualRNG::randFuncf(), randAngle = float(2*PI)*VisualRNG::randFuncf();
	float xEnd = randR*std::cos(randAngle), yEnd = randR*std::sin(randAngle);
	for (int i = 0; i < num; i++) {
		LightningBolt* l = new LightningBolt(0, 0, xEnd, yEnd, getDefaultNumBoltPoints(std::sqrt(xEnd*xEnd + yEnd*yEnd)));
		if (randomize) {
			pushBolt(l);
		} else {
			bolts.push_back(l);
		}
	}
}

bool CircularLightningHazard::validLocation() const {
	for (int i = 0; i < WallManager::getNumWalls(); i++) {
		const Wall* wa = WallManager::getWall(i);
		if (CollisionHandler::partiallyCollidedIgnoreEdge(wa->rec, this->cir)) {
			return false;
		}
	}
	return true;
}

bool CircularLightningHazard::reasonableLocation() const {
	for (int i = 0; i < HazardManager::getNumCircleHazards(); i++) {
		const CircleHazard* ch = HazardManager::getCircleHazard(i);
		if (ch->getName() != this->getName()) {
			if (CollisionHandler::partiallyCollided(this->cir, ch->cir)) {
				return false;
			}
		}
	}
	for (int i = 0; i < HazardManager::getNumRectHazards(); i++) {
		const RectHazard* rh = HazardManager::getRectHazard(i);
		if (rh->getName() != this->getName()) {
			if (CollisionHandler::partiallyCollided(this->cir, rh->rec)) {
				return false;
			}
		}
	}

	return validLocation();
}

void CircularLightningHazard::refreshBolt(LightningBolt* l) const {
	//see RectangularLightningHazard
	if (l->length <= 2) {
		return;
	}

	const float boltDeltaX = l->positions[l->length*2-2] - l->positions[0];
	const float boltDeltaY = l->positions[l->length*2-1] - l->positions[1];
	const SimpleVector2D boltVec = SimpleVector2D(boltDeltaX, boltDeltaY);
	const float sinAngle = std::sin(boltVec.getAngle());
	const float cosAngle = std::cos(boltVec.getAngle()); //to avoid recalculating each time (though it would probably get optimized out)
	const float newH = boltVec.getMagnitude() * 1.0f; //maybe *.5, I dunno
	const float maxVariance = (1.0f/4.0f) * boltVec.getMagnitude() * 1.0f; //(same here)

	const float polygonX[6] = {
		l->positions[0],
		l->positions[0] + boltDeltaX * (1.0f/4.0f) - (newH * .5f) * sinAngle,
		l->positions[0] + boltDeltaX * (3.0f/4.0f) - (newH * .5f) * sinAngle,
		l->positions[0] + boltDeltaX,
		l->positions[0] + boltDeltaX * (3.0f/4.0f) + (newH * .5f) * sinAngle,
		l->positions[0] + boltDeltaX * (1.0f/4.0f) + (newH * .5f) * sinAngle
	};
	const float polygonY[6] = {
		l->positions[1],
		l->positions[1] + boltDeltaY * (1.0f/4.0f) + (newH * .5f) * cosAngle,
		l->positions[1] + boltDeltaY * (3.0f/4.0f) + (newH * .5f) * cosAngle,
		l->positions[1] + boltDeltaY,
		l->positions[1] + boltDeltaY * (3.0f/4.0f) - (newH * .5f) * cosAngle,
		l->positions[1] + boltDeltaY * (1.0f/4.0f) - (newH * .5f) * cosAngle
	};

	for (int j = 1; j < l->length-1; j++) {
		float randTemp;
		float testX, testY;
		do {
			randTemp = maxVariance * (VisualRNG::randFuncf()*2-1);
			testX = l->positions[j*2 - 2] + (boltDeltaX/(l->length-1)) - randTemp * sinAngle;
			testY = l->positions[j*2 - 1] + (boltDeltaY/(l->length-1)) + randTemp * cosAngle;
		} while (!pointInPolygon(6, polygonX, polygonY, testX, testY));
		l->positions[j*2]   = testX;
		l->positions[j*2+1] = testY;
	}
}

void CircularLightningHazard::draw() const {
	drawBackground(false);
	drawBolts();
}

void CircularLightningHazard::draw(DrawingLayers layer) const {
	switch (layer) {
		case DrawingLayers::under:
			drawBackground(false);
			break;

		default:
			std::cerr << "WARNING: unknown DrawingLayer for " + getName() + " draw!\n";
			[[fallthrough]];
		case DrawingLayers::normal:
			drawBolts();
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

void CircularLightningHazard::poseDraw() const {
	drawBackground(true);
	drawBolts_Pose();
}

void CircularLightningHazard::poseDraw(DrawingLayers layer) const {
	switch (layer) {
		case DrawingLayers::under:
			drawBackground(true);
			break;

		default:
			std::cerr << "WARNING: unknown DrawingLayer for " + getName() + " poseDraw!\n";
			[[fallthrough]];
		case DrawingLayers::normal:
			drawBolts_Pose();
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

void CircularLightningHazard::ghostDraw(float alpha) const {
	//not too sure on this
	drawBackground(true, alpha);
	drawBolts_Pose(alpha);
}

void CircularLightningHazard::ghostDraw(DrawingLayers layer, float alpha) const {
	switch (layer) {
		case DrawingLayers::under:
			drawBackground(true, alpha);
			break;

		default:
			std::cerr << "WARNING: unknown DrawingLayer for " + getName() + " ghostDraw!\n";
			[[fallthrough]];
		case DrawingLayers::normal:
			drawBolts_Pose(alpha);
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

void CircularLightningHazard::drawBackground(bool pose, float alpha) const {
	//ColorValueHolder color = (pose ? getBackgroundColor_Pose() : getBackgroundColor());
	ColorValueHolder color = getBackgroundColor_Pose();
	color = ColorMixer::mix(BackgroundRect::getBackColor(), color, alpha);

	float scale;
	if (pose || currentlyActive) {
		scale = 1.0f;
	} else {
		scale = tickCount / (tickCycle * stateMultiplier[currentlyActive]);
	}
	scale = scale * scale;

	float coordsAndColor[(Circle::NumOfSides+1)*(2+4)];
	coordsAndColor[0] = cir.x;
	coordsAndColor[1] = cir.y;
	coordsAndColor[2] = color.getRf();
	coordsAndColor[3] = color.getGf();
	coordsAndColor[4] = color.getBf();
	coordsAndColor[5] = color.getAf();
	for (int i = 1; i < Circle::NumOfSides+1; i++) {
		coordsAndColor[i*6]   = static_cast<float>(cir.x) + (static_cast<float>(cir.r)*scale) * body_vertices[i].getXComp();
		coordsAndColor[i*6+1] = static_cast<float>(cir.y) + (static_cast<float>(cir.r)*scale) * body_vertices[i].getYComp();
		coordsAndColor[i*6+2] = color.getRf();
		coordsAndColor[i*6+3] = color.getGf();
		coordsAndColor[i*6+4] = color.getBf();
		coordsAndColor[i*6+5] = color.getAf();
	}

	Renderer::SubmitBatchedDraw(coordsAndColor, (Circle::NumOfSides+1)*(2+4), body_indices, Circle::NumOfSides*3);

	drawBackgroundOutline(alpha);
}

void CircularLightningHazard::drawBackgroundOutline(float alpha) const {
	//alpha set by drawBackground()

	ColorValueHolder color_outline = ColorValueHolder(0.5f, 0.5f, 0.5f); //use gray; black is a bit too strong for a lightning's outline
	color_outline = ColorMixer::mix(BackgroundRect::getBackColor(), color_outline, alpha);
	const float lineWidth = 0.5f;
	//using the same color for the background works well, though it's not used because the outline was added to make the lightning's boundary obvious

	float coordsAndColor_outline[(Circle::NumOfSides*2)*(2+4)];
	for (int i = 0; i < Circle::NumOfSides; i++) {
		coordsAndColor_outline[(i*2)  *6]   = static_cast<float>(cir.x) + (static_cast<float>(cir.r)-lineWidth) * body_vertices[i+1].getXComp();
		coordsAndColor_outline[(i*2)  *6+1] = static_cast<float>(cir.y) + (static_cast<float>(cir.r)-lineWidth) * body_vertices[i+1].getYComp();
		coordsAndColor_outline[(i*2+1)*6]   = static_cast<float>(cir.x) + (static_cast<float>(cir.r)+lineWidth) * body_vertices[i+1].getXComp();
		coordsAndColor_outline[(i*2+1)*6+1] = static_cast<float>(cir.y) + (static_cast<float>(cir.r)+lineWidth) * body_vertices[i+1].getYComp();

		coordsAndColor_outline[(i*2)  *6+2] = color_outline.getRf();
		coordsAndColor_outline[(i*2)  *6+3] = color_outline.getGf();
		coordsAndColor_outline[(i*2)  *6+4] = color_outline.getBf();
		coordsAndColor_outline[(i*2)  *6+5] = color_outline.getAf();
		coordsAndColor_outline[(i*2+1)*6+2] = color_outline.getRf();
		coordsAndColor_outline[(i*2+1)*6+3] = color_outline.getGf();
		coordsAndColor_outline[(i*2+1)*6+4] = color_outline.getBf();
		coordsAndColor_outline[(i*2+1)*6+5] = color_outline.getAf();
	}

	Renderer::SubmitBatchedDraw(coordsAndColor_outline, (Circle::NumOfSides*2)*(2+4), outline_indices, Circle::NumOfSides*6);
}

void CircularLightningHazard::drawBolts(float alpha) const {
	if (!currentlyActive) {
		return;
	}

	ColorValueHolder color = getBoltColor();
	color = ColorMixer::mix(BackgroundRect::getBackColor(), color, alpha);
	const float lineWidth = 0.75f;

	for (int i = 0; i < bolts.size(); i++) {
		float* coordsAndColor = new float[(bolts[i]->length-1)*4*(2+4)];
		unsigned int* indices = new unsigned int[(bolts[i]->length-1)*6];

		for (int j = 0; j < bolts[i]->length-1; j++) {
			const int startVertex = (j*4) * 6;
			const int startIndex = j*6;

			SimpleVector2D dist = SimpleVector2D(bolts[i]->positions[(j+1)*2] - bolts[i]->positions[j*2], bolts[i]->positions[(j+1)*2+1] - bolts[i]->positions[j*2+1]);
			SimpleVector2D distCW = SimpleVector2D(dist.getAngle() - float(PI/2), lineWidth, true);

			coordsAndColor[startVertex + 0*6]   = static_cast<float>(cir.x) + bolts[i]->positions[j*2]                     + distCW.getXComp();
			coordsAndColor[startVertex + 0*6+1] = static_cast<float>(cir.y) + bolts[i]->positions[j*2+1]                   + distCW.getYComp();
			coordsAndColor[startVertex + 1*6]   = static_cast<float>(cir.x) + bolts[i]->positions[j*2]   + dist.getXComp() + distCW.getXComp();
			coordsAndColor[startVertex + 1*6+1] = static_cast<float>(cir.y) + bolts[i]->positions[j*2+1] + dist.getYComp() + distCW.getYComp();
			coordsAndColor[startVertex + 2*6]   = static_cast<float>(cir.x) + bolts[i]->positions[j*2]   + dist.getXComp() - distCW.getXComp();
			coordsAndColor[startVertex + 2*6+1] = static_cast<float>(cir.y) + bolts[i]->positions[j*2+1] + dist.getYComp() - distCW.getYComp();
			coordsAndColor[startVertex + 3*6]   = static_cast<float>(cir.x) + bolts[i]->positions[j*2]                     - distCW.getXComp();
			coordsAndColor[startVertex + 3*6+1] = static_cast<float>(cir.y) + bolts[i]->positions[j*2+1]                   - distCW.getYComp();

			for (int k = 0; k < 4; k++) {
				coordsAndColor[startVertex + k*6+2] = color.getRf();
				coordsAndColor[startVertex + k*6+3] = color.getGf();
				coordsAndColor[startVertex + k*6+4] = color.getBf();
				coordsAndColor[startVertex + k*6+5] = color.getAf();
			}

			indices[startIndex + 0] = startVertex/6 + 0;
			indices[startIndex + 1] = startVertex/6 + 1;
			indices[startIndex + 2] = startVertex/6 + 2;
			indices[startIndex + 3] = startVertex/6 + 2;
			indices[startIndex + 4] = startVertex/6 + 3;
			indices[startIndex + 5] = startVertex/6 + 0;
		}

		Renderer::SubmitBatchedDraw(coordsAndColor, (bolts[i]->length-1)*4*(2+4), indices, (bolts[i]->length-1)*6);
		delete[] coordsAndColor; delete[] indices;
	}
}

void CircularLightningHazard::drawBolts_Pose(float alpha) const {
	//generate bolts
	std::vector<LightningBolt*> poseBolts;
	for (int i = 0; i < 4; i++) {
		//from pushDefaultBolt(), mostly
		float dist = cir.r * .75, angle = PI/4 + i*(PI/2);
		float xEnd = dist*std::cos(angle), yEnd = dist*std::sin(angle);
		LightningBolt* l = new LightningBolt(0, 0, xEnd, yEnd, getDefaultNumBoltPoints(std::sqrt(xEnd*xEnd + yEnd*yEnd)));

		refreshBolt(l);
		poseBolts.push_back(l);
	}

	//draw
	ColorValueHolder color = getBoltColor();
	color = ColorMixer::mix(BackgroundRect::getBackColor(), color, alpha);
	const float lineWidth = 0.75f;

	for (int i = 0; i < poseBolts.size(); i++) {
		//match with drawBolts()
		float* coordsAndColor = new float[(poseBolts[i]->length-1)*4*(2+4)];
		unsigned int* indices = new unsigned int[(poseBolts[i]->length-1)*6];

		for (int j = 0; j < poseBolts[i]->length-1; j++) {
			const int startVertex = (j*4) * 6;
			const int startIndex = j*6;

			SimpleVector2D dist = SimpleVector2D(poseBolts[i]->positions[(j+1)*2] - poseBolts[i]->positions[j*2], poseBolts[i]->positions[(j+1)*2+1] - poseBolts[i]->positions[j*2+1]);
			SimpleVector2D distCW = SimpleVector2D(dist.getAngle() - float(PI/2), lineWidth, true);

			coordsAndColor[startVertex + 0*6]   = static_cast<float>(cir.x) + poseBolts[i]->positions[j*2]                     + distCW.getXComp();
			coordsAndColor[startVertex + 0*6+1] = static_cast<float>(cir.y) + poseBolts[i]->positions[j*2+1]                   + distCW.getYComp();
			coordsAndColor[startVertex + 1*6]   = static_cast<float>(cir.x) + poseBolts[i]->positions[j*2]   + dist.getXComp() + distCW.getXComp();
			coordsAndColor[startVertex + 1*6+1] = static_cast<float>(cir.y) + poseBolts[i]->positions[j*2+1] + dist.getYComp() + distCW.getYComp();
			coordsAndColor[startVertex + 2*6]   = static_cast<float>(cir.x) + poseBolts[i]->positions[j*2]   + dist.getXComp() - distCW.getXComp();
			coordsAndColor[startVertex + 2*6+1] = static_cast<float>(cir.y) + poseBolts[i]->positions[j*2+1] + dist.getYComp() - distCW.getYComp();
			coordsAndColor[startVertex + 3*6]   = static_cast<float>(cir.x) + poseBolts[i]->positions[j*2]                     - distCW.getXComp();
			coordsAndColor[startVertex + 3*6+1] = static_cast<float>(cir.y) + poseBolts[i]->positions[j*2+1]                   - distCW.getYComp();

			for (int k = 0; k < 4; k++) {
				coordsAndColor[startVertex + k*6+2] = color.getRf();
				coordsAndColor[startVertex + k*6+3] = color.getGf();
				coordsAndColor[startVertex + k*6+4] = color.getBf();
				coordsAndColor[startVertex + k*6+5] = color.getAf();
			}

			indices[startIndex + 0] = startVertex/6 + 0;
			indices[startIndex + 1] = startVertex/6 + 1;
			indices[startIndex + 2] = startVertex/6 + 2;
			indices[startIndex + 3] = startVertex/6 + 2;
			indices[startIndex + 4] = startVertex/6 + 3;
			indices[startIndex + 5] = startVertex/6 + 0;
		}

		Renderer::SubmitBatchedDraw(coordsAndColor, (poseBolts[i]->length-1)*4*(2+4), indices, (poseBolts[i]->length-1)*6);
		delete[] coordsAndColor; delete[] indices;
	}

	for (int i = 0; i < poseBolts.size(); i++) {
		delete poseBolts[i];
	}
}

CircleHazard* CircularLightningHazard::randomizingFactory(double x_start, double y_start, double area_width, double area_height, const GenericFactoryConstructionData& args) {
	int attempts = 0;
	CircleHazard* randomized = nullptr;
	double xpos, ypos, radius;

	bool randomizeR;
	int count = 0;
	if (args.getDataCount() >= 1) {
		count = args.getDataPortionLength(0);
	}
	if (count >= 1) {
		const double* arr = static_cast<const double*>(args.getDataPortion(0).get());
		radius = arr[0];
		randomizeR = false;
	} else {
		randomizeR = true;
	}

	do {
		if (randomizeR) {
			radius = LevelRNG::randNumInRange(30, 60); //TODO: where should these constants be?
		}
		xpos = LevelRNG::randNumInRange(x_start + radius, x_start + area_width - radius);
		ypos = LevelRNG::randNumInRange(y_start + radius, y_start + area_height - radius);
		CircleHazard* testCircularLightning = new CircularLightningHazard(xpos, ypos, radius);
		if (testCircularLightning->reasonableLocation()) {
			randomized = testCircularLightning;
			break;
		} else {
			delete testCircularLightning;
		}
		attempts++;
	} while (attempts < 32);

	return randomized;
}
