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
Entity createEnemy(RenderSystem* renderer, vec2 position);

// a red line for debugging purposes
Entity createLine(vec2 position, vec2 size);

// create base UI that other UI elements will be layered on
Entity createBaseUI(RenderSystem* renderer);

// create UI element
Entity createTexturedUIElement(RenderSystem* renderer, vec2 pos, vec2 scale, std::string element_name, float element_value);

// create UI element using texture
Entity createTexturedUIElement(RenderSystem* renderer, vec2 pos, vec2 scale, std::string element_name, TEXTURE_ASSET_ID texture_id);