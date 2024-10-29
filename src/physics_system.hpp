#pragma once

#include "common.hpp"
#include "tiny_ecs.hpp"
#include "components.hpp"
#include "tiny_ecs_registry.hpp"

// A simple physics system that moves rigid bodies
class PhysicsSystem
{
private:
	bool check_velocity_threshold(Motion& motion); // unused
public:
	void step(float elapsed_ms);
	float lerp(float base, float target, float alpha);

	PhysicsSystem()
	{
	}
};