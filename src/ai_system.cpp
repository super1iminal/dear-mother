// internal
#include <queue>
#include <vector>
#include <iostream>
#include "ai_system.hpp"
#include "world_system.hpp"
#include "world_init.hpp"
using namespace std;

struct Pair {
	vec2 coor;
	vector<vec2> path;
};

vector<vec2> movements = {
	{1, 0},
	{0, 1},
	{-1, 0},
	{0, -1},
	{1, 1},
	{-1, -1},
	{1, -1},
	{-1, 1}
};

bool AISystem::isValid(vec2 coor, vector<vector<bool>> visited) {
	return (coor.x >= 0 && coor.x < window_width_px / 100 && coor.y >= 0 && coor.y < window_height_px / 100 && !visited[coor.x][coor.y]);
}

// not using this right now
vector<vec2> AISystem::bfs(vec2 start, vec2 target) {
	start.x = int(start.x / 100);
	start.y = int(start.y / 100);
	target.x = int(target.x / 100);
	target.y = int(target.y / 100);
	vector<vector<bool>> visited(window_width_px, vector<bool>(window_height_px, false));
	queue<Pair> q;

	visited[start.x][start.y] = true;
	q.push({ start,  {start} });

	while (!q.empty()) {
		Pair curr = q.front();
		q.pop();
		if (curr.coor == target) {
			return curr.path;
		}

		for (vec2 move : movements) {
			Pair temp = curr;
			temp.coor += move;
			if (isValid(temp.coor, visited)) {
				visited[temp.coor.x][temp.coor.y] = true;
				vector<vec2> path = temp.path;
				/*cout << "oldd: " << (temp.coor-move).x << " " << (temp.coor - move).y << "" << std::endl;
				cout << "move: " << move.x << " " << move.y << "" << std::endl;
				cout << "neww: " << temp.coor.x << " " << temp.coor.y << "\n" << std::endl;
				*/
				path.push_back(temp.coor);

				/*for (int i = 0; i < temp.path.size(); i++) {
					std::cout << temp.path[i].x << ", " << temp.path[i].y << "\n";
				}
				std::cout << std::endl;
				cout << "neww: " << temp.coor.x << " " << temp.coor.y << "\n" << std::endl;*/
				q.push({ temp.coor, path });
			}
		}

	}
	return {};
}

void AISystem::pathfinding(Entity& entity) {
	auto& motionRegistry = registry.motions;
	auto& worldObjectRegistry = registry.worldObjects;
	auto& playerRegistry = registry.players;
	vec2 coor_enemy = worldObjectRegistry.get(entity).position;
	vec2 coor_player = worldObjectRegistry.get(playerRegistry.entities[0]).position;
	int dx = coor_enemy.x - coor_player.x;
	int dy = coor_enemy.y - coor_player.y;
	float angle = atan2(dy, dx) - M_PI;
	if (angle < 0)
		angle += 2 * M_PI;
	motionRegistry.get(entity).target_velocity = v_from_sa(motionRegistry.get(entity).max_speed, angle);
}

void AISystem::handleStateChange(Entity& entity) {
	auto& deadlyRegistry = registry.deadlys;
	auto& motionRegistry = registry.motions;
	auto& shooterRegistry = registry.shooters;
	float elapsed_ms = (float)(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - deadlyRegistry.get(entity).t)).count()/1000;
	//switch code for different state
	switch(deadlyRegistry.get(entity).state) {
		case DeadlyState::idle:
			motionRegistry.get(entity).target_velocity = { 0, 0 };
			if (shooterRegistry.has(entity))
				shooterRegistry.get(entity).fire_rate = std::numeric_limits<int>::max();
			cout << "idle" << endl;
			break;
		case DeadlyState::patrol_left:
			motionRegistry.get(entity).target_velocity = v_from_sa(motionRegistry.get(entity).max_speed/2, M_PI);
			if (shooterRegistry.has(entity))
				shooterRegistry.get(entity).fire_rate = std::numeric_limits<int>::max();
			cout << "patrol left" << endl;
			break;
		case DeadlyState::patrol_right:
			motionRegistry.get(entity).target_velocity = v_from_sa(motionRegistry.get(entity).max_speed/2, 2*M_PI);
			if (shooterRegistry.has(entity))
				shooterRegistry.get(entity).fire_rate = std::numeric_limits<int>::max();
			cout << "patrol right" << endl;
			break;
		case DeadlyState::attack_moving:
			if (shooterRegistry.has(entity))
				shooterRegistry.get(entity).fire_rate = 10000.0f;
			cout << "attack_moving" << endl;
			break;
		case DeadlyState::attack_still:
			motionRegistry.get(entity).target_velocity = { 0, 0 };
			if (shooterRegistry.has(entity))
				shooterRegistry.get(entity).fire_rate = 10000.0f;
			cout << "attack_still" << endl;
			break;

	}



}
void AISystem::step(float elapsed_ms) {
	auto& worldObjectRegistry = registry.worldObjects;
	auto& playerRegistry = registry.players;
	auto& deadlyRegistry = registry.deadlys;

	Entity& player = playerRegistry.entities[0]; // assuming always only 1 player

	for (int i = 0; i < deadlyRegistry.entities.size(); i++) {
		Entity& enemy = deadlyRegistry.entities[i];
		float elapsed_time = (float)(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - deadlyRegistry.get(enemy).t)).count() / 1000;
		float elapsed_time_patrol = (float)(std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - deadlyRegistry.get(enemy).t_patrol)).count() / 1000;
		float distanceToPlayer = length(worldObjectRegistry.get(player).position - worldObjectRegistry.get(enemy).position);
		DeadlyState state = deadlyRegistry.get(enemy).state;

		switch (state) {
		case DeadlyState::idle:
			if (elapsed_time >= 1500) { // Wait for 1.5 seconds
				deadlyRegistry.get(enemy).state = DeadlyState::patrol_right;
				deadlyRegistry.get(enemy).t_patrol = std::chrono::high_resolution_clock::now(); // Reset patrol timer
			}
			break;

		case DeadlyState::patrol_right:
			if (distanceToPlayer < 600) { // If player is close enough, stop patrolling and attack
				deadlyRegistry.get(enemy).state = DeadlyState::attack_moving;
			}
			else if (elapsed_time_patrol >= 5000) { // Patrol right for 5 seconds
				deadlyRegistry.get(enemy).state = DeadlyState::patrol_left;
				deadlyRegistry.get(enemy).t_patrol = std::chrono::high_resolution_clock::now(); // Reset patrol timer
			}
			break;

		case DeadlyState::patrol_left:
			if (distanceToPlayer < 600) { // If player is close enough, stop patrolling and attack
				deadlyRegistry.get(enemy).state = DeadlyState::attack_moving;
			}
			else if (elapsed_time_patrol >= 10000) { // Patrol left for 10 seconds
				deadlyRegistry.get(enemy).state = DeadlyState::idle; // Return to idle state
				deadlyRegistry.get(enemy).t = std::chrono::high_resolution_clock::now(); // Reset idle timer
			}
			break;

		case DeadlyState::attack_moving:
			pathfinding(enemy);
			if (distanceToPlayer < 200 && registry.shooters.has(enemy)) { // Shoot at the player
				deadlyRegistry.get(enemy).state = DeadlyState::attack_still;
			}
			else if (distanceToPlayer >= 600) { // Stop attacking the player if they're too far away
				deadlyRegistry.get(enemy).state = DeadlyState::idle;
				deadlyRegistry.get(enemy).t = std::chrono::high_resolution_clock::now();
			}
			break;

		case DeadlyState::attack_still:
			if (distanceToPlayer > 200) { // Stop shooting at the player
				deadlyRegistry.get(enemy).state = DeadlyState::attack_moving;
			}
			else if (distanceToPlayer >= 600) { // Stop attacking the player if they're too far away
				deadlyRegistry.get(enemy).state = DeadlyState::idle;
				deadlyRegistry.get(enemy).t = std::chrono::high_resolution_clock::now();
			}
			break;
		}

		// Handle state change if the state has changed
		if (state != deadlyRegistry.get(enemy).state) {
			handleStateChange(enemy);
		}
	}
}

