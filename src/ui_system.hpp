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
	void init(GLFWwindow* window_arg, SCENE_TYPE* scene_arg);

	static Entity createPanel(
		RenderSystem* renderer,
		SCENE_TYPE scene_type,
		vec2 pos,
		float angle,
		vec2 scale,
		std::string name,
		TEXTURE_ASSET_ID texture);

	static Entity 
		createUIElement(
			RenderSystem* renderer,
			vec2 pos,
			vec2 scale,
			std::string element_name,
			int element_value,
			SCENE_TYPE scene_type);

	static Entity createTexturedUIElement(
			RenderSystem* renderer,
			vec2 pos,
			vec2 scale,
			std::string element_name,
			TEXTURE_ASSET_ID texture_id,
			SCENE_TYPE scene_type);

	static Entity createButton(
		RenderSystem* renderer,
		vec2 pos,
		vec2 scale,
		std::function<void()> action,
		std::string button_name,
		TEXTURE_ASSET_ID texture_id,
		SCENE_TYPE scene_type);

	static Entity createCrosshair(RenderSystem* renderer, TEXTURE_ASSET_ID texture, SCENE_TYPE scene_type);

private:
	// Window handle
	GLFWwindow* window;

	vec2 cursor_position;

	SCENE_TYPE* scene;
};