#pragma once
#include "render_system.hpp"
#include "ui_system.hpp"
#include "tiny_ecs_registry.hpp"
#include "scene_manager.hpp"

class HelpSystem {
public:
	HelpSystem();

	// Releases all associated resources
	~HelpSystem();

	// initialize
	void init(RenderSystem* renderer_arg, GLFWwindow* window_arg);

	void on_mouse_button(GLFWwindow* window, int button, int action, int mods);
	void on_mouse_move(vec2 mouse_position);
private:
	RenderSystem* renderer;

	// Window handle
	GLFWwindow* window;

	vec2 cursor_position;

	SCENE_TYPE* scene;

	bool is_mouse_within_button(WorldObject buttonObject);
};