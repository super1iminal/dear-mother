#pragma once
// include literally everything
#include "common.hpp"
#include "render_system.hpp"
#include "tiny_ecs_registry.hpp"
#include "tiny_ecs.hpp"

// game stuff
#include "world_system.hpp"
#include "ai_system.hpp"
#include "physics_system.hpp"
#include "collision_system.hpp"
#include <reloadability_system.hpp>

// WHEN ADDING A SCENE:
// 1. Add the scene to the SCENE_TYPE enum
// 2. Add a new component for the scene and add it to the registry
// 3. Wherever entities are created (currently world_init and ui_system), add the scene in the switch statement
// 4. Add a new system for the scene
// 5. Add the scene to the switch statement in GameManager::on_mouse_move
// 6. Add the scene to the switch statement in GameManager::on_mouse_button
// 7. Add the scene to the switch statement in GameManager::on_key
// 8. Add the scene to the switch statement in GameManager::init
// 9. Add the scene to switch statements in UISystem
// there may be more steps I don't remember
// 10. add the scene to the switch statement in RenderSystem (render_system.cpp)

class Scene;

class SceneManager
{
private:
	std::unique_ptr<Scene> current_scene; // pointer to the current scene object
	GLFWwindow* window; // pointer to the GLFW window

public:
	SceneManager(GLFWwindow* gl_window);
	~SceneManager();

	void set_scene(std::unique_ptr<Scene> scene);

    // scene actions
	void step(float elapsed_ms);
	void render(float elapsed_ms);
	void on_key(int key, int sc, int action, int mod);
	void on_mouse_button(GLFWwindow* window, int button, int action, int mods);
	void on_mouse_move(vec2 mouse_position);
};

extern SceneManager scene_manager;
