#include "vertical-lightning-hazard.h"

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

std::unordered_map<std::string, float> VerticalLightningHazard::getWeights() const {
	std::unordered_map<std::string, float> weights;
	weights.insert({ "vanilla", 1.0f });
	weights.insert({ "random-vanilla", 1.0f });
	weights.insert({ "old", 1.0f });
	weights.insert({ "random-old", 1.0f });
	weights.insert({ "random", 1.0f });
	return weights;
}

VerticalLightningHazard::VerticalLightningHazard(double xpos, double ypos, double width, double height) : RectangularLightningHazard(xpos,ypos,width,height) {
	//flexible = false;

	maxBolts = 2;
	//lengthOfBolt = 4;

	//canAcceptPowers = false;
}

VerticalLightningHazard::~VerticalLightningHazard() {
	//nothing
}

RectHazard* VerticalLightningHazard::factory(const GenericFactoryConstructionData& args) {
	if (args.getDataCount() >= 1) [[likely]] {
		int count = args.getDataPortionLength(0);

		if (count >= 4) [[likely]] {
			const double* arr = static_cast<const double*>(args.getDataPortion(0).get());
			double x = arr[0];
			double y = arr[1];
			double w = arr[2];
			double h = arr[3];
			return new VerticalLightningHazard(x, y, w, h);
		}
	}

	return new VerticalLightningHazard(0, 0, 0, 0);
}

void VerticalLightningHazard::specialEffectCircleCollision(const Circle& other) {
	const Circle bottomPoint = getBottomPoint();
	const Circle topPoint = getTopPoint();
	const Circle circleCenter = Circle(other.x, other.y, 0);
	double intersectionXD, intersectionXU, intersectionYD, intersectionYU;
	int boltPointsD, boltPointsU;

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

	if (CollisionHandler::fullyCollided(bottomPoint, other)) {
		intersectionXD = other.x;
		intersectionYD = other.y;
		boltPointsD = 2;
	} else {
		std::pair<PositionHolder, PositionHolder> intersections = CollisionHandler::circleLineIntersection(other, xpos, ypos, bottomPoint.x, bottomPoint.y);
		if (other.x < rec.x + rec.w/2) {
			intersectionXD = std::max(intersections.first.first, intersections.second.first);
		} else {
			intersectionXD = std::min(intersections.first.first, intersections.second.first);
		}
		intersectionYD = std::min(intersections.first.second, intersections.second.second);

		if (intersectionXD < rec.x || intersectionXD > rec.x+rec.w) [[unlikely]] {
			std::cerr << "WARNING: vertical lightning endpoint X (bottom half) out of range!\n";
			intersectionXD = std::clamp<double>(intersectionXD, rec.x, rec.x+rec.w);
		}
		if (intersectionYD < rec.y || intersectionYD > rec.y+rec.h) [[unlikely]] {
			std::cerr << "WARNING: vertical lightning endpoint Y (bottom half) out of range!\n";
			intersectionYD = std::clamp<double>(intersectionYD, rec.y, rec.y+rec.h);
		}
		boltPointsD = getDefaultNumBoltPoints(std::sqrt((intersectionXD - bottomPoint.x)*(intersectionXD - bottomPoint.x) + (intersectionYD - bottomPoint.y)*(intersectionYD - bottomPoint.y)));
	}

	if (CollisionHandler::fullyCollided(topPoint, other)) {
		intersectionXU = other.x;
		intersectionYU = other.y;
		boltPointsU = 2;
	} else {
		std::pair<PositionHolder, PositionHolder> intersections = CollisionHandler::circleLineIntersection(other, xpos, ypos, topPoint.x, topPoint.y);
		if (other.x < rec.x + rec.w/2) {
			intersectionXU = std::max(intersections.first.first, intersections.second.first);
		} else {
			intersectionXU = std::min(intersections.first.first, intersections.second.first);
		}
		intersectionYU = std::max(intersections.first.second, intersections.second.second);

		if (intersectionXU < rec.x || intersectionXU > rec.x+rec.w) [[unlikely]] {
			std::cerr << "WARNING: vertical lightning endpoint X (top half) out of range!\n";
			intersectionXU = std::clamp<double>(intersectionXU, rec.x, rec.x+rec.w);
		}
		if (intersectionYU < rec.y || intersectionYU > rec.y+rec.h) [[unlikely]] {
			std::cerr << "WARNING: vertical lightning endpoint Y (top half) out of range!\n";
			intersectionYU = std::clamp<double>(intersectionYU, rec.y, rec.y+rec.h);
		}
		boltPointsU = getDefaultNumBoltPoints(std::sqrt((intersectionXU - topPoint.x)*(intersectionXU - topPoint.x) + (intersectionYU - topPoint.y)*(intersectionYU - topPoint.y)));
	}

	pushBolt(new LightningBolt(rec.w/2, 0, intersectionXD-rec.x, intersectionYD- rec.y, boltPointsD), false);
	pushBolt(new LightningBolt(intersectionXD-rec.x, intersectionYD-rec.y, intersectionXU-rec.x, intersectionYU-rec.y, 2), false);
	pushBolt(new LightningBolt(intersectionXU-rec.x, intersectionYU-rec.y, rec.w/2, rec.h, boltPointsU), false);
}

void VerticalLightningHazard::pushBolt(LightningBolt* l, bool simpleRefresh) {
	bolts.push_back(l);
	if (simpleRefresh) {
		simpleRefreshBolt(l);
	} else {
		refreshBolt(l);
	}
}

void VerticalLightningHazard::pushDefaultBolt(int num, bool randomize) {
	for (int i = 0; i < num; i++) {
		LightningBolt* l = new LightningBolt(rec.w/2, 0, rec.w/2, rec.h, getDefaultNumBoltPoints(rec.h));
		if (randomize) {
			pushBolt(l, true);
		} else {
			bolts.push_back(l);
		}
	}
}

bool VerticalLightningHazard::validLocation() const {
	bool wallOnBottom = false, wallOnTop = false;
	for (int i = 0; i < WallManager::getNumWalls(); i++) {
		const Wall* wa = WallManager::getWall(i);
		if (CollisionHandler::partiallyCollidedIgnoreEdge(wa->rec, this->rec)) {
			return false;
		}
		if ((wa->rec.x <= rec.x) && (wa->rec.x + wa->rec.w >= rec.x + rec.w)) {
			if (rec.y == wa->rec.y + wa->rec.h) {
				wallOnBottom = true;
			} else if (rec.y + rec.h == wa->rec.y) {
				wallOnTop = true;
			}
		}
	}
	if (rec.y == 0) {
		wallOnBottom = true;
	}
	if (rec.y + rec.h == GAME_HEIGHT) {
		wallOnTop = true;
	}
	return (wallOnBottom && wallOnTop);
}

bool VerticalLightningHazard::reasonableLocation() const {
	bool wallOnLeft = false, wallOnRight = false;
	for (int i = 0; i < WallManager::getNumWalls(); i++) {
		const Wall* wa = WallManager::getWall(i);
		if ((wa->rec.y <= rec.y) && (wa->rec.y + wa->rec.y >= rec.y + rec.h)) {
			if (rec.x == wa->rec.x + wa->rec.w) {
				wallOnLeft = true;
			} else if (rec.x + rec.w == wa->rec.x) {
				wallOnRight = true;
			}
		}
		if (wallOnLeft && wallOnRight) {
			break;
		}
	}
	if (rec.x <= 0) { //should this be ==? //also, should this function return false if the lightning is out of bounds?
		wallOnLeft = true;
	}
	if (rec.x + rec.w >= GAME_WIDTH) {
		wallOnRight = true;
	}

	if (wallOnLeft && wallOnRight) {
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

void VerticalLightningHazard::simpleRefreshBolt(LightningBolt* l) const {
	const float maxVariance = rec.w/4;
	/* lightning bolts are allowed to be in an area that looks like this:
	 * (see HorizontalLightningHazard for a better diagram, then just mentally rotate it)
	 * +-------- /-\ --------+        <- h
	 *         /     \        
	 *      /           \     
	 *   /                 \  
	 * |                     |
	 * |                     |
	 * |                     |
	 *   \                 /
	 *      \           /
	 *         \     /
	 * +-------- \-/ --------+        <- 0
	 * 
	 * ^                     ^
	 * 0                     w
	 *
	 * the region is (from bottom to top) 1/4 triangle, 1/2 rectangle, then 1/4 triangle
	 */

	const float deltaY = (l->positions[l->length*2-1] - l->positions[1]) / (l->length - 1);
	for (int j = 1; j < l->length-1; j++) {
		float xRangeLower = l->positions[j*2 - 2] - maxVariance;
		float xRangeUpper = l->positions[j*2 - 2] + maxVariance;
		float xMin, xMax;
		if (j < l->length/4) { //bottom quarter
			//instead of y=ax+b, use x=(y-b)/a
			xMin = ((deltaY * j) - static_cast<float>(rec.h)/4) / (-.5f*static_cast<float>(rec.h/rec.w));
			xMax = ((deltaY * j) + static_cast<float>(rec.h)/4) / ( .5f*static_cast<float>(rec.h/rec.w));
		} else if (j < 3 * l->length / 4) { //middle half
			xMin = 0;
			xMax = rec.w;
		} else { //top quarter
			xMin = ((deltaY * j) - (3.0f/4.0f)*static_cast<float>(rec.h)) / ( .5f*static_cast<float>(rec.h/rec.w));
			xMax = ((deltaY * j) - (5.0f/4.0f)*static_cast<float>(rec.h)) / (-.5f*static_cast<float>(rec.h/rec.w));
		}
		xRangeLower = (xRangeLower < xMin ? xMin : xRangeLower);
		xRangeUpper = (xRangeUpper > xMax ? xMax : xRangeUpper);
		l->positions[j*2+0] = VisualRNG::randFloatInRange(xRangeLower, xRangeUpper);
	}
}

void VerticalLightningHazard::refreshBolt(LightningBolt* l) const {
	RectangularLightningHazard::refreshBolt(l, this->rec.w, this->rec.h);
}

void VerticalLightningHazard::drawBackground(bool pose, float alpha) const {
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
		rec.x,       rec.y+(rec.h/2) - (rec.h/2)*scale,   color.getRf(), color.getGf(), color.getBf(), color.getAf(),
		rec.x+rec.w, rec.y+(rec.h/2) - (rec.h/2)*scale,   color.getRf(), color.getGf(), color.getBf(), color.getAf(),
		rec.x+rec.w, rec.y+(rec.h/2) + (rec.h/2)*scale,   color.getRf(), color.getGf(), color.getBf(), color.getAf(),
		rec.x,       rec.y+(rec.h/2) + (rec.h/2)*scale,   color.getRf(), color.getGf(), color.getBf(), color.getAf()
	};
	unsigned int indices[] = {
		0, 1, 2,
		2, 3, 0
	};

	Renderer::SubmitBatchedDraw(coordsAndColor, 4 * (2+4), indices, 2 * 3);

	drawBackgroundOutline(alpha);
}

void VerticalLightningHazard::drawBackgroundOutline(float alpha) const {
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
		(rec.x)       + lineWidth, (rec.y)       - lineWidth,   color_outline.getRf(), color_outline.getGf(), color_outline.getBf(), color_outline.getAf(), //4
		(rec.x+rec.w) - lineWidth, (rec.y)       - lineWidth,   color_outline.getRf(), color_outline.getGf(), color_outline.getBf(), color_outline.getAf(), //5
		(rec.x+rec.w) - lineWidth, (rec.y+rec.h) + lineWidth,   color_outline.getRf(), color_outline.getGf(), color_outline.getBf(), color_outline.getAf(), //6
		(rec.x)       + lineWidth, (rec.y+rec.h) + lineWidth,   color_outline.getRf(), color_outline.getGf(), color_outline.getBf(), color_outline.getAf()  //7
	};
	unsigned int indices_outline[] = { //rects
		//right
		1, 2, 6,
		6, 5, 1,

		//left
		4, 7, 3,
		3, 0, 4
	};

	Renderer::SubmitBatchedDraw(coordsAndColor_outline, (4+4) * (2+4), indices_outline, (2*2) * 3);
}

void VerticalLightningHazard::drawBolts_Pose(float alpha) const {
	//generate bolts
	std::vector<LightningBolt*> poseBolts;
	//from pushDefaultBolt(), mostly
	LightningBolt* l = new LightningBolt(rec.w/2, 0, rec.w/2, rec.h, getDefaultNumBoltPoints(rec.h));
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

RectHazard* VerticalLightningHazard::randomizingFactory(double x_start, double y_start, double area_width, double area_height, const GenericFactoryConstructionData& args) {
	if (WallManager::getNumWalls() <= 1) [[unlikely]] {
		return nullptr; //don't bother trying to see if a vertical lightning could go from edge to edge
	}

	int attempts = 0;
	RectHazard* randomized = nullptr;
	double xpos, ypos, width, height;
	const double minHeight = 40, maxHeight = 160;

	do {
		width = LevelRNG::randNumInRange(12, 24);
		int i = LevelRNG::randFunc() * WallManager::getNumWalls();
		const Wall* wa = WallManager::getWall(i);
		xpos = wa->rec.x + LevelRNG::randFunc() * std::clamp<double>(wa->rec.w - width, 0, wa->rec.w);
		ypos = wa->rec.y + wa->rec.h;

		int j, wallAttempts = 0;
		do {
			j = LevelRNG::randFunc() * WallManager::getNumWalls();
			wallAttempts++;
		} while ((wallAttempts < 8) && (j == i));
		if (j != i) [[likely]] {
			const Wall* otherWall = WallManager::getWall(j);
			height = otherWall->rec.y - ypos;
		} else {
			height = 0;
		}

		if ((height >= minHeight) && (height <= maxHeight) && (ypos >= y_start) && (ypos + height <= y_start + area_height) && (xpos >= x_start) && (xpos + width <= x_start + area_width)) {
			RectHazard* testVerticalLightning = new VerticalLightningHazard(xpos, ypos, width, height);
			if (testVerticalLightning->reasonableLocation()) {
				randomized = testVerticalLightning;
				break;
			} else {
				delete testVerticalLightning;
			}
		}
		attempts++;
	} while (attempts < 64);

	return randomized;
}
