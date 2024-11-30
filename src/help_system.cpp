#include "help_system.hpp"

HelpSystem::HelpSystem()
{

}

HelpSystem::~HelpSystem() {

}

void HelpSystem::init(RenderSystem* renderer_arg, GLFWwindow* window_arg) {
	this->renderer = renderer_arg;
	this->window = window_arg;
	UISystem::createPanel(
		renderer,
		SCENE_TYPE::HELP,
		vec2(window_width_px / 2, window_height_px / 2),
		0.f,
		vec2(window_width_px, window_height_px),
		"help_screen",
		TEXTURE_ASSET_ID::HELP_SCREEN
	);

	UISystem::createButton(
		renderer,
		vec2(window_width_px - 507.f, window_height_px - 402.f),
		vec2(351.f, 90.f),
		[&]() {
			std::cout << "Back button pressed!" << std::endl;
			scene_manager.set_scene(SCENE_TYPE::MENU);
		},
		"return_to_menu_button",
		TEXTURE_ASSET_ID::BACK_BUTTON,
		TEXTURE_ASSET_ID::BOUNDBOX_BLUE,
		SCENE_TYPE::HELP
	);

	crosshair = UISystem::createCrosshair(renderer, TEXTURE_ASSET_ID::MENU_CROSSHAIR, SCENE_TYPE::HELP);
}

void HelpSystem::on_mouse_button(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		// if mouse position is within button boundaries
		// activate the button's action function
		auto& uiButtonsRegistry = registry.helpSceneButtons.entities;
		for (Entity buttonEntity : uiButtonsRegistry) {
			UIButton button = registry.uiButtons.get(buttonEntity);
			WorldObject buttonObject = registry.worldObjects.get(buttonEntity);
			if (is_mouse_within_button(buttonObject)) {
				button.action();
			}
		}
	}
}

bool HelpSystem::is_mouse_within_button(WorldObject buttonObject)
{
	bool passesX = (cursor_position.x > buttonObject.position.x - (buttonObject.scale.x / 2)
		&& cursor_position.x < buttonObject.position.x + (buttonObject.scale.x / 2));
	bool passesY = (cursor_position.y > buttonObject.position.y - (buttonObject.scale.y / 2)
		&& cursor_position.y < buttonObject.position.y + (buttonObject.scale.y / 2));
	return passesX && passesY;
}

void HelpSystem::on_mouse_move(vec2 mouse_position) {
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	cursor_position = vec2(xpos, ypos);

	auto& uiButtonsRegistry = registry.helpSceneButtons.entities;
	RenderRequest& crosshair_render_request = registry.renderRequests.get(crosshair);
	for (Entity buttonEntity : uiButtonsRegistry) {
		UIButton button = registry.uiButtons.get(buttonEntity);
		WorldObject buttonObject = registry.worldObjects.get(buttonEntity);
		bool within_button = is_mouse_within_button(buttonObject);
		if (within_button) {
			// change cursor to be hover
			crosshair_render_request.used_texture = TEXTURE_ASSET_ID::MENU_HOVER_CROSSHAIR;
			break;
		}
		else if (crosshair_render_request.used_texture == TEXTURE_ASSET_ID::MENU_HOVER_CROSSHAIR) {
			crosshair_render_request.used_texture = TEXTURE_ASSET_ID::MENU_CROSSHAIR;
		}
	}
}