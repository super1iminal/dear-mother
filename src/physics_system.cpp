// internal
#include "physics_system.hpp"
#include "world_init.hpp"

// Returns the local bounding coordinates scaled by the current size of the entity
vec2 get_bounding_box(const WorldObject& worldobject)
{
	// abs is to avoid negative scale due to the facing direction.
	return { abs(worldobject.scale.x), abs(worldobject.scale.y) };
}

// This is a SUPER APPROXIMATE check that puts a circle around the bounding boxes and sees
// if the center point of either object is inside the other's bounding-box-circle. You can
// surely implement a more accurate detection
bool collides(const WorldObject& object1, const WorldObject& object2)
{
	vec2 dp = object1.position - object2.position;
	float dist_squared = dot(dp,dp);
	const vec2 other_bonding_box = get_bounding_box(object1) / 2.f;
	const float other_r_squared = dot(other_bonding_box, other_bonding_box);
	const vec2 my_bonding_box = get_bounding_box(object2) / 2.f;
	const float my_r_squared = dot(my_bonding_box, my_bonding_box);
	const float r_squared = max(other_r_squared, my_r_squared);
	if (dist_squared < r_squared)
		return true;
	return false;
}

void checkCollision() {
	// Check for collisions between all relevant entities
	// note that the order of the loops depends strongly on how many of each we have. 
	// blockers are further in because presumably we'd only have a few (walls, accounting for doors) and we want to check them last
	// projectiles are first because we expect to have many of them

	// I have a lot of repetition that I don't know how to get rid of

	// Start with Projectiles
	for (Entity entity_projectile : registry.projectiles.entities) 
	{
		// Check for projectile-deadly collisions
		WorldObject worldobject_projectile = registry.worldobjects.get(entity_projectile);
		if (registry.projectiles.get(entity_projectile).friendly) {
			for (Entity entity_enemy : registry.deadlys.entities)
			{
				WorldObject worldobject_enemy = registry.worldobjects.get(entity_enemy);
				if (collides(worldobject_projectile, worldobject_enemy))
				{
					registry.collisions.emplace_with_duplicates(entity_projectile, entity_enemy, COLLISION_TYPE::PROJECTILE_DEADLY);
				}
			}
		}
		else {
			// Check for projectile-player collisions
			WorldObject worldobject_player = registry.worldobjects.get(registry.players.entities[0]);
			if (collides(worldobject_projectile, worldobject_player))
			{
				registry.collisions.emplace_with_duplicates(entity_projectile, registry.players.entities[0], COLLISION_TYPE::PROJECTILE_PLAYER);
			}
		}

		for (Entity entity_blocker : registry.blockers.entities)
		{
			WorldObject worldobject_blocker = registry.worldobjects.get(entity_blocker);
			if (collides(worldobject_projectile, worldobject_blocker))
			{
				registry.collisions.emplace_with_duplicates(entity_projectile, entity_blocker, COLLISION_TYPE::PROJECTILE_BLOCKER);
			}
		}

	}

	// next, check player/deadly-blockers
	for (Entity entity_blocker : registry.blockers.entities) {
		WorldObject worldobject_blocker = registry.worldobjects.get(entity_blocker);
		WorldObject worldobject_player = registry.worldobjects.get(registry.players.entities[0]);
		if (collides(worldobject_blocker, worldobject_player))
		{
			registry.collisions.emplace_with_duplicates(registry.players.entities[0], entity_blocker, COLLISION_TYPE::PLAYER_BLOCKER);
		}
		for (Entity entity_deadly : registry.deadlys.entities) {
			WorldObject worldobject_deadly = registry.worldobjects.get(entity_deadly);
			if (collides(worldobject_blocker, worldobject_deadly))
			{
				registry.collisions.emplace_with_duplicates(entity_deadly, entity_blocker, COLLISION_TYPE::DEADLY_BLOCKER);
			}
		}
	}

	for (Entity entity_deadly : registry.deadlys.entities) 
	{
		WorldObject worldobject_deadly = registry.worldobjects.get(entity_deadly);
		WorldObject worldobject_player = registry.worldobjects.get(registry.players.entities[0]);
		if (collides(worldobject_deadly, worldobject_player))
		{
			registry.collisions.emplace_with_duplicates(registry.players.entities[0], entity_deadly, COLLISION_TYPE::PLAYER_DEADLY);
		}
	}
}

void PhysicsSystem::step(float elapsed_ms)
{
	// Move entities based on how much time has passed, this is to (partially) avoid
	// having entities move at different speed based on the machine.
	// motion update step
	auto& motion_registry = registry.motions;
	auto& world_object_registry = registry.worldobjects;
	for (Entity entity : registry.motions.entities)
	{
		assert(world_object_registry.has(entity) && "Motion Entity has no worldobject component");
		Motion& motion = motion_registry.get(entity);
		WorldObject& worldobject = world_object_registry.get(entity);
		float step_seconds = elapsed_ms / 1000.f;
		vec2 velocity = { cos(motion.motion_angle) * motion.speed, sin(motion.motion_angle) * motion.speed };
		worldobject.position += (velocity) * step_seconds;
	}

	// collision detection step
	checkCollision();
}