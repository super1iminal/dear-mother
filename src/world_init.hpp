#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "render_system.hpp"
#include <random>
// BB = bounding box
const float ENEMY_BB_WIDTH = 100.f;
const float ENEMY_BB_HEIGHT = 130.f;

const float PLAYER_MAX_SPEED = 300.f;

const float PLAYER_SIZE = 100.f;

// particle stuff
const float MAX_NUM_PARTICLES = 4;
const int NUM_PARTICLES_OFFSET = 1;
const float MAX_PARTICLE_LIFETIME = 1000.f;
const float MAX_PARTICLE_SIZE = 20.f;
const float PARTICLE_SIZE_OFFSET = 10.f;
const float MAX_PARTICLE_SPEED = 1500.f;
const float PARTICLE_SPEED_OFFSET = 500.0f;
const float MAX_PARTICLE_ACCELERATION = -50.f; // unused for now
const float MAX_PARTICLE_MASS = 10.f; // in kg
const float PARTICLE_DRAG_COEF = 0.47f; // drag coefficient for particles. intermediate reynolds number
const float VELOCITY_THRESHOLD = 0.1f; // if a particle is slower than this, it will be removed, sort of. calculated and set as lifetime


// projectile stuff
const int PROJECTILE_LIFESPAN = 2000; // in milliseconds

// wall stuff
const float WALL_WIDTH = 75.f;

// the particles
void createParticles(RenderSystem* renderer, vec2 pos, std::uniform_real_distribution<float> uniform_dist, std::default_random_engine& rng, TEXTURE_ASSET_ID type);

// the player
Entity createPlayer(RenderSystem* renderer, vec2 pos);

// the walls
Entity createWall(RenderSystem* renderer, vec2 pos, vec2 size, float angle, TEXTURE_ASSET_ID type);

// the enemy
Entity createEnemy(RenderSystem* renderer, vec2 position, float speed);

// floors
Entity createFloor(RenderSystem* renderer, vec2 position, vec2 size);

Entity createInteractable(RenderSystem* renderer, vec2 position, vec2 size, void (*a)(int));

// Projectiles
Entity createProjectile(RenderSystem* renderer, vec2 pos, float angle, float speed, bool is_friendly);

// a red line for debugging purposes
Entity createLine(vec2 position, vec2 size);