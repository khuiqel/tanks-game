#include "developer-manager.h"

#include <iostream>

#include "window-initializer.h"
#include "tank-manager.h"
#include "powerup-manager.h"
#include "wall-manager.h"
#include "hazard-manager.h"
#include "level-manager.h"
#include "game-manager.h" //settings

bool DeveloperManager::leftMouse = false;
bool DeveloperManager::rightMouse = false;
int DeveloperManager::mousePosX = 0;
int DeveloperManager::mousePosY = 0;
int DeveloperManager::insertIndex = 0;

void DeveloperManager::setX(Circle* c, double x) { c->x = x; }
void DeveloperManager::setY(Circle* c, double y) { c->y = y; }
void DeveloperManager::setR(Circle* c, double r) { c->r = r; }

void DeveloperManager::setX(Rect* r, double x) { r->x = x; }
void DeveloperManager::setY(Rect* r, double y) { r->y = y; }
void DeveloperManager::setW(Rect* r, double w) { r->w = w; }
void DeveloperManager::setH(Rect* r, double h) { r->h = h; }

void DeveloperManager::mouseButtonCallbackFunc(GLFWwindow*, int button, int action, int mods) {
	switch (button) {
		case GLFW_MOUSE_BUTTON_1:
			leftMouse = (action == GLFW_PRESS);
			break;
		case GLFW_MOUSE_BUTTON_2:
			if (action == GLFW_PRESS) {
				rightMouse = !rightMouse; //bad name but whatever
			}
			break;
		case GLFW_MOUSE_BUTTON_3:
			if (action == GLFW_PRESS) {
				devInsert(mousePosX, mousePosY);
			}
			break;
	}
}

void DeveloperManager::mouseScrollCallbackFunc(GLFWwindow*, double xOffset, double yOffset) {
	const GameSettings& game_settings = GameManager::get_settings();
	#ifdef RELEASE_DISTRIBUTION
	if (!game_settings.DevMouseControls) {
		return;
	}
	#endif

	(void)xOffset;
	if (yOffset == 0) {
		//left/right scrolling happened
		return;
	}

	int scrollAmount;
	if (yOffset > 0) {
		scrollAmount = std::max(static_cast<int>(yOffset), 1);
	} else {
		scrollAmount = std::min(static_cast<int>(yOffset), -1);
	}

	const unsigned int insertIndexMax = insertListIdentifiers.size();
	if (insertIndexMax > 0) {
		insertIndex += scrollAmount;
		do {
			insertIndex += insertIndexMax;
			//modulo on a negative int is implementation-defined, so this is just about the only solution
			//compilers are very smart, so they should be able to recognize what this is trying to do and replace it with something simpler if possible
		} while (insertIndex < 0);
		insertIndex %= insertIndexMax;
	} else [[unlikely]] {
		insertIndex = 0;
	}

	std::cout << "DeveloperManager insert identifier: " << insertListIdentifiers[insertIndex] << std::endl;
}

void DeveloperManager::mouseCursorPosCallbackFunc(GLFWwindow*, double xPos, double yPos) {
	WindowInitializer::windowCoordsToGameCoords(xPos, yPos, mousePosX, mousePosY);

	const GameSettings& game_settings = GameManager::get_settings();
	#ifdef RELEASE_DISTRIBUTION
	if (!game_settings.DevMouseControls) {
		return;
	}
	#endif

	if (leftMouse) {
		if (!rightMouse) {
			TankManager::getTank(0)->x = mousePosX;
			TankManager::getTank(0)->y = mousePosY;
		} else {
			TankManager::getTank(1)->x = mousePosX;
			TankManager::getTank(1)->y = mousePosY;
		}
	}
}

std::vector<std::string> DeveloperManager::insertListIdentifiers = { "longinvincible", "temp", "banana", "homing", "barrier", "bounce", "mines", "multishot", "grenade", "blast", "godmode", "big", "inversion", "annoying", "swarm", "stationary_turret", "vert_wall", "horz_wall" };
int DeveloperManager::insertListIdentifiers_normalSize = insertListIdentifiers.size();
void DeveloperManager::devInsert(int x, int y) {
	const GameSettings& game_settings = GameManager::get_settings();
	#ifdef RELEASE_DISTRIBUTION
	if (!game_settings.DevMouseControls) {
		return;
	}
	#endif

	GenericFactoryConstructionData constructionData;
	double* posArr;
	switch (insertIndex) {
		case 0:
			#ifdef NDEBUG
			PowerupManager::pushPowerup(new PowerSquare(x, y, "dev", "longinvincible"));
			#else
			PowerupManager::pushPowerup(new PowerSquare(x, y, "dev", "colorless_longinvincible"));
			#endif
			break;
		case 1:
			PowerupManager::pushPowerup(new PowerSquare(x, y, "vanilla", "megadeath"));
			//PowerupManager::pushPowerup(new PowerSquare(x, y, "vanilla-extra", "shotgun"));
			//PowerupManager::pushPowerup(new PowerSquare(x, y, "dev", "other_stuff_is_poison"));
			break;
		case 2:
			PowerupManager::pushPowerup(new PowerSquare(x, y, "vanilla", "banana"));
			break;
		case 3:
			PowerupManager::pushPowerup(new PowerSquare(x, y, "vanilla", "homing"));
			break;
		case 4:
			PowerupManager::pushPowerup(new PowerSquare(x, y, "vanilla-extra", "barrier"));
			break;
		case 5:
			PowerupManager::pushPowerup(new PowerSquare(x, y, "vanilla", "bounce"));
			break;
		case 6:
			PowerupManager::pushPowerup(new PowerSquare(x, y, "vanilla-extra", "mines"));
			break;
		case 7:
			PowerupManager::pushPowerup(new PowerSquare(x, y, "vanilla", "multishot"));
			break;
		case 8:
			PowerupManager::pushPowerup(new PowerSquare(x, y, "vanilla", "grenade"));
			break;
		case 9:
			PowerupManager::pushPowerup(new PowerSquare(x, y, "vanilla", "blast"));
			break;
		case 10:
			PowerupManager::pushPowerup(new PowerSquare(x, y, "vanilla", "godmode"));
			break;
		case 11:
			PowerupManager::pushPowerup(new PowerSquare(x, y, "vanilla", "big"));
			break;
		case 12:
			PowerupManager::pushPowerup(new PowerSquare(x, y, "dev", "inversion"));
			break;
		case 13:
			PowerupManager::pushPowerup(new PowerSquare(x, y, "dev", "annoying"));
			break;
		case 14:
			PowerupManager::pushPowerup(new PowerSquare(x, y, "vanilla-extra", "swarm"));
			break;
		case 15:
			posArr = new double[3]{ double(x), double(y), 0 };
			constructionData = GenericFactoryConstructionData(3, posArr);
			HazardManager::pushCircleHazard("vanilla", "stationary_turret", constructionData);
			break;
		case 16:
			WallManager::pushWall(new Wall(x, y, 20, 60, LevelManager::getLevel(0)->getDefaultColor()));
			break;
		case 17:
			WallManager::pushWall(new Wall(x, y, 60, 20, LevelManager::getLevel(0)->getDefaultColor()));
			break;
		default:
			const std::pair<std::string, std::string>& powerIdentifier = insertList_extra[insertIndex - insertListIdentifiers_normalSize];
			PowerupManager::pushPowerup(new PowerSquare(x, y, powerIdentifier.first, powerIdentifier.second));
			break;
	}
}

std::vector<std::pair<std::string, std::string>> DeveloperManager::insertList_extra;
void DeveloperManager::addInsertPower(std::string identifier, std::string type, std::string name) {
	insertListIdentifiers.push_back(identifier);
	insertList_extra.push_back({ type, name });
}
