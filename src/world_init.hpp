#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "render_system.hpp"
#include <random>
// ==================== CONSTANTS ==================== 
const float ENEMY_BB_WIDTH = 98.f;
const float ENEMY_BB_HEIGHT = 131.f;
const float ENEMY_SPEED = 150.f;
const float PLAYER_MAX_SPEED = 450.f;
const float PLAYER_SIZE = 90.f;
const int PLAYER_MAX_HEALTH = 5;
const int DEADLY_MAX_HEALTH = 5;
const int PLAYER_BASE_INV_SIZE = 2;
const float OLD_ROBOT_WIDTH = 444.f;
const float OLD_ROBOT_HEIGHT = 264.f;

const float SCARECROW_WIDTH = 197.f;
const float SCARECROW_HEIGHT = 161.2;

const float DAMAGE_UPGRADE_MODIFIER = 0.15f;
const int CRIT_DAMAGE_UPGRADE_MODIFIER = 3;

const float HEAVY_HEALTH = 10;
const float HEAVY_TYPE = 4;
const float FLY_TYPE = 5;
const float BOSS_ONE = 3;
const float BOSS_THREE = 6;

// particle stuff
const float MAX_NUM_PARTICLES = 4;
const int   NUM_PARTICLES_OFFSET = 1;
const float MAX_PARTICLE_LIFETIME = 1000.f;
const float MAX_PARTICLE_SIZE = 35.f;
const float PARTICLE_SIZE_OFFSET = 15.f;
const float MAX_PARTICLE_SPEED = 2250.f;
const float PARTICLE_SPEED_OFFSET = 750.0f;
const float MAX_PARTICLE_ACCELERATION = -75.f; // unused for now
const float MAX_PARTICLE_MASS = 10.f; // in kg
const float PARTICLE_DRAG_COEF = 0.47f; // drag coefficient for particles. intermediate reynolds number
const float VELOCITY_THRESHOLD = 0.1f; // if a particle is slower than this, it will be removed, sort of. calculated and set as lifetime

// projectile stuff
const int PROJECTILE_LIFESPAN = 2000; // in milliseconds

// wall stuff
const float WALL_WIDTH = 90.f;

// center x pos
const float CENTER_X = window_width_px / 2;

// center y pos
const float CENTER_Y = window_height_px - (window_height_px - BASE_UI_HEIGHT) / 2;

// item stuff
const float ITEM_SIZE = 113.f;
const float MAX_ACCURACY = 1.f; // 0 is perfect accuracy
const float MIN_RANGE = 200.f;

// floor item stuff
const float FLOOR_ITEM_SIZE = 113.f;
const float FLOOR_ITEM_BUFFER = 23.f;

// Item drop chance
const float DROP_CHANCE = 25;

// ==================== CREATE FUNCTIONS ====================

Entity createParticle(RenderSystem* renderer, 
	vec2 pos, 
	std::uniform_real_distribution<float> uniform_dist, 
	std::default_random_engine& rng, 
	TEXTURE_ASSET_ID type, 
	ivec2 room_coord
);

// the player
Entity createPlayer(RenderSystem* renderer,vec2 pos,int curr_health = PLAYER_MAX_HEALTH,
	ivec2 room_coord = {0, 0});

// the walls
Entity createWall(RenderSystem* renderer, vec2 pos, vec2 size, float angle, TEXTURE_ASSET_ID type, ivec2 room_coord);

// the enemy
Entity createEnemy(
	RenderSystem* renderer,
	vec2 position,
	float speed,
	ivec2 room_coord,
	int curr_health = 5,
	int type = (int)rand() % 2
);

// put text on the floor
Entity createFloorText(RenderSystem* renderer, std::string text, vec2 pos, vec2 scale, ivec2 room_coord);

// boss one (should be 3rd boss)
Entity createBossOne(RenderSystem* renderer, vec2 pos, BOSS_ONE_POS boss_pos, ivec2 room_coord);

// boss two (should be 1st boss)
Entity createBossTwo(RenderSystem* renderer, vec2 pos, ivec2 room_coord);

// boss three (should be 2nd boss)
Entity createBossThree(RenderSystem* renderer, vec2 pos, ivec2 room_coord);

// floors
Entity createFloor(RenderSystem* renderer, vec2 position, vec2 size, ivec2 room_coord, FLOOR_TYPE floor_type = FLOOR_TYPE::DEFAULT);

// doors
Entity createDoor(RenderSystem* renderer, ivec2 room_coord, ivec2 leads_to, DIRECTION orientation);

Entity createInteractable(RenderSystem* renderer, vec2 position, vec2 size, std::function<void(int)> function, int value, ivec2 room_coord);

// Projectiles
Entity createProjectile(RenderSystem* renderer, vec2 pos, float angle, float speed, bool is_friendly, ivec2 room_coord, int lifespan = PROJECTILE_LIFESPAN);

// items
Entity createItem(RenderSystem* renderer, vec2 position, vec2 size,  std::uniform_real_distribution<float> uniform_dist, std::default_random_engine& rng, ivec2 room_coord,ITEM_TYPE spec_type = ITEM_TYPE::RANDOM, ItemStat* loaded_item = nullptr);
Entity createSparkles(RenderSystem* renderer, vec2 position, vec2 size, ivec2 room_coord);

// a red line for debugging purposes
Entity createLine(vec2 position, vec2 size);

Entity create_self_destruct_text(RenderSystem* renderer, ivec2 current_room);

Entity createNPC(RenderSystem* renderer, vec2 pos, vec2 size, ivec2 room_coord, NPC_TYPE npc_type);


// ==================== COMPLEX CREATE FUNCTIONS ====================
// the particles
void createParticles(RenderSystem* renderer, 
	vec2 pos, 
	std::uniform_real_distribution<float> uniform_dist, // TODO: might wanna pass by reference!
	std::default_random_engine& rng, 
	TEXTURE_ASSET_ID type, 
	ivec2 room_coord
);


// map stuff
void createEmptyRoom(RenderSystem* renderer, ivec2 coord);

void createEnemyRoom(RenderSystem* renderer, ivec2 coord, TEXTURE_ASSET_ID type);
void enemyRoomGenerateFloorItems(RenderSystem* renderer, ivec2 coord, ROOM_TYPE type);
void enemyRoomGenerateEnemies(RenderSystem* renderer, ivec2 coord, ROOM_TYPE type);

void createBossRoomOne(RenderSystem* renderer, ivec2 coord);

void createBossRoomTwo(RenderSystem* renderer, ivec2 coord);

void createBossRoomThree(RenderSystem* renderer, ivec2 coord);

void generate_map();

void generate_rooms(RenderSystem* renderer, ivec2 current_room, std::uniform_real_distribution<float> uniform_dist, std::default_random_engine& rng);



// ==================== ITEM STUFF ====================
// create all item components
void buildItemSet();

void createLoadedGame(RenderSystem* renderer);
TEXTURE_ASSET_ID randomFloorItem();
