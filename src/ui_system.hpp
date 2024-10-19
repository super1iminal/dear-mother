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
	void UISystem::init(RenderSystem* renderer_arg, GLFWwindow* window_arg);

	// init game UI
	void initGameUI(uint scrap, uint level);

	// init start menu
	void initMenuUI();

	void on_mouse_button(GLFWwindow* window, int button, int action, int mods);
	void on_mouse_move(vec2 mouse_position);

private:
	RenderSystem* renderer;

	// Window handle
	GLFWwindow* window;

	Entity health_ui;
	Entity scrap_ui;
	Entity level_ui;
	Entity item_ui;

	Entity createPanel(SCENE_TYPE scene_type, vec2 pos, float angle, vec2 scale, TEXTURE_ASSET_ID texture);
	Entity createUIElement(vec2 pos, vec2 scale, std::string element_name, float element_value, SCENE_TYPE scene_type);
	Entity createTexturedUIElement(vec2 pos, vec2 scale, std::string element_name, TEXTURE_ASSET_ID texture_id, SCENE_TYPE scene_type);

	Entity createButton(
		vec2 pos, 
		vec2 scale, 
		std::function<void()> action, 
		std::string button_name, 
		TEXTURE_ASSET_ID texture_id, 
		SCENE_TYPE scene_type);
};