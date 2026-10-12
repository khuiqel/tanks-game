#include "basic-ini-parser.h"

#include <stdexcept>
#include <algorithm> //std::remove
#include <fstream>
#include <iostream>

void BasicINIParser::BasicINIData::insert(std::string section, std::string property, std::string value) noexcept {
	if (exists(section, property)) {
		this->data[section][property].push_back(value);
	} else {
		this->data[section].insert({ property, std::vector<std::string>{value} });
	}
}

bool BasicINIParser::BasicINIData::exists(std::string section, std::string property) const noexcept {
	if (this->data.find(section) == this->data.end()) {
		return false;
	}
	if (this->data.at(section).find(property) == this->data.at(section).end()) {
		return false;
	}
	return true;
}

unsigned int BasicINIParser::BasicINIData::length(std::string section, std::string property) const noexcept {
	return this->data.at(section).at(property).size();
}

std::string BasicINIParser::BasicINIData::get(std::string section, std::string property) const {
	return this->data.at(section).at(property).at(0);
}

std::string BasicINIParser::BasicINIData::get(std::string section, std::string property, int index) const {
	return this->data.at(section).at(property).at(index);
}

void BasicINIParser::removeLeftWhitespace(std::string& str) noexcept {
	size_t space_pos = str.find_first_not_of(" \t");
	str.erase(0, space_pos); //no need to check space_pos != std::string::npos
}

void BasicINIParser::removeComments(std::string& str) noexcept {
	int comment_index = -1;
	for (int i = 0; i < str.size(); i++) {
		if (str[i] == '\\') {
			i++; //skip the next character
			continue;
		} else if (str[i] == ';' || str[i] == '#') {
			comment_index = i;
			break;
		}
	}
	if (comment_index != -1) {
		str.erase(comment_index);
	}
}

void BasicINIParser::removeRightWhitespace(std::string& str) noexcept {
	//this doesn't handle a backslash right before the space, but if a space is needed, use quotes
	size_t space_pos = str.find_last_not_of(" \t");
	str.erase(space_pos+1);
}

int BasicINIParser::findEndSectionIndex(const std::string& str) noexcept {
	bool found_section = false;
	int section_index;
	for (int i = 1; i < str.size(); i++) {
		if (str[i] == '\\') {
			i++;
			continue;
		}
		if (str[i] == ']') {
			section_index = i;
			found_section = true;
			break;
		}
	}
	if (found_section) {
		return section_index;
	}
	return -1;
}

int BasicINIParser::findSeparatorIndex(const std::string& str) noexcept {
	bool found_separator = false;
	int separator_index;
	for (int i = 1; i < str.size(); i++) {
		if (str[i] == '\\') {
			i++;
			continue;
		}
		if (str[i] == '=') {
			separator_index = i;
			found_separator = true;
			break;
		}
	}
	if (found_separator) {
		return separator_index;
	}
	return -1;
}

int BasicINIParser::findSubstringEndIndex(const std::string& str) noexcept {
	//assumes str[0] is part of the substring (basically, no whitespace on the left)
	int substr_end = str.size();
	if (str[0] == '\"') { //safe because operator[] takes a size_t (which is unsigned)
		//look until a \" is found to finish the substring
		for (int i = 1; i < str.size(); i++) {
			if (str[i] == '\\') {
				i++;
				continue;
			}
			if (str[i] == '\"') {
				substr_end = i;
				break;
			}
		}
	} else {
		//look until a \" or whitespace is found to finish the substring
		for (int i = 1; i < str.size(); i++) {
			if (str[i] == '\\') {
				i++;
				continue;
			}
			if (str[i] == '\"' || str[i] == ' ' || str[i] == '\t') {
				substr_end = i;
				break;
			}
		}
	}

	return substr_end;
}

void BasicINIParser::processEscapeSequences_most(std::string& str) noexcept {
	//escape sequences: \, ', ", =, ;, #
	if (str.size() == 0) {
		//handles str.size()-1 underflowing
		return;
	}
	for (int i = 0; i < str.size()-1; i++) {
		if (str[i] == '\\') {
			switch (str[i+1]) {
				case '\\':
					str.replace(i, 2, "\\");
					break;
				case '\'':
					str.replace(i, 2, "\'");
					break;
				case '\"':
					str.replace(i, 2, "\"");
					break;
				case '=':
					//shouldn't really need its own case; handled by default
					str.replace(i, 2, "=");
					break;
				case ';':
					str.replace(i, 2, ";");
					break;
				case '#':
					str.replace(i, 2, "#");
					break;
				default:
					//just leave the next character as is
					str.erase(str.begin() + i);
					break;
			}
		}
	}
}

void BasicINIParser::processEscapeSequences_all(std::string& str) noexcept {
	//escape sequences: \, ', ", =, ;, #, \t, \n
	if (str.size() == 0) {
		//handles str.size()-1 underflowing
		return;
	}
	for (int i = 0; i < str.size()-1; i++) {
		if (str[i] == '\\') {
			switch (str[i+1]) {
				case '\\':
					str.replace(i, 2, "\\");
					break;
				case '\'':
					str.replace(i, 2, "\'");
					break;
				case '\"':
					str.replace(i, 2, "\"");
					break;
				case '=':
					//shouldn't really need its own case; handled by default
					str.replace(i, 2, "=");
					break;
				case ';':
					str.replace(i, 2, ";");
					break;
				case '#':
					str.replace(i, 2, "#");
					break;
				case 't':
					str.replace(i, 2, "\t");
					break;
				case 'n':
					str.replace(i, 2, "\n");
					break;
				default:
					//just leave the next character as is
					str.erase(str.begin() + i);
					break;
			}
		}
	}
}

BasicINIParser::BasicINIData BasicINIParser::ReadFile(std::string path) {
	std::ifstream ini_file;
	ini_file.open(path);

	BasicINIData ini_data;

	if (ini_file.is_open()) {
		//where everything happens
		std::string line;
		std::string current_section = "";
		while (std::getline(ini_file, line)) {
			line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
			line.erase(std::remove(line.begin(), line.end(), '\n'), line.end());
			removeLeftWhitespace(line);
			removeComments(line);
			removeRightWhitespace(line);
			if (line.size() == 0) {
				continue;
			}

			if (line[0] == '[') { //section
				int section_index = findEndSectionIndex(line);
				if (section_index == -1) {
					//std::cerr << "bad section header; line: " << line << std::endl;
				} else {
					std::string section = line.substr(1, section_index-1);
					processEscapeSequences_most(section);
					current_section = section;
				}
			} else { //data
				int separator_index = findSeparatorIndex(line);
				if (separator_index == -1) {
					//std::cerr << "bad section header; line: " << line << std::endl;
				} else {
					std::string property = line.substr(0, separator_index);
					std::string value = line.substr(separator_index+1);
					removeRightWhitespace(property);
					processEscapeSequences_most(property);

					std::vector<std::string> value_list;
					while (value.size() != 0) {
						removeLeftWhitespace(value);
						int substr_end = findSubstringEndIndex(value);
						if (value[0] == '\"') {
							value_list.push_back(value.substr(1, substr_end-1));
							value.erase(0, substr_end+1);
						} else {
							value_list.push_back(value.substr(0, substr_end));
							value.erase(0, substr_end);
						}
					}

					for (int i = 0; i < value_list.size(); i++) {
						processEscapeSequences_all(value_list[i]);
						ini_data.insert(current_section, property, value_list[i]);
						//std::cout << "inserted {[" << current_section << "]," << property << "|" << value_list[i] << "}" << std::endl;
					}
				}
			}
		}
	} else {
		throw std::runtime_error("Could not read file \"" + path + "\"");
	}

	return ini_data;
}
