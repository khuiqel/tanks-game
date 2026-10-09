#include "priority-handler.h"

#include <iostream>

PriorityResult PriorityHandler::determinePriority(const Bullet* a, const Bullet* b) {
	float a_offense = a->getOffenseTier();
	float a_defense = a->getDefenseTier();
	float b_offense = b->getOffenseTier();
	float b_defense = b->getDefenseTier();

	bool a_dies = false, b_dies = false;

	if (a_offense >= b_defense) {
		b_dies = true;
	}
	if (b_offense >= a_defense) {
		a_dies = true;
	}

	if (a_dies && b_dies) {
		return PriorityResult::bothDie;
	}
	if (a_dies) {
		return PriorityResult::firstDies;
	}
	if (b_dies) {
		return PriorityResult::secondDies;
	}
	return PriorityResult::neitherDie;
}

PriorityResult PriorityHandler::determinePriority(const Tank* a, const Bullet* b) {
	float a_offense = a->getOffenseTier();
	float a_defense = a->getDefenseTier();
	float b_offense = b->getOffenseTier();
	float b_defense = b->getDefenseTier();

	bool a_dies = false, b_dies = false;

	if (a_offense >= b_defense || a_defense >= b_offense) {
		b_dies = true;
	}
	if (b_offense >= a_defense) {
		a_dies = true;
	}

	if (a_dies && b_dies) {
		return PriorityResult::bothDie;
	}
	if (a_dies) {
		return PriorityResult::firstDies;
	}
	if (b_dies) {
		return PriorityResult::secondDies;
	}
	return PriorityResult::neitherDie;
}

PriorityResult PriorityHandler::determinePriority(const Tank* a, const Tank* b) {
	float a_offense = a->getOffenseTier();
	float a_defense = a->getDefenseTier();
	float b_offense = b->getOffenseTier();
	float b_defense = b->getDefenseTier();

	bool a_dies = false, b_dies = false;

	if (a_offense > b_defense) {
		b_dies = true;
	}
	if (b_offense > a_defense) {
		a_dies = true;
	}

	if (a_dies && b_dies) {
		return PriorityResult::bothDie;
	}
	if (a_dies) {
		return PriorityResult::firstDies;
	}
	if (b_dies) {
		return PriorityResult::secondDies;
	}
	return PriorityResult::neitherDie;
}

PriorityResult PriorityHandler::determinePriority(const Bullet* a, const CircleHazard* b) {
	float a_offense = a->getOffenseTier();
	float a_defense = a->getDefenseTier();
	float b_offense = b->getOffenseTier();
	float b_defense = b->getDefenseTier();

	bool a_dies = false, b_dies = false;

	if (a_offense >= b_defense) {
		b_dies = true;
	}
	switch (b->getCollisionType()) {
		default:
			std::cerr << "WARNING: unknown CircleHazardCollisionType!\n";
			[[fallthrough]];
		case CircleHazardCollisionType::solid:
			if (b_offense >= a_defense || b_defense > a_offense) {
				a_dies = true;
			}
			break;
		case CircleHazardCollisionType::under:
			if (b_offense >= a_defense) {
				a_dies = true;
			}
			break;
	}

	if (a_dies && b_dies) {
		return PriorityResult::bothDie;
	}
	if (a_dies) {
		return PriorityResult::firstDies;
	}
	if (b_dies) {
		return PriorityResult::secondDies;
	}
	return PriorityResult::neitherDie;
}

PriorityResult PriorityHandler::determinePriority(const Bullet* a, const RectHazard* b) {
	float a_offense = a->getOffenseTier();
	float a_defense = a->getDefenseTier();
	float b_offense = b->getOffenseTier();
	float b_defense = b->getDefenseTier();

	bool a_dies = false, b_dies = false;

	if (a_offense >= b_defense) {
		b_dies = true;
	}
	switch (b->getCollisionType()) {
		default:
			std::cerr << "WARNING: unknown RectHazardCollisionType!\n";
			[[fallthrough]];
		case RectHazardCollisionType::solid:
			if (b_offense >= a_defense || b_defense > a_offense) {
				a_dies = true;
			}
			break;
		case RectHazardCollisionType::under:
			if (b_offense >= a_defense) {
				a_dies = true;
			}
			break;
	}

	if (a_dies && b_dies) {
		return PriorityResult::bothDie;
	}
	if (a_dies) {
		return PriorityResult::firstDies;
	}
	if (b_dies) {
		return PriorityResult::secondDies;
	}
	return PriorityResult::neitherDie;
}

PriorityResult PriorityHandler::determinePriority(const Tank* a, const CircleHazard* b) {
	float a_offense = a->getOffenseTier();
	float a_defense = a->getDefenseTier();
	float b_offense = b->getOffenseTier();
	float b_defense = b->getDefenseTier();

	bool a_dies = false, b_dies = false;

	if (a_offense > b_defense) {
		b_dies = true;
	}
	if (b_offense > a_defense) {
		a_dies = true;
	}

	if (a_dies && b_dies) {
		return PriorityResult::bothDie;
	}
	if (a_dies) {
		return PriorityResult::firstDies;
	}
	if (b_dies) {
		return PriorityResult::secondDies;
	}
	return PriorityResult::neitherDie;
}

PriorityResult PriorityHandler::determinePriority(const Tank* a, const RectHazard* b) {
	float a_offense = a->getOffenseTier();
	float a_defense = a->getDefenseTier();
	float b_offense = b->getOffenseTier();
	float b_defense = b->getDefenseTier();

	bool a_dies = false, b_dies = false;

	if (a_offense > b_defense) {
		b_dies = true;
	}
	if (b_offense > a_defense) {
		a_dies = true;
	}

	if (a_dies && b_dies) {
		return PriorityResult::bothDie;
	}
	if (a_dies) {
		return PriorityResult::firstDies;
	}
	if (b_dies) {
		return PriorityResult::secondDies;
	}
	return PriorityResult::neitherDie;
}
