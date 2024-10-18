#pragma once

// internal
#include "common.hpp"

// stlib
#include <vector>
#include <random>
#include <chrono>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>

#include "render_system.hpp"
#include "collision_system.hpp"
#include "physics_system.hpp"

// Container for all our entities and game logic. Individual rendering / update is
// deferred to the relative update() methods
class WorldSystem
{
public:
	WorldSystem();

	// starts the game
	void WorldSystem::init(RenderSystem* renderer_arg, GLFWwindow* window);

	// Releases all associated resources
	~WorldSystem();

	// Steps the game ahead by ms milliseconds
	bool step(float elapsed_ms, double fps);

	// Check for collisions
	void handle_collisions();

	// Check for deaths
	void handle_deaths();

	// check for interactions
	void handle_interactions();

	// Input callback functions
	void on_key(int key, int sc, int action, int mod);
	void on_mouse_move(vec2 pos);
	void on_mouse_button(GLFWwindow* window, int button, int action, int mods);
private:

	// restart level
	void restart_game();

	// Shooting stuff
	bool left_mouse_button = false;
	bool first_shot = true;
	void shoot(Entity& entity);

	// Time management
	std::chrono::steady_clock::time_point t;
	std::chrono::steady_clock::time_point WorldSystem::get_curr_time();
	void set_last_shot_time(Entity& entity);
	std::chrono::steady_clock::time_point get_last_shot_time(Entity& entity);

	// OpenGL window handle
	GLFWwindow* window;

	// Number of fish eaten by the salmon, displayed in the window title
	unsigned int points;

	// Player health displayed on window
	unsigned int player_health;

	// Game state
	RenderSystem* renderer;
	float current_speed;
	int level = 1;
	int scrap = 28;
	Entity items [8];
	Entity player;
	Entity floor;

	// UI entities
	// TODO: why are these constants here?
	Entity health_ui;
	Entity scrap_ui;
	Entity level_ui;
	Entity item_ui;

	// Collision handling helpers
	void handlePlayerDeadly(Entity player, Entity deadly);

	void handleActorBlocker(Entity actor, Entity blocker);

	void handleProjectileBlocker(Entity projectile, Entity blocker);

	void handleProjectileDeadly(Entity projectile, Entity deadly);

	void handleProjectilePlayer(Entity projectile, Entity player);

	// music references
	Mix_Music* background_music;
	Mix_Chunk* salmon_dead_sound;
	Mix_Chunk* salmon_eat_sound;

	// C++ random number generator
	std::default_random_engine rng;
	std::uniform_real_distribution<float> uniform_dist; // number between 0..1
};
