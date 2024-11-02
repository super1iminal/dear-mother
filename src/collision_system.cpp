#include "collision_system.hpp"

// Returns the local bounding coordinates scaled by the current size of the entity
vec2 get_bounding_box(const WorldObject& worldobject)
{
	// abs is to avoid negative scale due to the facing direction.
	return { abs(worldobject.scale.x), abs(worldobject.scale.y) };
}

// keeping this for later.. muwahahahah
//// This is a SUPER APPROXIMATE check that puts a circle around the bounding boxes and sees
//// if the center point of either object is inside the other's bounding-box-circle. You can
//// surely implement a more accurate detection
//bool collides(const WorldObject& object1, const WorldObject& object2)
//{
//	vec2 dp = object1.position - object2.position;
//	float dist_squared = dot(dp,dp);
//	const vec2 other_bonding_box = get_bounding_box(object1) / 2.f;
//	const float other_r_squared = dot(other_bonding_box, other_bonding_box);
//	const vec2 my_bonding_box = get_bounding_box(object2) / 2.f;
//	const float my_r_squared = dot(my_bonding_box, my_bonding_box);
//	const float r_squared = max(other_r_squared, my_r_squared);
//	if (dist_squared < r_squared)
//		return true;
//	return false;
//}


// This is a KINDA APPROXIMATE check that puts a retangle around the bounding boxes and sees
// if the center point of either object is inside the other's bounding-box. 
bool CollisionSystem::collides(const WorldObject& object1, const WorldObject& object2)
{
	vec2 dp = object1.position - object2.position;
	vec2 half_extent1 = get_bounding_box(object1) / 2.f;
	vec2 half_extent2 = get_bounding_box(object2) / 2.f;

	// Check collision in x-axis
	if (std::abs(dp.x) < (half_extent1.x + half_extent2.x))
	{
		// Check collision in y-axis
		if (std::abs(dp.y) < (half_extent1.y + half_extent2.y))
		{
			// Collision detected
			return true;
		}
	}
	// No collision
	return false;
}

/*
 - Check for collisions between all relevant entities
 - note that the order of the loops depends strongly on how many of each we have.
 - blockers are further in because presumably we'd only have a few (walls, accounting for doors) and we want to check them last
 - projectiles are first because we expect to have many of them
 - projectiles can only collide with one thing at a time
 - note that the order of processing is extremely important for projectiles
 - if a bullet is colliding with both a wall and a player, we are currently processing the player (and deadlys) first
 - players and deadlys can collide with multiple things at a time, which means the order of their processing is less important
 - this will probably be the source of a lot of bugs. if bug, check here
 - this has a lot of repetition that I don't know how to get rid of
*/
void CollisionSystem::add_collisions() {
	// Start with Projectiles. Projectiles can only collide with one thing at a time.
	for (Entity entity_projectile : registry.projectiles.entities)
	{
		WorldObject worldobject_projectile = registry.worldObjects.get(entity_projectile);
		if (registry.projectiles.get(entity_projectile).friendly)
		{
			// Check for projectile-deadly collisions
			// Check if we've already processed this entity (don't technically need it here, but adding in case I rearrange)
			if (registry.collisions.has(entity_projectile)) { continue; } // O(1)
			for (Entity entity_enemy : registry.deadlys.entities)
			{
				WorldObject worldobject_enemy = registry.worldObjects.get(entity_enemy);
				if (collides(worldobject_projectile, worldobject_enemy))
				{
					assert(registry.collisions.has(entity_projectile) == false && "Projectile already collided with something");
					registry.collisions.emplace(entity_projectile, entity_enemy, COLLISION_TYPE::PROJECTILE_DEADLY);
					break; // we don't want any more collisions for this projectile
				}
			}
		}
		else {
			// Check for projectile-player collisions
			// Check if we've already processed this entity (don't technically need it here, but adding in case I rearrange)
			if (registry.collisions.has(entity_projectile)) { continue; } // O(1)
			WorldObject worldobject_player = registry.worldObjects.get(registry.players.entities[0]);
			if (collides(worldobject_projectile, worldobject_player))
			{
				assert(registry.collisions.has(entity_projectile) == false && "Projectile already collided with something");
				registry.collisions.emplace(entity_projectile, registry.players.entities[0], COLLISION_TYPE::PROJECTILE_PLAYER);
			}
		}
		// Check for projectile-blocker collisions
		// Check if we've already processed this entity
		if (registry.collisions.has(entity_projectile)) { continue; } // O(1)
		for (Entity entity_blocker : registry.blockers.entities)
		{
			WorldObject worldobject_blocker = registry.worldObjects.get(entity_blocker);
			if (collides(worldobject_projectile, worldobject_blocker))
			{
				assert(registry.collisions.has(entity_projectile) == false && "Projectile already collided with something");
				registry.collisions.emplace(entity_projectile, entity_blocker, COLLISION_TYPE::PROJECTILE_BLOCKER);
				break; // we don't want any more collisions for this projectile
			}
		}

	}

	// next, check player/deadly-blocker collisions
	for (Entity entity_blocker : registry.blockers.entities) {
		WorldObject worldobject_blocker = registry.worldObjects.get(entity_blocker);
		WorldObject worldobject_player = registry.worldObjects.get(registry.players.entities[0]);
		if (collides(worldobject_blocker, worldobject_player))
		{
			// i emplace with duplicates because player can collide with multiple things.
			registry.collisions.emplace_with_duplicates(registry.players.entities[0], entity_blocker, COLLISION_TYPE::PLAYER_BLOCKER);
			if (registry.collisions.has(registry.players.entities[0])) {
			}

		}
		for (Entity entity_deadly : registry.deadlys.entities) {
			WorldObject worldobject_deadly = registry.worldObjects.get(entity_deadly);
			if (collides(worldobject_blocker, worldobject_deadly))
			{
				// i emplace with duplicates because deadly can collide with multiple things.
				registry.collisions.emplace_with_duplicates(entity_deadly, entity_blocker, COLLISION_TYPE::DEADLY_BLOCKER);
			}
		}
	}

	// next, check player-deadly collisions
	for (Entity entity_deadly : registry.deadlys.entities)
	{
		WorldObject worldobject_deadly = registry.worldObjects.get(entity_deadly);
		WorldObject worldobject_player = registry.worldObjects.get(registry.players.entities[0]);
		Deadly& deadly = registry.deadlys.get(entity_deadly);
		if (collides(worldobject_deadly, worldobject_player))
		{
			for (Entity entity : registry.collisions.entities) {
				Entity entity_other = registry.collisions.get(entity).other;
				if (registry.players.has(entity) && registry.blockers.has(entity_other)) {
				}
			}
			registry.collisions.emplace_with_duplicates(registry.players.entities[0], entity_deadly, COLLISION_TYPE::PLAYER_DEADLY);
			deadly.attacking = true;
			for (Entity entity : registry.collisions.entities) {
				Entity entity_other = registry.collisions.get(entity).other;
				if (registry.players.has(entity) && registry.blockers.has(entity_other)) {
				}
			}
		}
	}
}