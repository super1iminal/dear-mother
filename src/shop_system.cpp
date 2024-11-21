#include "shop_system.hpp"
#include <iostream>
#include <fstream>
#include <sstream>

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

	initButtons();

	description_line_one = UISystem::createTextUIElement(
		renderer,
		vec2(600.f, 190.f),
		vec2(40.f, 3.f),
		"upgrade_description_line_1",
		"Add another item slot, ",
		vec3(0.35, 0.76, 0.32),
		SCENE_TYPE::SHOP
	);

	description_line_two = UISystem::createTextUIElement(
		renderer,
		vec2(600.f, 220.f),
		vec2(40.f, 3.f),
		"upgrade_description_line_2",
		"so that you can hold more",
		vec3(0.35, 0.76, 0.32),
		SCENE_TYPE::SHOP
	);

	description_line_three = UISystem::createTextUIElement(
		renderer,
		vec2(600.f, 250.f),
		vec2(40.f, 3.f),
		"upgrade_description_line_3",
		"items during a run.",
		vec3(0.35, 0.76, 0.32),
		SCENE_TYPE::SHOP
	);

	item_slot_level = UISystem::createTextUIElement(
		renderer,
		vec2(514.f, 292.f),
		vec2(40.f, 2.f),
		"item_slot_level",
		std::to_string(getUpgradeLevel(UPGRADE_TYPE::ITEM_SLOT)),
		vec3(0.35, 0.76, 0.32),
		SCENE_TYPE::SHOP
	);

	dmg_upgrade_level = UISystem::createTextUIElement(
		renderer,
		vec2(514.f, 326.f),
		vec2(40.f, 2.f),
		"dmg_upgrade_level",
		std::to_string(getUpgradeLevel(UPGRADE_TYPE::DMG_UPGRADE)),
		vec3(0.35, 0.76, 0.32),
		SCENE_TYPE::SHOP
	);

	health_upgrade_level = UISystem::createTextUIElement(
		renderer,
		vec2(514.f, 364.f),
		vec2(40.f, 2.f),
		"health_upgrade_level",
		std::to_string(getUpgradeLevel(UPGRADE_TYPE::HEALTH_UPGRADE)),
		vec3(0.35, 0.76, 0.32),
		SCENE_TYPE::SHOP
	);

	crit_upgrade_level = UISystem::createTextUIElement(
		renderer,
		vec2(514.f, 404.f),
		vec2(40.f, 2.f),
		"crit_upgrade_level",
		std::to_string(getUpgradeLevel(UPGRADE_TYPE::CRIT_UPGRADE)),
		vec3(0.35, 0.76, 0.32),
		SCENE_TYPE::SHOP
	);

	dodge_upgrade_level = UISystem::createTextUIElement(
		renderer,
		vec2(514.f, 442.f),
		vec2(40.f, 2.f),
		"dodge_upgrade_level",
		std::to_string(getUpgradeLevel(UPGRADE_TYPE::DODGE_UPGRADE)),
		vec3(0.35, 0.76, 0.32),
		SCENE_TYPE::SHOP
	);

	viewed_upgrade_cost = UISystem::createTextUIElement(
		renderer,
		vec2(944.f, 406.f),
		vec2(8.f, 4.f),
		"viewed_upgrade_cost",
		"10",
		vec3(0.35, 0.76, 0.32),
		SCENE_TYPE::SHOP
	);

	current_scrap_display = UISystem::createTextUIElement(
		renderer,
		vec2(772.f, 460.f),
		vec2(8.f, 3.f),
		"current_scrap_display",
		std::to_string(getScrapLevel()),
		vec3(0.35, 0.76, 0.32),
		SCENE_TYPE::SHOP
	);

	UISystem::createCrosshair(renderer, TEXTURE_ASSET_ID::MENU_CROSSHAIR, SCENE_TYPE::SHOP);

	updateUpgrade(UPGRADE_TYPE::ITEM_SLOT);
}

void ShopSystem::initButtons() {
	UISystem::createButton(
		renderer,
		vec2(window_width_px - 338.f, window_height_px - 270.f),
		vec2(234.f, 60.f),
		[&]() {
			scene_manager.set_scene(SCENE_TYPE::MENU);
		},
		"return_to_menu_button",
		TEXTURE_ASSET_ID::BACK_BUTTON,
		SCENE_TYPE::SHOP
	);

	buy_button = UISystem::createTextButton(
		renderer,
		vec2(592.f, 405.f),
		vec2(8.f, 4.f),
		[&]() {
			buyUpgrade();
		},
		"buy_button",
		"BUY LVL",
		vec3(0.35, 0.76, 0.32),
		SCENE_TYPE::SHOP
	);

	UISystem::createButton(
		renderer,
		vec2(360.f, 284.f),
		vec2(146.f, 26.f),
		[&]() {
			updateUpgrade(UPGRADE_TYPE::ITEM_SLOT);
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
			updateUpgrade(UPGRADE_TYPE::DMG_UPGRADE);
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
			updateUpgrade(UPGRADE_TYPE::HEALTH_UPGRADE);
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
			updateUpgrade(UPGRADE_TYPE::CRIT_UPGRADE);
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
			updateUpgrade(UPGRADE_TYPE::DODGE_UPGRADE);
		},
		"dodge_upgrade_button",
		TEXTURE_ASSET_ID::DODGE_UPGRADE_BUTTON,
		SCENE_TYPE::SHOP
	);
}

void ShopSystem::updateUpgrade(UPGRADE_TYPE upgrade_type) {
	viewed_upgrade = upgrade_type;
	int viewed_upgrade_level = 0;

	std::string upgrade_name = "";
	UIElement* upgrade_ui_element = nullptr;
	switch (viewed_upgrade) {
	case UPGRADE_TYPE::ITEM_SLOT:
		upgrade_name = "item_slots";
		upgrade_ui_element = &registry.uiElements.get(item_slot_level);
		break;
	case UPGRADE_TYPE::DMG_UPGRADE:
		upgrade_name = "damage_upgrade";
		upgrade_ui_element = &registry.uiElements.get(dmg_upgrade_level);
		break;
	case UPGRADE_TYPE::HEALTH_UPGRADE:
		upgrade_name = "health_upgrade";
		upgrade_ui_element = &registry.uiElements.get(health_upgrade_level);
		break;
	case UPGRADE_TYPE::CRIT_UPGRADE:
		upgrade_name = "crit_upgrade";
		upgrade_ui_element = &registry.uiElements.get(crit_upgrade_level);
		break;
	case UPGRADE_TYPE::DODGE_UPGRADE:
		upgrade_name = "dodge_upgrade";
		upgrade_ui_element = &registry.uiElements.get(dodge_upgrade_level);
		break;
	}

	UIElement& scrap_element = registry.uiElements.get(current_scrap_display);

	std::ifstream read_file(std::string(PROJECT_SOURCE_DIR) + "/data/misc/upgrades.csv");
	if (read_file.is_open()) {
		std::string line;
		while (std::getline(read_file, line)) {
			std::stringstream ss(line);
			std::string tag;
			int value;
			if (std::getline(ss, tag, ',') && ss >> value) {
				if (tag == upgrade_name) {
					viewed_upgrade_level = value;
					if (upgrade_ui_element != nullptr) {
						upgrade_ui_element->value = std::to_string(viewed_upgrade_level);
					}
					else {
						std::cerr << "Upgrade UI element (to update text) was not found.\n";
					}
				}
				else if (tag == "scrap") {
					scrap_element.value = std::to_string(value);
				}
			}
		}
		read_file.close();
	}
	else {
		std::cerr << "Unable to open file for reading.\n";
	}

	UIElement& buy_button_element = registry.uiElements.get(buy_button);
	UIElement& cost_element = registry.uiElements.get(viewed_upgrade_cost);
	if (viewed_upgrade_level == 6) {
		buy_button_element.value = "MAX LVL";
		cost_element.value = "N/A";
	}
	else {
		buy_button_element.value = "BUY LVL " + std::to_string(viewed_upgrade_level + 1);
		cost_element.value = std::to_string((viewed_upgrade_level + 1) * 10);
	}

	updateUpgradeDescription();
}

void ShopSystem::updateUpgradeDescription() {
	UIElement& description_one = registry.uiElements.get(description_line_one);
	UIElement& description_two = registry.uiElements.get(description_line_two);
	UIElement& description_three = registry.uiElements.get(description_line_three);
	switch (viewed_upgrade) {
		case UPGRADE_TYPE::ITEM_SLOT:
			description_one.value = "Add another item slot, ";
			description_two.value = "so that you can hold more";
			description_three.value = "items during a run.";
			break;
		case UPGRADE_TYPE::DMG_UPGRADE:
			description_one.value = "Increase the amount of";
			description_two.value = "damage you deal by 15%.";
			description_three.value = "";
			break;
		case UPGRADE_TYPE::HEALTH_UPGRADE:
			description_one.value = "Increase the amount of";
			description_two.value = "health you have by 1";
			description_three.value = "hitpoint.";
			break;
		case UPGRADE_TYPE::CRIT_UPGRADE:
			description_one.value = "Increase the chance of";
			description_two.value = "a critical hit (x2 damage)";
			description_three.value = "by 3%.";
			break;
		case UPGRADE_TYPE::DODGE_UPGRADE:
			description_one.value = "Increase the chance of";
			description_two.value = "dodging an enemy hit";
			description_three.value = "(avoiding damage) by 2%.";
			break;
	}
}

int ShopSystem::getUpgradeLevel(UPGRADE_TYPE upgrade_type) {
	std::string upgrade = "";
	switch (upgrade_type) {
	case UPGRADE_TYPE::ITEM_SLOT:
		upgrade = "item_slots";
		break;
	case UPGRADE_TYPE::DMG_UPGRADE:
		upgrade = "damage_upgrade";
		break;
	case UPGRADE_TYPE::HEALTH_UPGRADE:
		upgrade = "health_upgrade";
		break;
	case UPGRADE_TYPE::CRIT_UPGRADE:
		upgrade = "crit_upgrade";
		break;
	case UPGRADE_TYPE::DODGE_UPGRADE:
		upgrade = "dodge_upgrade";
		break;
	}

	std::ifstream read_file(std::string(PROJECT_SOURCE_DIR) + "/data/misc/upgrades.csv");
	std::vector<std::pair<std::string, int>> data;
	if (read_file.is_open()) {
		std::string line;
		while (std::getline(read_file, line)) {
			std::stringstream ss(line);
			std::string tag;
			int value;
			if (std::getline(ss, tag, ',') && ss >> value) {
				if (tag == upgrade) {
					return value;
				}
			}
		}
		read_file.close();
		return -1;
	}
	else {
		std::cerr << "Unable to open file for reading.\n";
		return -1;
	}
}

int ShopSystem::getScrapLevel() {
	std::ifstream read_file(std::string(PROJECT_SOURCE_DIR) + "/data/misc/upgrades.csv");
	std::vector<std::pair<std::string, int>> data;
	if (read_file.is_open()) {
		std::string line;
		while (std::getline(read_file, line)) {
			std::stringstream ss(line);
			std::string tag;
			int value;
			if (std::getline(ss, tag, ',') && ss >> value) {
				if (tag == "scrap") {
					std::cout << "Scrap: " << value << std::endl;
					return value;
				}
			}
		}
		read_file.close();
		return -1;
	}
	else {
		std::cerr << "Unable to open file for reading.\n";
		return -1;
	}
}

void ShopSystem::updateScrapLevel(int updated_value) {
	std::ifstream read_file(std::string(PROJECT_SOURCE_DIR) + "/data/misc/upgrades.csv");
	std::vector<std::pair<std::string, int>> data;
	if (read_file.is_open()) {
		std::string line;
		while (std::getline(read_file, line)) {
			std::stringstream ss(line);
			std::string tag;
			int value;
			if (std::getline(ss, tag, ',') && ss >> value) {
				if (tag == "scrap") {
					data.emplace_back(tag, updated_value);
				}
				else {
					data.emplace_back(tag, value);
				}
			}
		}
		read_file.close();
	}
	else {
		std::cerr << "Unable to open file for reading.\n";
	}

	// now save the updated data to the csv file
	std::ofstream write_file(std::string(PROJECT_SOURCE_DIR) + "/data/misc/upgrades.csv");
	if (write_file.is_open()) {
		for (const auto& entry : data) {
			write_file << entry.first << "," << entry.second << "\n";
		}
		write_file.close();
		for (Entity entity : registry.uiElements.entities) {
			if (registry.uiElements.get(entity).name == "current_scrap_display") {
				registry.uiElements.get(entity).value = std::to_string(updated_value);
			}
		}
	}
	else {
		std::cerr << "Unable to open file for writing.\n";
	}
}

void ShopSystem::buyUpgrade() {
	std::string upgrade = "";
	switch (viewed_upgrade) {
		case UPGRADE_TYPE::ITEM_SLOT:
			upgrade = "item_slots";
			break;
		case UPGRADE_TYPE::DMG_UPGRADE:
			upgrade = "damage_upgrade";
			break;
		case UPGRADE_TYPE::HEALTH_UPGRADE:
			upgrade = "health_upgrade";
			break;
		case UPGRADE_TYPE::CRIT_UPGRADE:
			upgrade = "crit_upgrade";
			break;
		case UPGRADE_TYPE::DODGE_UPGRADE:
			upgrade = "dodge_upgrade";
			break;
	}
	UIElement& cost_element = registry.uiElements.get(viewed_upgrade_cost);
	if (cost_element.value == "N/A") {
		return;	// don't attempt the purchase; we are at max lvl already for this upgrade
	}
	int upgrade_cost = std::stoi(cost_element.value);
	std::cout << "upgrade_cost: " << upgrade_cost << std::endl;
	std::cout << "upgrade: " << upgrade << std::endl;
	bool purchased = false;

	std::ifstream read_file(std::string(PROJECT_SOURCE_DIR) + "/data/misc/upgrades.csv");
	std::vector<std::pair<std::string, int>> data;
	if (read_file.is_open()) {
		std::string line;
		while (std::getline(read_file, line)) {
			std::stringstream ss(line);
			std::string tag;
			int value;
			if (std::getline(ss, tag, ',') && ss >> value) {
				if (tag == upgrade && (getScrapLevel() - upgrade_cost) < 0) {
					// we do not have enough scrap for this upgrade
					// TODO make the scrap value flash red

					data.emplace_back(tag, value);
					std::cout << "Not enough scrap!" << std::endl;
				}
				else if (tag == upgrade && value + 1 <= MAX_UPGRADE_LEVEL) {
					data.emplace_back(tag, value + 1);
					purchased = true;
				}
				else if (tag == "scrap" && purchased) {
					data.emplace_back(tag, value - upgrade_cost);
				}
				else {
					data.emplace_back(tag, value);
				}
			}
		}
		read_file.close();
	}
	else {
		std::cerr << "Unable to open file for reading.\n";
	}

	// now save the updated data to the csv file 
	std::ofstream write_file(std::string(PROJECT_SOURCE_DIR) + "/data/misc/upgrades.csv");
	if (write_file.is_open()) {
		for (const auto& entry : data) {
			write_file << entry.first << "," << entry.second << "\n";
		}
		write_file.close();
	}
	else {
		std::cerr << "Unable to open file for writing.\n";
	}

	// update the UI to show the next level the player will purchase
	updateUpgrade(viewed_upgrade);
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