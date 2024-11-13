#include "shop_system.hpp"

ShopSystem::ShopSystem()
{

}

ShopSystem::~ShopSystem() {

}

void ShopSystem::init(RenderSystem* renderer_arg, GLFWwindow* window_arg) {
	this->renderer = renderer_arg;
	this->window = window_arg;
	Entity base_ui = UISystem::createPanel(
		renderer,
		SCENE_TYPE::SHOP,
		vec2(window_width_px / 2, window_height_px / 2),
		0.f,
		vec2(window_width_px, window_height_px),
		"shop_menu",
		TEXTURE_ASSET_ID::SHOP_SCREEN
	);

	UISystem::createButton(
		renderer,
		vec2(window_width_px - 338.f, window_height_px - 270.f),
		vec2(234.f, 60.f),
		[&]() {
			std::cout << "Back button pressed!" << std::endl;
			scene_manager.set_scene(SCENE_TYPE::MENU);
		},
		"return_to_menu_button",
		TEXTURE_ASSET_ID::BACK_BUTTON,
		SCENE_TYPE::SHOP
	);

	UISystem::createTextButton(
		renderer,
		vec2(592.f, 405.f),
		vec2(8.f, 4.f),
		[&]() {
			std::cout << "Buy button pressed!" << std::endl;
		},
		"buy_button",
		"BUY LVL",
		SCENE_TYPE::SHOP
	);

	UISystem::createButton(
		renderer,
		vec2(360.f, 284.f),
		vec2(146.f, 26.f),
		[&]() {
			std::cout << "Item slot upgrade button pressed!" << std::endl;
			viewed_upgrade = UPGRADE_TYPE::ITEM_SLOT;
			std::cout << static_cast<int>(viewed_upgrade) << std::endl;
			updateUpgradeDescription();
		},
		"item_slot_upgrade_button",
		TEXTURE_ASSET_ID::ITEM_SLOT_BUTTON,
		SCENE_TYPE::SHOP
	);

	UISystem::createButton(
		renderer,
		vec2(380.f, 320.f),
		vec2(192.f, 38.f),
		[&]() {
			std::cout << "Damage upgrade button pressed!" << std::endl;
			viewed_upgrade = UPGRADE_TYPE::DMG_UPGRADE;
			std::cout << static_cast<int>(viewed_upgrade) << std::endl;
			updateUpgradeDescription();
		},
		"damage_upgrade_button",
		TEXTURE_ASSET_ID::DMG_UPGRADE_BUTTON,
		SCENE_TYPE::SHOP
	);

	UISystem::createButton(
		renderer,
		vec2(370.f, 360.f),
		vec2(172.f, 34.f),
		[&]() {
			std::cout << "Health upgrade button pressed!" << std::endl;
			viewed_upgrade = UPGRADE_TYPE::HEALTH_UPGRADE;
			std::cout << static_cast<int>(viewed_upgrade) << std::endl;
			updateUpgradeDescription();
		},
		"health_upgrade_button",
		TEXTURE_ASSET_ID::HEALTH_UPGRADE_BUTTON,
		SCENE_TYPE::SHOP
	);

	UISystem::createButton(
		renderer,
		vec2(380.f, 396.f),
		vec2(200.f, 30.f),
		[&]() {
			std::cout << "Crit upgrade button pressed!" << std::endl;
			viewed_upgrade = UPGRADE_TYPE::CRIT_UPGRADE;
			std::cout << static_cast<int>(viewed_upgrade) << std::endl;
			updateUpgradeDescription();
		},
		"crit_upgrade_button",
		TEXTURE_ASSET_ID::CRIT_UPGRADE_BUTTON,
		SCENE_TYPE::SHOP
	);

	UISystem::createButton(
		renderer,
		vec2(352.f, 438.f),
		vec2(152.f, 36.f),
		[&]() {
			std::cout << "Dodge upgrade button pressed!" << std::endl;
			viewed_upgrade = UPGRADE_TYPE::DODGE_UPGRADE;
			std::cout << static_cast<int>(viewed_upgrade) << std::endl;
			updateUpgradeDescription();
		},
		"dodge_upgrade_button",
		TEXTURE_ASSET_ID::DODGE_UPGRADE_BUTTON,
		SCENE_TYPE::SHOP
	);

	description_line_one = UISystem::createTextUIElement(
		renderer,
		vec2(600.f, 190.f),
		vec2(40.f, 3.f),
		"upgrade_description_line_1",
		"Add another item slot, ",
		SCENE_TYPE::SHOP
	);

	description_line_two = UISystem::createTextUIElement(
		renderer,
		vec2(600.f, 220.f),
		vec2(40.f, 3.f),
		"upgrade_description_line_2",
		"so that you can hold more",
		SCENE_TYPE::SHOP
	);

	description_line_three = UISystem::createTextUIElement(
		renderer,
		vec2(600.f, 250.f),
		vec2(40.f, 3.f),
		"upgrade_description_line_3",
		"items during a run. Max 8.",
		SCENE_TYPE::SHOP
	);

	UISystem::createCrosshair(renderer, TEXTURE_ASSET_ID::MENU_CROSSHAIR, SCENE_TYPE::SHOP);
}

void ShopSystem::updateUpgradeDescription() {
	UIElement& description_one = registry.uiElements.get(description_line_one);
	UIElement& description_two = registry.uiElements.get(description_line_two);
	UIElement& description_three = registry.uiElements.get(description_line_three);
	switch (viewed_upgrade) {
		case UPGRADE_TYPE::ITEM_SLOT:
			std::cout << "Item slot upgrade description should be shown." << std::endl;
			description_one.value = "Add another item slot, ";
			description_two.value = "so that you can hold more";
			description_three.value = "items during a run.";
			break;
		case UPGRADE_TYPE::DMG_UPGRADE:
			std::cout << "Dmg upgrade description should be shown." << std::endl;
			description_one.value = "Increase the amount of";
			description_two.value = "damage you deal by 15%.";
			description_three.value = "";
			break;
		case UPGRADE_TYPE::HEALTH_UPGRADE:
			std::cout << "Health upgrade description should be shown." << std::endl;
			description_one.value = "Increase the amount of";
			description_two.value = "health you have by 1";
			description_three.value = "hitpoint.";
			break;
		case UPGRADE_TYPE::CRIT_UPGRADE:
			std::cout << "Crit upgrade description should be shown." << std::endl;
			description_one.value = "Increase the chance of";
			description_two.value = "a critical hit (x2 damage)";
			description_three.value = "by 3%.";
			break;
		case UPGRADE_TYPE::DODGE_UPGRADE:
			std::cout << "Dodge upgrade description should be shown." << std::endl;
			description_one.value = "Increase the chance of";
			description_two.value = "dodging an enemy hit";
			description_three.value = "(avoiding damage) by 2%.";
			break;
	}
}

void ShopSystem::on_mouse_button(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		// if mouse position is within button boundaries
		// activate the button's action function
		auto& uiButtonsRegistry = registry.shopSceneButtons.entities;
		for (Entity buttonEntity : uiButtonsRegistry) {
			UIButton button = registry.uiButtons.get(buttonEntity);
			WorldObject buttonObject = registry.worldObjects.get(buttonEntity);
			if (registry.uiElements.has(buttonEntity) && is_mouse_within_text_button(buttonObject)) {
				button.action();
			} else if (is_mouse_within_button(buttonObject)) {
				button.action();
			}
		}
	}
}

bool ShopSystem::is_mouse_within_button(WorldObject buttonObject)
{
	bool passesX = (cursor_position.x > buttonObject.position.x - (buttonObject.scale.x / 2)
		&& cursor_position.x < buttonObject.position.x + (buttonObject.scale.x / 2));
	bool passesY = (cursor_position.y > buttonObject.position.y - (buttonObject.scale.y / 2)
		&& cursor_position.y < buttonObject.position.y + (buttonObject.scale.y / 2));
	return passesX && passesY;
}

bool ShopSystem::is_mouse_within_text_button(WorldObject buttonObject)
{
	float scale_x = buttonObject.scale.x * 20;
	float scale_y = buttonObject.scale.y * 20;
	
	bool passesX = (cursor_position.x > buttonObject.position.x
		&& cursor_position.x < buttonObject.position.x + (scale_x));
	bool passesY = (cursor_position.y > buttonObject.position.y - (scale_y)
		&& cursor_position.y < buttonObject.position.y + (scale_y));
	return passesX && passesY;
}

void ShopSystem::on_mouse_move(vec2 mouse_position) {
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	cursor_position = vec2(xpos, ypos);
	// change cursor to be hover
}