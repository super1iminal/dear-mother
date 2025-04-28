#pragma once

// internal
#include "common.hpp"
#include <render_system.hpp>

// constants
const float DIALOGUE_BOX_MARGINS = 12.5f;

const float DIALOGUE_TEXT_SCALE = 2.f;
const float POPUP_TEXT_SCALE = 1.5f;

class UISystem
{
public:
	UISystem();

	// Releases all associated resources
	~UISystem();

	// intialize
	void init(GLFWwindow* window_arg);

	static Entity createPanel(
		RenderSystem* renderer,
		SCENE_TYPE scene_type,
		vec2 pos,
		float angle,
		vec2 scale,
		std::string name,
		TEXTURE_ASSET_ID texture);

	static Entity createSquareUIElement(
		RenderSystem* renderer,
		vec2 pos,
		vec2 scale,
		std::string element_name,
		SCENE_TYPE scene_type);

	static Entity createTextUIElement(
		RenderSystem* renderer,
		vec2 pos,
		vec2 scale,
		std::string element_name,
		std::string element_value,
		vec3 color,
		SCENE_TYPE scene_type);

	static Entity createTextPopUp(
		RenderSystem* renderer,
		vec2 pos,
		vec2 scale,
		std::string element_name,
		std::string element_value,
		vec3 color,
		SCENE_TYPE scene_type);

	static Entity createTextPopUpAsher(RenderSystem* renderer, 
		vec2 pos, 
		vec2 size, 
		vec3 color, 
		SCENE_TYPE scene_type, 
		std::string text,
		Entity parent,
		float lifetime);

	static Entity createTexturedUIElement(
		RenderSystem* renderer,
		vec2 pos,
		vec2 scale,
		std::string element_name,
		TEXTURE_ASSET_ID texture_id,
		SCENE_TYPE scene_type);

	static Entity UISystem::createButton(
		RenderSystem* renderer,
		vec2 pos,
		vec2 scale,
		std::function<void()> action,
		std::string button_name,
		TEXTURE_ASSET_ID texture_id,
		TEXTURE_ASSET_ID hover_texture_id,
		SCENE_TYPE scene_type);

	static Entity createTextButton(
		RenderSystem* renderer,
		vec2 pos,
		vec2 scale,
		std::function<void()> action,
		std::string button_name,
		std::string text,
		vec3 color,
		SCENE_TYPE scene_type);

	static Entity createCrosshair(RenderSystem* renderer, TEXTURE_ASSET_ID texture, SCENE_TYPE scene_type);
	static Entity createTextBox(RenderSystem* renderer, vec2 pos, vec2 size, vec3 color, SCENE_TYPE scene_type, std::string text, TEXT_BOX_TYPE text_box_type = TEXT_BOX_TYPE::DEFAULT, float text_size = DIALOGUE_TEXT_SCALE);
private:
	// Window handle
	GLFWwindow* window;

	vec2 cursor_position;
};