#pragma once


// WHEN ADDING A SCENE:
// 1. Add the scene to the SCENE_TYPE enum
// 2. Add a new component for the scene and add it to the registry
// 3. Wherever entities are created (currently world_init and ui_system), add the scene in the switch statement
// 4. Add a new system for the scene
// 5. Add the scene to the switch statement in GameManager::on_mouse_move
// 6. Add the scene to the switch statement in GameManager::on_mouse_button
// 7. Add the scene to the switch statement in GameManager::on_key
// 8. Add the scene to the switch statement in GameManager::init
// there may be more steps I don't remember

enum SCENE_TYPE {
	GAME = 0,
	MENU = GAME + 1,
	HELP = MENU + 1,
	PAUSE = HELP + 1,
	TEST = PAUSE + 1,
	SCENE_COUNT = TEST + 1
};

class SceneManager
{
private:
	SCENE_TYPE previous_scene;
	SCENE_TYPE current_scene;
	bool just_changed; // used in game manager for scene transitions

public:
	SceneManager();
	// Releases all associated resources
	~SceneManager();

	void set_scene(SCENE_TYPE scene);
	SCENE_TYPE get_scene() { return current_scene; };
	bool has_just_changed() { return just_changed; }
	void set_just_changed(bool changed) { just_changed = changed; }
	SCENE_TYPE get_previous_scene() { return previous_scene; }
	
};

extern SceneManager scene_manager;
