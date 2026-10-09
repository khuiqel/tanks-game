#include "game-settings.h"

#include "constants.h"

#include <stdexcept>
#include <iostream>

void GameSettings::Initialize(const BasicINIParser::BasicINIData& ini_data) {
	//[DEBUG]
	DevMouseControls = ini_data.exists("DEBUG", "DevMouseControls") ? std::stoi(ini_data.get("DEBUG", "DevMouseControls")) : false;
	PerformanceGraphEnable = ini_data.exists("DEBUG", "PerformanceGraphEnable") ? std::stoi(ini_data.get("DEBUG", "PerformanceGraphEnable")) : false;
	EnableDebugDrawing = ini_data.exists("DEBUG", "EnableDebugDrawing") ? std::stoi(ini_data.get("DEBUG", "EnableDebugDrawing")) : false;

	//[GRAPHICS_SETTINGS]
	Bullet_DrawAlpha = ini_data.exists("GRAPHICS_SETTINGS", "Bullet.DrawAlpha") ? std::stoi(ini_data.get("GRAPHICS_SETTINGS", "Bullet.DrawAlpha")) : false;

	//[GAME_OPTIONS]
	GameLevelPlaylist = ini_data.exists("GAME_OPTIONS", "GameLevelPlaylist")
		? ini_data.get("GAME_OPTIONS", "GameLevelPlaylist") : "random-vanilla";
	if (ini_data.exists("GAME_OPTIONS", "GameForceSameLevel") && ini_data.length("GAME_OPTIONS", "GameForceSameLevel") >= 2) {
		GameForceSameLevel = true;
		GameForceSameLevel_identifier = { ini_data.get("GAME_OPTIONS", "GameForceSameLevel", 0), ini_data.get("GAME_OPTIONS", "GameForceSameLevel", 1) };
	} else {
		GameForceSameLevel = false;
		GameForceSameLevel_identifier = { "", "" };
	}
	if (ini_data.exists("GAME_OPTIONS", "CustomLevelPlaylist")) {
		CustomLevelPlaylist = {};
		bool onlyZeroWeights = true;
		for (int i = 0; i < ini_data.length("GAME_OPTIONS", "CustomLevelPlaylist") / 3; i++) {
			float weight;
			try {
				weight = std::max(0.0f, std::stof(ini_data.get("GAME_OPTIONS", "CustomLevelPlaylist", i*3+2)));
			}
			catch (const std::exception&) {
				std::cerr << "Could not parse CustomLevelPlaylist weight \"" + ini_data.get("GAME_OPTIONS", "CustomLevelPlaylist", i*3+2) + "\", cancelling...\n\n";
				CustomLevelPlaylist.clear();
				break;
			}
			CustomLevelPlaylist.push_back({{ ini_data.get("GAME_OPTIONS", "CustomLevelPlaylist", i*3),
			                                 ini_data.get("GAME_OPTIONS", "CustomLevelPlaylist", i*3+1) },
			                                 weight });
			if (weight > 0) {
				onlyZeroWeights = false;
			}
		}
		if (!CustomLevelPlaylist.empty() && onlyZeroWeights) {
			std::cerr << "CustomLevelPlaylist only has zero weights, cancelling...\n\n";
			CustomLevelPlaylist.clear();
		}
	} else {
		CustomLevelPlaylist = {};
	}
	ReportCurrentLevel = ini_data.exists("GAME_OPTIONS", "ReportCurrentLevel") ? std::stoi(ini_data.get("GAME_OPTIONS", "ReportCurrentLevel")) : true;

	LimitBullets = ini_data.exists("GAME_OPTIONS", "LimitBullets") ? std::stoi(ini_data.get("GAME_OPTIONS", "LimitBullets")) : true;
	MaxBullets = ini_data.exists("GAME_OPTIONS", "MaxBullets")
		? std::stoi(ini_data.get("GAME_OPTIONS", "MaxBullets")) : -1; //-1 does not mean infinite, it means use the default in BulletManager
	FewerExtraShootingBullets = ini_data.exists("GAME_OPTIONS", "FewerExtraShootingBullets") ? std::stoi(ini_data.get("GAME_OPTIONS", "FewerExtraShootingBullets")) : false;
	ShootingCooldown = ini_data.exists("GAME_OPTIONS", "ShootingCooldown")
		? std::stod(ini_data.get("GAME_OPTIONS", "ShootingCooldown")) : SHOOT_COOLDOWN;
	PowerupDurationBaseTime = ini_data.exists("GAME_OPTIONS", "PowerupDurationBaseTime")
		? std::stod(ini_data.get("GAME_OPTIONS", "PowerupDurationBaseTime")) : POWER_COOLDOWN;

	RestrictTankTurning = ini_data.exists("GAME_OPTIONS", "RestrictTankTurning") ? std::stoi(ini_data.get("GAME_OPTIONS", "RestrictTankTurning")) : false;
	NoTankAcceleration = ini_data.exists("GAME_OPTIONS", "NoTankAcceleration") ? std::stoi(ini_data.get("GAME_OPTIONS", "NoTankAcceleration")) : false;
	AlwaysShootingMode = ini_data.exists("GAME_OPTIONS", "AlwaysShootingMode") ? std::stoi(ini_data.get("GAME_OPTIONS", "AlwaysShootingMode")) : false;

	//[SPECIFIC_ADJUSTMENTS]
	//BananaPower_ExplodeCount;
}

GameSettings::GameSettings() {
	//nothing?
}
