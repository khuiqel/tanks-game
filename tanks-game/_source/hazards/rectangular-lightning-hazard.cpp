#include "rectangular-lightning-hazard.h"

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
#include "../simple-vector-2d.h"
#include "../wall-manager.h"
#include "../hazard-manager.h"

std::unordered_map<std::string, float> RectangularLightningHazard::getWeights() const {
	std::unordered_map<std::string, float> weights;
	weights.insert({ "vanilla", 1.0f });
	weights.insert({ "random-vanilla", 1.0f });
	weights.insert({ "old", 1.0f });
	weights.insert({ "random-old", 1.0f });
	weights.insert({ "random", 1.0f });
	return weights;
}

RectangularLightningHazard::RectangularLightningHazard(double xpos, double ypos, double width, double height) : RectHazard(HAZARD_TEAM) {
	rec.x = xpos;
	rec.y = ypos;
	rec.w = width;
	rec.h = height;

	tickCount = 0;
	tickCycle = 100; //100 is JS default (because of power speed)
	constexpr double temp[2] = { 2, 2 };
	std::copy(temp, temp+2, stateMultiplier);
	currentlyActive = false;

	maxBolts = 1;
	lengthOfBolt = 4; //JS: 2
	boltTick = 0;
	boltCycle = 4; //JS: none
	boltsNeeded = false;

	//canAcceptPowers = false;
}

RectangularLightningHazard::~RectangularLightningHazard() {
	//nothing
}

RectHazard* RectangularLightningHazard::factory(const GenericFactoryConstructionData& args) {
	if (args.getDataCount() >= 1) [[likely]] {
		int count = args.getDataPortionLength(0);

		if (count >= 4) [[likely]] {
			const double* arr = static_cast<const double*>(args.getDataPortion(0).get());
			double x = arr[0];
			double y = arr[1];
			double w = arr[2];
			double h = arr[3];
			return new RectangularLightningHazard(x, y, w, h);
		}
	}

	return new RectangularLightningHazard(0, 0, 0, 0);
}

void RectangularLightningHazard::specialEffectCircleCollision(const Circle& other) {
	const Circle centerPoint = getCenterPoint();
	double intersectionX, intersectionY;
	int boltPoints;
	if (CollisionHandler::fullyCollided(centerPoint, other)) {
		intersectionX = other.x;
		intersectionY = other.y;
		boltPoints = 2;
	} else {
		//if the circle's center isn't fully in the rectangle area, then the endpoint may not be inside the rectangle
		const Circle circleCenter = Circle(other.x, other.y, 0);
		if (CollisionHandler::fullyCollided(circleCenter, this->rec)) {
			//circle center inside rectangle area
			SimpleVector2D distToCircle = SimpleVector2D(other.x - centerPoint.x, other.y - centerPoint.y);
			distToCircle.changeMagnitude(-1 * other.r);
			intersectionX = centerPoint.x + distToCircle.getXComp();
			intersectionY = centerPoint.y + distToCircle.getYComp();
			boltPoints = getDefaultNumBoltPoints(distToCircle.getMagnitude());
		} else {
			//circle center outside rectangle area
			double xpos, ypos;
			if (other.x < rec.x) {
				xpos = rec.x; //circle too far to the left
			} else if (other.x > rec.x+rec.w) {
				xpos = rec.x+rec.w; //circle too far to the right
			} else {
				xpos = other.x; //circle's x-position is fine
			}
			if (other.y < rec.y) {
				ypos = rec.y; //circle too low
			} else if (other.y > rec.y+rec.h) {
				ypos = rec.y+rec.h; //circle too high
			} else {
				ypos = other.y; //circle's y-position is fine
			}

			std::pair<PositionHolder, PositionHolder> intersections = CollisionHandler::circleLineIntersection(other, xpos, ypos, centerPoint.x, centerPoint.y);
			if (other.x < rec.x + rec.w/2) {
				intersectionX = std::max(intersections.first.first, intersections.second.first);
			} else {
				intersectionX = std::min(intersections.first.first, intersections.second.first);
			}
			if (other.y < rec.y + rec.h/2) {
				intersectionY = std::max(intersections.first.second, intersections.second.second);
			} else {
				intersectionY = std::min(intersections.first.second, intersections.second.second);
			}

			if (intersectionX < rec.x || intersectionX > rec.x+rec.w) [[unlikely]] {
				std::cerr << "WARNING: rectangular lightning endpoint X out of range!\n";
				intersectionX = std::clamp<double>(intersectionX, rec.x, rec.x+rec.w);
			}
			if (intersectionY < rec.y || intersectionY > rec.y+rec.h) [[unlikely]] {
				std::cerr << "WARNING: rectangular lightning endpoint Y out of range!\n";
				intersectionY = std::clamp<double>(intersectionY, rec.y, rec.y+rec.h);
			}
			boltPoints = getDefaultNumBoltPoints(std::sqrt((intersectionX - centerPoint.x)*(intersectionX - centerPoint.x) + (intersectionY - centerPoint.y)*(intersectionY - centerPoint.y)));
		}
	}

	pushBolt(new LightningBolt(rec.w/2, rec.h/2, intersectionX - rec.x, intersectionY - rec.y, boltPoints));
}

void RectangularLightningHazard::specialEffectTankCollision(const Tank* t) {
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

void RectangularLightningHazard::specialEffectBulletCollision(const Bullet* b) {
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

void RectangularLightningHazard::pushBolt(LightningBolt* l) {
	bolts.push_back(l);
	refreshBolt(l);
}

void RectangularLightningHazard::pushDefaultBolt(int num, bool randomize) {
	//the default bolt is from center to a random point
	for (int i = 0; i < num; i++) {
		float xEnd = rec.w*VisualRNG::randFunc(), yEnd = rec.h*VisualRNG::randFunc();
		float xDelta = xEnd - static_cast<float>(rec.w)/2, yDelta = yEnd - static_cast<float>(rec.h)/2;
		LightningBolt* l = new LightningBolt(rec.w/2, rec.h/2, xEnd, yEnd, getDefaultNumBoltPoints(std::sqrt(xDelta*xDelta + yDelta*yDelta)));
		if (randomize) {
			pushBolt(l);
		} else {
			bolts.push_back(l);
		}
	}
}

bool RectangularLightningHazard::validLocation() const {
	for (int i = 0; i < WallManager::getNumWalls(); i++) {
		const Wall* wa = WallManager::getWall(i);
		if (CollisionHandler::partiallyCollidedIgnoreEdge(wa->rec, this->rec)) {
			return false;
		}
	}
	return true;
}

bool RectangularLightningHazard::reasonableLocation() const {
	for (int i = 0; i < HazardManager::getNumCircleHazards(); i++) {
		const CircleHazard* ch = HazardManager::getCircleHazard(i);
		if (ch->getName() != this->getName()) {
			if (CollisionHandler::partiallyCollided(this->rec, ch->cir)) {
				return false;
			}
		}
	}
	for (int i = 0; i < HazardManager::getNumRectHazards(); i++) {
		const RectHazard* rh = HazardManager::getRectHazard(i);
		if (rh->getName() != this->getName()) {
			if (CollisionHandler::partiallyCollided(this->rec, rh->rec)) {
				return false;
			}
		}
	}

	return validLocation();
}

void RectangularLightningHazard::refreshBolt(LightningBolt* l) const {
	refreshBolt(l, std::min(rec.w, rec.h), std::max(rec.w, rec.h));
}

void RectangularLightningHazard::refreshBolt(LightningBolt* l, double smaller, double larger) const {
	if (l->length <= 2) {
		return;
	}

	const float boltDeltaX = l->positions[l->length*2-2] - l->positions[0];
	const float boltDeltaY = l->positions[l->length*2-1] - l->positions[1];
	const SimpleVector2D boltVec = SimpleVector2D(boltDeltaX, boltDeltaY);
	const float sinAngle = std::sin(boltVec.getAngle());
	const float cosAngle = std::cos(boltVec.getAngle()); //to avoid recalculating each time (though it would probably get optimized out)
	const float newH = boltVec.getMagnitude() * static_cast<float>(smaller/larger);
	const float maxVariance = (1.0f/4.0f) * boltVec.getMagnitude() * static_cast<float>(smaller/larger);

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
		} while ((testX < 0 || testX > static_cast<float>(rec.w) || testY < 0 || testY > static_cast<float>(rec.h)) || !pointInPolygon(6, polygonX, polygonY, testX, testY));
		l->positions[j*2]   = testX;
		l->positions[j*2+1] = testY;
	}
}

void RectangularLightningHazard::draw() const {
	drawBackground(false);
	drawBolts();
}

void RectangularLightningHazard::draw(DrawingLayers layer) const {
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

void RectangularLightningHazard::poseDraw() const {
	drawBackground(true);
	drawBolts_Pose();
}

void RectangularLightningHazard::poseDraw(DrawingLayers layer) const {
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

void RectangularLightningHazard::ghostDraw(float alpha) const {
	drawBackground(true, alpha);
	drawBolts_Pose(alpha);
}

void RectangularLightningHazard::ghostDraw(DrawingLayers layer, float alpha) const {
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

void RectangularLightningHazard::drawBackground(bool pose, float alpha) const {
	//ColorValueHolder color = (pose ? getBackgroundColor_Pose() : getBackgroundColor());
	ColorValueHolder color = getBackgroundColor_Pose();
	color = ColorMixer::mix(BackgroundRect::getBackColor(), color, alpha);

	double scale;
	if (pose || currentlyActive) {
		scale = 1.0;
	} else {
		scale = tickCount / (tickCycle * stateMultiplier[currentlyActive]);
	}
	scale = scale * scale;

	float coordsAndColor[] = {
		rec.x+(rec.w/2) - (rec.w/2)*scale, rec.y+(rec.h/2) - (rec.h/2)*scale,   color.getRf(), color.getGf(), color.getBf(), color.getAf(),
		rec.x+(rec.w/2) + (rec.w/2)*scale, rec.y+(rec.h/2) - (rec.h/2)*scale,   color.getRf(), color.getGf(), color.getBf(), color.getAf(),
		rec.x+(rec.w/2) + (rec.w/2)*scale, rec.y+(rec.h/2) + (rec.h/2)*scale,   color.getRf(), color.getGf(), color.getBf(), color.getAf(),
		rec.x+(rec.w/2) - (rec.w/2)*scale, rec.y+(rec.h/2) + (rec.h/2)*scale,   color.getRf(), color.getGf(), color.getBf(), color.getAf()
	};
	unsigned int indices[] = {
		0, 1, 2,
		2, 3, 0
	};

	Renderer::SubmitBatchedDraw(coordsAndColor, 4 * (2+4), indices, 2 * 3);

	drawBackgroundOutline(alpha);
}

void RectangularLightningHazard::drawBackgroundOutline(float alpha) const {
	//alpha set by drawBackground()

	ColorValueHolder color_outline = ColorValueHolder(0.5f, 0.5f, 0.5f); //use gray; black is a bit too strong for a lightning's outline
	color_outline = ColorMixer::mix(BackgroundRect::getBackColor(), color_outline, alpha);
	const float lineWidth = 0.5f;
	//using the same color for the background works well, though it's not used because the outline was added to make the lightning's boundary obvious

	float coordsAndColor_outline[] = {
		//outer
		(rec.x)       - lineWidth, (rec.y)       - lineWidth,   color_outline.getRf(), color_outline.getGf(), color_outline.getBf(), color_outline.getAf(), //0
		(rec.x+rec.w) + lineWidth, (rec.y)       - lineWidth,   color_outline.getRf(), color_outline.getGf(), color_outline.getBf(), color_outline.getAf(), //1
		(rec.x+rec.w) + lineWidth, (rec.y+rec.h) + lineWidth,   color_outline.getRf(), color_outline.getGf(), color_outline.getBf(), color_outline.getAf(), //2
		(rec.x)       - lineWidth, (rec.y+rec.h) + lineWidth,   color_outline.getRf(), color_outline.getGf(), color_outline.getBf(), color_outline.getAf(), //3

		//inner
		(rec.x)       + lineWidth, (rec.y)       + lineWidth,   color_outline.getRf(), color_outline.getGf(), color_outline.getBf(), color_outline.getAf(), //4
		(rec.x+rec.w) - lineWidth, (rec.y)       + lineWidth,   color_outline.getRf(), color_outline.getGf(), color_outline.getBf(), color_outline.getAf(), //5
		(rec.x+rec.w) - lineWidth, (rec.y+rec.h) - lineWidth,   color_outline.getRf(), color_outline.getGf(), color_outline.getBf(), color_outline.getAf(), //6
		(rec.x)       + lineWidth, (rec.y+rec.h) - lineWidth,   color_outline.getRf(), color_outline.getGf(), color_outline.getBf(), color_outline.getAf()  //7
	};
	unsigned int indices_outline[] = { //trapezoids
		//bottom
		0, 1, 5,
		5, 4, 0,

		//right
		1, 2, 6,
		6, 5, 1,

		//left
		4, 7, 3,
		3, 0, 4,

		//top
		2, 3, 7,
		7, 6, 2
	};

	Renderer::SubmitBatchedDraw(coordsAndColor_outline, (4+4) * (2+4), indices_outline, (4*2) * 3);
}

void RectangularLightningHazard::drawBolts(float alpha) const {
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

			coordsAndColor[startVertex + 0*6]   = static_cast<float>(rec.x) + bolts[i]->positions[j*2]                     + distCW.getXComp();
			coordsAndColor[startVertex + 0*6+1] = static_cast<float>(rec.y) + bolts[i]->positions[j*2+1]                   + distCW.getYComp();
			coordsAndColor[startVertex + 1*6]   = static_cast<float>(rec.x) + bolts[i]->positions[j*2]   + dist.getXComp() + distCW.getXComp();
			coordsAndColor[startVertex + 1*6+1] = static_cast<float>(rec.y) + bolts[i]->positions[j*2+1] + dist.getYComp() + distCW.getYComp();
			coordsAndColor[startVertex + 2*6]   = static_cast<float>(rec.x) + bolts[i]->positions[j*2]   + dist.getXComp() - distCW.getXComp();
			coordsAndColor[startVertex + 2*6+1] = static_cast<float>(rec.y) + bolts[i]->positions[j*2+1] + dist.getYComp() - distCW.getYComp();
			coordsAndColor[startVertex + 3*6]   = static_cast<float>(rec.x) + bolts[i]->positions[j*2]                     - distCW.getXComp();
			coordsAndColor[startVertex + 3*6+1] = static_cast<float>(rec.y) + bolts[i]->positions[j*2+1]                   - distCW.getYComp();

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

void RectangularLightningHazard::drawBolts_Pose(float alpha) const {
	//generate bolts
	std::vector<LightningBolt*> poseBolts;
	for (int i = 0; i < 4; i++) {
		//from pushDefaultBolt(), mostly
		float xEnd = (i%2==0 ? -1 : 1) * (rec.w*.375), yEnd = (i/2==0 ? -1 : 1) * (rec.h*.375);
		float xDelta = xEnd - static_cast<float>(rec.w)/2, yDelta = yEnd - static_cast<float>(rec.h)/2;
		LightningBolt* l = new LightningBolt(rec.w/2, rec.h/2, xEnd, yEnd, getDefaultNumBoltPoints(std::sqrt(xDelta*xDelta + yDelta*yDelta)));

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

			coordsAndColor[startVertex + 0*6]   = static_cast<float>(rec.x) + poseBolts[i]->positions[j*2]                     + distCW.getXComp();
			coordsAndColor[startVertex + 0*6+1] = static_cast<float>(rec.y) + poseBolts[i]->positions[j*2+1]                   + distCW.getYComp();
			coordsAndColor[startVertex + 1*6]   = static_cast<float>(rec.x) + poseBolts[i]->positions[j*2]   + dist.getXComp() + distCW.getXComp();
			coordsAndColor[startVertex + 1*6+1] = static_cast<float>(rec.y) + poseBolts[i]->positions[j*2+1] + dist.getYComp() + distCW.getYComp();
			coordsAndColor[startVertex + 2*6]   = static_cast<float>(rec.x) + poseBolts[i]->positions[j*2]   + dist.getXComp() - distCW.getXComp();
			coordsAndColor[startVertex + 2*6+1] = static_cast<float>(rec.y) + poseBolts[i]->positions[j*2+1] + dist.getYComp() - distCW.getYComp();
			coordsAndColor[startVertex + 3*6]   = static_cast<float>(rec.x) + poseBolts[i]->positions[j*2]                     - distCW.getXComp();
			coordsAndColor[startVertex + 3*6+1] = static_cast<float>(rec.y) + poseBolts[i]->positions[j*2+1]                   - distCW.getYComp();

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

RectHazard* RectangularLightningHazard::randomizingFactory(double x_start, double y_start, double area_width, double area_height, const GenericFactoryConstructionData& args) {
	int attempts = 0;
	RectHazard* randomized = nullptr;
	double xpos, ypos, width, height;

	bool randomizeWH;
	int count = 0;
	if (args.getDataCount() >= 1) {
		count = args.getDataPortionLength(0);
	}
	if (count >= 2) {
		const double* arr = static_cast<const double*>(args.getDataPortion(0).get());
		width = arr[0];
		height = arr[1];
		randomizeWH = false;
	} else {
		randomizeWH = true;
	}

	do {
		if (randomizeWH) {
			width = LevelRNG::randNumInRange(30, 80); //TODO: where should these constants be?
			height = LevelRNG::randNumInRange(30, 80); //TODO: where should these constants be?
		}
		xpos = LevelRNG::randNumInRange(x_start, x_start + area_width - width);
		ypos = LevelRNG::randNumInRange(y_start, y_start + area_height - height);
		RectHazard* testRectangularLightning = new RectangularLightningHazard(xpos, ypos, width, height);
		if (testRectangularLightning->reasonableLocation()) {
			randomized = testRectangularLightning;
			break;
		} else {
			delete testRectangularLightning;
		}
		attempts++;
	} while (attempts < 32);

	return randomized;
}
