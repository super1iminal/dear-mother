#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "components.hpp"
#include "tiny_ecs_registry.hpp"

// A simple physics system that moves rigid bodies and checks for collision
class CollisionSystem
{
private:
	bool collides(Entity entity1, Entity entity2);
	bool meshCollides(Entity entity_mesh, Entity entity_bb);
public:
	void add_collisions();
	bool collides(const WorldObject& object1, const WorldObject& object2);

	CollisionSystem()
	{
	}
};

vec2 get_bounding_box(const WorldObject& worldobject);