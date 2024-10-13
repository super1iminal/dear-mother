// external
#include <unordered_set>

// internal
#include "physics_system.hpp"
#include "world_init.hpp"

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
bool collides(const WorldObject& object1, const WorldObject& object2)
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
void checkCollision() {
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
		if (collides(worldobject_deadly, worldobject_player))
		{
			for (Entity entity : registry.collisions.entities) {
				Entity entity_other = registry.collisions.get(entity).other;
				if (registry.players.has(entity) && registry.blockers.has(entity_other)) {
				}
			}
			registry.collisions.emplace_with_duplicates(registry.players.entities[0], entity_deadly, COLLISION_TYPE::PLAYER_DEADLY);
			for (Entity entity : registry.collisions.entities) {
				Entity entity_other = registry.collisions.get(entity).other;
				if (registry.players.has(entity) && registry.blockers.has(entity_other)) {
				}
			}
		}
	}
}

void PhysicsSystem::step(float elapsed_ms)
{
	auto& motion_registry = registry.motions;
	auto& world_object_registry = registry.worldObjects;
	float lerpFactor = 0.1f;

	float step_seconds = elapsed_ms / 1000.f;

	for (Entity entity : registry.motions.entities)
	{
		assert(world_object_registry.has(entity) && "Motion Entity has no worldobject component");
		Motion& motion = motion_registry.get(entity);
		WorldObject& worldobject = world_object_registry.get(entity);

		// For non-projectile and non-particle entities, adjust velocity towards target_velocity using lerp
		if (registry.players.has(entity) || registry.deadlys.has(entity)) {
			float angle = a_from_v(motion.velocity);
			vec2 target_velocity = { cos(motion.angle) * motion.speed, sin(motion.angle) * motion.speed };
			motion.velocity.x = lerp(motion.velocity.x, target_velocity.x, lerpFactor);
			motion.velocity.y = lerp(motion.velocity.y, target_velocity.y, lerpFactor);
		}
		else {
			// Update velocity based on the acceleration first
			motion.velocity += (motion.acceleration * step_seconds);

			// Apply drag if the entity has friction
			if (registry.frictions.has(entity)) {
				auto& friction = registry.frictions.get(entity);

				// Calculate the magnitude of the velocity
				float velocity_magnitude_sq = glm::dot(motion.velocity, motion.velocity);
				float velocity_magnitude = sqrt(velocity_magnitude_sq);

				friction.force = 0.5f * M_RHO * velocity_magnitude_sq * PARTICLE_DRAG_COEF;

				// Calculate drag acceleration (force / mass)
				float drag_acceleration_magnitude = friction.force / motion.mass;

				// Potential change in velocity due to drag during this time step
				float delta_v = drag_acceleration_magnitude * step_seconds;

				if (delta_v >= velocity_magnitude) {
					// Drag would reverse the velocity; set it to zero
					motion.velocity = glm::vec2(0.0f, 0.0f);
				}
				else {
					// Apply drag as a force opposing the velocity
					glm::vec2 velocity_direction = glm::normalize(motion.velocity);
					glm::vec2 drag_acceleration = -velocity_direction * drag_acceleration_magnitude;

					// Adjust velocity by applying the drag acceleration
					motion.velocity += drag_acceleration * step_seconds;
				}
			}
		}

		// Update the position based on the new velocity
		worldobject.position += motion.velocity * step_seconds;
	}

	// Collision detection step
	checkCollision();
}

float PhysicsSystem::lerp(float base, float target, float alpha) {
	return (1 - alpha) * base + target + alpha;
}
