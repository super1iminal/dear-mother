#pragma once
#include <queue>
#include <vector>
#include <iostream>

#include "tiny_ecs_registry.hpp"
#include "common.hpp"
using namespace std;

class AISystem
{
public:
	bool isValid(vec2 coor, vector<vector<bool>> visited);

	vector<vec2> bfs(vec2 start, vec2 target);

	void pathfinding(Entity& entity);
	void handleStateChange(Entity& entity);
	void handleBossOneStateChange(Entity& entity);
	void boss_one_ai();
	void boss_three_ai();
	void step(float elapsed_ms);
};