#include "mod-processor.h"

#include <stdexcept>
#include <algorithm> //std::find, std::remove
#include <filesystem> //C++17
#include <fstream>
#include <iostream>
#include <vector>
#include <unordered_map> //could use a set, not a big deal

#include "game-manager.h" //INI file
#include "custom-power-interpreter.h"
#include "custom-level-interpreter.h"

const std::string ModProcessor::ModOrderPath = "mods/order.txt";
const std::string ModProcessor::IgnoreListPath = "mods/ignore.txt";

void ModProcessor::ProcessMods() noexcept {
	const BasicINIParser::BasicINIData& ini_data = GameManager::get_INI();
	if (ini_data.exists("MODS", "LoadMods") && std::stoi(ini_data.get("MODS", "LoadMods"))) {
		CustomPowerInterpreter::ProcessCustomPowers();
		CustomLevelInterpreter::ProcessCustomLevels();
	}
}

std::vector<std::string> ModProcessor::getListOfKnownMods() {
	std::ifstream modOrder_file;
	modOrder_file.open(ModOrderPath);

	std::vector<std::string> modOrder_list;

	if (modOrder_file.is_open()) {
		std::string line;
		while (std::getline(modOrder_file, line)) {
			line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
			line.erase(std::remove(line.begin(), line.end(), '\n'), line.end());
			modOrder_list.push_back(line);
		}
	} else {
		throw std::runtime_error("Could not read file \"" + ModOrderPath + "\"");
	}

	return modOrder_list;
}

std::vector<std::string> ModProcessor::getListOfUnknownMods() {
	std::vector<std::string> directory_list;

	for (const auto& dir_entry : std::filesystem::directory_iterator("mods", std::filesystem::directory_options::skip_permission_denied)) {
		if (std::filesystem::is_directory(dir_entry)) {
			directory_list.push_back(dir_entry.path().filename().string());
		}
		//can throw std::filesystem::filesystem_error
	}

	return directory_list;
}

std::vector<std::string> ModProcessor::getListOfIgnoredMods() {
	//copied from getListOfKnownMods()
	std::ifstream ignoreList_file;
	ignoreList_file.open(IgnoreListPath);

	std::vector<std::string> ignoreList_list;

	if (ignoreList_file.is_open()) {
		std::string line;
		while (std::getline(ignoreList_file, line)) {
			line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
			line.erase(std::remove(line.begin(), line.end(), '\n'), line.end());
			ignoreList_list.push_back(line);
		}
	} else {
		throw std::runtime_error("Could not read file \"" + IgnoreListPath + "\"");
	}

	return ignoreList_list;
}

std::optional<std::vector<std::string>> ModProcessor::getListOfMods() noexcept {
	std::vector<std::string> modOrder_list;
	std::unordered_map<std::string, bool> modOrder_set;

	try {
		std::vector<std::string> knownList = getListOfKnownMods();
		for (int i = 0; i < knownList.size(); i++) {
			if (modOrder_set.find(knownList[i]) == modOrder_set.end()) {
				modOrder_set.insert({ knownList[i], true });
				modOrder_list.push_back(knownList[i]);
			} else [[unlikely]] {
				std::cerr << "Item duplicated in " + ModOrderPath + ":" + std::to_string(i) + ", ignoring (" + knownList[i] + ")\n";
			}
		}
	}
	catch (const std::runtime_error& e) {
		std::cerr << "ModProcessor::getListOfUnknownMods() error: " << e.what() << std::endl;
		//return; //keep going
	}

	try {
		std::vector<std::string> unknownList = getListOfUnknownMods();
		for (int i = 0; i < unknownList.size(); i++) {
			if (modOrder_set.find(unknownList[i]) == modOrder_set.end()) {
				modOrder_set.insert({ unknownList[i], true });
				modOrder_list.push_back(unknownList[i]);
			} else {
				//std::cerr << "Item already in " + ModOrderPath + ":" + std::to_string(i) + ", ignoring (" + unknownList[i] + ")\n";
			}
		}
		//originally, this was going to push the "new" mods to the order.txt file, but that doesn't feel right
	}
	catch (const std::filesystem::filesystem_error& e) {
		std::cerr << "ModProcessor::getListOfUnknownMods() filesystem_error: " << e.what() << std::endl;
		return {}; //likely some major problem, so quit
	}

	try {
		std::vector<std::string> ignoreList = getListOfIgnoredMods();
		for (int i = 0; i < ignoreList.size(); i++) {
			if (modOrder_set.find(ignoreList[i]) != modOrder_set.end()) {
				modOrder_set.erase(ignoreList[i]);
				modOrder_list.erase(std::find(modOrder_list.begin(), modOrder_list.end(), ignoreList[i]));
			} else {
				std::cerr << "Item not found in " + IgnoreListPath + ":" + std::to_string(i) + ", skipping (" + ignoreList[i] + ")\n";
			}
		}
	}
	catch (const std::runtime_error& e) {
		std::cerr << "ModProcessor::getListOfIgnoredMods() error: " << e.what() << std::endl;
		//return; //it's fine
	}

	return modOrder_list;
}

std::vector<std::string> ModProcessor::getListOfFiles(std::string modPath) {
	//basically the same as getListOfUnknownMods()
	std::vector<std::string> file_list;

	for (const auto& dir_entry : std::filesystem::directory_iterator(modPath, std::filesystem::directory_options::skip_permission_denied)) {
		if (std::filesystem::is_regular_file(dir_entry)) {
			file_list.push_back(dir_entry.path().filename().string());
		}
	}

	return file_list;
}
