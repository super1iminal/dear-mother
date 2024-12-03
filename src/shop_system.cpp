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
	getUpgradeValuesFromCSV();

	description_line_one = UISystem::createTextUIElement(
		renderer,
		vec2(900.f, 285.f),
		vec2(60.f, 5.f),
		"upgrade_description_line_1",
		"Add another item slot, ",
		vec3(0.35, 0.76, 0.32),
		SCENE_TYPE::SHOP
	);

	description_line_two = UISystem::createTextUIElement(
		renderer,
		vec2(900.f, 330.f),
		vec2(60.f, 5.f),
		"upgrade_description_line_2",
		"so that you can hold",
		vec3(0.35, 0.76, 0.32),
		SCENE_TYPE::SHOP
	);

	description_line_three = UISystem::createTextUIElement(
		renderer,
		vec2(900.f, 375.f),
		vec2(60.f, 5.f),
		"upgrade_description_line_3",
		"more items during a run.",
		vec3(0.35, 0.76, 0.32),
		SCENE_TYPE::SHOP
	);

	item_slot_level = UISystem::createTextUIElement(
		renderer,
		vec2(771.f, 438.f),
		vec2(60.f, 3.f),
		"item_slot_level",
		std::to_string(upgrades["item_slots"][0]),
		vec3(0.35, 0.76, 0.32),
		SCENE_TYPE::SHOP
	);

	dmg_upgrade_level = UISystem::createTextUIElement(
		renderer,
		vec2(771.f, 489.f),
		vec2(60.f, 3.f),
		"dmg_upgrade_level",
		std::to_string(upgrades["damage_upgrade"][0]),
		vec3(0.35, 0.76, 0.32),
		SCENE_TYPE::SHOP
	);

	health_upgrade_level = UISystem::createTextUIElement(
		renderer,
		vec2(771.f, 546.f),
		vec2(60.f, 3.f),
		"health_upgrade_level",
		std::to_string(upgrades["health_upgrade"][0]),
		vec3(0.35, 0.76, 0.32),
		SCENE_TYPE::SHOP
	);

	crit_upgrade_level = UISystem::createTextUIElement(
		renderer,
		vec2(771.f, 606.f),
		vec2(60.f, 3.f),
		"crit_upgrade_level",
		std::to_string(upgrades["crit_upgrade"][0]),
		vec3(0.35, 0.76, 0.32),
		SCENE_TYPE::SHOP
	);

	dodge_upgrade_level = UISystem::createTextUIElement(
		renderer,
		vec2(771.f, 663.f),
		vec2(60.f, 3.f),
		"dodge_upgrade_level",
		std::to_string(upgrades["dodge_upgrade"][0]),
		vec3(0.35, 0.76, 0.32),
		SCENE_TYPE::SHOP
	);

	viewed_upgrade_cost = UISystem::createTextUIElement(
		renderer,
		vec2(1416.f, 609.f),
		vec2(12.f, 6.f),
		"viewed_upgrade_cost",
		"10",
		vec3(0.35, 0.76, 0.32),
		SCENE_TYPE::SHOP
	);

	current_scrap_display = UISystem::createTextUIElement(
		renderer,
		vec2(1158.f, 690.f),
		vec2(12.f, 5.f),
		"current_scrap_display",
		std::to_string(upgrades["scrap"][0]),
		vec3(0.35, 0.76, 0.32),
		SCENE_TYPE::SHOP
	);

	crosshair = UISystem::createCrosshair(renderer, TEXTURE_ASSET_ID::MENU_CROSSHAIR, SCENE_TYPE::SHOP);

	updateViewedUpgrade(UPGRADE_TYPE::ITEM_SLOT);
	updateScrapUI();
}

void ShopSystem::initButtons() {
	UISystem::createButton(
		renderer,
		vec2(window_width_px - 506.f, window_height_px - 405.f),
		vec2(351.f, 90.f),
		[&]() {
			scene_manager.set_scene(SCENE_TYPE::MENU);
		},
		"return_to_menu_button",
		TEXTURE_ASSET_ID::BACK_BUTTON,
		TEXTURE_ASSET_ID::BACK_BUTTON_HOVER,
		SCENE_TYPE::SHOP
	);

	buy_button = UISystem::createTextButton(
		renderer,
		vec2(888.f, 608.f),
		vec2(12.f, 6.f),
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
		vec2(540.f, 426.f),
		vec2(219.f, 39.f),
		[&]() {
			updateViewedUpgrade(UPGRADE_TYPE::ITEM_SLOT);
		},
		"item_slot_upgrade_button",
		TEXTURE_ASSET_ID::ITEM_SLOT_BUTTON,
		TEXTURE_ASSET_ID::ITEM_SLOT_BUTTON_HOVER,
		SCENE_TYPE::SHOP
	);

	UISystem::createButton(
		renderer,
		vec2(570.f, 480.f),
		vec2(288.f, 57.f),
		[&]() {
			updateViewedUpgrade(UPGRADE_TYPE::DMG_UPGRADE);
		},
		"damage_upgrade_button",
		TEXTURE_ASSET_ID::DMG_UPGRADE_BUTTON,
		TEXTURE_ASSET_ID::DMG_UPGRADE_BUTTON_HOVER,
		SCENE_TYPE::SHOP
	);

	UISystem::createButton(
		renderer,
		vec2(555.f, 540.f),
		vec2(258.f, 51.f),
		[&]() {
			updateViewedUpgrade(UPGRADE_TYPE::HEALTH_UPGRADE);
		},
		"health_upgrade_button",
		TEXTURE_ASSET_ID::HEALTH_UPGRADE_BUTTON,
		TEXTURE_ASSET_ID::HEALTH_UPGRADE_BUTTON_HOVER,
		SCENE_TYPE::SHOP
	);

	UISystem::createButton(
		renderer,
		vec2(570.f, 594.f),
		vec2(300.f, 45.f),
		[&]() {
			updateViewedUpgrade(UPGRADE_TYPE::CRIT_UPGRADE);
		},
		"crit_upgrade_button",
		TEXTURE_ASSET_ID::CRIT_UPGRADE_BUTTON,
		TEXTURE_ASSET_ID::CRIT_UPGRADE_BUTTON_HOVER,
		SCENE_TYPE::SHOP
	);

	UISystem::createButton(
		renderer,
		vec2(528.f, 657.f),
		vec2(228.f, 54.f),
		[&]() {
			updateViewedUpgrade(UPGRADE_TYPE::DODGE_UPGRADE);
		},
		"dodge_upgrade_button",
		TEXTURE_ASSET_ID::DODGE_UPGRADE_BUTTON,
		TEXTURE_ASSET_ID::DODGE_UPGRADE_BUTTON_HOVER,
		SCENE_TYPE::SHOP
	);
}

void ShopSystem::updateScrapUI() {
	UIElement& scrap_element = registry.uiElements.get(current_scrap_display);
	scrap_element.value = std::to_string((upgrades["scrap"])[0]);
}

void ShopSystem::updateViewedUpgrade(UPGRADE_TYPE upgrade_type) {
	getUpgradeValuesFromCSV();

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

	if (upgrade_ui_element == nullptr) {
		std::cerr << "No UI Element for upgrade: " << upgrade_name << " found.";
		return;
	}

	getUpgradeValuesFromCSV();
	upgrade_ui_element->value = std::to_string((upgrades[upgrade_name])[0]);

	viewed_upgrade_level = upgrades[upgrade_name][0];

	UIElement& buy_button_element = registry.uiElements.get(buy_button);
	UIElement& cost_element = registry.uiElements.get(viewed_upgrade_cost);
	if (viewed_upgrade_level == upgrades[upgrade_name][1]) {
		buy_button_element.value = "MAX LVL";
		cost_element.value = "N/A";
	}
	else {
		buy_button_element.value = "BUY LVL " + std::to_string(viewed_upgrade_level + 1);
		cost_element.value = std::to_string((viewed_upgrade_level + 1) * 10 * upgrades[upgrade_name][2]);
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
			description_two.value = "so that you can hold";
			description_three.value = "more items in a run.";
			break;
		case UPGRADE_TYPE::DMG_UPGRADE:
			description_one.value = "Increase the amount of";
			description_two.value = "damage you deal by 1.";
			description_three.value = "";
			break;
		case UPGRADE_TYPE::HEALTH_UPGRADE:
			description_one.value = "Increase the amount of";
			description_two.value = "health you have by 1";
			description_three.value = "hitpoint.";
			break;
		case UPGRADE_TYPE::CRIT_UPGRADE:
			description_one.value = "Increase the chance of";
			description_two.value = "a critical hit (dealing ";
			description_three.value = "x2 damage) by 3%";
			break;
		case UPGRADE_TYPE::DODGE_UPGRADE:
			description_one.value = "Increase the chance of";
			description_two.value = "dodging an enemy hit";
			description_three.value = "(no damage) by 2%.";
			break;
	}
}

void ShopSystem::getUpgradeValuesFromCSV() {
	std::ifstream read_file(std::string(PROJECT_SOURCE_DIR) + "/data/misc/upgrades.csv");
	std::vector<std::pair<std::string, int>> data;
	if (read_file.is_open()) {
		std::string line;
		while (std::getline(read_file, line)) {
			std::stringstream ss(line);
			std::string tag;
			if (std::getline(ss, tag, ',')) {
				std::vector<int> values;
				std::string value;
				while (std::getline(ss, value, ',')) {
					try {
						values.push_back(std::stoi(value));
					}
					catch (const std::invalid_argument&) {
						std::cerr << "Error: Invalid number in CSV for " << tag << std::endl;
						continue;
					}
				}
				upgrades[tag] = values;
			}
		}
		read_file.close();
	}
	else {
		std::cerr << "Unable to open file for reading.\n";
	}
}

void ShopSystem::updateUpgradeValuesForCSV() {
	std::ofstream write_file(std::string(PROJECT_SOURCE_DIR) + "/data/misc/upgrades.csv");
	if (!write_file.is_open()) {
		std::cerr << "Error: Could not open file for writing." << std::endl;
		return;
	}

	for (const auto& pair : upgrades) {
		const std::string& key = pair.first;
		const std::vector<int>& values = pair.second;

		write_file << key;
		for (int value : values) {
			write_file << "," << value;
		}
		write_file << "\n";
	}

	write_file.close();
}

int ShopSystem::getScrapLevel() {
	std::ifstream read_file(std::string(PROJECT_SOURCE_DIR) + "/data/misc/upgrades.csv");
	std::vector<std::pair<std::string, int>> data;
	if (read_file.is_open()) {
		std::string line;
		while (std::getline(read_file, line)) {
			std::stringstream ss(line);
			std::string tag;
			std::vector<int> values;
			std::string value;
			if (std::getline(ss, tag, ',')) {
				if (tag == "scrap") {
					while (std::getline(ss, value, ',')) {
						try {
							values.push_back(std::stoi(value));
						}
						catch (const std::invalid_argument&) {
							std::cerr << "Error: Invalid number in CSV for " << tag << std::endl;
							continue;
						}
					}
					std::cout << "Scrap: " << values[0] << std::endl;
					return values[0];
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
	std::unordered_map<std::string, std::vector<int>> data;

	if (read_file.is_open()) {
		std::string line;
		while (std::getline(read_file, line)) {
			std::stringstream ss(line);
			std::string tag;
			std::string values_str;
			std::vector<int> values;

			// Read the tag
			if (std::getline(ss, tag, ',')) {
				while (std::getline(ss, values_str, ',')) {
					try {
						values.push_back(std::stoi(values_str));
					}
					catch (const std::invalid_argument&) {
						std::cerr << "Error: Invalid number in CSV for " << tag << std::endl;
						continue;
					}
				}

				if (tag == "scrap") {
					data[tag] = { updated_value };
				}
				else {
					data[tag] = values;
				}
			}
		}
		read_file.close();
	}
	else {
		std::cerr << "Unable to open file for reading.\n";
		return;
	}

	// Save the updated data back to the CSV file
	std::ofstream write_file(std::string(PROJECT_SOURCE_DIR) + "/data/misc/upgrades.csv");
	if (!write_file.is_open()) {
		std::cerr << "Error: Could not open file for writing." << std::endl;
		return;
	}

	for (const auto& pair : data) {
		const std::string& key = pair.first;
		const std::vector<int>& values = pair.second;

		write_file << key;
		for (int value : values) {
			write_file << "," << value;
		}
		write_file << "\n";
	}

	write_file.close();
}


void ShopSystem::buyUpgrade() {
	std::string upgrade_name = "";
	switch (viewed_upgrade) {
	case UPGRADE_TYPE::ITEM_SLOT:
		upgrade_name = "item_slots";
		break;
	case UPGRADE_TYPE::DMG_UPGRADE:
		upgrade_name = "damage_upgrade";
		break;
	case UPGRADE_TYPE::HEALTH_UPGRADE:
		upgrade_name = "health_upgrade";
		break;
	case UPGRADE_TYPE::CRIT_UPGRADE:
		upgrade_name = "crit_upgrade";
		break;
	case UPGRADE_TYPE::DODGE_UPGRADE:
		upgrade_name = "dodge_upgrade";
		break;
	}

	std::vector<int> levels = upgrades[upgrade_name];
	int current_level = levels[0];
	int max_level = levels[1];

	UIElement& cost_element = registry.uiElements.get(viewed_upgrade_cost);
	if (cost_element.value == "N/A") {
		return;	// don't attempt the purchase; we are at max lvl already for this upgrade
	}
	int upgrade_cost = std::stoi(cost_element.value);

	bool purchased = false;

	if (upgrades["scrap"][0] - upgrade_cost < 0) {
		// we do not have enough scrap for this upgrade
		// TODO make the scrap value flash red
		std::cout << "Not enough scrap!" << std::endl;
	}
	else if (current_level + 1 <= max_level) {
		upgrades["scrap"][0] -= upgrade_cost;
		upgrades[upgrade_name][0] = current_level + 1;
	}

	updateUpgradeValuesForCSV();
	// update UI for this upgrade
	updateViewedUpgrade(viewed_upgrade);
	updateScrapUI();
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
	float scale_y = buttonObject.scale.y * 7;
	
	bool passesX = (cursor_position.x > buttonObject.position.x
		&& cursor_position.x < buttonObject.position.x + (scale_x));
	bool passesY = (cursor_position.y > buttonObject.position.y - (1.5 * scale_y)
		&& cursor_position.y < buttonObject.position.y + (0.5 * scale_y));
	return passesX && passesY;
}

void ShopSystem::on_mouse_move(vec2 mouse_position) {
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	cursor_position = vec2(xpos, ypos);

	auto& uiButtonsRegistry = registry.shopSceneButtons.entities;
	RenderRequest& crosshair_render_request = registry.renderRequests.get(crosshair);
	bool is_any_button_hovered = false;
	for (Entity buttonEntity : uiButtonsRegistry) {
		UIButton& button = registry.uiButtons.get(buttonEntity);
		WorldObject& buttonObject = registry.worldObjects.get(buttonEntity);
		RenderRequest& button_render_request = registry.renderRequests.get(buttonEntity);
		bool mouse_within_button = (registry.uiElements.has(buttonEntity) && is_mouse_within_text_button(buttonObject)) || is_mouse_within_button(buttonObject);
		if (mouse_within_button) {
			is_any_button_hovered = true;

			if (!button.is_hovered) {
				button.is_hovered = true;
				if (registry.uiElements.has(buttonEntity)) {	// if this is a text button, change the text color
					registry.colors.remove(buttonEntity);
					registry.colors.emplace(buttonEntity) = vec3(0.58, 0.83, 0.39);
				}
				else {
					button_render_request.used_texture = button.hover_texture;	// change the texture
				}
			}
		}
		else {
			if (button.is_hovered) {
				button.is_hovered = false;
				if (registry.uiElements.has(buttonEntity)) {	// if this is a text button, change the text color
					registry.colors.remove(buttonEntity);
					registry.colors.emplace(buttonEntity) = vec3(0.35, 0.76, 0.32);
				}
				else {
					button_render_request.used_texture = button.base_texture;	// change the texture
				}
			}
		}
	}

	crosshair_render_request.used_texture = is_any_button_hovered ? TEXTURE_ASSET_ID::MENU_HOVER_CROSSHAIR
		: TEXTURE_ASSET_ID::MENU_CROSSHAIR;
}