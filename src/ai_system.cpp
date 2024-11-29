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
			//cout << "idle" << endl;
			break;
		case DeadlyState::patrol_left:
			motionRegistry.get(entity).target_velocity = v_from_sa(motionRegistry.get(entity).max_speed/2, M_PI);
			if (shooterRegistry.has(entity))
				shooterRegistry.get(entity).fire_rate = std::numeric_limits<int>::max();
			//cout << "patrol left" << endl;
			break;
		case DeadlyState::patrol_right:
			motionRegistry.get(entity).target_velocity = v_from_sa(motionRegistry.get(entity).max_speed/2, 2*M_PI);
			if (shooterRegistry.has(entity))
				shooterRegistry.get(entity).fire_rate = std::numeric_limits<int>::max();
			//cout << "patrol right" << endl;
			break;
		case DeadlyState::attack_moving:
			if (shooterRegistry.has(entity))
				shooterRegistry.get(entity).fire_rate = 10000.0f;
			//cout << "attack_moving" << endl;
			break;
		case DeadlyState::attack_still:
			motionRegistry.get(entity).target_velocity = { 0, 0 };
			if (shooterRegistry.has(entity))
				shooterRegistry.get(entity).fire_rate = 10000.0f;
			//cout << "attack_still" << endl;
			break;

	}



}

void AISystem::handleBossOneStateChange(Entity& entity) {

	auto& bossOneRegistry = registry.bossOnes;
	BossOne& currBoss = bossOneRegistry.get(entity);
	BOSS_ONE_STATE state = currBoss.boss_state;
	Shooter& bossShooting = registry.shooters.get(entity);
	Motion& bossMotion = registry.motions.get(entity);
	vec2 bossPos = registry.worldObjects.get(entity).position;

	switch (state) {
	case BOSS_ONE_STATE::FOUR_ALIVE:
		switch (currBoss.boss_pos) {
		case BOSS_ONE_POS::TOP_LEFT:
			bossMotion.target_velocity = { 200,0 };
			break;
		case BOSS_ONE_POS::TOP_RIGHT:
			bossMotion.target_velocity = {- 200,0 };
			break;
		case BOSS_ONE_POS::BOT_LEFT:
			bossMotion.target_velocity = { 200,0 };
			break;
		case BOSS_ONE_POS::BOT_RIGHT:
			bossMotion.target_velocity = { -200,0 };
			break;
		}
		break;
	case BOSS_ONE_STATE::THREE_ALIVE_ONE_TOP_LEFT:
		bossShooting.fire_rate = 750;
		break;
	case BOSS_ONE_STATE::THREE_ALIVE_ONE_TOP_RIGHT:
		bossShooting.fire_rate = 750;
		switch (currBoss.boss_pos) {
		case BOSS_ONE_POS::TOP_RIGHT:
			bossMotion.target_velocity = { -200,0 };
			break;
		case BOSS_ONE_POS::BOT_LEFT:
			bossMotion.target_velocity = { 200,0 };
			break;
		case BOSS_ONE_POS::BOT_RIGHT:
			bossMotion.target_velocity = { -200,0 };
			break;
		}
		break;
	case BOSS_ONE_STATE::THREE_ALIVE_ONE_BOT_LEFT:
		bossShooting.fire_rate = 750;
		switch (currBoss.boss_pos) {
		case BOSS_ONE_POS::TOP_LEFT:
			bossMotion.target_velocity = { 200,0 };
			break;
		case BOSS_ONE_POS::TOP_RIGHT:
			bossMotion.target_velocity = { -200,0 };
			break;
		case BOSS_ONE_POS::BOT_LEFT:
			bossMotion.target_velocity = { 200,0 };
			break;
		}
		break;
	case BOSS_ONE_STATE::THREE_ALIVE_ONE_BOT_RIGHT:
		bossShooting.fire_rate = 750;
		switch (currBoss.boss_pos) {
		case BOSS_ONE_POS::TOP_LEFT:
			bossMotion.target_velocity = { 200,0 };
			break;
		case BOSS_ONE_POS::TOP_RIGHT:
			bossMotion.target_velocity = { -200,0 };
			break;
		case BOSS_ONE_POS::BOT_RIGHT:
			bossMotion.target_velocity = { -200,0 };
			break;
		}
		break;
	case BOSS_ONE_STATE::TWO_OPPOSITE_SIDE_ALIVE_L_L:
		bossShooting.fire_rate = 1000;
		break;
	case BOSS_ONE_STATE::TWO_OPPOSITE_SIDE_ALIVE_R_R:
		bossShooting.fire_rate = 1000;
		break;
	case BOSS_ONE_STATE::TWO_OPPOSITE_SIDE_ALIVE_BL_TR:
		bossShooting.fire_rate = 1000;
		break;
	case BOSS_ONE_STATE::TWO_OPPOSITE_SIDE_ALIVE_BR_TL:
		bossShooting.fire_rate = 1000;
		break;
	case BOSS_ONE_STATE::TWO_SAME_SIDE_ALIVE_BOT:
		bossShooting.fire_rate = 1000;
		break;
	case BOSS_ONE_STATE::TWO_SAME_SIDE_ALIVE_TOP:
		bossShooting.fire_rate = 1000;
		break;
	case BOSS_ONE_STATE::ONE_ALIVE_T_L:
		if (currBoss.boss_pos != BOSS_ONE_POS::MOTHER) {
			registry.deadlys.get(entity).melee_damge = 3;
		}
		break;
	case BOSS_ONE_STATE::ONE_ALIVE_B_L:
		if (currBoss.boss_pos != BOSS_ONE_POS::MOTHER)
			registry.deadlys.get(entity).melee_damge = 3;
		break;
	case BOSS_ONE_STATE::ONE_ALIVE_T_R:
		if (currBoss.boss_pos != BOSS_ONE_POS::MOTHER)
			registry.deadlys.get(entity).melee_damge = 3;
		break;
	case BOSS_ONE_STATE::ONE_ALIVE_B_R:
		if (currBoss.boss_pos != BOSS_ONE_POS::MOTHER)
			registry.deadlys.get(entity).melee_damge = 3;
		break;
	case BOSS_ONE_STATE::ALL_DEAD:
		registry.deadlys.get(entity).melee_damge = 1;
		registry.deadlys.get(entity).immune = false;
		bossShooting.fire_rate = 1000;
		break;
	}	
}

void check_state_four(BossOne& currBoss) {
	if (!currBoss.top_left_alive && currBoss.top_right_alive
		&& currBoss.bot_left_alive && currBoss.bot_right_alive) {
		currBoss.boss_state = BOSS_ONE_STATE::THREE_ALIVE_ONE_TOP_RIGHT;
	}
	else if (currBoss.top_left_alive && !currBoss.top_right_alive
		&& currBoss.bot_left_alive && currBoss.bot_right_alive) {
		currBoss.boss_state = BOSS_ONE_STATE::THREE_ALIVE_ONE_TOP_LEFT;
	}
	else if (currBoss.top_left_alive && currBoss.top_right_alive
		&& !currBoss.bot_left_alive && currBoss.bot_right_alive) {
		currBoss.boss_state = BOSS_ONE_STATE::THREE_ALIVE_ONE_BOT_RIGHT;
	}
	else if (currBoss.top_left_alive && currBoss.top_right_alive
		&& currBoss.bot_left_alive && !currBoss.bot_right_alive) {
		currBoss.boss_state = BOSS_ONE_STATE::THREE_ALIVE_ONE_BOT_LEFT;
	}
}

void check_state_three(BossOne& currBoss) {
	if (currBoss.top_left_alive && currBoss.top_right_alive
		&& !currBoss.bot_left_alive && !currBoss.bot_right_alive) {
		currBoss.boss_state = BOSS_ONE_STATE::TWO_SAME_SIDE_ALIVE_TOP;
	}
	else if (currBoss.top_left_alive && !currBoss.top_right_alive
		&& currBoss.bot_left_alive && !currBoss.bot_right_alive) {
		currBoss.boss_state = BOSS_ONE_STATE::TWO_OPPOSITE_SIDE_ALIVE_L_L;
	}
	else if (!currBoss.top_left_alive && currBoss.top_right_alive
		&& !currBoss.bot_left_alive && currBoss.bot_right_alive) {
		currBoss.boss_state = BOSS_ONE_STATE::TWO_OPPOSITE_SIDE_ALIVE_R_R;
	}
	else if (!currBoss.top_left_alive && currBoss.top_right_alive
		&& currBoss.bot_left_alive && !currBoss.bot_right_alive) {
		currBoss.boss_state = BOSS_ONE_STATE::TWO_OPPOSITE_SIDE_ALIVE_BL_TR;
	}
	else if (currBoss.top_left_alive && !currBoss.top_right_alive
		&& !currBoss.bot_left_alive && currBoss.bot_right_alive) {
		currBoss.boss_state = BOSS_ONE_STATE::TWO_OPPOSITE_SIDE_ALIVE_BR_TL;
	}
	else if (!currBoss.top_left_alive && !currBoss.top_right_alive
		&& currBoss.bot_left_alive && currBoss.bot_right_alive) {
		currBoss.boss_state = BOSS_ONE_STATE::TWO_SAME_SIDE_ALIVE_BOT;
	}
}

void check_state_two(BossOne& currBoss) {
	if (currBoss.top_left_alive && !currBoss.top_right_alive
		&& !currBoss.bot_left_alive && !currBoss.bot_right_alive) {
		currBoss.boss_state = BOSS_ONE_STATE::ONE_ALIVE_T_L;
	}
	else if (!currBoss.top_left_alive && currBoss.top_right_alive
		&& !currBoss.bot_left_alive && !currBoss.bot_right_alive) {
		currBoss.boss_state = BOSS_ONE_STATE::ONE_ALIVE_T_R;
	}
	else if (!currBoss.top_left_alive && !currBoss.top_right_alive
		&& currBoss.bot_left_alive && !currBoss.bot_right_alive) {
		currBoss.boss_state = BOSS_ONE_STATE::ONE_ALIVE_B_L;
	}
	else if (!currBoss.top_left_alive && !currBoss.top_right_alive
		&& !currBoss.bot_left_alive && currBoss.bot_right_alive) {
		currBoss.boss_state = BOSS_ONE_STATE::ONE_ALIVE_B_R;
	}
}

void check_state_one(BossOne& currBoss) {
	if (!currBoss.top_left_alive && !currBoss.top_right_alive
		&& !currBoss.bot_left_alive && !currBoss.bot_right_alive) {
		currBoss.boss_state = BOSS_ONE_STATE::ALL_DEAD;
	}
}

void AISystem::boss_one_ai() {
	int LEFT_OUTER_X_BOUND = 136;
	int LEFT_INNER_X_BOUND = 628;
	int RIGHT_OUTER_X_BOUND = 1136;
	int RIGHT_INNER_X_BOUND = 714;

	int TOP_OUTER_Y_BOUND = 270 + 50;
	//int TOP_INNER_Y_BOUND = 350;
	int BOT_OUTER_Y_BOUND = 550;
	//int BOT_INNER_Y_BOUND = 500;

	int CENTER_X = 638;

	auto& bossOneRegistry = registry.bossOnes;
	for (Entity& boss : bossOneRegistry.entities) {
		BossOne& currBoss = bossOneRegistry.get(boss);
		BOSS_ONE_STATE state = currBoss.boss_state;
		Motion& bossMotion = registry.motions.get(boss);
		vec2 bossPos = registry.worldObjects.get(boss).position;

		switch (state) {
		case BOSS_ONE_STATE::FOUR_ALIVE:
			switch (currBoss.boss_pos) {
			case BOSS_ONE_POS::TOP_LEFT:
				if (bossPos.x <= LEFT_OUTER_X_BOUND) {
					bossMotion.target_velocity = { 200,0 };
				}
				else if (bossPos.x >= LEFT_INNER_X_BOUND) {
					bossMotion.target_velocity = { -200,0 };
				}
				break;
			case BOSS_ONE_POS::TOP_RIGHT:
				if (bossPos.x >= RIGHT_OUTER_X_BOUND) {
					bossMotion.target_velocity = { -200,0 };
				}
				else if (bossPos.x <= RIGHT_INNER_X_BOUND) {
					bossMotion.target_velocity = { 200,0 };
				}
				break;
			case BOSS_ONE_POS::BOT_LEFT:
				if (bossPos.x <= LEFT_OUTER_X_BOUND) {
					bossMotion.target_velocity = { 200,0 };
				}
				else if (bossPos.x >= LEFT_INNER_X_BOUND) {
					bossMotion.target_velocity = { -200,0 };
				}
				break;
			case BOSS_ONE_POS::BOT_RIGHT:
				if (bossPos.x >= RIGHT_OUTER_X_BOUND) {
					bossMotion.target_velocity = { -200,0 };
				}
				else if (bossPos.x <= RIGHT_INNER_X_BOUND) {
					bossMotion.target_velocity = { 200,0 };
				}
				break;
			}
			check_state_four(currBoss);
			break;
		case BOSS_ONE_STATE::THREE_ALIVE_ONE_TOP_LEFT:
			switch (currBoss.boss_pos) {
			case BOSS_ONE_POS::TOP_LEFT:
				if (bossPos.x <= CENTER_X) {
					bossMotion.target_velocity = { 200,0 };
				}
				else {
					bossMotion.target_velocity = { 0,0 };
				}
				break;
			case BOSS_ONE_POS::BOT_LEFT:
				if (bossPos.x < CENTER_X - 300) {
					bossMotion.target_velocity = { 200,0 };
				}
				else if (bossPos.x > CENTER_X - 300) {
					bossMotion.target_velocity = { -200,0 };
				}
				else {
					bossMotion.target_velocity = { 0,0 };
				}
				break;
			case BOSS_ONE_POS::BOT_RIGHT:
				if (bossPos.x > CENTER_X + 300) {
					bossMotion.target_velocity = { -200,0 };
				}
				else if (bossPos.x < CENTER_X + 300) {
					bossMotion.target_velocity = { 200,0 };
				}
				else {
					bossMotion.target_velocity = { 0,0 };
				}
				break;
			}
			check_state_three(currBoss);
			break;
		case BOSS_ONE_STATE::THREE_ALIVE_ONE_BOT_LEFT:
			switch (currBoss.boss_pos) {
			case BOSS_ONE_POS::BOT_LEFT:
				if (bossPos.x <= CENTER_X) {
					bossMotion.target_velocity = { 200,0 };
				}
				else {
					bossMotion.target_velocity = { 0,0 };
				}
				break;
			case BOSS_ONE_POS::TOP_LEFT:
				if (bossPos.x < CENTER_X - 300) {
					bossMotion.target_velocity = { 200,0 };
				}
				else if (bossPos.x > CENTER_X - 300) {
					bossMotion.target_velocity = { -200,0 };
				}
				else {
					bossMotion.target_velocity = { 0,0 };
				}
				break;
			case BOSS_ONE_POS::TOP_RIGHT:
				if (bossPos.x > CENTER_X + 300) {
					bossMotion.target_velocity = { -200,0 };
				}
				else if (bossPos.x < CENTER_X + 300) {
					bossMotion.target_velocity = { 200,0 };
				}
				else {
					bossMotion.target_velocity = { 0,0 };
				}
				break;
			}
			check_state_three(currBoss);
			break;
		case BOSS_ONE_STATE::THREE_ALIVE_ONE_TOP_RIGHT:
			switch (currBoss.boss_pos) {
			case BOSS_ONE_POS::BOT_LEFT:
				if (bossPos.x < CENTER_X - 300) {
					bossMotion.target_velocity = { 200,0 };
				}
				else if (bossPos.x > CENTER_X - 300) {
					bossMotion.target_velocity = { -200,0 };
				}
				else {
					bossMotion.target_velocity = { 0,0 };
				}
				break;
			case BOSS_ONE_POS::BOT_RIGHT:
				if (bossPos.x > CENTER_X + 300) {
					bossMotion.target_velocity = { -200,0 };
				}
				else if (bossPos.x < CENTER_X + 300) {
					bossMotion.target_velocity = { 200,0 };
				}
				else {
					bossMotion.target_velocity = { 0,0 };
				}
				break;
			case BOSS_ONE_POS::TOP_RIGHT:
				if (bossPos.x >= CENTER_X) {
					bossMotion.target_velocity = { -200,0 };
				}
				else {
					bossMotion.target_velocity = { 0,0 };
				}
				break;
			}
			check_state_three(currBoss);
			break;
		case BOSS_ONE_STATE::THREE_ALIVE_ONE_BOT_RIGHT:
			switch (currBoss.boss_pos) {
			case BOSS_ONE_POS::BOT_RIGHT:
				if (bossPos.x >= CENTER_X) {
					bossMotion.target_velocity = { -200,0 };
				}
				else {
					bossMotion.target_velocity = { 0,0 };
				}
				break;
			case BOSS_ONE_POS::TOP_LEFT:
				if (bossPos.x < CENTER_X - 300) {
					bossMotion.target_velocity = { 200,0 };
				}
				else if (bossPos.x > CENTER_X - 300) {
					bossMotion.target_velocity = { -200,0 };
				}
				else {
					bossMotion.target_velocity = { 0,0 };
				}
				break;
			case BOSS_ONE_POS::TOP_RIGHT:
				if (bossPos.x > CENTER_X + 300) {
					bossMotion.target_velocity = { -200,0 };
				}
				else if (bossPos.x < CENTER_X + 300) {
					bossMotion.target_velocity = { 200,0 };
				}
				else {
					bossMotion.target_velocity = { 0,0 };
				}
				break;
			}
			check_state_three(currBoss);
			break;
		case BOSS_ONE_STATE::TWO_OPPOSITE_SIDE_ALIVE_L_L:
			switch (currBoss.boss_pos) {
			case BOSS_ONE_POS::TOP_LEFT:
				if (bossPos.x < RIGHT_OUTER_X_BOUND) {
					bossMotion.target_velocity = { 200,0 };
				}
				else {
					if (bossPos.y <= TOP_OUTER_Y_BOUND) {
						bossMotion.target_velocity = { 0, 200 };
					}
					else if (bossPos.y >= BOT_OUTER_Y_BOUND) {
						bossMotion.target_velocity = { 0, -200 };
					}
				}
				break;
			case BOSS_ONE_POS::BOT_LEFT:
				if (bossPos.x > LEFT_OUTER_X_BOUND) {
					bossMotion.target_velocity = { -200,0 };
				}
				else {
					if (bossPos.y <= TOP_OUTER_Y_BOUND) {
						bossMotion.target_velocity = { 0, 200 };
					}
					else if (bossPos.y >= BOT_OUTER_Y_BOUND) {
						bossMotion.target_velocity = { 0, -200 };
					}
				}
				break;
			}
			check_state_two(currBoss);
			break;
		case BOSS_ONE_STATE::TWO_OPPOSITE_SIDE_ALIVE_BR_TL:
			switch (currBoss.boss_pos) {
			case BOSS_ONE_POS::TOP_LEFT:
				if (bossPos.x < RIGHT_OUTER_X_BOUND) {
					bossMotion.target_velocity = { 200,0 };
				}
				else {
					if (bossPos.y <= TOP_OUTER_Y_BOUND) {
						bossMotion.target_velocity = { 0, 200 };
					}
					else if (bossPos.y >= BOT_OUTER_Y_BOUND) {
						bossMotion.target_velocity = { 0, -200 };
					}
				}
				break;
			case BOSS_ONE_POS::BOT_RIGHT:
				if (bossPos.x > LEFT_OUTER_X_BOUND) {
					bossMotion.target_velocity = { -200,0 };
				}
				else {
					if (bossPos.y <= TOP_OUTER_Y_BOUND) {
						bossMotion.target_velocity = { 0, 200 };
					}
					else if (bossPos.y >= BOT_OUTER_Y_BOUND) {
						bossMotion.target_velocity = { 0, -200 };
					}
				}
				break;
			}
			check_state_two(currBoss);
			break;
		case BOSS_ONE_STATE::TWO_OPPOSITE_SIDE_ALIVE_BL_TR:
			switch (currBoss.boss_pos) {
			case BOSS_ONE_POS::TOP_RIGHT:
				if (bossPos.x < RIGHT_OUTER_X_BOUND) {
					bossMotion.target_velocity = { 200,0 };
				}
				else {
					if (bossPos.y <= TOP_OUTER_Y_BOUND) {
						bossMotion.target_velocity = { 0, 200 };
					}
					else if (bossPos.y >= BOT_OUTER_Y_BOUND) {
						bossMotion.target_velocity = { 0, -200 };
					}
				}
				break;
			case BOSS_ONE_POS::BOT_LEFT:
				if (bossPos.x > LEFT_OUTER_X_BOUND) {
					bossMotion.target_velocity = { -200,0 };
				}
				else {
					if (bossPos.y <= TOP_OUTER_Y_BOUND) {
						bossMotion.target_velocity = { 0, 200 };
					}
					else if (bossPos.y >= BOT_OUTER_Y_BOUND) {
						bossMotion.target_velocity = { 0, -200 };
					}
				}
				break;
			}
			check_state_two(currBoss);
			break;
		case BOSS_ONE_STATE::TWO_OPPOSITE_SIDE_ALIVE_R_R:
			switch (currBoss.boss_pos) {
			case BOSS_ONE_POS::TOP_RIGHT:
				if (bossPos.x < RIGHT_OUTER_X_BOUND) {
					bossMotion.target_velocity = { 200,0 };
				}
				else {
					if (bossPos.y <= TOP_OUTER_Y_BOUND) {
						bossMotion.target_velocity = { 0, 200 };
					}
					else if (bossPos.y >= BOT_OUTER_Y_BOUND) {
						bossMotion.target_velocity = { 0, -200 };
					}
				}
				break;
			case BOSS_ONE_POS::BOT_RIGHT:
				if (bossPos.x > LEFT_OUTER_X_BOUND) {
					bossMotion.target_velocity = { -200,0 };
				}
				else {
					if (bossPos.y <= TOP_OUTER_Y_BOUND) {
						bossMotion.target_velocity = { 0, 200 };
					}
					else if (bossPos.y >= BOT_OUTER_Y_BOUND) {
						bossMotion.target_velocity = { 0, -200 };
					}
				}
				break;
			}
			check_state_two(currBoss);
			break;
		case BOSS_ONE_STATE::TWO_SAME_SIDE_ALIVE_TOP:
			switch (currBoss.boss_pos) {
			case BOSS_ONE_POS::TOP_RIGHT:
				if (bossPos.x < RIGHT_OUTER_X_BOUND) {
					bossMotion.target_velocity = { 200,0 };
				}
				else {
					if (bossPos.y <= TOP_OUTER_Y_BOUND) {
						bossMotion.target_velocity = { 0, 200 };
					}
					else if (bossPos.y >= BOT_OUTER_Y_BOUND) {
						bossMotion.target_velocity = { 0, -200 };
					}
				}
				break;
			case BOSS_ONE_POS::TOP_LEFT:
				if (bossPos.x > LEFT_OUTER_X_BOUND) {
					bossMotion.target_velocity = { -200,0 };
				}
				else {
					if (bossPos.y <= TOP_OUTER_Y_BOUND) {
						bossMotion.target_velocity = { 0, 200 };
					}
					else if (bossPos.y >= BOT_OUTER_Y_BOUND) {
						bossMotion.target_velocity = { 0, -200 };
					}
				}
				break;
			}
			check_state_two(currBoss);
			break;
		case BOSS_ONE_STATE::TWO_SAME_SIDE_ALIVE_BOT:
			switch (currBoss.boss_pos) {
			case BOSS_ONE_POS::BOT_RIGHT:
				if (bossPos.x < RIGHT_OUTER_X_BOUND) {
					bossMotion.target_velocity = { 200,0 };
				}
				else {
					if (bossPos.y <= TOP_OUTER_Y_BOUND) {
						bossMotion.target_velocity = { 0, 200 };
					}
					else if (bossPos.y >= BOT_OUTER_Y_BOUND) {
						bossMotion.target_velocity = { 0, -200 };
					}
				}
				break;
			case BOSS_ONE_POS::BOT_LEFT:
				if (bossPos.x > LEFT_OUTER_X_BOUND) {
					bossMotion.target_velocity = { -200,0 };
				}
				else {
					if (bossPos.y <= TOP_OUTER_Y_BOUND) {
						bossMotion.target_velocity = { 0, 200 };
					}
					else if (bossPos.y >= BOT_OUTER_Y_BOUND) {
						bossMotion.target_velocity = { 0, -200 };
					}
				}
				break;
			}
			check_state_two(currBoss);
			break;
		case BOSS_ONE_STATE::ONE_ALIVE_T_L:
			if (currBoss.boss_pos != BOSS_ONE_POS::MOTHER) {
				bossMotion.max_speed = 100;
				pathfinding(boss);
			}
			check_state_one(currBoss);
			break;
		case BOSS_ONE_STATE::ONE_ALIVE_T_R:
			if (currBoss.boss_pos != BOSS_ONE_POS::MOTHER) {
				bossMotion.max_speed = 100;
				pathfinding(boss);
			}
			check_state_one(currBoss);
			break;
		case BOSS_ONE_STATE::ONE_ALIVE_B_L:
			if (currBoss.boss_pos != BOSS_ONE_POS::MOTHER) {
				bossMotion.max_speed = 100;
				pathfinding(boss);
			}
			check_state_one(currBoss);
			break;
		case BOSS_ONE_STATE::ONE_ALIVE_B_R:
			if (currBoss.boss_pos != BOSS_ONE_POS::MOTHER) {
				bossMotion.max_speed = 100;
				pathfinding(boss);
			}
			check_state_one(currBoss);
			break;
		case BOSS_ONE_STATE::START:
			bossOneRegistry.get(boss).boss_state = BOSS_ONE_STATE::FOUR_ALIVE;
			break;
		}

		if (state != bossOneRegistry.get(boss).boss_state) {
			handleBossOneStateChange(boss);
		}
	}
}

void AISystem::boss_three_ai() {
	Entity& boss_entity = registry.bossThrees.entities[0];
	BossThree& boss_three = registry.bossThrees.get(boss_entity);
	Motion& boss_motion = registry.motions.get(boss_entity);
	Health& boss_health = registry.healthComponents.get(boss_entity);
	switch (boss_three.boss_phase)
	{
	case BOSS_THREE_PHASE::PHASE_ONE:
		pathfinding(boss_entity);
		registry.shooters.get(boss_entity).fire_rate = 2000;
		if (boss_health.curr_health <= 40) {
			boss_three.boss_phase = BOSS_THREE_PHASE::PHASE_TWO;
		}
		else if (boss_health.curr_health <= 10) {
			boss_three.boss_phase = BOSS_THREE_PHASE::PHASE_THREE;
		}
		break;
	case BOSS_THREE_PHASE::PHASE_TWO:
		pathfinding(boss_entity);
		registry.shooters.get(boss_entity).fire_rate = 1000;
		boss_motion.max_speed = 120;
		if (boss_health.curr_health <= 10) {
			boss_three.boss_phase = BOSS_THREE_PHASE::PHASE_THREE;
		}
		break;
	case BOSS_THREE_PHASE::PHASE_THREE:
		pathfinding(boss_entity);
		boss_motion.max_speed = 140; // Maybe this is too much?
		//registry.shooters.get(boss_entity).fire_rate = 750;
		break;
	}
}

void AISystem::step(float elapsed_ms) {
	auto& worldObjectRegistry = registry.worldObjects;
	auto& playerRegistry = registry.players;
	auto& deadlyRegistry = registry.deadlys;

	Entity& player = playerRegistry.entities[0]; // assuming always only 1 player

	if (playerRegistry.get(player).combat_state == COMBAT_STATE::BOSS_ONE_COMBAT) {
		boss_one_ai();
	}
	else if (playerRegistry.get(player).combat_state == COMBAT_STATE::BOSS_THREE_COMBAT) {
		boss_three_ai();
	}
	else {
		for (int i = 0; i < deadlyRegistry.entities.size(); i++) {
			Entity& enemy = deadlyRegistry.entities[i];
			if (!registry.bossOnes.has(enemy)) {
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
	}
}

