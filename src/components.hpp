#pragma once
#include "common.hpp"
#include <vector>
#include <unordered_map>
#include "../ext/stb_image/stb_image.h"
#include <string>

#include <iostream>
#include <chrono>

// All data relevant to the motion of entities
struct Motion {
	float max_speed;
	vec2 velocity = { 0.f, 0.f };
	vec2 target_velocity = { 0.f, 0.f };
	vec2 acceleration = { 0.0f, 0.0f };
	float mass;
};

// a worldobject has a position, angle, and scale
struct WorldObject {
	vec2 position = { 0, 0 };
	float angle = 0;
	vec2 scale = { 10, 10 };
};

// Data structure for toggling debug mode
struct Debug {
	bool in_debug_mode = 0;
	bool in_freeze_mode = 0;
};
extern Debug debugging;

// Sets the brightness of the screen
struct ScreenState
{
	float darken_screen_factor = -1;
	int health_status = 0;
};

// gamescene stuff
struct GameScene
{
};

// information relating to animation
// rows and cols are for the spritesheet
// frames is the # of frames that the animation will cycle through
// current frame is the current frame for this animation.
// it must be specific to this animation to avoid other animations
// playing when they're not supposed to.
struct Animation {
	int rows;
	int cols;
	int frames;
	int current_frame; // starts at 0
	int time_since_last_frame;
};

// a struct for a flashing-color rendering effect
struct FlashingColor {
	bool flashing = true;	// if the modified color is displayed right now
	int flash_rate;			// time between flashes in ms
	int time_since_last_flash = 0;
	vec3 color;
};

struct Projectile
{
	bool friendly = true;
	int damage = 1;
};

// Stucture to store collision information
struct Collision
{
	// Note, the first object is stored in the ECS container.entities
	Entity other; // the second object involved in the collision
	COLLISION_TYPE type = COLLISION_TYPE::COLLISION_COUNT;

	Collision(Entity& other) { this->other = other; };
	Collision(Entity& other, COLLISION_TYPE type) : other(other), type(type) {}

};

// Player component
struct Player
{

};
struct Shooter
{
	// Fire Rate in ms (Temp: change to ranged weapon later)
	float fire_rate = 0.0f;
	std::chrono::steady_clock::time_point t;
};
// Common Health component
struct Health
{
	int max_health = 0;
	int curr_health = 0;
};

struct Modifier {

	int damage_modifier_flat = 0;

	float speed_modifier_flat = 0;
	float speed_modifier_percent = 0;

	float fire_rate_modifier_flat = 0;
	float fire_rate_modifier_percent = 0;

	float range_modifier_flat = 0;
	float range_modifier_percent = 0;

	float accuracy_modifier = 0;  // 0 would be perfect accuracy
};

// anything that is deadly to the player
struct Deadly
{
	DeadlyState state = DeadlyState::idle;
	std::chrono::steady_clock::time_point t;
	std::chrono::steady_clock::time_point t_patrol;
	bool attacking = false;
	unsigned int type = 0;
};

// anything that the player can interact with
struct Interactable {
	// the range that the player must be within to interact
	float range;
	// placeholder, not sure what we want the interaction function to do yet
	std::function<void(int)> interaction;
	// value to be used in function call
	int value;
};

// A timer that will be associated to dying salmon
struct DeathTimer
{
	float counter_ms = 3000;
};

// A timer that
struct InvincibleTimer
{
	float counter_ms = 3000;
};

struct Friction {
	float force;
};

struct Lifetime
{
	float time_remaining_ms = 0;
};

struct Particle
{

};

struct Blocker
{

};

struct Floor
{

};

struct Wall
{

};

struct MenuScene
{
};

struct HelpScene
{
};

struct PauseScene
{
};

struct ShopScene
{
};

struct TestScene
{
};

struct RoomCoordinate
{
	ivec2 position;
	RoomCoordinate(ivec2 position) : position(position) {}
};

// for GAME objects that are in the current room. nothing else.
struct Active
{
};

struct Door
{
	ivec2 leads_to; // is the room coords that the door leads to
	DIRECTION direction;
	Door(ivec2 leads_to, DIRECTION direction) : leads_to(leads_to), direction(direction) {};
};

// A struct to refer to debugging graphics in the ECS
struct DebugComponent
{
	// Note, an empty struct has size 1
};


// Single Vertex Buffer element for non-textured meshes (coloured.vs.glsl & salmon.vs.glsl)
struct ColoredVertex
{
	vec3 position;
	vec3 color;
};

// Single Vertex Buffer element for textured sprites (textured.vs.glsl)
struct TexturedVertex
{
	vec3 position;
	vec2 texcoord;
};

// contains information relating to UI elements
struct UIElement {
	std::string name;
	std::string value;
};

// contains information relating to UI buttons
struct UIButton {
	std::string name;
	std::function<void()> action;
};

struct PendingRemove {

};

// Mesh datastructure for storing vertex and index buffers
struct Mesh
{
	static bool loadFromOBJFile(std::string obj_path, std::vector<ColoredVertex>& out_vertices, std::vector<uint16_t>& out_vertex_indices, vec2& out_size);
	vec2 original_size = {1,1};
	std::vector<ColoredVertex> vertices;
	std::vector<uint16_t> vertex_indices;
};

struct MeshFlag {

};



struct FloorItem
{

};

struct BaseUI
{
	std::string name;
};

struct Crosshair {

};

/**
 * The following enumerators represent global identifiers refering to graphic
 * assets. For example TEXTURE_ASSET_ID are the identifiers of each texture
 * currently supported by the system.
 *
 * So, instead of referring to a game asset directly, the game logic just
 * uses these enumerators and the RenderRequest struct to inform the renderer
 * how to structure the next draw command.
 *
 * There are 2 reasons for this:
 *
 * First, game assets such as textures and meshes are large and should not be
 * copied around as this wastes memory and runtime. Thus separating the data
 * from its representation makes the system faster.
 *
 * Second, it is good practice to decouple the game logic from the render logic.
 * Imagine, for example, changing from OpenGL to Vulkan, if the game logic
 * depends on OpenGL semantics it will be much harder to do the switch than if
 * the renderer encapsulates all asset data and the game logic is agnostic to it.
 *
 * The final value in each enumeration is both a way to keep track of how many
 * enums there are, and as a default value to represent uninitialized fields.
 */

enum class TEXTURE_ASSET_ID { // if you add/change something here, you need to also add/change it in the texture_paths array in render_system.hpp
	PLAYER = 0,
	BOUNDBOX = PLAYER + 1,
	FLOOR = BOUNDBOX + 1,
	BOUNDBOX_BLUE = FLOOR + 1,
	BULLET_FRIENDLY = BOUNDBOX_BLUE + 1,
	BULLET_ENEMY = BULLET_FRIENDLY + 1,
	GAME_CROSSHAIR = BULLET_ENEMY + 1,
	MENU_CROSSHAIR = GAME_CROSSHAIR + 1,
	MENU_HOVER_CROSSHAIR = MENU_CROSSHAIR + 1,
	UI = MENU_HOVER_CROSSHAIR + 1,
	ENEMY = UI + 1,
	ENEMY_2 = ENEMY + 1,
	HORZ_WALL = ENEMY_2 + 1,
	VERT_WALL = HORZ_WALL + 1,
	DOOR_LEFT_RIGHT = VERT_WALL + 1,
	DOOR_UP_DOWN = DOOR_LEFT_RIGHT + 1,
	BATTERY_PACK = DOOR_UP_DOWN + 1,
	SHATTERED_QUARTZ = BATTERY_PACK + 1,
	CREAKY_WHEEL = SHATTERED_QUARTZ + 1,
	HEATSINK = CREAKY_WHEEL + 1,
	REPEATER = HEATSINK + 1,
	HIT_PARTICLE = REPEATER + 1,
	HIT_PARTICLE_PLAYER = HIT_PARTICLE + 1,
	START_MENU = HIT_PARTICLE_PLAYER + 1,
	HELP_SCREEN = START_MENU + 1,
	SHOP_SCREEN = HELP_SCREEN + 1,
	START_BUTTON = SHOP_SCREEN + 1,
	HELP_BUTTON = START_BUTTON + 1,
	SHOP_BUTTON = HELP_BUTTON + 1,
	QUIT_BUTTON = SHOP_BUTTON + 1,
	BACK_BUTTON = QUIT_BUTTON + 1,
	RESUME_BUTTON = BACK_BUTTON + 1,
	MENU_BUTTON = RESUME_BUTTON + 1, 
	ITEM_SLOT_BUTTON = MENU_BUTTON + 1,
	DMG_UPGRADE_BUTTON = ITEM_SLOT_BUTTON + 1,
	HEALTH_UPGRADE_BUTTON = DMG_UPGRADE_BUTTON + 1,
	CRIT_UPGRADE_BUTTON = HEALTH_UPGRADE_BUTTON + 1,
	DODGE_UPGRADE_BUTTON = CRIT_UPGRADE_BUTTON + 1,
	PLAYER_WALK = DODGE_UPGRADE_BUTTON + 1,
	ENEMY_WALK = PLAYER_WALK + 1,
	ENEMY_ATTACK = ENEMY_WALK + 1,
	ENEMY_2_WALK = ENEMY_ATTACK + 1,
	ENEMY_2_ATTACK = ENEMY_2_WALK + 1,

	// floor items must be kept together ====================================================================
	BROKEN_GENERATOR = ENEMY_2_ATTACK + 1,
	BROKEN_CONTROL_PANEL = BROKEN_GENERATOR + 1,
	DEAD_ROBOT = BROKEN_CONTROL_PANEL + 1,
	FLOOR_HOLE = DEAD_ROBOT + 1,
	FURNACE = FLOOR_HOLE + 1,
	RUSTY_PIPES = FURNACE + 1,
	SLAG_PIT = RUSTY_PIPES  + 1,
	// floor items must be kept together ====================================================================

	TEXTURE_COUNT = SLAG_PIT + 1,
};
const int texture_count = (int)TEXTURE_ASSET_ID::TEXTURE_COUNT;

enum class EFFECT_ASSET_ID {
	COLOURED = 0,
	EGG = COLOURED + 1,
	UI_ELEMENT = EGG + 1,
	FONT = UI_ELEMENT + 1,
	SALMON = FONT + 1,
	TEXTURED = SALMON + 1,
	ANIM = TEXTURED + 1,
	WATER = ANIM + 1,
	EFFECT_COUNT = WATER + 1
};
const int effect_count = (int)EFFECT_ASSET_ID::EFFECT_COUNT;

enum class GEOMETRY_BUFFER_ID {
	SALMON = 0,
	BULLET_FRIENDLY = SALMON + 1,
	BULLET_ENEMY = BULLET_FRIENDLY + 1,
	SQUARE = BULLET_ENEMY + 1,
	SPRITE = SQUARE + 1,
	EGG = SPRITE + 1,
	DEBUG_LINE = EGG + 1,
	SCREEN_TRIANGLE = DEBUG_LINE + 1,
	GEOMETRY_COUNT = SCREEN_TRIANGLE + 1
};
const int geometry_count = (int)GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;

struct RenderRequest {
	TEXTURE_ASSET_ID used_texture = TEXTURE_ASSET_ID::TEXTURE_COUNT;
	EFFECT_ASSET_ID used_effect = EFFECT_ASSET_ID::EFFECT_COUNT;
	GEOMETRY_BUFFER_ID used_geometry = GEOMETRY_BUFFER_ID::GEOMETRY_COUNT;
	uint render_order;
	static int compareFunction(RenderRequest request_one, RenderRequest request_two);
};


// moved here bc we need TEXTURE_ASSET_ID to be defined before this

struct ItemStat {
	ITEM_NAME name;
	ITEM_TYPE type;

	int flat_damage_mod = 0;

	float flat_speed_mod = 0;
	float percent_speed_mod = 0;

	float flat_fire_rate = 0;
	float percent_fire_rate = 0;

	float flat_range = 0;
	float percent_range = 0;

	float accuracy = 0;

	int heal_size = 0;
};


struct Inventory {
	std::vector<struct ItemStat> items;
	int size;	// how many slots unlocked
};