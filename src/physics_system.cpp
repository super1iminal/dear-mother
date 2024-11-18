// external
#include <unordered_set>

// internal
#include "physics_system.hpp"
#include "world_init.hpp"


// unused
bool PhysicsSystem::check_velocity_threshold(Motion& motion) {
	return ((motion.velocity.x > 1.1) || (motion.velocity.x < -0.1)) || ((motion.velocity.y > 1.1) || (motion.velocity.y < -0.1));
}

void PhysicsSystem::step(float elapsed_ms)
{
	auto& motion_registry = registry.motions;
	auto& world_object_registry = registry.worldObjects;
	float lerpFactor = 0.3f;

	float step_seconds = elapsed_ms / 1000.f;

	for (Entity entity : registry.motions.entities)
	{
		assert(world_object_registry.has(entity) && "Motion Entity has no worldobject component");
		// skip inactive entities
		if (!registry.activeComponents.has(entity)) {
			continue;
		}
		Motion& motion = motion_registry.get(entity);
		WorldObject& worldobject = world_object_registry.get(entity);

		// For non-projectile and non-particle entities, adjust velocity towards target_velocity using lerp
		if (registry.players.has(entity) || registry.deadlys.has(entity)) {
			float angle = a_from_v(motion.velocity);
			motion.velocity.x = lerp(motion.velocity.x, motion.target_velocity.x, lerpFactor);
			motion.velocity.y = lerp(motion.velocity.y, motion.target_velocity.y, lerpFactor);
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
}

float PhysicsSystem::lerp(float base, float target, float alpha) {
	return (1 - alpha) * base + alpha * target;
}


