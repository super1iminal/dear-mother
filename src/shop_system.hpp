#pragma once
#include "common.hpp"
#include "render_system.hpp"
#include "ui_system.hpp"
#include "tiny_ecs_registry.hpp"
#include "scene_manager.hpp"

class ShopSystem
{
public:
	ShopSystem();

	// Releases all associated resources
	~ShopSystem();

	// initialize
	void init(RenderSystem* renderer_arg, GLFWwindow* window_arg);

	void on_mouse_button(GLFWwindow* window, int button, int action, int mods);
	void on_mouse_move(vec2 mouse_position);
	static void updateScrapLevel(int updated_value);
	static int getScrapLevel();
private:
	RenderSystem* renderer;

	// Window handle
	GLFWwindow* window;

	vec2 cursor_position;

	SCENE_TYPE* scene;

	// storing this here for now
	// if we need this somewhere else, feel free to put it into common.hpp
	enum class UPGRADE_TYPE {
		ITEM_SLOT = 1,
		DMG_UPGRADE = ITEM_SLOT + 1,
		HEALTH_UPGRADE = DMG_UPGRADE + 1,
		CRIT_UPGRADE = HEALTH_UPGRADE + 1,
		DODGE_UPGRADE = CRIT_UPGRADE + 1
	};

	Entity crosshair;

	UPGRADE_TYPE viewed_upgrade;

	Entity viewed_upgrade_cost;

	Entity description_line_one;
	Entity description_line_two;
	Entity description_line_three;

	Entity buy_button;

	Entity item_slot_level;
	Entity dmg_upgrade_level;
	Entity health_upgrade_level;
	Entity crit_upgrade_level;
	Entity dodge_upgrade_level;
	Entity current_scrap_display;

	void initButtons();

	int getUpgradeLevel(UPGRADE_TYPE upgrade_type);

	void updateUpgrade(UPGRADE_TYPE upgrade_type);

	void updateUpgradeDescription();

	void buyUpgrade();

	bool is_mouse_within_button(WorldObject buttonObject);

	bool is_mouse_within_text_button(WorldObject buttonObject);
};