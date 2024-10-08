#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "render_system.hpp"

// BB = bounding box
const float ENEMY_BB_WIDTH = 100.f;
const float ENEMY_BB_HEIGHT = 175.f;

// the player
Entity createPlayer(RenderSystem* renderer, vec2 pos);

// the enemy
Entity createEnemy(RenderSystem* renderer, vec2 position, float velocity);

// Projectiles
Entity createProjectile(RenderSystem* renderer, vec2 pos, float angle, float speed, bool is_friendly);

// a red line for debugging purposes
Entity createLine(vec2 position, vec2 size);