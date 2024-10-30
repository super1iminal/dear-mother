#include "menu_system.hpp"
#include <tiny_ecs_registry.hpp>


MenuSystem::MenuSystem()
{

}

MenuSystem::~MenuSystem() {

}

void MenuSystem::init(RenderSystem* renderer_arg, GLFWwindow* window_arg, SCENE_TYPE* scene_arg) {
	this->renderer = renderer_arg;
	this->window = window_arg;
	this->scene = scene_arg;
}

void MenuSystem::initStartMenu()
{
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
		vec2(window_width_px / 2 - 1.f, 275.f),
		vec2(162.f, 42.f),
		[&]() {
			*(this->scene) = SCENE_TYPE::GAME;
		},
		"start_button",
		TEXTURE_ASSET_ID::START_BUTTON,
		SCENE_TYPE::MENU
	);

	UISystem::createButton(
		renderer,
		vec2(window_width_px / 2 + 3.f, 322.f),
		vec2(90.f, 44.f),
		[&]() {
			std::cout << "Help button pressed!" << std::endl;
			initHelpScreen();
		},
		"help_button",
		TEXTURE_ASSET_ID::HELP_BUTTON,
		SCENE_TYPE::MENU
	);

	UISystem::createButton(
		renderer,
		vec2(window_width_px / 2, 372.f),
		vec2(165.f, 44.f),
		[&]() {
			std::cout << "Upgrades button pressed!" << std::endl;
		},
		"shop_button",
		TEXTURE_ASSET_ID::SHOP_BUTTON,
		SCENE_TYPE::MENU
	);

	UISystem::createButton(
		renderer,
		vec2(window_width_px / 2 + 7.f, 420.f),
		vec2(90.f, 44.f),
		[&]() {
			std::cout << "Quit button pressed!" << std::endl;
			// TODO: SAVE GAME!!
			glfwSetWindowShouldClose(window, GLFW_TRUE);
		},
		"quit_button",
		TEXTURE_ASSET_ID::QUIT_BUTTON,
		SCENE_TYPE::MENU
	);

	UISystem::createCrosshair(renderer, TEXTURE_ASSET_ID::MENU_CROSSHAIR, SCENE_TYPE::MENU);
}

void MenuSystem::initHelpScreen() {
	UISystem::createPanel(
		renderer,
		SCENE_TYPE::MENU,
		vec2(window_width_px / 2, window_height_px / 2),
		0.f,
		vec2(window_width_px, window_height_px),
		"help_screen",
		TEXTURE_ASSET_ID::HELP_SCREEN
	);

	UISystem::createButton(
		renderer,
		vec2(window_width_px - 330.f, window_height_px - 268.f),
		vec2(234.f, 60.f),
		[&]() {
			std::cout << "Back button pressed!" << std::endl;
			closeHelpScreen();
		},
		"return_to_menu_button",
		TEXTURE_ASSET_ID::BACK_BUTTON,
		SCENE_TYPE::MENU
	);
}

void MenuSystem::closeHelpScreen() {
	// remove the help panel
	for (Entity entity : registry.menuSceneComponents.entities) {
		if (registry.baseUI.has(entity)) {
			BaseUI button = registry.baseUI.get(entity);
			if (button.name == "help_screen") {
				registry.pendingRemoves.emplace(entity);
				break;
			}
		}
	}

	// remove the back button
	for (Entity entity : registry.menuSceneComponents.entities) {
		if (registry.uiButtons.has(entity)) {
			UIButton button = registry.uiButtons.get(entity);
			if (button.name == "return_to_menu_button") {
				registry.pendingRemoves.emplace(entity);
				break;
			}
		}
	}
}

void MenuSystem::on_mouse_button(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		// if mouse position is within button boundaries
		// activate the button's action function
		auto& uiButtonsRegistry = registry.uiButtons;
		auto& worldObjectsRegistry = registry.worldObjects;
		for (uint i = 0; i < uiButtonsRegistry.size(); i++) {
			UIButton button = uiButtonsRegistry.components[i];
			Entity buttonEntity = uiButtonsRegistry.entities[i];
			WorldObject buttonObject = worldObjectsRegistry.get(buttonEntity);
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