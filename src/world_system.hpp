#pragma once

// internal
#include "common.hpp"

// stlib
#include <vector>
#include <random>
#include <chrono>
#include <SDL.h>
#include <SDL_mixer.h>

#include "render_system.hpp"
#include "collision_system.hpp"
#include "physics_system.hpp"
#include "ui_system.hpp"
#include "scene_manager.hpp"
#include "scene.hpp"
#include "dialogue_system.hpp"
#include "audio_system.hpp"


// Container for all our entities and game logic. Individual rendering / update is
// deferred to the relative update() methods
class WorldSystem : Scene
{
public:
	// ==================== BASIC FUNCTIONS ====================
	WorldSystem(SceneManager& manager);
	// Releases all associated resources
	~WorldSystem();
	// starts the game
	void init(RenderSystem* renderer_arg, GLFWwindow* window, AudioSystem* audio_arg);
	// restart level
	void restart_game();
	void go_next_stage();
	void load_game();
	// Steps the game ahead by ms milliseconds
	void step(float elapsed_ms);

	void exit();
	void enter();

	// ==================== ACTION FUNCTIONS ==================== 
	void set_player_velocity(vec2 velocity);
	void set_combat_state(COMBAT_STATE cs);

	// ==================== HANDLING FUNCTIONS ====================
	// ran once per step. public ones are called from game_manager.cpp
	// Check for deaths
	void handle_deaths();


	// ==================== UPDATE FUNCTIONS ====================
	// (not always) ran once per step. public ones are called from game_manager.cpp
	// update animations
	void update_animations();
	void update_music();
	void update_crosshair_cooldown();
	void update_doors(); // switches door textures to be open_closed

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

	void pre_start();
	void post_start();

	AudioSystem* audio_system;
	

	// ==================== MEMBER VARIABLES ====================
	// Basics
	// OpenGL window handle
	GLFWwindow* window;
	RenderSystem* renderer;

	// C++ random number generator
	std::default_random_engine rng;
	std::uniform_real_distribution<float> uniform_dist; // number between 0..1

	// Mouse vars
	bool left_mouse_button = false;
	vec2 cursor_position;

	// Boss one vars
	Entity final_phase_text;
	bool text_shown = false;

	// Player state
	Entity player;
	// player health displayed on window
	unsigned int player_health;
	uint level = 1;
	uint scrap = 0;
	vec2 death_coor = {-1,-1};

	// Game state
	float current_speed;
	Entity floor;
	ivec2 current_room;

	// HUD
	Entity health_ui;
	Entity scrap_ui;
	Entity level_ui;
	std::vector<Entity> health_segments_ui;
	int health_segment_length;

	// In Game Crosshair
	Entity game_crosshair;
	bool cooldown_in_progress = false;
	
	// Item stuff
	int shop_crit_upgrade = 0;
	int shop_dodge_upgrade = 0;

	// Const for ITEM_NAME to string conversion
	const std::map<ITEM_NAME, std::string> itemNameToString = { { ITEM_NAME::BATTERY_PACK, "Battery Pack" }, 
																{ ITEM_NAME::HEATSINK, "Heatsink" }, 
																{ ITEM_NAME::CREAKY_WHEEL, "Creaky Wheel" },
																{ ITEM_NAME::REPEATER, "Repeater" },
																{ ITEM_NAME::SHATTERED_QUARTZ, "Shattered Quartz" },
																{ ITEM_NAME::SUPERCHARGED_BATTERY_PACK, "Supercharged Battery Pack" },
																{ ITEM_NAME::WD4000, "WD-4000" },
																{ ITEM_NAME::VOLITILE_BLASTER, "Volitile Blaster" },
																{ ITEM_NAME::HOT_DIESEL, "Hot Diesel" },
																{ ITEM_NAME::OPTICAL_SENSOR, "Optical Sensor" },
																{ ITEM_NAME::NOS, "NOS" },
																{ ITEM_NAME::STABILIZER, "Stabilizer" },
																{ ITEM_NAME::UNSTABLE_TRANSFORMER, "Unstable Transformer" },
																{ ITEM_NAME::THERMAL_PASTE, "Thermal Paste" },};

	const std::unordered_map<ITEM_NAME, std::string> itemNameToDescription = {
		{ ITEM_NAME::BATTERY_PACK, "Battery Pack (Heals 1 HP)" },
		{ ITEM_NAME::HEATSINK, "Heatsink (+10% fire rate)" },
		{ ITEM_NAME::CREAKY_WHEEL, "Creaky Wheel (+20% speed)" },
		{ ITEM_NAME::REPEATER, "Repeater (+100 range)" },
		{ ITEM_NAME::SHATTERED_QUARTZ, "Shattered Quartz (+100% damage, -200 range, -30% fire rate, -30% accuracy)" },
		{ ITEM_NAME::SUPERCHARGED_BATTERY_PACK, "Supercharged Battery Pack (Heals 2 HP)" },
		{ ITEM_NAME::WD4000, "WD-4000 (+20% fire rate, -10% accuracy)" },
		{ ITEM_NAME::VOLITILE_BLASTER, "Volatile Blaster (+3% crit chance)" },
		{ ITEM_NAME::HOT_DIESEL, "Hot Diesel (+10% speed, +2% dodge chance)" },
		{ ITEM_NAME::OPTICAL_SENSOR, "Optical Sensor (+75 range, +1% dodge chance)" },
		{ ITEM_NAME::NOS, "NOS (+5% speed, +5% dodge chance)" },
		{ ITEM_NAME::STABILIZER, "Stabilizer (+75 range, +10% accuracy)" },
		{ ITEM_NAME::UNSTABLE_TRANSFORMER, "Unstable Transformer (+6% crit, -10% accuracy)" },
		{ ITEM_NAME::THERMAL_PASTE, "Thermal Paste (+5% fire rate)" },
	};

	const std::unordered_map<NPC_TYPE, std::string> NPCtypeToDescription = {
		{NPC_TYPE::OLD_ROBOT_NPC, "A wanderer, an advisor, a relic."},
		{NPC_TYPE::BILLYBOY_NPC, "Traditional, survivalist, rowdy."},
	};
	
	// ==================== ACTION FUNCTIONS ====================
	// functions that set/get/act things directly
	void shoot(Entity& entity);
	void increaseScrap(int amt);
	// Boss One Stuff
	void boss_one_shoot(Entity& entity, WorldObject& entity_object);
	// Boss Three Stuff
	void boss_three_shoot(Entity& entity, WorldObject& entity_object);
	// room stuff
	void change_rooms(ivec2 new_room);
	void set_last_shot_time(Entity& entity);
	// For selecting item texture
	TEXTURE_ASSET_ID getItemTexture(ItemStat item);
	ITEM_NAME WorldSystem::getItemNameFromTexture(TEXTURE_ASSET_ID texture_id);
	std::chrono::steady_clock::time_point get_curr_time();
	// animation playing
	void playEnemyAttack(Entity enemy);
	void display_death_screen();
	void display_victory_screen();


	// ==================== HANDLING FUNCTIONS ====================
	// ran once per step. private ones are called from inside world_system.cpp
	void handle_boss_one_death(Entity& entity);
	void handle_boss_two();
	void handle_boss_two_death(Entity& entity);
	void handle_boss_three_death(Entity& entity);
	void boss_disable_items();
	void enable_all_items();
	void handle_item_pickup(Entity item);
	void handle_scrapping(Entity item);
	void handle_item_drop(int item_key);
	// check for interactions
	void handle_interactions(void (WorldSystem::*func)(Entity));



	// ==================== UPDATE FUNCTIONS ====================
	// ran once per step. private ones are called from inside world_system.cpp
	// update HUD
	void updateGameUI();
	void drawItemInventory(); // helper for initializing and updating the inventory UI

	// animation updates
	void updatePlayerAnimation();
	void updateEnemyAnimation(Entity enemy);

	// Item stuff
	void update_player_modifier() const;

	// ======================== DEATH ANIMS ==============================
	void WorldSystem::show_player_death(Entity& entity);
};
