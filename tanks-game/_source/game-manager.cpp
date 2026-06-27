#include "game-manager.h"

#include "mylib.h"
#include <fstream>
#include <filesystem>
#include <stdexcept>
#include <iostream>

#include <tracy_include.h>

std::vector<GameThing*> GameManager::objectsInGame;
size_t GameManager::objectsInGame_previousSize = 0;

void GameManager::initializeObjectList() {
	//nothing
}

GameThing* GameManager::getObject(unsigned int index) {
	return objectsInGame[index];
}

GameThing* GameManager::getObjectByID(Game_ID gameID) {
	for (int i = 0; i < objectsInGame.size(); i++) {
		if (objectsInGame[i]->getGameID() == gameID) {
			return objectsInGame[i];
		}
	}
	return nullptr;
}

void GameManager::pushObject(GameThing* g) {
	//TODO?
	objectsInGame.push_back(g);
}

void GameManager::deleteObject(unsigned int index) {
	//TODO?
	objectsInGame.erase(objectsInGame.begin() + index);
}

void GameManager::deleteObjectByID(Game_ID gameID) {
	for (int i = 0; i < objectsInGame.size(); i++) {
		if (objectsInGame[i]->getGameID() == gameID) {
			deleteObject(i);
			break;
		}
	}
}

void GameManager::clearObjects() {
	//TODO?
	objectsInGame.clear();
	objectsInGame_previousSize = 0;
}

void GameManager::updateEveryAABB() {
	ZoneScoped;
	for (int i = 0; i < objectsInGame.size(); i++) {
		objectsInGame[i]->updateAABB();
	}

	static const auto sortFunc = [](const GameThing* lhs, const GameThing* rhs) { return lhs->get_xStart() < rhs->get_xStart(); };
	constexpr size_t extraObjectSortNum = 63; //arbitrarily chosen
	if (objectsInGame.size() > objectsInGame_previousSize + extraObjectSortNum) [[unlikely]] {
		//"significant" amount of new objects; technically the InsertionSort can sort new objects due to deletions not being tracked, but it's fine
		//(if 1 object is removed and 100 are added, this check thinks there is 99 new objects, meaning the insertion sort would include 1 new object while quicksort works on 99)

		InsertionSort(objectsInGame.begin(), objectsInGame.begin() + objectsInGame_previousSize, sortFunc);
		std::sort(objectsInGame.begin() + objectsInGame_previousSize, objectsInGame.end(), sortFunc);

		//the merge takes about as long as the quicksort, however this is still faster than insertion sort on the whole thing
		std::inplace_merge(objectsInGame.begin(), objectsInGame.begin() + objectsInGame_previousSize, objectsInGame.end(), sortFunc);
	} else {
		//"regular" amount of new objects (which could be negative)
		InsertionSort(objectsInGame.begin(), objectsInGame.end(), sortFunc);
	}

	objectsInGame_previousSize = objectsInGame.size();
}

Game_ID GameManager::nextID = -1;
double GameManager::tickCount = 0;
BasicINIParser::BasicINIData GameManager::INI_file;
GameSettings GameManager::settings_file;

void GameManager::Initialize() {
	//does not initialize the INI or the settings
}

void GameManager::initializeINI(std::string path) {
	try {
		INI_file = BasicINIParser::ReadFile(path);
	}
	catch (const std::runtime_error& e) {
		std::cerr << e.what() << std::endl;
	}
	catch (const std::exception& e) {
		std::cerr << "Unexpected error: " << e.what() << std::endl;
		return;
	}
}

void GameManager::initializeSettings() {
	settings_file.Initialize(INI_file);
}

const BasicINIParser::BasicINIData& GameManager::get_INI() {
	return INI_file;
}
const GameSettings& GameManager::get_settings() {
	return settings_file;
}

void GameManager::Tick() {
	tickCount++;
	//this is probably all it's ever going to do
}

void GameManager::Reset() {
	tickCount = 0;
}

Game_ID GameManager::getNextID() {
	nextID++;
	return nextID;
}

double GameManager::getTickCount() {
	return tickCount;
}

void GameManager::CreateDefaultINIFileIfNeeded(std::string path) {
	if (std::filesystem::exists(path)) {
		return;
	}

	//note: this uses LF as the line ending while CRLF is the default (to make sure the parsers handle that case)
	//can't be bothered to change it

	static const std::string ini_string =

	"[UNIVERSAL]\n"
	"; options: OpenGL, software (not supported), NULL (not supported)\n"
	"GraphicsContext = OpenGL\n"
	"; blank = random\n"
	"RNGSeed = \n"
	"; number of total threads to use for multithreading; 1 is single-threaded, <=0 is all cores on your system minus the number (so -2 is all but two cores)\n"
	"ThreadCount = 4\n"
	"; task size for each thread\n"
	"ThreadTaskSize = 256\n"
	"\n"

	"[MODS]\n"
	"LoadMods = 1\n"
	"ModSafetyChecks = 1 ; forces checks on mods' types (for example, \"vanilla\" is reserved thanks to this setting)\n"
	"\n"

	"[DEBUG]\n"
	"DevMouseControls = 0\n"
	"PerformanceGraphEnable = 0\n"
	"PerformanceGraphOffsetMultiplier = 0.0 ; range: [0,1]\n"
	"EnableDebugDrawing = 0\n"
	"EnableDebugDrawingObjects = \"patrolling_turret\" \"mother_turret\" \"ginormous_turret\"\n"
	"DeveloperInsertMenuPowerAdditions = \"special bounce\" \"dev\" \"ultrabounce\" \"different triple\" \"dev\" \"triple_spread\" \"testpower2\" \"testmod1\" \"test-power-2\"\n"
	"\n"

	"[GRAPHICS_SETTINGS]\n"
	"Position.StartX = 120\n"
	"Position.StartY = 120\n"
	"Position.SizeMultiplier = ;2.5\n"
	"Bullet.DrawAlpha = 0 ; draws transparency rather than a circle outline\n"
	"\n"

	"[GAME_OPTIONS]\n"
	"GameFirstLevel = \"vanilla\" \"default_random\"\n"
	"GameLevelPlaylist = \"random-vanilla\"\n"
	"GameForceSameLevel = ;\"vanilla\" \"default_random\"\n"
	"CustomLevelPlaylist = ;\"vanilla\" \"bigfun\" 0.5 \"random-vanilla\" \"default_random\" 1.0 \"dev\" \"dev0\" 0.25 ; overrides GameLevelPlaylist if set\n"
	"ReportCurrentLevel = 1 ; prints the level being played\n"
	"; global config\n"
	"LimitBullets = 1\n"
	"MaxBullets = 8192\n"
	"FewerExtraShootingBullets = 1 ; determines whether something like triple + fire shoots 7 or 12 bullets\n"
	"ShootingCooldown = 100\n"
	"PowerupDurationBaseTime = 500\n"
	"; other options\n"
	"DisableTraps = 1 ; does nothing since traps don't exist\n"
	"RestrictTankTurning = 0\n"
	"NoTankAcceleration = 0\n"
	"AlwaysShootingMode = 0\n"
	"\n"

	"[CONTROLS]\n"
	"; names\n"
	"Tank1.Name = WASD\n"
	"Tank2.Name = Arrow Keys\n"
	"Tank1.TeamName = \"\\\"WASD 4 Life\\\"\"\n"
	"Tank2.TeamName = \"\\\"Arrow Keys R WINZ\\\"\"\n"
	"; actual controls, key names from AHK: https://www.autohotkey.com/docs/commands/Send.htm#keynames\n"
	"Tank1.Forward = w\n"
	"Tank1.Left = a\n"
	"Tank1.Right = d\n"
	"Tank1.Shoot = s\n"
	"Tank1.Special = e\n"
	"Tank2.Forward = Up\n"
	"Tank2.Left = Left\n"
	"Tank2.Right = Right\n"
	"Tank2.Shoot = Down\n"
	"Tank2.Special = /\n"
	;

	std::ofstream ini_file;
	ini_file.open(path);
	if (ini_file.is_open()) {
		ini_file << ini_string;
		ini_file.close();
	} else {
		throw std::runtime_error("Could not create file \"" + path + "\"");
	}
}
