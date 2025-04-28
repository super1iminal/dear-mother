#pragma once
#include "common.hpp"
#include "tiny_ecs.hpp"
#include "components.hpp"
#include "tiny_ecs_registry.hpp"

class CollisionHandlerSystem
{
public:
	void handle_collisions();
private:
	// Collision handling helpers
	void handle_player_deadly(Entity player, Entity deadly);
	void handle_player_boss_one(Entity player, Entity boss);
	void handle_actor_blocker(Entity actor, Entity blocker);
	void handle_projectile_blocker(Entity projectile, Entity blocker);
	void handle_projectile_deadly(Entity projectile, Entity deadly);
	void handle_projectile_player(Entity projectile, Entity player);
	void handle_player_door(Entity entity, Entity entity_other);
	void handle_crosshair_textured_UIElement(Entity crosshair, Entity texturedUIElement);
	void handle_crosshair_interactable(Entity crosshair, Entity texturedUIElement);
}