#include "horizontal-lightning-hazard.h"

#include "../constants.h"
#include <cmath>
#include <stdexcept>
#include <algorithm> //std::copy, std::clamp
#include <iostream>
#include "../rng.h"

#include "../renderer.h"
#include "../color-mixer.h"
#include "../background-rect.h"

#include "../collision-handler.h"
#include "../simple-vector-2d.h"
#include "../wall-manager.h"
#include "../hazard-manager.h"

std::unordered_map<std::string, float> HorizontalLightningHazard::getWeights() const {
	std::unordered_map<std::string, float> weights;
	weights.insert({ "vanilla", 1.0f });
	weights.insert({ "random-vanilla", 1.0f });
	weights.insert({ "old", 1.0f });
	weights.insert({ "random-old", 1.0f });
	weights.insert({ "random", 1.0f });
	return weights;
}

HorizontalLightningHazard::HorizontalLightningHazard(double xpos, double ypos, double width, double height) : RectangularLightningHazard(xpos,ypos,width,height) {
	//flexible = false;

	maxBolts = 2;
	//lengthOfBolt = 4;

	//canAcceptPowers = false;
}

HorizontalLightningHazard::~HorizontalLightningHazard() {
	//nothing
}

RectHazard* HorizontalLightningHazard::factory(const GenericFactoryConstructionData& args) {
	if (args.getDataCount() >= 1) [[likely]] {
		int count = args.getDataPortionLength(0);

		if (count >= 4) [[likely]] {
			const double* arr = static_cast<const double*>(args.getDataPortion(0).get());
			double x = arr[0];
			double y = arr[1];
			double w = arr[2];
			double h = arr[3];
			return new HorizontalLightningHazard(x, y, w, h);
		}
	}

	return new HorizontalLightningHazard(0, 0, 0, 0);
}

void HorizontalLightningHazard::specialEffectCircleCollision(const Circle& other) {
	const Circle leftPoint = getLeftPoint();
	const Circle rightPoint = getRightPoint();
	const Circle circleCenter = Circle(other.x, other.y, 0);
	double intersectionXL, intersectionXR, intersectionYL, intersectionYR;
	int boltPointsL, boltPointsR;

	double xpos, ypos; //circle adjusted x and y

	if (CollisionHandler::fullyCollided(circleCenter, this->rec)) {
		//circle center inside rectangle area
		xpos = other.x;
		ypos = other.y;
	} else {
		//circle center outside rectangle area
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
	}

	if (CollisionHandler::fullyCollided(leftPoint, other)) {
		intersectionXL = other.x;
		intersectionYL = other.y;
		boltPointsL = 2;
	} else {
		std::pair<PositionHolder, PositionHolder> intersections = CollisionHandler::circleLineIntersection(other, xpos, ypos, leftPoint.x, leftPoint.y);
		intersectionXL = std::min(intersections.first.first, intersections.second.first);
		if (other.y < rec.y + rec.h/2) {
			intersectionYL = std::max(intersections.first.second, intersections.second.second);
		} else {
			intersectionYL = std::min(intersections.first.second, intersections.second.second);
		}

		if (intersectionXL < rec.x || intersectionXL > rec.x+rec.w) [[unlikely]] {
			std::cerr << "WARNING: horizontal lightning endpoint X (left half) out of range!\n";
			intersectionXL = std::clamp<double>(intersectionXL, rec.x, rec.x+rec.w);
		}
		if (intersectionYL < rec.y || intersectionYL > rec.y+rec.h) [[unlikely]] {
			std::cerr << "WARNING: horizontal lightning endpoint Y (left half) out of range!\n";
			intersectionYL = std::clamp<double>(intersectionYL, rec.y, rec.y+rec.h);
		}
		boltPointsL = getDefaultNumBoltPoints(std::sqrt((intersectionXL - leftPoint.x)*(intersectionXL - leftPoint.x) + (intersectionYL - leftPoint.y)*(intersectionYL - leftPoint.y)));
	}

	if (CollisionHandler::fullyCollided(rightPoint, other)) {
		intersectionXR = other.x;
		intersectionYR = other.y;
		boltPointsR = 2;
	} else {
		std::pair<PositionHolder, PositionHolder> intersections = CollisionHandler::circleLineIntersection(other, xpos, ypos, rightPoint.x, rightPoint.y);
		intersectionXR = std::max(intersections.first.first, intersections.second.first);
		if (other.y < rec.y + rec.h/2) {
			intersectionYR = std::max(intersections.first.second, intersections.second.second);
		} else {
			intersectionYR = std::min(intersections.first.second, intersections.second.second);
		}

		if (intersectionXR < rec.x || intersectionXR > rec.x+rec.w) [[unlikely]] {
			std::cerr << "WARNING: horizontal lightning endpoint X (right half) out of range!\n";
			intersectionXR = std::clamp<double>(intersectionXR, rec.x, rec.x+rec.w);
		}
		if (intersectionYR < rec.y || intersectionYR > rec.y+rec.h) [[unlikely]] {
			std::cerr << "WARNING: horizontal lightning endpoint Y (right half) out of range!\n";
			intersectionYR = std::clamp<double>(intersectionYR, rec.y, rec.y+rec.h);
		}
		boltPointsR = getDefaultNumBoltPoints(std::sqrt((intersectionXR - rightPoint.x)*(intersectionXR - rightPoint.x) + (intersectionYR - rightPoint.y)*(intersectionYR - rightPoint.y)));
	}

	pushBolt(new LightningBolt(0, rec.h/2, intersectionXL-rec.x, intersectionYL-rec.y, boltPointsL), false);
	pushBolt(new LightningBolt(intersectionXL-rec.x, intersectionYL-rec.y, intersectionXR-rec.x, intersectionYR-rec.y, 2), false);
	pushBolt(new LightningBolt(intersectionXR-rec.x, intersectionYR-rec.y, rec.w, rec.h/2, boltPointsR), false);
}

void HorizontalLightningHazard::pushBolt(LightningBolt* l, bool simpleRefresh) {
	bolts.push_back(l);
	if (simpleRefresh) {
		simpleRefreshBolt(l);
	} else {
		refreshBolt(l);
	}
}

void HorizontalLightningHazard::pushDefaultBolt(int num, bool randomize) {
	for (int i = 0; i < num; i++) {
		LightningBolt* l = new LightningBolt(0, rec.h/2, rec.w, rec.h/2, getDefaultNumBoltPoints(rec.w));
		if (randomize) {
			pushBolt(l, true);
		} else {
			bolts.push_back(l);
		}
	}
}

bool HorizontalLightningHazard::validLocation() const {
	bool wallOnLeft = false, wallOnRight = false;
	for (int i = 0; i < WallManager::getNumWalls(); i++) {
		const Wall* wa = WallManager::getWall(i);
		if (CollisionHandler::partiallyCollidedIgnoreEdge(wa->rec, this->rec)) {
			return false;
		}
		if ((wa->rec.y <= rec.y) && (wa->rec.y + wa->rec.h >= rec.y + rec.h)) {
			if (rec.x == wa->rec.x + wa->rec.w) {
				wallOnLeft = true;
			} else if (rec.x + rec.w == wa->rec.x) {
				wallOnRight = true;
			}
		}
	}
	if (rec.x == 0) {
		wallOnLeft = true;
	}
	if (rec.x + rec.w == GAME_WIDTH) {
		wallOnRight = true;
	}
	return (wallOnLeft && wallOnRight);
}

bool HorizontalLightningHazard::reasonableLocation() const {
	bool wallOnBottom = false, wallOnTop = false;
	for (int i = 0; i < WallManager::getNumWalls(); i++) {
		const Wall* wa = WallManager::getWall(i);
		if ((wa->rec.x <= rec.x) && (wa->rec.x + wa->rec.w >= rec.x + rec.w)) {
			if (rec.y == wa->rec.y + wa->rec.h) {
				wallOnBottom = true;
			} else if (rec.y + rec.h == wa->rec.y) {
				wallOnTop = true;
			}
		}
		if (wallOnBottom && wallOnTop) {
			break;
		}
	}
	if (rec.y <= 0) { //should this be ==? //also, should this function return false if the lightning is out of bounds?
		wallOnBottom = true;
	}
	if (rec.y + rec.h >= GAME_HEIGHT) {
		wallOnTop = true;
	}

	if (wallOnBottom && wallOnTop) {
		return false;
	}

	//don't check general lightnings
	for (int i = 0; i < HazardManager::getNumCircleHazards(); i++) {
		if (CollisionHandler::partiallyCollided(this->rec, HazardManager::getCircleHazard(i)->cir)) {
			return false;
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

void HorizontalLightningHazard::simpleRefreshBolt(LightningBolt* l) const {
	const float maxVariance = rec.h/4;
	/* lightning bolts are allowed to be in an area that looks like this:
	 * 
	 * +   --------------   +        <- h
	 * |  /              \  |
	 * | /                \ |
	 * |<       HERE       >|
	 * | \                / |
	 * |  \              /  |
	 * +   --------------   +        <- 0
	 * 
	 * ^                    ^
	 * 0                    w
	 * 
	 * the region is 1/4 triangle, 1/2 rectangle, then 1/4 triangle
	 */

	const float deltaX = (l->positions[l->length*2-2] - l->positions[0]) / (l->length - 1);
	for (int j = 1; j < l->length-1; j++) {
		float yRangeLower = l->positions[j*2 - 1] - maxVariance;
		float yRangeUpper = l->positions[j*2 - 1] + maxVariance;
		float yMin, yMax;
		if (j < l->length/4) { //first quarter
			yMin = -2*static_cast<float>(rec.h/rec.w) * (deltaX * j) + static_cast<float>(rec.h)/2;
			yMax =  2*static_cast<float>(rec.h/rec.w) * (deltaX * j) + static_cast<float>(rec.h)/2;
		} else if (j < 3 * l->length / 4) { //middle half
			yMin = 0;
			yMax = rec.h;
		} else { //last quarter
			yMin =  2*static_cast<float>(rec.h/rec.w) * (deltaX * (j - 3 * l->length / 4)) + 0;
			yMax = -2*static_cast<float>(rec.h/rec.w) * (deltaX * (j - 3 * l->length / 4)) + static_cast<float>(rec.h);
		}
		yRangeLower = (yRangeLower < yMin ? yMin : yRangeLower);
		yRangeUpper = (yRangeUpper > yMax ? yMax : yRangeUpper);
		l->positions[j*2+1] = VisualRNG::randFloatInRange(yRangeLower, yRangeUpper);
	}
}

void HorizontalLightningHazard::refreshBolt(LightningBolt* l) const {
	RectangularLightningHazard::refreshBolt(l, this->rec.h, this->rec.w);
}

void HorizontalLightningHazard::drawBackground(bool pose, float alpha) const {
	//ColorValueHolder color = (pose ? getBackgroundColor_Pose() : getBackgroundColor());
	ColorValueHolder color = getBackgroundColor_Pose();
	color = ColorMixer::mix(BackgroundRect::getBackColor(), color, alpha);

	double scale;
	if (pose || currentlyActive) {
		scale = 1.0;
	} else {
		scale = tickCount / (tickCycle * stateMultiplier[currentlyActive]);
	}
	//scale = scale * scale;

	float coordsAndColor[] = {
		rec.x+(rec.w/2) - (rec.w/2)*scale, rec.y,         color.getRf(), color.getGf(), color.getBf(), color.getAf(),
		rec.x+(rec.w/2) + (rec.w/2)*scale, rec.y,         color.getRf(), color.getGf(), color.getBf(), color.getAf(),
		rec.x+(rec.w/2) + (rec.w/2)*scale, rec.y+rec.h,   color.getRf(), color.getGf(), color.getBf(), color.getAf(),
		rec.x+(rec.w/2) - (rec.w/2)*scale, rec.y+rec.h,   color.getRf(), color.getGf(), color.getBf(), color.getAf()
	};
	unsigned int indices[] = {
		0, 1, 2,
		2, 3, 0
	};

	Renderer::SubmitBatchedDraw(coordsAndColor, 4 * (2+4), indices, 2 * 3);

	drawBackgroundOutline(alpha);
}

void HorizontalLightningHazard::drawBackgroundOutline(float alpha) const {
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
		(rec.x)       - lineWidth, (rec.y)       + lineWidth,   color_outline.getRf(), color_outline.getGf(), color_outline.getBf(), color_outline.getAf(), //4
		(rec.x+rec.w) + lineWidth, (rec.y)       + lineWidth,   color_outline.getRf(), color_outline.getGf(), color_outline.getBf(), color_outline.getAf(), //5
		(rec.x+rec.w) + lineWidth, (rec.y+rec.h) - lineWidth,   color_outline.getRf(), color_outline.getGf(), color_outline.getBf(), color_outline.getAf(), //6
		(rec.x)       - lineWidth, (rec.y+rec.h) - lineWidth,   color_outline.getRf(), color_outline.getGf(), color_outline.getBf(), color_outline.getAf()  //7
	};
	unsigned int indices_outline[] = { //rects
		//bottom
		0, 1, 5,
		5, 4, 0,

		//top
		2, 3, 7,
		7, 6, 2
	};

	Renderer::SubmitBatchedDraw(coordsAndColor_outline, (4+4) * (2+4), indices_outline, (2*2) * 3);
}

void HorizontalLightningHazard::drawBolts_Pose(float alpha) const {
	//generate bolts
	std::vector<LightningBolt*> poseBolts;
	//from pushDefaultBolt(), mostly
	LightningBolt* l = new LightningBolt(0, rec.h/2, rec.w, rec.h/2, getDefaultNumBoltPoints(rec.w));
	refreshBolt(l);
	poseBolts.push_back(l);

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

RectHazard* HorizontalLightningHazard::randomizingFactory(double x_start, double y_start, double area_width, double area_height, const GenericFactoryConstructionData& args) {
	if (WallManager::getNumWalls() <= 1) [[unlikely]] {
		return nullptr; //don't bother trying to see if a horizontal lightning could go from edge to edge
	}

	int attempts = 0;
	RectHazard* randomized = nullptr;
	double xpos, ypos, width, height;
	const double minWidth = 40, maxWidth = 160;

	do {
		height = LevelRNG::randNumInRange(12, 24);
		int i = LevelRNG::randFunc() * WallManager::getNumWalls();
		const Wall* wa = WallManager::getWall(i);
		xpos = wa->rec.x + wa->rec.w;
		ypos = wa->rec.y + LevelRNG::randFunc() * std::clamp<double>(wa->rec.h - height, 0, wa->rec.h);

		int j, wallAttempts = 0;
		do {
			j = LevelRNG::randFunc() * WallManager::getNumWalls();
			wallAttempts++;
		} while ((wallAttempts < 8) && (j == i));
		if (j != i) [[likely]] {
			const Wall* otherWall = WallManager::getWall(j);
			width = otherWall->rec.x - xpos;
		} else {
			width = 0;
		}

		if ((width >= minWidth) && (width <= maxWidth) && (xpos >= x_start) && (xpos + width <= x_start + area_width) && (ypos >= y_start) && (ypos + height <= y_start + area_height)) {
			RectHazard* testHorizontalLightning = new HorizontalLightningHazard(xpos, ypos, width, height);
			if (testHorizontalLightning->reasonableLocation()) {
				randomized = testHorizontalLightning;
				break;
			} else {
				delete testHorizontalLightning;
			}
		}
		attempts++;
	} while (attempts < 64);

	return randomized;
}
