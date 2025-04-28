#pragma once



// ui stuff
#include "ui_system.hpp"
#include "tiny_ecs.hpp"
#include "menu_system.hpp"
#include "help_system.hpp"
#include "pause_system.hpp"
#include "shop_system.hpp"
#include "dialogue_system.hpp"

// scene stuff
#include "scene_manager.hpp"

class GameManager
{
private:
	// Input callback functions
	void cleanup();

	// Game systems
	PhysicsSystem physics;
	AISystem ai;
	DialogueSystem dialogue; // kinda
	WorldSystem world;

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