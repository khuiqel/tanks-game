#pragma once
#include <string>
#include <utility>
#include <vector>

#include "circle.h"
#include "rect.h"

#include <glad/gl.h>
#include <GLFW/glfw3.h>

class DeveloperManager { //interpret as "developer testing manager" or "developer menu manager"
private:
	static bool leftMouse;
	static bool rightMouse;
	static int mousePosX;
	static int mousePosY;

	static int insertIndex;
	static std::vector<std::string> insertListIdentifiers;
	static int insertListIdentifiers_normalSize; //before addInsertPower()
	static std::vector<std::pair<std::string, std::string>> insertList_extra; //before addInsertPower()
	static void devInsert(int x, int y);

public:
	static void setX(Circle& c, double x) { c.x = x; }
	static void setY(Circle& c, double y) { c.y = y; }
	static void setR(Circle& c, double r) { c.r = r; }

	static void setX(Rect& r, double x) { r.x = x; }
	static void setY(Rect& r, double y) { r.y = y; }
	static void setW(Rect& r, double w) { r.w = w; }
	static void setH(Rect& r, double h) { r.h = h; }

	static void mouseButtonCallbackFunc(GLFWwindow* window, int button, int action, int mods);
	static void mouseScrollCallbackFunc(GLFWwindow* window, double xOffset, double yOffset);
	static void mouseCursorPosCallbackFunc(GLFWwindow* window, double xPos, double yPos);

	static void addInsertPower(std::string identifier, std::string type, std::string name);

private:
	DeveloperManager() = delete;
	DeveloperManager(const DeveloperManager&) = delete;
};
