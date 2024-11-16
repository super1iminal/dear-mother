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


enum class ROOM_TYPE {
	EMPTY = 0,
	ENEMY_ROOM = EMPTY + 1,
	BOSS_ROOM_ONE = ENEMY_ROOM + 1,
	BOSS_ROOM_TWO = BOSS_ROOM_ONE + 1,
	// ...
};

// Container for all our entities and game logic. Individual rendering / update is
// deferred to the relative update() methods
class WorldSystem
{
public:
	WorldSystem();

	// starts the game
	void init(RenderSystem* renderer_arg, GLFWwindow* window);

	// Releases all associated resources
	~WorldSystem();

	// Steps the game ahead by ms milliseconds
	bool step(float elapsed_ms);

	// update animations
	void update_animations();

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

	// restart level
	void restart_game();
	
	// C++ random number generator
	std::default_random_engine rng;
	std::uniform_real_distribution<float> uniform_dist; // number between 0..1

private:

	// Shooting stuff
	bool left_mouse_button = false;
	bool first_shot = true;
	void shoot(Entity& entity);

	// Boss One Stuff
	void boss_one_shoot(Entity& entity, WorldObject& entity_object);
	Entity final_phase_text;
	bool text_shown = false;
	Entity create_self_destruct_text(RenderSystem* renderer, ivec2 current_room);
	void handle_boss_one_death(Entity& entity);

	// Boss Two Stuff
	void createBossRoomTwo(ivec2 coord);
	void handle_boss_two();

	// Time management
	std::chrono::steady_clock::time_point t;
	std::chrono::steady_clock::time_point get_curr_time();
	void set_last_shot_time(Entity& entity);
	std::chrono::steady_clock::time_point get_last_shot_time(Entity& entity);

	// animation updates
	void updatePlayerAnimation();
	void updateEnemyAnimation(Entity enemy);
	void playEnemyAttack(Entity enemy);

	// Item stuff
	void update_player_modifier() const;
	void handle_item_pickup(Entity item);

	// OpenGL window handle
	GLFWwindow* window;

	// Number of fish eaten by the salmon, displayed in the window title
	unsigned int points;

	// Player health displayed on window
	unsigned int player_health;

	void increaseScrap(int amt);

	// Game state
	RenderSystem* renderer;
	float current_speed;
	uint level = 1;
	uint scrap = 0;
	Entity player;
	Entity boss_two;
	Entity floor;

	// HUD
	Entity health_ui;
	Entity scrap_ui;
	Entity level_ui;
	std::vector<Entity> items_ui;

	// upgrades
	void initUpgrades();

	// initialize HUD
	void initGameUI();

	// update HUD
	void updateGameUI();

	// Collision handling helpers
	void handlePlayerDeadly(Entity player, Entity deadly);
	void handlePlayerBossOne(Entity player, Entity boss);
	void handleActorBlocker(Entity actor, Entity blocker);
	void handleProjectileBlocker(Entity projectile, Entity blocker);
	void handleProjectileDeadly(Entity projectile, Entity deadly);
	void handleProjectilePlayer(Entity projectile, Entity player);
	void handlePlayerDoor(Entity entity, Entity entity_other);

	TEXTURE_ASSET_ID randomFloorItem();

	// music references
	Mix_Chunk* melee_sound;
	Mix_Chunk* player_shooting_sound;
	Mix_Chunk* enemy_shooting_sound;
	Mix_Chunk* player_projectile_damage_sound;
	Mix_Music* post_combat_music;
	Mix_Music* combat_music;

	//Music control
	bool change_music = true;
	bool enable_music = true;

	

	// For selecting item texture
	TEXTURE_ASSET_ID getItemTexture(ItemStat item);
	void generate_rooms();
	void generate_map();
	void change_rooms(ivec2 new_room);
	void createEnemyRoom(ivec2 coord);
	void createEmptyRoom(ivec2 coord);
	void createBossRoomOne(ivec2 coord);
	bool notSafe(vec2 position);
	ivec2 current_room;
};
