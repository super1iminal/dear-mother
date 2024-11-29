#pragma once

// standard libs
#include <string>
#include <tuple>
#include <vector>

// glfw (OpenGL)
#define NOMINMAX
#include <gl3w.h>
#include <GLFW/glfw3.h>

// The glm library provides vector and matrix operations as in GLSL
#include <glm/vec2.hpp>				// vec2
#include <glm/ext/vector_int2.hpp>  // ivec2
#include <glm/vec3.hpp>             // vec3
#include <glm/mat3x3.hpp>           // mat3
using namespace glm;

#include "tiny_ecs.hpp"

// Simple utility functions to avoid mistyping directory name
// audio_path("audio.ogg") -> data/audio/audio.ogg
// Get defintion of PROJECT_SOURCE_DIR from:
#include "../ext/project_path.hpp"
inline std::string data_path() { return std::string(PROJECT_SOURCE_DIR) + "data"; };
inline std::string shader_path(const std::string& name) {return std::string(PROJECT_SOURCE_DIR) + "/shaders/" + name;};
inline std::string textures_path(const std::string& name) {return data_path() + "/textures/" + std::string(name);};
inline std::string audio_path(const std::string& name) {return data_path() + "/audio/" + std::string(name);};
inline std::string mesh_path(const std::string& name) {return data_path() + "/meshes/" + std::string(name);};
inline std::string reload_path(const std::string& name) {
	return data_path() + "/saved_game/" + std::string(name);
};
inline std::string dialogue_path(const std::string& name) {
	return data_path() + "/dialogue/" + std::string(name);
};

inline std::string npc_path(const std::string& name) {
	return textures_path("npcs/") + std::string(name);
};
const int window_width_px = 1920;
const int window_height_px = 1080;
bool on_screen(vec2 position);


// gravity
#ifndef M_G
#define M_G 9.81f
#endif

// air pressure at sea level
#ifndef M_RHO
#define M_RHO 1.225f
#endif

// pi
#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

// The 'Transform' component handles transformations passed to the Vertex shader
// (similar to the gl Immediate mode equivalent, e.g., glTranslate()...)
// We recomment making all components non-copyable by derving from ComponentNonCopyable
struct Transform {
	mat3 mat = { { 1.f, 0.f, 0.f }, { 0.f, 1.f, 0.f}, { 0.f, 0.f, 1.f} }; // start with the identity
	void scale(vec2 scale);
	void rotate(float radians);
	void translate(vec2 offset);
};

// constants
const float CROSSHAIR_SIZE = 75.f;
const float BASE_UI_HEIGHT = window_height_px / 6;
const int MAX_INVENTORY_SIZE = 8;
const int MAX_UPGRADE_LEVEL = 6;
const int MIN_INVENTORY_SIZE = 2;
const int MIN_HEALTH = 2;

// constants for UI
const float INITIAL_ITEM_UI_OFFSET_X = (window_width_px / 22);
const float INITIAL_ITEM_UI_OFFSET_Y = window_height_px / 12;
const float ITEM_UI_OFFSET_X = window_width_px * 0.081;
const float HEALTH_UI_LENGTH = 330.f;
const float HEALTH_UI_HEIGHT = 122.f;

bool gl_has_errors();

vec2 v_from_sa(float speed, float angle);
float a_from_v(vec2 velocity);
float s_from_v(vec2 velocity);

enum class COLLISION_TYPE {
	PROJECTILE_DEADLY = 0,
	PROJECTILE_PLAYER = PROJECTILE_DEADLY + 1,
	PROJECTILE_BLOCKER = PROJECTILE_PLAYER + 1,
	PLAYER_BLOCKER = PROJECTILE_BLOCKER + 1,
	DEADLY_BLOCKER = PLAYER_BLOCKER + 1,
	PLAYER_DEADLY = DEADLY_BLOCKER + 1,
	PLAYER_DOOR = PLAYER_DEADLY + 1,
	PLAYER_BOSS_ONE = PLAYER_DOOR + 1,
	COLLISION_COUNT = PLAYER_BOSS_ONE + 1
};

// font character structure
struct Character {
	unsigned int TextureID;  // ID handle of the glyph texture
	glm::ivec2   Size;       // Size of glyph
	glm::ivec2   Bearing;    // Offset from baseline to left/top of glyph
	unsigned int Advance;    // Offset to advance to next glyph
	char character;
};


enum class ITEM_NAME {
	BATTERY_PACK = 0,
	SHATTERED_QUARTZ = BATTERY_PACK + 1,
	CREAKY_WHEEL = SHATTERED_QUARTZ + 1,
	HEATSINK = CREAKY_WHEEL + 1,
	REPEATER = HEATSINK + 1,
	WD4000 = REPEATER + 1,
	SUPERCHARGED_BATTERY_PACK = WD4000 + 1,
	VOLITILE_BLASTER = SUPERCHARGED_BATTERY_PACK + 1,
	HOT_DIESEL = VOLITILE_BLASTER + 1,
	OPTICAL_SENSOR = HOT_DIESEL + 1,
};

enum class ITEM_TYPE {
	DAMAGE = 0,
	SPEED = DAMAGE + 1,
	FIRE_RATE = SPEED + 1,
	RANGE = FIRE_RATE + 1,
	HEALTH_PACK = RANGE + 1,
	RANDOM = HEALTH_PACK + 1,
};

enum class DIRECTION {
	UP = 1,
	DOWN = UP + 1,
	LEFT = DOWN + 1,
	RIGHT = LEFT + 1
};

enum class DeadlyState {
	idle = 0,
	patrol_left = idle + 1,
	patrol_right = patrol_left + 1,
	attack_still = patrol_right + 1,
	attack_moving = attack_still + 1,
	rage = attack_moving + 1
};

enum class BOSS_ONE_POS {
	TOP_LEFT = 0,
	TOP_RIGHT = TOP_LEFT + 1,
	BOT_LEFT = TOP_RIGHT + 1,
	BOT_RIGHT = BOT_LEFT + 1,
	MOTHER = BOT_RIGHT + 1,
};

enum class BOSS_TWO_WAVE {
	WAVE_ONE = 0,
	WAVE_TWO = WAVE_ONE + 1,
	WAVE_THREE = WAVE_TWO + 1,
	WAVE_FOUR = WAVE_THREE + 1,
};

enum class BOSS_ONE_STATE {
	TWO_OPPOSITE_SIDE_ALIVE_L_L = 0,
	TWO_OPPOSITE_SIDE_ALIVE_BR_TL = TWO_OPPOSITE_SIDE_ALIVE_L_L + 1,
	TWO_OPPOSITE_SIDE_ALIVE_BL_TR = TWO_OPPOSITE_SIDE_ALIVE_BR_TL + 1,
	TWO_OPPOSITE_SIDE_ALIVE_R_R = TWO_OPPOSITE_SIDE_ALIVE_BL_TR + 1,
	TWO_SAME_SIDE_ALIVE_TOP = TWO_OPPOSITE_SIDE_ALIVE_R_R + 1,
	TWO_SAME_SIDE_ALIVE_BOT = TWO_SAME_SIDE_ALIVE_TOP + 1,
	THREE_ALIVE_ONE_TOP_LEFT = TWO_SAME_SIDE_ALIVE_BOT + 1,
	THREE_ALIVE_ONE_BOT_LEFT = THREE_ALIVE_ONE_TOP_LEFT + 1,
	THREE_ALIVE_ONE_TOP_RIGHT = THREE_ALIVE_ONE_BOT_LEFT + 1,
	THREE_ALIVE_ONE_BOT_RIGHT = THREE_ALIVE_ONE_TOP_RIGHT + 1,
	FOUR_ALIVE = THREE_ALIVE_ONE_BOT_RIGHT + 1,
	ONE_ALIVE_T_L = FOUR_ALIVE + 1,
	ONE_ALIVE_T_R = ONE_ALIVE_T_L + 1,
	ONE_ALIVE_B_L = ONE_ALIVE_T_R + 1,
	ONE_ALIVE_B_R = ONE_ALIVE_B_L + 1,
	START = ONE_ALIVE_B_R + 1,
	ALL_DEAD = START + 1,
};

enum class BOSS_THREE_PHASE {
	PHASE_ONE = 0,
	PHASE_TWO = PHASE_ONE + 1,
	PHASE_THREE = PHASE_TWO + 1,
};

enum class ROOM_TYPE {
	EMPTY = 0,
	ENEMY_ROOM = EMPTY + 1,
	BOSS_ROOM_ONE = ENEMY_ROOM + 1,
	BOSS_ROOM_TWO = BOSS_ROOM_ONE + 1,
	BOSS_ROOM_THREE = BOSS_ROOM_TWO + 1,
	OLD_ROBOT_ROOM = BOSS_ROOM_THREE + 1,
	SCARECROW_ROOM = OLD_ROBOT_ROOM + 1,
	TWO_SIMPLE = SCARECROW_ROOM + 1,
	MIDLINE_PROJ = TWO_SIMPLE + 1,
	CORNER_MIX = MIDLINE_PROJ + 1,
	ONE_HEAVY = CORNER_MIX + 1,
	LAPS = ONE_HEAVY + 1,
	CHECKERBOARD = LAPS + 1,
	BIG_X = CHECKERBOARD + 1,
	TUNNELS = BIG_X + 1,
	SCATTER = TUNNELS + 1,
	TWO_HEAVY = SCATTER + 1,
	ENEMY_SOCIAL = TWO_HEAVY + 1,
	FLY_TUNNELS = ENEMY_SOCIAL + 1,
	FLY_LAPS = FLY_TUNNELS + 1,
	// ...
};

enum class COMBAT_STATE {
	NO_COMBAT = 0,
	NORMAL_COMBAT = NO_COMBAT + 1,
	BOSS_ONE_COMBAT = NORMAL_COMBAT + 1,
	BOSS_TWO_COMBAT = BOSS_ONE_COMBAT + 1,
	BOSS_THREE_COMBAT = BOSS_TWO_COMBAT + 1,
};

enum class NPC_TYPE {
	OLD_ROBOT_NPC = 0,
	SCARECROW_NPC = OLD_ROBOT_NPC + 1,
	// ...
};

enum class RENDER_ORDER {
	FLOOR = 2,
	WALL = 3,
	DOOR = 4,
	INTERACTABLE = 5,
	ITEM = 6,
	PARTICLE = 7,
	ENEMY = 9,
	PLAYER = 10,
	FRIENDLY_PROJECTILE = 11,
	DEADLY_PROJECTILE = 12,
	UI_PANEL = 14,
	TEXTURED_UI_ELEMENT = 15,
	UI_ELEMENT = 16,
	UI_BUTTON = 17,
	LINE = 20,
	CROSSHAIR = 25,
};

enum class FLOOR_TYPE {
	DEFAULT = 0,
	BOSS_ROOM_ONE = DEFAULT + 1,
	// ...
};

enum class TEXT_BOX_TYPE {
	DEFAULT = 0,
	DEATH_SCREEN = DEFAULT + 1,
};