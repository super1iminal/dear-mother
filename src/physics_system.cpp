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

void PhysicsSystem::step(float elapsed_ms)
{
	// Move entities based on how much time has passed, this is to (partially) avoid
	// having entities move at different speed based on the machine.
	auto& motion_registry = registry.motions;
	auto& world_object_registry = registry.worldobjects;
	auto& projectile_registry = registry.projectiles;
	for (Entity entity : registry.motions.entities)
	{
		assert(world_object_registry.has(entity) && "Motion Entity has no worldobject component");
		Motion& motion = motion_registry.get(entity);
		WorldObject& worldobject = world_object_registry.get(entity);
		float step_seconds = elapsed_ms / 1000.f;

		//if (projectile_registry.has(entity)) {
		//	worldobject.position.x = worldobject.position.x + (cos(worldobject.angle) * motion.input_velocity.x * step_seconds);
		//	worldobject.position.y = worldobject.position.y + (sin(worldobject.angle) * motion.input_velocity.y * step_seconds);
		//} else if (registry.players.has(entity)) {
		//	worldobject.position.x += (motion.external_velocity.x + motion.input_velocity.x) * step_seconds;
		//	worldobject.position.y += (motion.external_velocity.y + motion.input_velocity.y) * step_seconds;
		//}
		vec2 velocity = { cos(motion.motion_angle) * motion.speed, sin(motion.motion_angle) * motion.speed };
		worldobject.position += (velocity) * step_seconds;
	}

	// Check for collisions between all moving entities
    ComponentContainer<Motion> &motion_container = registry.motions;
	for(uint i = 0; i<motion_container.components.size(); i++)
	{
		Motion& motion_i = motion_container.components[i];
		Entity entity_i = motion_container.entities[i];
		WorldObject worldobject_i = registry.worldobjects.get(entity_i);
		
		// note starting j at i+1 to compare all (i,j) pairs only once (and to not compare with itself)
		for(uint j = i+1; j<motion_container.components.size(); j++)
		{
			Motion& motion_j = motion_container.components[j];
			Entity entity_j = motion_container.entities[j];
			WorldObject worldobject_j = registry.worldobjects.get(entity_j);
			if (collides(worldobject_i, worldobject_j))
			{
				Entity entity_j = motion_container.entities[j];
				// Create a collisions event
				// We are abusing the ECS system a bit in that we potentially insert muliple collisions for the same entity
				registry.collisions.emplace_with_duplicates(entity_i, entity_j);
				registry.collisions.emplace_with_duplicates(entity_j, entity_i);
			}
		}
	}
}