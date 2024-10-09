#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "render_system.hpp"

// BB = bounding box
const float ENEMY_BB_WIDTH = 100.f;
const float ENEMY_BB_HEIGHT = 175.f;

const float PLAYER_MAX_SPEED = 150.f;

const float PLAYER_SIZE = 100.f;

const float CROSSHAIR_SIZE = 75.f;

// the player
Entity createPlayer(RenderSystem* renderer, vec2 pos);

// the walls
Entity createWall(RenderSystem* renderer, vec2 pos, vec2 size, float angle);

// the enemy
Entity createEnemy(RenderSystem* renderer, vec2 position, float speed);

// the crosshair
Entity createCrosshair(RenderSystem* renderer);

// floors
Entity createFloor(RenderSystem* renderer, vec2 position, vec2 size);

Entity createInteractable(RenderSystem* renderer, vec2 position, vec2 size, void (*a)(int));

// Projectiles
Entity createProjectile(RenderSystem* renderer, vec2 pos, float angle, float speed, bool is_friendly);

// a red line for debugging purposes
Entity createLine(vec2 position, vec2 size);