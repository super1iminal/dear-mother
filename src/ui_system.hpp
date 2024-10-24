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

	Entity createCrosshair(RenderSystem* renderer, TEXTURE_ASSET_ID texture, SCENE_TYPE scene_type);

private:
	RenderSystem* renderer;

	// Window handle
	GLFWwindow* window;

	vec2 cursor_position;

	SCENE_TYPE* scene;
};