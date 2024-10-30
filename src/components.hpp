#pragma once
#include "common.hpp"
#include <vector>
#include <unordered_map>
#include "../ext/stb_image/stb_image.h"
#include <string>

#include <iostream>
#include <chrono>

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

struct Inventory {
	std::vector<struct ItemStat> items;
};

// anything that is deadly to the player
struct Deadly
{

};

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

struct GameScene
{
};

struct MenuScene
{
};

struct PauseScene
{
};

struct TestScene
{
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

// A struct to refer to debugging graphics in the ECS
struct DebugComponent
{
	// Note, an empty struct has size 1
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
	float value;
};

// contains information relating to UI buttons
struct UIButton {
	std::string name;
	std::function<void()> action;
};

struct PendingRemove {

};

struct Friction {
	float force;
};

// Mesh datastructure for storing vertex and index buffers
struct Mesh
{
	static bool loadFromOBJFile(std::string obj_path, std::vector<ColoredVertex>& out_vertices, std::vector<uint16_t>& out_vertex_indices, vec2& out_size);
	vec2 original_size = {1,1};
	std::vector<ColoredVertex> vertices;
	std::vector<uint16_t> vertex_indices;
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

struct BaseUI
{
	std::string name;
};

struct Crosshair {

};

// font character structure
struct Character {
	unsigned int TextureID;  // ID handle of the glyph texture
	glm::ivec2   Size;       // Size of glyph
	glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
	unsigned int Advance;    // Offset to advance to next glyph
	char character;
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
	HORZ_WALL = ENEMY + 1,
	VERT_WALL = HORZ_WALL + 1,
	ITEM = VERT_WALL + 1,
	HIT_PARTICLE = ITEM + 1,
	HIT_PARTICLE_PLAYER = HIT_PARTICLE + 1,
	START_MENU = HIT_PARTICLE_PLAYER + 1,
	HELP_SCREEN = START_MENU + 1,
	START_BUTTON = HELP_SCREEN + 1,
	HELP_BUTTON = START_BUTTON + 1,
	SHOP_BUTTON = HELP_BUTTON + 1,
	QUIT_BUTTON = SHOP_BUTTON + 1,
	BACK_BUTTON = QUIT_BUTTON + 1,
	TEXTURE_COUNT = BACK_BUTTON + 1,
};
const int texture_count = (int)TEXTURE_ASSET_ID::TEXTURE_COUNT;

enum class EFFECT_ASSET_ID {
	COLOURED = 0,
	EGG = COLOURED + 1,
	UI_ELEMENT = EGG + 1,
	FONT = UI_ELEMENT + 1,
	SALMON = FONT + 1,
	TEXTURED = SALMON + 1,
	WATER = TEXTURED + 1,
	EFFECT_COUNT = WATER + 1
};
const int effect_count = (int)EFFECT_ASSET_ID::EFFECT_COUNT;

enum class GEOMETRY_BUFFER_ID {
	SALMON = 0,
	SQUARE = SALMON + 1,
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
};


// moved here bc we need TEXTURE_ASSET_ID to be defined before this

struct ItemStat {
	std::string name;
	std::string type;

	int flat_damage_mod = 0;

	float flat_speed_mod = 0;
	float percent_speed_mod = 0;

	float flat_fire_rate = 0;
	float percent_fire_rate = 0;

	float flat_range = 0;
	float percent_range = 0;

	float accuracy = 0;

	int heal_size = 0;

	TEXTURE_ASSET_ID item_texture;
};

