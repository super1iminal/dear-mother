#pragma once

// include literally everything
#include "common.hpp"
#include "render_system.hpp"
#include "tiny_ecs_registry.hpp"
#include "tiny_ecs.hpp"

// game stuff
#include "world_system.hpp"
#include "ai_system.hpp"
#include "physics_system.hpp"
#include "collision_system.hpp"
#include <reloadability_system.hpp>

// ui stuff
#include "ui_system.hpp"
#include "tiny_ecs.hpp"
#include "menu_system.hpp"
#include "help_system.hpp"
#include "pause_system.hpp"
#include "shop_system.hpp"

// scene stuff
#include "scene_manager.hpp"

class GameManager
{
private:
	// Input callback functions
	void on_key(int key, int sc, int action, int mod);
	void on_mouse_move(vec2 pos);
	void on_mouse_button(GLFWwindow* window, int button, int action, int mods);
	void cleanup();

	// Game systems
	WorldSystem world;
	PhysicsSystem physics;
	AISystem ai;

	// when adding here, remember to add init calls in init() function and add the mouse and keyboard callbacks in the switch statements
	// you'll also need to add the scene name in the enum in common.hpp
	// 
	MenuSystem menu;
	HelpSystem help;
	PauseSystem pause;
	ReloadabilitySystem reload;
	ShopSystem shop;


	// Scene systems
	CollisionSystem collisions;
	RenderSystem renderer;
	UISystem ui;

	// Window handle
	GLFWwindow* window;

public:
	GameManager();
	// Releases all associated resources
	~GameManager();
	bool init();
	bool is_over()const;
	bool step(float elapsed_ms, double fps);

};