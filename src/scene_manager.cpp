#include "scene_manager.hpp"
#include "fstream"

SceneManager::SceneManager() : current_scene(SCENE_TYPE::MENU), previous_scene(SCENE_TYPE::MENU), just_changed(false)
{
}

SceneManager::~SceneManager()
{
}

void SceneManager::set_scene(SCENE_TYPE scene)
{
	printf("setting scene\n");
	previous_scene = current_scene;
	current_scene = scene;
	just_changed = true;
	return;
}

SceneManager scene_manager;