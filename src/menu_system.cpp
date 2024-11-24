#include "menu_system.hpp"
#include "reloadability_system.hpp"


MenuSystem::MenuSystem()
{

}

MenuSystem::~MenuSystem() {

}

void MenuSystem::init(RenderSystem* renderer_arg, GLFWwindow* window_arg) {
	this->renderer = renderer_arg;
	this->window = window_arg;
	Entity base_ui = UISystem::createPanel(
		renderer,
		SCENE_TYPE::MENU,
		vec2(window_width_px / 2, window_height_px / 2),
		0.f,
		vec2(window_width_px, window_height_px),
		"start_menu",
		TEXTURE_ASSET_ID::START_MENU
	);

	UISystem::createButton(
		renderer,
		vec2(window_width_px / 2, 428.f),
		vec2(243.f, 39.f),
		[&]() {

			scene_manager.set_scene(SCENE_TYPE::GAME);
		},
		"run_button",
		TEXTURE_ASSET_ID::RUN_BUTTON,
		SCENE_TYPE::MENU
	);

	UISystem::createButton(
		renderer,
		vec2(window_width_px / 2, 486.f),
		vec2(243.f, 47.f),
		[&]() {
			std::cout << "Loading game" << std::endl;
		if (registry.gameLoadingHelper.size() > 0) {
			registry.gameLoadingHelper.components[0].savedGame = true;
		} else {
			auto entity = Entity();
			GameLoadingHelper& option = registry.gameLoadingHelper.emplace(entity);
			option.savedGame = true;
		}
			scene_manager.set_scene(SCENE_TYPE::GAME);
		},
		"continue_button",
		TEXTURE_ASSET_ID::CONTINUE_BUTTON,
		SCENE_TYPE::MENU
	);


	UISystem::createButton(
		renderer,
		vec2(window_width_px / 2, 543.f),
		vec2(135.f, 66.f),
		[&]() {
			std::cout << "Help button pressed!" << std::endl;
			scene_manager.set_scene(SCENE_TYPE::HELP);
		},
		"help_button",
		TEXTURE_ASSET_ID::HELP_BUTTON,
		SCENE_TYPE::MENU
	);

	UISystem::createButton(
		renderer,
		vec2(window_width_px / 2, 606.f),
		vec2(248.f, 66.f),
		[&]() {
			std::cout << "Upgrades button pressed!" << std::endl;
			scene_manager.set_scene(SCENE_TYPE::SHOP);
		},
		"shop_button",
		TEXTURE_ASSET_ID::SHOP_BUTTON,
		SCENE_TYPE::MENU
	);

	UISystem::createButton(
		renderer,
		vec2(window_width_px / 2, 666.f),
		vec2(135.f, 66.f),
		[&]() {
			std::cout << "Quit button pressed!" << std::endl;
			glfwSetWindowShouldClose(window, GLFW_TRUE);
		},
		"quit_button",
		TEXTURE_ASSET_ID::QUIT_BUTTON,
		SCENE_TYPE::MENU
	);

	UISystem::createCrosshair(renderer, TEXTURE_ASSET_ID::MENU_CROSSHAIR, SCENE_TYPE::MENU);

	// menu music
	menu_music = Mix_LoadMUS(audio_path("Alex Roe - Darksign - 14 Vereor Nox-compressed.wav").c_str());
	if (menu_music == nullptr) {
		fprintf(stderr, "Failed to load pause music. Error: %s\n", Mix_GetError());
		exit(1);
	}
}

void MenuSystem::update_music() {
	Mix_VolumeMusic(8);
	Mix_FadeInMusic(menu_music, -1, 2000);
}

void MenuSystem::on_mouse_button(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		// if mouse position is within button boundaries
		// activate the button's action function
		auto& uiButtonsRegistry = registry.menuSceneButtons.entities;
		for (Entity buttonEntity : uiButtonsRegistry) {
			UIButton button = registry.uiButtons.get(buttonEntity);
			WorldObject buttonObject = registry.worldObjects.get(buttonEntity);
			if (is_mouse_within_button(buttonObject)) {
				button.action();
			}
		}
	}
}

bool MenuSystem::is_mouse_within_button(WorldObject buttonObject)
{
	bool passesX = (cursor_position.x > buttonObject.position.x - (buttonObject.scale.x / 2)
		&& cursor_position.x < buttonObject.position.x + (buttonObject.scale.x / 2));
	bool passesY = (cursor_position.y > buttonObject.position.y - (buttonObject.scale.y / 2)
		&& cursor_position.y < buttonObject.position.y + (buttonObject.scale.y / 2));
	return passesX && passesY;
}

void MenuSystem::on_mouse_move(vec2 mouse_position) {
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	cursor_position = vec2(xpos, ypos);
	// change cursor to be hover
}

//tempory for load game testing
void MenuSystem::on_key(int key, int sc, int action, int mod) {
	// if (action == GLFW_RELEASE && key == GLFW_KEY_C) {
	// 	std::cout << "Loading game" << std::endl;
	// 	if (registry.gameLoadingHelper.size() > 0) {
	// 		registry.gameLoadingHelper.components[0].savedGame = true;
	// 	} else {
	// 		auto entity = Entity();
	// 		GameLoadingHelper& option = registry.gameLoadingHelper.emplace(entity);
	// 		option.savedGame = true;
	// 	}
	//
	// 	scene_manager.set_scene(SCENE_TYPE::GAME);
	// 	ReloadabilitySystem::loadGame();
	// }
}