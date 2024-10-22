#pragma once

// internal
#include "common.hpp"
#include <render_system.hpp>

class UISystem
{
public:
	UISystem();

	// Releases all associated resources
	~UISystem();

	// intialize
	void init(RenderSystem* renderer_arg, GLFWwindow* window_arg, SCENE_TYPE* scene_arg);

	// init start menu
	void initMenuUI();

	Entity createPanel(SCENE_TYPE scene_type, vec2 pos, float angle, vec2 scale, std::string name, TEXTURE_ASSET_ID texture);
	Entity createUIElement(vec2 pos, vec2 scale, std::string element_name, float element_value, SCENE_TYPE scene_type);
	Entity createTexturedUIElement(vec2 pos, vec2 scale, std::string element_name, TEXTURE_ASSET_ID texture_id, SCENE_TYPE scene_type);

	Entity createButton(
		vec2 pos,
		vec2 scale,
		std::function<void()> action,
		std::string button_name,
		TEXTURE_ASSET_ID texture_id,
		SCENE_TYPE scene_type);

	void on_mouse_button(GLFWwindow* window, int button, int action, int mods);
	void on_mouse_move(vec2 mouse_position);

private:
	RenderSystem* renderer;

	// Window handle
	GLFWwindow* window;

	vec2 cursor_position;

	SCENE_TYPE* scene;

	Entity health_ui;
	Entity scrap_ui;
	Entity level_ui;
	Entity item_ui;

	bool is_mouse_within_button(WorldObject buttonObject);

	// close the start menu
	void closeMenuUI();

	// open help screen
	void initHelpUI();

	// close help screen
	void closeHelpUI();
};