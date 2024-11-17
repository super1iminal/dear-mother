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
#include "ui_system.hpp"
#include "scene_manager.hpp"


// Container for all our entities and game logic. Individual rendering / update is
// deferred to the relative update() methods
class WorldSystem
{
public:
	// ==================== BASIC FUNCTIONS ====================
	WorldSystem();
	// Releases all associated resources
	~WorldSystem();
	// starts the game
	void init(RenderSystem* renderer_arg, GLFWwindow* window);
	// restart level
	void restart_game();
	// Steps the game ahead by ms milliseconds
	bool step(float elapsed_ms);


	// ==================== HANDLING FUNCTIONS ====================
	// ran once per step. public ones are called from game_manager.cpp
	// Check for collisions
	void handle_collisions();
	// Check for deaths
	void handle_deaths();


	// ==================== UPDATE FUNCTIONS ====================
	// ran once per step. public ones are called from game_manager.cpp
	// update animations
	void update_animations();


	// ==================== CALLBACK FUNCTIONS ====================
	// Input callback functions
	void on_key(int key, int sc, int action, int mod);
	void on_mouse_move(vec2 pos);
	void on_mouse_button(GLFWwindow* window, int button, int action, int mods);



private:
	// ==================== INIT FUNCTIONS ====================
	// upgrades
	void initUpgrades();
	// initialize HUD
	void initGameUI();
	

	// ==================== MEMBER VARIABLES ====================
	// Basics
	// OpenGL window handle
	GLFWwindow* window;
	RenderSystem* renderer;

	// C++ random number generator
	std::default_random_engine rng;
	std::uniform_real_distribution<float> uniform_dist; // number between 0..1

	// Shooting vars
	bool left_mouse_button = false;

	// Boss one vars
	Entity final_phase_text;
	bool text_shown = false;

	// Audio
	// music references
	Mix_Chunk* melee_sound;
	Mix_Chunk* player_shooting_sound;
	Mix_Chunk* enemy_shooting_sound;
	Mix_Chunk* player_projectile_damage_sound;
	Mix_Music* post_combat_music;
	Mix_Music* combat_music;
	//Music control
	bool change_music = true;

	// Player state
	Entity player;
	// player health displayed on window
	unsigned int player_health;
	uint level = 1;
	uint scrap = 0;

	// Game state
	float current_speed;
	Entity floor;
	ivec2 current_room;

	// HUD
	Entity health_ui;
	Entity scrap_ui;
	Entity level_ui;
	std::vector<Entity> items_ui;
	

	// ==================== ACTION FUNCTIONS ====================
	// functions that set/get/act things directly
	void shoot(Entity& entity);
	void increaseScrap(int amt);
	// Boss One Stuff
	void boss_one_shoot(Entity& entity, WorldObject& entity_object);
	// room stuff
	void change_rooms(ivec2 new_room);
	void set_last_shot_time(Entity& entity);
	// For selecting item texture
	TEXTURE_ASSET_ID getItemTexture(ItemStat item);
	std::chrono::steady_clock::time_point get_curr_time();
	// animation playing
	void playEnemyAttack(Entity enemy);


	// ==================== HANDLING FUNCTIONS ====================
	// ran once per step. private ones are called from inside world_system.cpp
	void handle_boss_one_death(Entity& entity);
	void handle_boss_two();
	void handle_item_pickup(Entity item);
	// check for interactions
	void handle_interactions();

	// Collision handling helpers
	void handlePlayerDeadly(Entity player, Entity deadly);
	void handlePlayerBossOne(Entity player, Entity boss);
	void handleActorBlocker(Entity actor, Entity blocker);
	void handleProjectileBlocker(Entity projectile, Entity blocker);
	void handleProjectileDeadly(Entity projectile, Entity deadly);
	void handleProjectilePlayer(Entity projectile, Entity player);
	void handlePlayerDoor(Entity entity, Entity entity_other);


	// ==================== UPDATE FUNCTIONS ====================
	// ran once per step. private ones are called from inside world_system.cpp
	// update HUD
	void updateGameUI();

	// animation updates
	void updatePlayerAnimation();
	void updateEnemyAnimation(Entity enemy);

	// Item stuff
	void update_player_modifier() const;	
};
