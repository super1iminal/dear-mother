// internal
#include <queue>
#include <vector>
#include <iostream>
#include "ai_system.hpp"
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

void AISystem::step(float elapsed_ms)
{
	(void)elapsed_ms; // placeholder to silence unused warning until implemented
	auto& motionRegistry = registry.motions;
	auto& worldObjectRegistry = registry.worldObjects;
	auto& playerRegistry = registry.players;
	auto& deadlyRegistry = registry.deadlys;

	if (playerRegistry.entities.size() > 0) {
		Entity player = playerRegistry.entities[0]; //assuming always only 1 player
		WorldObject wo_player = worldObjectRegistry.get(player);
		vec2 coor_player = wo_player.position;
		for (int i = 0; i < deadlyRegistry.entities.size(); i++) {
			Entity enemy = deadlyRegistry.entities[i];
			vec2 coor_enemy = worldObjectRegistry.get(enemy).position;
			//cout << coor_enemy.x << ' ' << coor_enemy.y << endl;
			//cout << coor_player.x << ' ' << coor_player.y << endl;
			/*vector<vec2> path = bfs(coor_enemy, coor_player);
			if (path.size() >= 2) {
				pathFindingRegistry.get(enemy).direction = path[1] - path[0];
			}*/

			//worldObjectRegistry.get(enemy).angle = -M_PI/2  + atan2(coor_enemy.y - coor_player.y, coor_enemy.x - coor_player.x);
			int dx = coor_enemy.x - coor_player.x;
			int dy = coor_enemy.y - coor_player.y;
			float angle = atan2(dy, dx) - M_PI;
			if (angle < 0)
				angle += 2 * M_PI;
			//worldObjectRegistry.get(enemy).angle = angle;
			motionRegistry.get(enemy).target_velocity = v_from_sa(motionRegistry.get(enemy).max_speed, angle);
			//cout << "{" << coor_player.x << "||" << coor_player.y << "}" << "{" << coor_enemy.x << "||" << coor_enemy.y << "}" << dx <<"|"<<dy<<"|" <<angle << endl;
		}
	}
}