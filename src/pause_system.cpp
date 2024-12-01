#include "pause_system.hpp"

#include "reloadability_system.hpp"

PauseSystem::PauseSystem()
{

}

PauseSystem::~PauseSystem() {

}

void PauseSystem::init(RenderSystem* renderer_arg, GLFWwindow* window_arg) {
	this->renderer = renderer_arg;
	this->window = window_arg;
	UISystem::createPanel(
		renderer,
		SCENE_TYPE::PAUSE,
		vec2(window_width_px / 2, window_height_px / 2),
		0.f,
		vec2(window_width_px, window_height_px),
		"help_screen",
		TEXTURE_ASSET_ID::START_MENU
	);
	UISystem::createButton(
		renderer,
		vec2(window_width_px / 2, window_height_px / 2),
		vec2(129.f, 36.f),
		[&]() {
			ReloadabilitySystem::saveGame();
			scene_manager.set_scene(SCENE_TYPE::MENU);
		},
		"save_and_quit_button",
		TEXTURE_ASSET_ID::SAVE_BUTTON,
		TEXTURE_ASSET_ID::SAVE_BUTTON_HOVER,
		SCENE_TYPE::PAUSE
	);

	UISystem::createButton(
		renderer,
		vec2(window_width_px / 2, window_height_px / 2 - 82.f),
		vec2(195.f, 42.f),
		[&]() {
			printf("return to game button presssed\n");
			scene_manager.set_scene(SCENE_TYPE::GAME);
		},
		"return_to_game_button",
		TEXTURE_ASSET_ID::RESUME_BUTTON,
		TEXTURE_ASSET_ID::RESUME_BUTTON_HOVER,
		SCENE_TYPE::PAUSE
	);

	UISystem::createButton(
		renderer,
		vec2(window_width_px / 2, window_height_px / 2 + 42.f),
		vec2(276.f, 48.f),
		[&]() {
			printf("return to menu button pressed\n");
			scene_manager.set_scene(SCENE_TYPE::MENU);
		},
		"return_to_menu_button",
		TEXTURE_ASSET_ID::MENU_BUTTON,
		TEXTURE_ASSET_ID::MENU_BUTTON_HOVER,
		SCENE_TYPE::PAUSE
	);

	UISystem::createButton(
		renderer,
		vec2(window_width_px / 2, window_height_px / 2 - 42.f),
		vec2(90.f, 44.f),
		[&]() {
			std::cout << "Help button pressed!" << std::endl;
			scene_manager.set_scene(SCENE_TYPE::HELP);
		},
		"help_button",
		TEXTURE_ASSET_ID::HELP_BUTTON,
		TEXTURE_ASSET_ID::HELP_BUTTON_HOVER,
		SCENE_TYPE::PAUSE
	);

	crosshair = UISystem::createCrosshair(renderer, TEXTURE_ASSET_ID::MENU_CROSSHAIR, SCENE_TYPE::PAUSE);

	// pause music
	pause_music = Mix_LoadMUS(audio_path("Alex Roe - Darksign - 01 Demons from the Dark-compressed.wav").c_str());
	if (pause_music == nullptr) {
		fprintf(stderr, "Failed to load pause music. Error: %s\n", Mix_GetError());
		exit(1);
	}
}

void PauseSystem::update_music() {
	Mix_VolumeMusic(8);
	Mix_FadeInMusic(pause_music, -1, 2000);
}

void PauseSystem::on_key(int key, int sc, int action, int mod) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		scene_manager.set_scene(SCENE_TYPE::GAME);
	}
}

void PauseSystem::on_mouse_button(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		// if mouse position is within button boundaries
		// activate the button's action function
		auto& uiButtonsRegistry = registry.pauseSceneButtons.entities;
		for (Entity buttonEntity : uiButtonsRegistry) {
			UIButton button = registry.uiButtons.get(buttonEntity);
			WorldObject buttonObject = registry.worldObjects.get(buttonEntity);
			if (is_mouse_within_button(buttonObject)) {
				button.action();
			}
		}
	}
}

bool PauseSystem::is_mouse_within_button(WorldObject buttonObject)
{
	bool passesX = (cursor_position.x > buttonObject.position.x - (buttonObject.scale.x / 2)
		&& cursor_position.x < buttonObject.position.x + (buttonObject.scale.x / 2));
	bool passesY = (cursor_position.y > buttonObject.position.y - (buttonObject.scale.y / 2)
		&& cursor_position.y < buttonObject.position.y + (buttonObject.scale.y / 2));
	return passesX && passesY;
}

void PauseSystem::on_mouse_move(vec2 mouse_position) {
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	cursor_position = vec2(xpos, ypos);

	auto& uiButtonsRegistry = registry.pauseSceneButtons.entities;
	RenderRequest& crosshair_render_request = registry.renderRequests.get(crosshair);
	bool is_any_button_hovered = false;
	for (Entity buttonEntity : uiButtonsRegistry) {
		UIButton& button = registry.uiButtons.get(buttonEntity);
		WorldObject& buttonObject = registry.worldObjects.get(buttonEntity);
		RenderRequest& button_render_request = registry.renderRequests.get(buttonEntity);
		bool mouse_within_button = is_mouse_within_button(buttonObject);
		if (mouse_within_button) {
			is_any_button_hovered = true;

			if (!button.is_hovered) {
				button.is_hovered = true;
				button_render_request.used_texture = button.hover_texture;	// change the texture
				buttonObject.scale.x = buttonObject.scale.x * 1.16;
				buttonObject.position.x = buttonObject.position.x - 10.f;
			}
		}
		else {
			if (button.is_hovered) {
				button.is_hovered = false;
				button_render_request.used_texture = button.base_texture;	// change the texture
				buttonObject.scale.x = buttonObject.scale.x * 0.86;
				buttonObject.position.x = buttonObject.position.x + 10.f;
			}
		}
	}

	crosshair_render_request.used_texture = is_any_button_hovered ? TEXTURE_ASSET_ID::MENU_HOVER_CROSSHAIR
		: TEXTURE_ASSET_ID::MENU_CROSSHAIR;
}