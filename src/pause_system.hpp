#pragma once
#include "common.hpp"
#include "render_system.hpp"
#include "ui_system.hpp"
#include "tiny_ecs_registry.hpp"
#include "scene_manager.hpp"

class PauseSystem
{
public:
	PauseSystem();

	// Releases all associated resources
	~PauseSystem();

	// initialize
	void init(RenderSystem* renderer_arg, GLFWwindow* window_arg);

	void on_mouse_button(GLFWwindow* window, int button, int action, int mods);
	void on_mouse_move(vec2 mouse_position);
	void PauseSystem::on_key(int key, int sc, int action, int mod);

private:
	RenderSystem* renderer;

	// Window handle
	GLFWwindow* window;

	vec2 cursor_position;

	bool is_mouse_within_button(WorldObject buttonObject);
};