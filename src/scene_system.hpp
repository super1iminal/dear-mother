#pragma once
#include "physics_system.hpp"
#include "render_system.hpp"
#include "world_system.hpp"
#include "ai_system.hpp"
#include "collision_system.hpp"
#include "common.hpp"
#include "tiny_ecs_registry.hpp"
#include "tiny_ecs.hpp"

class SceneSystem
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

	// Scene systems
	CollisionSystem collisions;
	RenderSystem renderer;

	// Window handle
	GLFWwindow* window;

public:
	SceneSystem();
	// Releases all associated resources
	~SceneSystem();
	bool init();
	SCENE_TYPE scene;
	bool is_over()const;
	bool step(float elapsed_ms);

};