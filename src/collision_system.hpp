#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "components.hpp"
#include "tiny_ecs_registry.hpp"

// A simple physics system that moves rigid bodies and checks for collision
class CollisionSystem
{
private:
	bool collides(const WorldObject& object1, const WorldObject& object2);
public:
	void add_collisions();

	CollisionSystem()
	{
	}
};

vec2 get_bounding_box(const WorldObject& worldobject);