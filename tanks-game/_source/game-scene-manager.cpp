#include "game-scene-manager.h"

#include "window-initializer.h"
#include "renderer.h"
#include "frame-time-graph.h"

#include <tracy_include.h>

std::vector<std::pair<GameScene*, Scene_ID>> GameSceneManager::scenes;
int GameSceneManager::nextSceneID = 0;

void GameSceneManager::Initialize() {
	//nothing
}

void GameSceneManager::TickScenes() {
	ZoneScoped;
	/*
	auto start = FrameTimeGraph::getTime();
	*/

	for (int i = 0; i < scenes.size(); i++) {
		scenes[i].first->Tick();
	}
	DrawScenes();

	/*
	auto end = FrameTimeGraph::getTime();
	auto timeTakenMS = FrameTimeGraph::getDiff(start, end);
	auto sleepTimeMS = (100.0 - timeTakenMS);
	*/

	FrameMark;
}

void GameSceneManager::DrawScenes() {
	WindowInitializer::BeginningStuff();
	Renderer::Clear();
	for (int i = 0; i < scenes.size(); i++) {
		scenes[i].first->Draw();
	}
	Renderer::Flush();
}

void GameSceneManager::DrawScenes_WindowResize() {
	//WindowInitializer::BeginningStuff(); //this is the only difference
	Renderer::Clear();
	for (int i = 0; i < scenes.size(); i++) {
		scenes[i].first->Draw();
	}
	Renderer::Flush();
}

GameScene* GameSceneManager::getScene(unsigned int index) {
	return scenes[index].first;
}

GameScene* GameSceneManager::getSceneByID(Scene_ID sceneID) {
	for (int i = 0; i < scenes.size(); i++) {
		if (scenes[i].second == sceneID) {
			return scenes[i].first;
		}
	}
	return nullptr;
}

void GameSceneManager::pushScene(GameScene* gs) {
	scenes.push_back({ gs, ++nextSceneID });
}

void GameSceneManager::deleteScene(unsigned int index) {
	delete scenes[index].first;
	scenes.erase(scenes.begin() + index);
}

void GameSceneManager::deleteSceneByID(Scene_ID sceneID) {
	for (int i = 0; i < scenes.size(); i++) {
		if (scenes[i].second == sceneID) {
			deleteScene(i);
			break;
		}
	}
}

/*
Scene_ID GameSceneManager::getNextID() {
	nextSceneID++;
	return nextSceneID;
}
*/
