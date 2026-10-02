#include "big-fun-level.h"

#include "../constants.h"
#include "../mylib.h" //weightedSelect

#include "../reset-things.h"
#include "../level-helper.h"
#include "../powerup-manager.h"
#include "../wall-manager.h"

std::unordered_map<std::string, float> BigFunLevel::getWeights() const {
	std::unordered_map<std::string, float> weights;
	weights.insert({ "vanilla", 1.0f });
	weights.insert({ "random-vanilla", 1.0f });
	weights.insert({ "old", 2.0f });
	weights.insert({ "random-old", 2.0f });
	weights.insert({ "random", 1.0f });
	return weights;
}

ColorValueHolder BigFunLevel::getDefaultColor() const {
	//not random likely to clearly differentiate it from DefaultRandomLevel
	return ColorValueHolder(0.75f, 0.25f, 0.5f);
}

void BigFunLevel::initialize() {
	ResetThings::tankPositionReset(40);

	ColorValueHolder color = getDefaultColor();
	PositionHolder pos;
	//GenericFactoryConstructionData constructionData;
	//double* posArr;

	LevelHelper::pushClassicWalls(color);

	LevelHelper::pushRandomWalls(12, 80+32+40, 40, GAME_WIDTH - 2*(80+32+40), GAME_HEIGHT - 2*40, color); //JS used 8

	//classic speed/invincible/wallhack
	LevelHelper::pushSymmetricPowerups_UD_Alternate(GAME_WIDTH/2, GAME_HEIGHT/2, GAME_HEIGHT/2 - 16,
		"vanilla", "speed", "vanilla", "invincible");
	PowerupManager::pushPowerup(new PowerSquare(GAME_WIDTH/2, GAME_HEIGHT/2, "vanilla", "wallhack"));
	//note: in JS, speed and invincible didn't alternate; it was possible to get 2 speed or 2 invincible

	//timeline for what powers existed: this was after color but possibly before power mixing, leaving these powers:
	//definitely existed: speed, invincible, wallhack, technically trap, bounce, multishot, big
	//almost certainly existed: barrier, shotgun, mines (but that wasn't on the random level)
	//not certain if existed: fire, triple, blast
	//definitely didn't exist: homing, tracking, grenade, banana, godmode

	//that leaves these powers reasonable for randomization: speed, bounce, multishot, big, barrier, shotgun, fire, maybe triple
	//fire + big is not good, triple is unnecessary when shotgun is already in, so that leaves these powers:
	const std::string possiblePowers[] = { "speed", "bounce", "multishot", "big", "barrier", "shotgun" };
	constexpr int possiblePowersCount = std::size(possiblePowers);
	//TODO: rewrite how powers (and levels and hazards) are called: use a pair of {type, name} instead of separate type and name

	float weights[] = { 6.0f, 3.0f, 1.0f }; //JS: 1/4 chance to get +1 power (so theoretically unlimited amount)
	for (int i = 0; i < 4; i++) {
		int count = 1 + weightedSelect<float>(weights, 3); //{1, 2, 3}
		std::string* randPowers = LevelHelper::getRandomPowers(count, "random-vanilla", possiblePowers, possiblePowersCount);
		//std::string* randPowers = LevelHelper::getRandomPowersOld(count, true, possiblePowers, possiblePowersCount);
		pos = LevelHelper::getSymmetricPowerupPositions_Corners(i, GAME_WIDTH/2, GAME_HEIGHT/2, GAME_WIDTH/2-(80+32+16), GAME_HEIGHT/2-16);
		PowerupManager::pushPowerup(new PowerSquare(pos.x, pos.y, "random-vanilla", randPowers, count));
		delete[] randPowers;
	}
}

Level* BigFunLevel::factory() {
	return new BigFunLevel();
}

BigFunLevel::BigFunLevel() { return; }
