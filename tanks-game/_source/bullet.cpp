#include "bullet.h"

#include "constants.h"
#include <cmath>
#include <algorithm> //std::min, std::max
#include <iostream>

#include "renderer.h"
#include "color-cache.h"
#include "color-mixer.h"
#include "background-rect.h"

//#include <tracy_include.h>

float Bullet::instanced_vertices[2 * ((Bullet::BulletSideCount + 2) + (Bullet::BulletSideCount + 1) + (Bullet::BulletSideCount * 2))];
unsigned int Bullet::instanced_indices[Bullet::BulletSideCount * 3 * (1+1+2)];
bool Bullet::initialized_vertices = false;

const ColorValueHolder Bullet::default_color = ColorValueHolder(0.5f, 0.5f, 0.5f);
const double Bullet::default_radius = 4;

Bullet::Bullet(double x_, double y_, float angle, Team_ID teamID, BulletParentType parentType, Game_ID parentID) : GameThing(teamID, ObjectType::Bullet) { //every bullet constructor does this stuff
	this->x = x_;
	this->y = y_;
	this->velocity = SimpleVector2D(angle, 0, true);
	this->parentType = parentType;
	this->parentID = parentID;
	this->lifeValue = 100;
	this->m_colorIdentifier = 0;

	//initializeVertices(); //called in main() to reduce the tiny overhead in calling this every time a bullet is created
}

Bullet::Bullet(double x_, double y_, float angle, Team_ID teamID, BulletParentType parentType, Game_ID parentID, const std::vector<BulletPower*>* bp) : Bullet(x_,y_,angle,teamID,parentType,parentID) {
	bulletPowers.reserve(bp->size());
	for (int i = 0; i < bp->size(); i++) {
		bulletPowers.push_back(bp->data()[i]);
	}
	for (int i = 0; i < bulletPowers.size(); i++) {
		bulletPowers[i]->initialize(this);
	}
	updateColorIdentifier(); //TODO: regenerating this each time is not optimal, but oh well; the tank could make it once for the many bullets it makes, but that needs a new parameter to the constructor
	//bp not deleted
}

//probably just for banana bullet creation:
//TODO: make a special constructor for banana (and wallspark/edgespark) that does the duplication in there? reduces code duplication, that's about it
Bullet::Bullet(double x_, double y_, double r_, float angle, float vel, Team_ID teamID, BulletParentType parentType, Game_ID parentID, const std::vector<BulletPower*>* bp, bool) : Bullet(x_,y_,angle,teamID,parentType,parentID,bp) {
	this->r = r_;
	this->velocity.setMagnitude(vel); // * getBulletSpeedMultiplier(); //not wanted
	this->initial_velocity = this->velocity.getMagnitude();
	this->acceleration = getBulletAcceleration();
}

//avoid using:
Bullet::Bullet(double x_, double y_, double r_, float angle, float vel, float acc, Team_ID teamID, BulletParentType parentType, Game_ID parentID, const std::vector<BulletPower*>* bp, bool) : Bullet(x_,y_,angle,teamID,parentType,parentID,bp) {
	this->r = r_ * getBulletRadiusMultiplier();
	this->velocity.setMagnitude(vel * getBulletSpeedMultiplier());
	this->initial_velocity = this->velocity.getMagnitude();
	this->acceleration = acc;
}

//regular 1:
Bullet::Bullet(double x_, double y_, double r_, float angle, float vel, Team_ID teamID, BulletParentType parentType, Game_ID parentID) : Bullet(x_,y_,angle,teamID,parentType,parentID) {
	this->r = r_;
	this->velocity.setMagnitude(vel);
	this->initial_velocity = this->velocity.getMagnitude();
	this->acceleration = 0;
}

//regular 2:
Bullet::Bullet(double x_, double y_, double r_, float angle, float vel, Team_ID teamID, BulletParentType parentType, Game_ID parentID, const std::vector<BulletPower*>* bp) : Bullet(x_,y_,angle,teamID,parentType,parentID,bp) {
	this->r = r_ * getBulletRadiusMultiplier();
	this->velocity.setMagnitude(vel * getBulletSpeedMultiplier());
	this->initial_velocity = this->velocity.getMagnitude();
	this->acceleration = getBulletAcceleration();
}

Bullet::~Bullet() {
	for (int i = 0; i < bulletPowers.size(); i++) {
		delete bulletPowers[i];
	}
	//bulletPowers.clear();
}

bool Bullet::initializeVertices() {
	if (initialized_vertices) [[likely]] {
		return false;
	}

	//death circle:
	instanced_vertices[0] = 0;
	instanced_vertices[1] = 0;
	for (int i = 0; i < (Bullet::BulletSideCount + 2) - 1; i++) {
		const unsigned int startIdx = 2;
		instanced_vertices[startIdx + i*2]   = std::cos(i * (2*PI / Bullet::BulletSideCount) + PI/2);
		instanced_vertices[startIdx + i*2+1] = std::sin(i * (2*PI / Bullet::BulletSideCount) + PI/2);
	}

	//body:
	instanced_vertices[2*(Bullet::BulletSideCount + 2)]     = 0;
	instanced_vertices[2*(Bullet::BulletSideCount + 2) + 1] = 0;
	for (int i = 0; i < (Bullet::BulletSideCount + 1) - 1; i++) {
		const unsigned int startIdx = 2 * ((Bullet::BulletSideCount + 2) + 1);
		instanced_vertices[startIdx + i*2]   = std::cos(i * (2*PI / Bullet::BulletSideCount));
		instanced_vertices[startIdx + i*2+1] = std::sin(i * (2*PI / Bullet::BulletSideCount));
	}

	//outline:
	for (int i = 0; i < Bullet::BulletSideCount; i++) {
		const unsigned int startIdx = 2 * ((Bullet::BulletSideCount + 2) + (Bullet::BulletSideCount + 1));
		instanced_vertices[startIdx + i*4]   = std::cos(i * (2*PI / Bullet::BulletSideCount));
		instanced_vertices[startIdx + i*4+1] = std::sin(i * (2*PI / Bullet::BulletSideCount));
		instanced_vertices[startIdx + i*4+2] = std::cos(i * (2*PI / Bullet::BulletSideCount));
		instanced_vertices[startIdx + i*4+3] = std::sin(i * (2*PI / Bullet::BulletSideCount));
	}

	//death circle:
	for (int i = 0; i < Bullet::BulletSideCount; i++) {
		instanced_indices[i*3]   = 0;
		instanced_indices[i*3+1] = (i+1);
		instanced_indices[i*3+2] = (i+2);
	}
	//body:
	for (int i = 0; i < Bullet::BulletSideCount; i++) {
		const unsigned int startPos = (Bullet::BulletSideCount + 2);
		instanced_indices[Bullet::BulletSideCount*3 + i*3]   = startPos + 0;
		instanced_indices[Bullet::BulletSideCount*3 + i*3+1] = startPos + (i+1);
		instanced_indices[Bullet::BulletSideCount*3 + i*3+2] = startPos + (i+1) % Bullet::BulletSideCount + 1;
	}
	//outline:
	for (int i = 0; i < Bullet::BulletSideCount; i++) {
		const unsigned int startPos = (Bullet::BulletSideCount + 2) + (Bullet::BulletSideCount + 1);
		instanced_indices[Bullet::BulletSideCount*2*3 + i*6]   = startPos +  i*2;
		instanced_indices[Bullet::BulletSideCount*2*3 + i*6+1] = startPos +  i*2+1;
		instanced_indices[Bullet::BulletSideCount*2*3 + i*6+2] = startPos + (i*2+3) % (Bullet::BulletSideCount*2);
		instanced_indices[Bullet::BulletSideCount*2*3 + i*6+3] = startPos + (i*2+3) % (Bullet::BulletSideCount*2);
		instanced_indices[Bullet::BulletSideCount*2*3 + i*6+4] = startPos + (i*2+2) % (Bullet::BulletSideCount*2);
		instanced_indices[Bullet::BulletSideCount*2*3 + i*6+5] = startPos +  i*2;
	}

	initialized_vertices = true;
	return true;
}

bool Bullet::canCollideWith(const GameThing* thing) const {
	switch (this->parentType) {
		case BulletParentType::team:
			return (this->getTeamID() != thing->getTeamID());
		case BulletParentType::individual:
			return (parentID != thing->getGameID());
		case BulletParentType::name:
			//TODO
			[[fallthrough]];
		case BulletParentType::none:
			//TODO
			[[fallthrough]];
		default:
			return true;
	}
}

bool Bullet::canCollideWith(const Bullet* b_other) const {
	if (this->parentType == BulletParentType::individual) {
		return ((parentID != b_other->getGameID()) && (parentID != b_other->getParentID()));
	}
	return canCollideWith(static_cast<const GameThing*>(b_other));
}

void Bullet::update(const BulletUpdateStruct* up) {
	this->x += up->x;
	this->y += up->y;
	this->velocity = SimpleVector2D(velocity.getAngle() + up->angle, velocity.getMagnitude() + up->speed, true);
	this->lifeValue = std::min(this->lifeValue + up->alpha, 100.0f);
}

bool Bullet::move() {
	bool shouldBeKilled = false;
	bool overridedMovement = false;
	//TODO: handle killing the bulletpowers

	for (int k = 0; k < bulletPowers.size(); k++) {
		if (bulletPowers[k]->modifiesMovement) {
			if (bulletPowers[k]->overridesMovement) {
				overridedMovement = true;
			}

			InteractionBoolHolder check_temp = bulletPowers[k]->modifiedMovement(this);
			if (check_temp.firstShouldDie) {
				shouldBeKilled = true;
				overridedMovement = true;
				break;
			}

			if (!bulletPowers[k]->modifiedMovementCanWorkWithOthers) {
				break;
			}
		}
	}

	if (!overridedMovement) {
		move_base();
	}

	degradeHandle();
	growHandle();

	return shouldBeKilled;
}

inline void Bullet::move_base() {
	velocity.changeMagnitude(acceleration);
	x += velocity.getXComp();
	y += velocity.getYComp();
}

inline void Bullet::degradeHandle() {
	if (velocity.getMagnitude() == 0) {
		this->lifeValue -= getBulletDegradeAmount();
	}
}

inline void Bullet::growHandle() {
	double additiveR, multiplierR;
	if (velocity.getMagnitude() > 0) {
		std::pair<double, double> growValues = getBulletRadiusGrowNumber_Moving();
		additiveR = growValues.first;
		multiplierR = growValues.second;
	} else {
		std::pair<double, double> growValues = getBulletRadiusGrowNumber_Stationary();
		additiveR = growValues.first;
		multiplierR = growValues.second;
	}

	//if multiplierR>1, choose whichever is greater; else apply both (add then mult)
	if (multiplierR > 1) {
		if (this->r + additiveR >= this->r * multiplierR) {
			//this means additiveR > the radius increase multiplierR will bring
			this->r += additiveR;
		} else {
			this->r *= multiplierR;
		}
	} else {
		this->r += additiveR;
		this->r *= multiplierR;
	}

	//TODO: negative additive values could work, as long as when b->r <= 0 the bullet dies
}

float Bullet::getBulletSpeedMultiplier() const {
	//look at Tank::getFiringRateMultiplier()

	float highest = 1;
	float lowest = 1;
	std::vector<float> stackList;

	for (int i = 0; i < bulletPowers.size(); i++) {
		float value = bulletPowers[i]->getBulletSpeedMultiplier();
		if (bulletPowers[i]->bulletSpeedStacks) {
			stackList.push_back(value);
		} else {
			if (value < lowest) {
				lowest = value;
			} else if (value > highest) {
				highest = value;
			}
		}
	}

	float value = 1;
	for (int i = 0; i < stackList.size(); i++) {
		value *= stackList[i];
	}

	return highest * lowest * value; //TODO: should bullets be able to set a minimum speed? fire + big doesn't behave very well
}

double Bullet::getBulletRadiusMultiplier() const {
	//look at Tank::getFiringRateMultiplier()

	double highest = 1;
	double lowest = 1;
	std::vector<double> stackList;

	for (int i = 0; i < bulletPowers.size(); i++) {
		double value = bulletPowers[i]->getBulletRadiusMultiplier();
		if (bulletPowers[i]->bulletRadiusStacks) {
			stackList.push_back(value);
		} else {
			if (value < lowest) {
				lowest = value;
			} else if (value > highest) {
				highest = value;
			}
		}
	}

	double value = 1;
	for (int i = 0; i < stackList.size(); i++) {
		value *= stackList[i];
	}
	return highest * lowest * value;
}

float Bullet::getBulletAcceleration() const {
	//look at Tank::getFiringRateMultiplier()
	//(this has importance though)

	float importance = LOW_IMPORTANCE;
	for (int i = 0; i < bulletPowers.size(); i++) {
		float value = bulletPowers[i]->getBulletAccelerationImportance();
		if (value > importance) {
			importance = value;
		}
	}

	float highest = 0;
	float lowest = 0;
	for (int i = 0; i < bulletPowers.size(); i++) {
		if (bulletPowers[i]->getBulletAccelerationImportance() == importance) {
			float value = bulletPowers[i]->getBulletAcceleration();
			if (value < lowest) {
				lowest = value;
			} else if (value > highest) {
				highest = value;
			}
		}
	}
	return highest + lowest; //idea: / getBulletSpeedMultiplier()
}

float Bullet::getBulletDegradeAmount() const {
	//look at Tank::getFiringRateMultiplier()
	//(this has importance though)
	//negative degrade values are not valid, so this just finds the highest value

	float importance = LOW_IMPORTANCE;
	for (int i = 0; i < bulletPowers.size(); i++) {
		float value = bulletPowers[i]->getBulletDegradeImportance();
		if (value > importance) {
			importance = value;
		}
	}

	float highest = 0;
	for (int i = 0; i < bulletPowers.size(); i++) {
		if (bulletPowers[i]->getBulletDegradeImportance() == importance) {
			float value = bulletPowers[i]->getBulletDegradeAmount();
			if (value > highest) {
				highest = value;
			}
		}
	}
	return highest;
}

std::pair<double, double> Bullet::getBulletRadiusGrowNumber_Stationary() const {
	//look at Tank::getFiringRateMultiplier()
	//negative values are not valid, but not enforced

	double highestAdditive = 0;
	double highestMultiplier = 1;
	double lowestMultiplier = 1;

	for (int i = 0; i < bulletPowers.size(); i++) {
		double value = bulletPowers[i]->getBulletRadiusGrowNumber_Stationary();
		if (bulletPowers[i]->bulletRadiusGrowMultiplies_Stationary) {
			if (value < lowestMultiplier) {
				lowestMultiplier = value;
			} else if (value > highestMultiplier) {
				highestMultiplier = value;
			}
		} else {
			if (value > highestAdditive) {
				highestAdditive = value;
			}
		}
	}
	return { highestAdditive, highestMultiplier * lowestMultiplier };
}

std::pair<double, double> Bullet::getBulletRadiusGrowNumber_Moving() const {
	//look at Tank::getFiringRateMultiplier()
	//negative values are not valid, but not enforced

	double highestAdditive = 0;
	double highestMultiplier = 1;
	double lowestMultiplier = 1;

	for (int i = 0; i < bulletPowers.size(); i++) {
		double value = bulletPowers[i]->getBulletRadiusGrowNumber_Moving();
		if (bulletPowers[i]->bulletRadiusGrowMultiplies_Moving) {
			if (value < lowestMultiplier) {
				lowestMultiplier = value;
			} else if (value > highestMultiplier) {
				highestMultiplier = value;
			}
		} else {
			if (value > highestAdditive) {
				highestAdditive = value;
			}
		}
	}
	return { highestAdditive, highestMultiplier * lowestMultiplier };
}

void Bullet::powerTick() {
	for (int i = bulletPowers.size() - 1; i >= 0; i--) {
		bulletPowers[i]->tick(this); //I don't think any power will use this, but whatever
	}
}

void Bullet::removePower(int i) {
	bulletPowers[i]->removeEffects(this);
	delete bulletPowers[i];
	bulletPowers.erase(bulletPowers.begin() + i);
	updateColorIdentifier();
}

void Bullet::updateColorIdentifier() {
	float highest = LOW_IMPORTANCE;
	for (int i = 0; i < bulletPowers.size(); i++) {
		if (bulletPowers[i]->getColorImportance() > highest) {
			highest = bulletPowers[i]->getColorImportance();
		}
	}
	if (highest < 0) [[unlikely]] {
		m_colorIdentifier = 0;
		return;
	}

	std::string colorIdentifierStr = "";
	for (int i = 0; i < bulletPowers.size(); i++) {
		if (bulletPowers[i]->getColorImportance() == highest) {
			colorIdentifierStr += "|" + bulletPowers[i]->getColorIdentifier();
		}
	}
	m_colorIdentifier = ColorCacheBullet::getColorID(colorIdentifierStr);
}

const ColorValueHolder& Bullet::getColor() const {
	if (m_colorIdentifier == 0) [[unlikely]] {
		return default_color;
	}

	if (ColorCacheBullet::colorIsCached(m_colorIdentifier)) [[likely]] {
		return ColorCacheBullet::getColor(m_colorIdentifier);
	} else {
		float highest = LOW_IMPORTANCE;
		for (int i = 0; i < bulletPowers.size(); i++) {
			if (bulletPowers[i]->getColorImportance() > highest) {
				highest = bulletPowers[i]->getColorImportance();
			}
		}

		std::vector<ColorValueHolder> mixingColors; mixingColors.reserve(bulletPowers.size());
		for (int i = 0; i < bulletPowers.size(); i++) {
			if (bulletPowers[i]->getColorImportance() == highest) {
				mixingColors.push_back(bulletPowers[i]->getColor());
			}
		}
		return ColorCacheBullet::insertColor(m_colorIdentifier, mixingColors.data(), mixingColors.size());
	}
}

void Bullet::draw() const {
	ghostDraw(1.0f);
}

void Bullet::draw(DrawingLayers layer) const {
	switch (layer) {
		case DrawingLayers::under:
			//nothing
			break;

		default:
			std::cerr << "WARNING: unknown DrawingLayer for Bullet::draw!" << std::endl;
			[[fallthrough]];
		case DrawingLayers::normal:
			draw();
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

void Bullet::poseDraw() const {
	draw();
}

void Bullet::poseDraw(DrawingLayers layer) const {
	switch (layer) {
		case DrawingLayers::under:
			//nothing
			break;

		default:
			std::cerr << "WARNING: unknown DrawingLayer for Bullet::poseDraw!" << std::endl;
			[[fallthrough]];
		case DrawingLayers::normal:
			poseDraw();
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

void Bullet::ghostDraw(float alpha) const {
	//ZoneScoped;
	alpha = alpha * alpha;
	ColorValueHolder color = getColor();
	color = ColorMixer::mix(BackgroundRect::getBackColor(), color, alpha);

	//note: technically the alpha is separate from the bullet life, especially noticeable on minefield level effects...
	//oh well, not a big deal; maybe it's a good thing because ghost bullets should probably have their outline fully visible
	Renderer::SubmitBulletDrawCall(this->x, this->y, this->r, color.getRf(), color.getGf(), color.getBf(), this->lifeValue / 100.0f);
}

void Bullet::ghostDraw(DrawingLayers layer, float alpha) const {
	switch (layer) {
		case DrawingLayers::under:
			//nothing
			break;

		default:
			std::cerr << "WARNING: unknown DrawingLayer for Bullet::ghostDraw!" << std::endl;
			[[fallthrough]];
		case DrawingLayers::normal:
			ghostDraw(alpha);
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

bool Bullet::kill() {
	//see Tank::kill()

	bool shouldBeKilled = true;

	for (int i = 0; i < bulletPowers.size(); i++) {
		bool killBulletPower = false;

		if (bulletPowers[i]->modifiesDeathHandling) {
			InteractionBoolHolder check_temp = bulletPowers[i]->modifiedDeathHandling(this);
			if (!check_temp.firstShouldDie) {
				shouldBeKilled = false;
			}
			if (check_temp.secondShouldDie) {
				killBulletPower = true;
			}
		}

		if (killBulletPower) {
			removePower(i);
			i--;
			//continue;
		}
		if (!shouldBeKilled) {
			break;
		}
	}

	return shouldBeKilled;
}

float Bullet::getHighestOffenseImportance() const {
	float highest = LOW_IMPORTANCE;
	for (int i = 0; i < bulletPowers.size(); i++) {
		if (bulletPowers[i]->getOffenseImportance() > highest) {
			highest = bulletPowers[i]->getOffenseImportance();
		}
	}
	return highest;
}

float Bullet::getHighestOffenseTier(float importance) const {
	if (bulletPowers.size() == 0) {
		return 0;
	}
	float highest = LOW_TIER;
	for (int i = 0; i < bulletPowers.size(); i++) {
		if (bulletPowers[i]->getOffenseImportance() == importance) {
			if (bulletPowers[i]->getOffenseTier(this) > highest) {
				highest = bulletPowers[i]->getOffenseTier(this);
			}
		}
	}
	return highest;
}

float Bullet::getOffenseTier() const {
	return getHighestOffenseTier(getHighestOffenseImportance());
}

float Bullet::getHighestDefenseImportance() const {
	float highest = LOW_IMPORTANCE;
	for (int i = 0; i < bulletPowers.size(); i++) {
		if (bulletPowers[i]->getDefenseImportance() > highest) {
			highest = bulletPowers[i]->getDefenseImportance();
		}
	}
	return highest;
}

float Bullet::getHighestDefenseTier(float importance) const {
	if (bulletPowers.size() == 0) {
		return 0;
	}
	float highest = LOW_TIER;
	for (int i = 0; i < bulletPowers.size(); i++) {
		if (bulletPowers[i]->getDefenseImportance() == importance) {
			if (bulletPowers[i]->getDefenseTier(this) > highest) {
				highest = bulletPowers[i]->getDefenseTier(this);
			}
		}
	}
	return highest;
}

float Bullet::getDefenseTier() const {
	return getHighestDefenseTier(getHighestDefenseImportance());
}

BulletUpdateStruct::BulletUpdateStruct(double x, double y, float speed, float angle, float alpha) {
	this->x = x;
	this->y = y;
	this->speed = speed;
	this->angle = angle;
	this->alpha = alpha;
	//this->newBullets = std::vector<Bullet*>();
	//this->newBullets = std::vector<Bullet*>(newBullets);
}

void BulletUpdateStruct::add(const BulletUpdateStruct& other) {
	this->x += other.x;
	this->y += other.y;
	this->speed += other.speed;
	this->angle += other.angle;
	this->alpha += other.alpha;
	//newBullets.insert(newBullets.end(), other.newBullets.begin(), other.newBullets.end());
}
