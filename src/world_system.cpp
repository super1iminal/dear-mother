// Header
#include "world_system.hpp"
#include "world_init.hpp"
#include "collision_system.hpp"

// stlib
#include <cassert>
#include <sstream>
#include <chrono>
#include <iostream>
#include <ui_system.hpp>
#include <glm/gtx/string_cast.hpp>
#include <reloadability_system.hpp>
#include "shop_system.hpp"

#include <fstream>
#include <iomanip>

// ==================== PUBLIC ====================
// ==================== BASIC FUNCTIONS ====================
// create the  world
WorldSystem::WorldSystem()
	: player_health(0)
	, current_speed(1.0)
	, current_room({ 0, 0 }) {
	// Seeding rng with random device
	rng = std::default_random_engine(std::random_device()());
}

WorldSystem::~WorldSystem() {

	// destroy music components
	if (melee_sound != nullptr)
		Mix_FreeChunk(melee_sound);
	if (player_shooting_sound != nullptr)
		Mix_FreeChunk(player_shooting_sound);
	if (player_projectile_damage_sound != nullptr)
		Mix_FreeChunk(player_projectile_damage_sound);
	if (enemy_shooting_sound != nullptr)
		Mix_FreeChunk(enemy_shooting_sound);
	if (post_combat_music != nullptr)
		Mix_FreeMusic(post_combat_music);

	Mix_CloseAudio();
}

void WorldSystem::init(RenderSystem* renderer_arg, GLFWwindow* window) {
	this->renderer = renderer_arg;
	this->window = window;
	//////////////////////////////////////

	// Audio from:
	// https://kenney.nl/assets/category:Audio
	// https://soundimage.org/sci-fi/
	// https://www.youtube.com/watch?v=BSpR0DJEgxM
	melee_sound = Mix_LoadWAV(audio_path("impactMetal_medium_003.wav").c_str());
	player_shooting_sound = Mix_LoadWAV(audio_path("laserSmall_000.wav").c_str());
	player_projectile_damage_sound = Mix_LoadWAV(audio_path("forceField_002.wav").c_str());
	enemy_shooting_sound = Mix_LoadWAV(audio_path("laserLarge_000.wav").c_str());
	post_combat_music = Mix_LoadMUS(audio_path("Factory-On-Mercury_Looping.wav").c_str());
	combat_music = Mix_LoadMUS(audio_path("The Death of Gods Will-[AudioTrimmer.com].wav").c_str());

	if (melee_sound == nullptr || player_shooting_sound == nullptr || player_projectile_damage_sound == nullptr
		|| enemy_shooting_sound == nullptr || post_combat_music == nullptr || combat_music == nullptr) {
		fprintf(stderr, "Failed to load sounds\n %s\n %s\n %s\n %s\n %s\n %s\n make sure the data directory is present",
			audio_path("impactMetal_medium_003.wav").c_str(),
			audio_path("laserSmall_000.wav").c_str(),
			audio_path("forceField_002.wav").c_str(),
			audio_path("laserLarge_000.wav").c_str(),
			audio_path("Factory-On-Mercury_Looping.wav").c_str(),
			audio_path("The Death of Gods Will-[AudioTrimmer.com].wav").c_str());
		exit(1);
	}

	// Create Item Set
	srand(time(0));
	buildItemSet();
	// Set all states to default
	restart_game();
}

void WorldSystem::restart_game() {
	// Debugging for memory/component leaks
	registry.list_all_components();
	printf("Restarting\n");

	reset_dialogue_run_status(); // dialogue-related, 
	// TODO: the above needs to occur when a new run happens, but not upon continue. currently happens on both

	// Reset the game speed
	current_speed = 1.f;

	// reset current room
	current_room = { 0, 0 };

	// reset boss one stuff
	text_shown = false;

	// Remove all entities that we created
	// i.e. All world objects
	for (int i = registry.gameSceneWorldObjects.entities.size() - 1; i >= 0; --i) {
		Entity entity = registry.gameSceneWorldObjects.entities[i];
		registry.remove_all_components_of(entity);
	}

	// Debugging for memory/component leaks
	registry.list_all_components();

	// create a new Player entity

	if (registry.gameLoadingHelper.components.size() == 0 || registry.gameLoadingHelper.components[0].savedGame == false) {
		player = createPlayer(renderer, { window_width_px / 2, window_height_px - 200 });
	}
	else {
		ReloadabilitySystem::loadGame();
		player = registry.players.entities[0];
		change_rooms(registry.roomCoords.get(player).position);
		update_player_modifier();
	}
	// function to use for interactable
	auto bound_interactable_fn = std::bind(&WorldSystem::increaseScrap, this, std::placeholders::_1);

	generate_rooms(renderer, current_room, uniform_dist, rng);

	// Set initial cooldown time
	for (Entity entity : registry.shooters.entities) {
		set_last_shot_time(entity);
	}

	// read upgrade values
	initUpgrades();

	initGameUI();
	if (registry.gameLoadingHelper.components.size() > 0) {
		registry.gameLoadingHelper.components[0].savedGame = false;
	}
}

void WorldSystem::update_music() {
	if (registry.players.get(player).combat_state == COMBAT_STATE::NO_COMBAT) {
		Mix_VolumeMusic(8);
		Mix_FadeInMusic(post_combat_music, -1, 2500);
	}
	else if (registry.players.get(player).combat_state != COMBAT_STATE::NO_COMBAT) {
		Mix_VolumeMusic(16);
		Mix_FadeInMusic(combat_music, -1, 5000);
	}

	// TODO: can changed based on boss
}

// Update our game world
bool WorldSystem::step(float elapsed_ms_since_last_update) {
	// Remove debug info from the last step. Need to iterate backwards to avoid catastrophic error
		// Remove debug info from the last step
	while (registry.debugComponents.entities.size() > 0)
		registry.remove_all_components_of(registry.debugComponents.entities.back());


	// Removing out of screen entities
	auto& worldObjects_registry = registry.worldObjects;

	// Remove entities that leave the screen on any side
	// Iterate backwards to be able to remove without unterfering with the next object to visit
	// (the containers exchange the last element with the current)
	for (Entity entity : worldObjects_registry.entities) {
		WorldObject& worldobject = worldObjects_registry.get(entity);
		if ((worldobject.position.x + abs(worldobject.scale.x) < 0.f ||
			worldobject.position.x - abs(worldobject.scale.x) > window_width_px ||
			worldobject.position.y + abs(worldobject.scale.y) < 0.f ||
			worldobject.position.y - abs(worldobject.scale.y) > window_height_px) &&
			!registry.players.has(entity)) {
			registry.pendingRemoves.emplace_with_duplicates(entity);
		}
	}

	// Remove entities with expired lifetimes. Need to iterate backwards to avoid catastrophic error
	for (int i = registry.lifetimes.entities.size() - 1; i >= 0; --i) {
		Entity entity = registry.lifetimes.entities[i];
		Lifetime& lifetime = registry.lifetimes.get(entity);
		lifetime.time_remaining_ms -= elapsed_ms_since_last_update;
		if (lifetime.time_remaining_ms < 0) {
			registry.pendingRemoves.emplace_with_duplicates(entity);
		}
	}

	// Shoot on cue
	for (Entity entity : registry.activeShooters.entities) {
		shoot(entity);
	}

	// Set player state
	int activeDeadlyCounter = registry.activeDeadlys.size();
	if (activeDeadlyCounter > 0) {
		if (registry.players.get(player).combat_state == COMBAT_STATE::NO_COMBAT) {
			registry.players.get(player).combat_state = COMBAT_STATE::NORMAL_COMBAT;
			update_music();
		}
	}
	else {
		if (registry.players.get(player).combat_state != COMBAT_STATE::NO_COMBAT) {
			if (registry.players.get(player).combat_state == COMBAT_STATE::BOSS_TWO_COMBAT) {
				handle_boss_two();
			}
			else {
				registry.players.get(player).combat_state = COMBAT_STATE::NO_COMBAT;
				update_music();
			}
		}
	}

	// Processing the player state
	assert(registry.screenStates.components.size() <= 1);
	ScreenState& screen = registry.screenStates.components[0];
	screen.health_status = player_health;

	float min_counter_ms = 3000.f;
	// cplayer is the only possible entitiy that should have a death timer (currently). use lifetimes for anything else
	for (Entity entity : registry.deathTimers.entities) {
		// progress timer
		DeathTimer& counter = registry.deathTimers.get(entity);
		counter.counter_ms -= elapsed_ms_since_last_update;
		if (counter.counter_ms < min_counter_ms) {
			min_counter_ms = counter.counter_ms;
		}

		// restart the game once the death timer expired
		if (counter.counter_ms < 0) {
			registry.deathTimers.remove(entity);
			screen.darken_screen_factor = 0;
			scene_manager.set_scene(SCENE_TYPE::MENU);
			return true;
		}
	}
	auto& invincibleTimerRegistry = registry.invincibleTimers;
	// iterate backwards
	for (int i = invincibleTimerRegistry.entities.size() - 1; i >= 0; --i) {
		Entity entity = invincibleTimerRegistry.entities[i];
		InvincibleTimer& counter = invincibleTimerRegistry.get(entity);
		counter.counter_ms -= elapsed_ms_since_last_update;
		if (counter.counter_ms < 0) {
			printf("counter.counter_ms: %f\n", counter.counter_ms);
			invincibleTimerRegistry.remove(entity);
			// remove the on-dmg flashing effect once the player can be hurt again
			registry.flashingColors.remove(entity);
		}
	}

	/*if (registry.players.get(player).combat_state == COMBAT_STATE::BOSS_TWO_COMBAT) {
		handle_boss_two();
	}*/

	// reduce window brightness if the salmon is dying
	screen.darken_screen_factor = 1 - min_counter_ms / 3000;

	return true;
}



// ==================== HANDLING FUNCTIONS ====================
// ran once per step. called from game_manager.cpp
// Compute collisions between entities, called after physics_system::step which checks for collisions
void WorldSystem::handle_collisions() {
	// god damn. 
	for (Entity entity : registry.collisions.entities) {
		std::vector<Collision*> collisions = registry.collisions.get_all(entity);

		for (int j = 0; j < collisions.size(); j++) {
			Collision collision = *collisions[j];
			Entity entity_other = collision.other;
			COLLISION_TYPE type = collision.type;
			if ((!registry.activeComponents.has(entity)) || (!registry.activeComponents.has(entity_other)))
				continue;

			// Handle collisions
					// Note that enum words are ordered in terms of what is main and what is other (DEADLY BLOCKER will be DEADLY and then other is BLOCKER)
			switch (type) {
			case COLLISION_TYPE::PLAYER_DEADLY:
				handlePlayerDeadly(entity, entity_other);
				break;
			case COLLISION_TYPE::DEADLY_BLOCKER:
				handleActorBlocker(entity, entity_other);
				break;
			case COLLISION_TYPE::PLAYER_BLOCKER:
				handleActorBlocker(entity, entity_other);
				break;
			case COLLISION_TYPE::PROJECTILE_BLOCKER:
				handleProjectileBlocker(entity, entity_other);
				break;
			case COLLISION_TYPE::PROJECTILE_DEADLY:
				handleProjectileDeadly(entity, entity_other);
				// note that this collision is only added if the projectile is friendly
				break;
			case COLLISION_TYPE::PROJECTILE_PLAYER:
				handleProjectilePlayer(entity, entity_other);
				// note that this collision is only added if the projectile is not friendly
				break;
			case COLLISION_TYPE::PLAYER_DOOR:
				handlePlayerDoor(entity, entity_other);
				break;
			case COLLISION_TYPE::PLAYER_BOSS_ONE:
				handlePlayerBossOne(entity, entity_other);
				break;
			default:
				printf("Unhandled collision\n");
				break;
			}
		}

	}

	// Remove all collisions from this simulation step
	registry.collisions.clear();
}

void WorldSystem::handle_deaths() {
	for (Entity entity : registry.healthComponents.entities)
	{
		Health& health = registry.healthComponents.get(entity);
		if (health.curr_health <= 0)
		{
			// Scream, reset timer, and make the salmon sink
			if (registry.players.has(entity)) {
				updateGameUI();//todo
				if (!registry.deathTimers.has(entity)) {
					ReloadabilitySystem::recordPlayerDeathRoom();
					registry.deathTimers.emplace(entity);
				}
			}
			else if (registry.activeDeadlys.has(entity)) {
				if (!registry.bossOnes.has(entity) && !registry.bossTwos.has(entity)) {
					if (uniform_dist(rng) * 100 > (100 - DROP_CHANCE)) {
						createItem(renderer, registry.worldObjects.get(entity).position, vec2(75, 75), uniform_dist, rng, current_room, ITEM_TYPE::RANDOM);
					}
						//createItem(renderer, registry.worldObjects.get(entity).position, vec2(75, 75), uniform_dist, rng, current_room, ITEM_TYPE::RANDOM);
				}
				else if (registry.bossOnes.has(entity)) {
					handle_boss_one_death(entity);
				}
				else if (registry.bossTwos.has(entity)) {
					///handle_boss_two_death(entity); TODO: implement this
				}
				registry.pendingRemoves.emplace_with_duplicates(entity);
			}
		}
	}
}



// ==================== UPDATE FUNCTIONS ====================
// ran once per step. called from game_manager.cpp
void WorldSystem::update_animations() {
	updatePlayerAnimation();

	// update enemy animations
	auto& enemyRegistry = registry.deadlys;
	auto& collisionsRegistry = registry.collisions;
	for (Entity entity : enemyRegistry.entities) {
		Deadly deadly = registry.deadlys.get(entity);
		if (deadly.attacking) {
			// play attack animation
			playEnemyAttack(entity);
		}
		else {
			updateEnemyAnimation(entity);
		}
	}
}



// ==================== CALLBACK FUNCTIONS ====================
// Input callback functions
void WorldSystem::on_mouse_button(GLFWwindow* window, int button, int action, int mods)
{
	left_mouse_button = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
}

// sc is scancode, we don't use it
void WorldSystem::on_key(int key, int sc, int action, int mod) {
	// Resetting game
	if (action == GLFW_RELEASE && key == GLFW_KEY_R) {
		restart_game();
	}

	Entity& player = registry.players.entities[0];
	Motion& player_motion = registry.motions.get(player);

	// Handle movement keys
	if (key == GLFW_KEY_W || key == GLFW_KEY_A || key == GLFW_KEY_S || key == GLFW_KEY_D) {
		bool up = glfwGetKey(window, GLFW_KEY_W) != GLFW_RELEASE;
		bool left = glfwGetKey(window, GLFW_KEY_A) != GLFW_RELEASE;
		bool down = glfwGetKey(window, GLFW_KEY_S) != GLFW_RELEASE;
		bool right = glfwGetKey(window, GLFW_KEY_D) != GLFW_RELEASE;

		int dx = (int)right - (int)left;
		int dy = (int)down - (int)up;

		if (dx == 0 && dy == 0) {
			player_motion.target_velocity = { 0.f, 0.f };
		}
		else {
			float angle = atan2f(dy, dx);
			if (angle < 0)
				angle += 2 * M_PI;
			// Apply speed modifier
			Modifier& speed_modifier = registry.modifiers.get(player);
			float new_max_speed = player_motion.max_speed + speed_modifier.speed_modifier_flat + (player_motion.max_speed * speed_modifier.speed_modifier_percent);
			player_motion.target_velocity = v_from_sa(new_max_speed, angle);
		}
	}

	if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE) {
		scene_manager.set_scene(SCENE_TYPE::PAUSE);
		//registry.players.get(player).in_combat = false;
	}

	// Interaction
	if (action == GLFW_PRESS && key == GLFW_KEY_E) {
		handle_interactions(&WorldSystem::handle_item_pickup);
	}
	if (action == GLFW_PRESS && key == GLFW_KEY_X) {
		handle_interactions(&WorldSystem::handle_scrapping);
	}

	// Adjust current speed with `<` and `>`
	if (action == GLFW_RELEASE && (mod & GLFW_MOD_SHIFT)) {
		if (key == GLFW_KEY_COMMA) {
			current_speed = fmax(0.f, current_speed - 0.1f);
			printf("Current speed = %f\n", current_speed);
		}
		else if (key == GLFW_KEY_PERIOD) {
			current_speed += 0.1f;
			printf("Current speed = %f\n", current_speed);
		}
	}

	// if (action == GLFW_RELEASE && key == GLFW_KEY_Z) {
	// 	ReloadabilitySystem::saveGame();
	// }

	vector<int> item_keys = { GLFW_KEY_1, GLFW_KEY_2, GLFW_KEY_3, GLFW_KEY_4, GLFW_KEY_5, GLFW_KEY_6, GLFW_KEY_7, GLFW_KEY_8, GLFW_KEY_9 };
	if (action == GLFW_RELEASE && std::find(item_keys.begin(), item_keys.end(), key) != item_keys.end()) {
		handle_item_drop(key - GLFW_KEY_1);
	}
}

void WorldSystem::on_mouse_move(vec2 mouse_position) {
	// nothing yet
}





// ==================== PRIVATE ====================
// ==================== INIT FUNCTIONS ====================
void WorldSystem::initUpgrades() {
	// get the inventory size
	Inventory& player_inventory = registry.inventory.get(player);
	Modifier& player_modifier = registry.modifiers.get(player);
	Health& player_health = registry.healthComponents.get(player);

	int item_slots = 0;
	int damage_upgrade = 0;
	int health_upgrade = 0;
	int crit_upgrade = 0;
	int dodge_upgrade = 0;
	int scrap = 0;

	std::map<std::string, int*> tagMap = {
		{"item_slots", &item_slots},
		{"damage_upgrade", &damage_upgrade},
		{"health_upgrade", &health_upgrade},
		{"crit_upgrade", &crit_upgrade},
		{"dodge_upgrade", &dodge_upgrade},
		{"scrap", &scrap}
	};

	std::ifstream read_file(std::string(PROJECT_SOURCE_DIR) + "/data/misc/upgrades.csv");
	if (read_file.is_open()) {
		std::string line;
		while (std::getline(read_file, line)) {
			std::stringstream ss(line);
			std::string tag;
			int value;
			if (std::getline(ss, tag, ',') && ss >> value) {
				if (tagMap.find(tag) != tagMap.end()) {
					*tagMap[tag] = value;
				}
			}
		}
		read_file.close();
	}
	else {
		std::cerr << "Unable to open file for reading.\n";
	}

	player_inventory.size = PLAYER_BASE_INV_SIZE + item_slots;
	player_modifier.damage_modifier_percentage = 1.0f + (damage_upgrade * DAMAGE_UPGRADE_MODIFIER);
	player_health.max_health = PLAYER_MAX_HEALTH + health_upgrade;
	player_modifier.crit_chance = 1 + (crit_upgrade * CRIT_DAMAGE_UPGRADE_MODIFIER);
	player_modifier.dodge_chance = (dodge_upgrade * 2);

	std::cout << "Item slots upgrade: " << item_slots << std::endl;
	std::cout << "Damage upgrade: " << damage_upgrade << std::endl;
	std::cout << "Health upgrade: " << health_upgrade << std::endl;
	std::cout << "Crit upgrade: " << crit_upgrade << std::endl;
	std::cout << "Dodge upgrade: " << dodge_upgrade << std::endl;
}

void WorldSystem::initGameUI() {
	// Add the base UI
	Entity base_ui = UISystem::createPanel(
		renderer,
		SCENE_TYPE::GAME,
		vec2(window_width_px / 2, BASE_UI_HEIGHT / 2),
		0.f,
		vec2(window_width_px, BASE_UI_HEIGHT),
		"game_HUD",
		TEXTURE_ASSET_ID::UI
	);

	// grab player health
	int player_health = registry.healthComponents.get(registry.players.entities[0]).curr_health;

	// create health_ui entity
	health_ui = UISystem::createTextUIElement(
		renderer,
		vec2(120.f, 75.f),
		vec2(8.f, 3.f),
		"health_ui",
		std::to_string(player_health),
		vec3(1.0, 1.0, 1.0),
		SCENE_TYPE::GAME);

	// create scrap_ui entity
	scrap_ui = UISystem::createTextUIElement(
		renderer,
		vec2(360.f, 45.f),
		vec2(8.f, 3.f),
		"scrap_ui",
		std::to_string(registry.players.components[0].scrap),
		vec3(1.0, 1.0, 1.0),
		SCENE_TYPE::GAME);

	// create level_ui entity
	level_ui = UISystem::createTextUIElement(
		renderer,
		vec2(368.f, 84.f),
		vec2(8.f, 3.f),
		"level_ui",
		std::to_string(level),
		vec3(1.0, 1.0, 1.0),
		SCENE_TYPE::GAME);

	// create item_ui entities
	// later, we will want to render all the items and show locked slots too
	Inventory& player_inventory = registry.inventory.get(player);
	for (auto item : player_inventory.items) {
		UISystem::createTexturedUIElement(
			renderer,
			vec2(window_width_px - ((item.first * ITEM_UI_OFFSET_X) + INITIAL_ITEM_UI_OFFSET_X), INITIAL_ITEM_UI_OFFSET_Y),
			vec2(75.f, 75.f),
			"item_ui_" + std::to_string(item.first),
			getItemTexture(player_inventory.items[item.first]),
			SCENE_TYPE::GAME);
	}

	// cover the locked slots
	for (uint i = 0; i < MAX_INVENTORY_SIZE - player_inventory.size; i++) {
		int opposite_offset = MAX_INVENTORY_SIZE - i - 1;
		UISystem::createSquareUIElement(
			renderer,
			vec2(window_width_px - ((opposite_offset * ITEM_UI_OFFSET_X) + INITIAL_ITEM_UI_OFFSET_X) - 5, INITIAL_ITEM_UI_OFFSET_Y),
			vec2(100.f, 100.f),
			"locked_slot_ui" + std::to_string(i),
			SCENE_TYPE::GAME
		);
	}

	UISystem::createCrosshair(renderer, TEXTURE_ASSET_ID::GAME_CROSSHAIR, SCENE_TYPE::GAME);

	// the following was a test. you can safely delete it. i may have forgotten to.
	//UISystem::createTextBox(renderer, vec2(window_width_px / 2, window_height_px / 2), vec2(DIALOGUE_BOX_WIDTH, DIALOGUE_BOX_HEIGHT), vec3(1.f, 1.f, 1.f), SCENE_TYPE::GAME, 
	//	"Hello. My name is Asher. \nThis is a test for dialogue boxes. The line needs a line break at some point. Let's see! Bah be boo be bahh be boo be bahh be boo be");
}



// ==================== ACTION FUNCTIONS ====================
void WorldSystem::shoot(Entity& entity) {
	auto now = get_curr_time();
	float elapsed_ms = (float)(std::chrono::duration_cast<std::chrono::microseconds>(now - registry.shooters.get(entity).t)).count() / 1000;
	Motion& entity_motion = registry.motions.get(entity);
	WorldObject& entity_object = registry.worldObjects.get(entity);

	if (registry.players.has(entity)) {
		if (left_mouse_button) {
			// Apply fire rate modifier 
			float old_fire_rate = registry.shooters.get(entity).fire_rate;
			Modifier player_modifier = registry.modifiers.get(entity);
			float new_fire_rate = old_fire_rate - player_modifier.fire_rate_modifier_flat - (old_fire_rate * player_modifier.fire_rate_modifier_percent);
			if (elapsed_ms >= new_fire_rate) {
				double xpos, ypos;
				glfwGetCursorPos(window, &xpos, &ypos);
				float angle = atan2(ypos - entity_object.position.y, xpos - entity_object.position.x);

				// Apply modifiers to player bullets
				angle += (2 * (uniform_dist(rng) - 0.5)) * player_modifier.accuracy_modifier;
				Entity projectile = createProjectile(renderer, entity_object.position, angle, 350.0f, true, current_room);
				float bullet_range = registry.lifetimes.get(projectile).time_remaining_ms;
				registry.lifetimes.get(projectile).time_remaining_ms += player_modifier.range_modifier_flat + (bullet_range * player_modifier.range_modifier_percent);
				Modifier& player_modifier = registry.modifiers.get(player);

				int projectile_damage = (registry.projectiles.get(projectile).damage + player_modifier.damage_modifier_flat) * player_modifier.damage_modifier_percentage;
				// check if this hit could be a crit
				if (uniform_dist(rng) * 100 > (100 - player_modifier.crit_chance)) {
					projectile_damage *= 2;
					registry.colors.insert(projectile, vec3(1.0, 0.4, 1.0));
				}
				registry.projectiles.get(projectile).damage = projectile_damage;

				Mix_Volume(Mix_PlayChannel(-1, player_shooting_sound, 0), 5);

				set_last_shot_time(entity);
			}
		}
	}
	else if (registry.deadlys.has(entity)) {
		if ((elapsed_ms >= registry.shooters.get(entity).fire_rate)) {
			if (registry.players.get(player).combat_state == COMBAT_STATE::BOSS_ONE_COMBAT) {
				boss_one_shoot(entity, entity_object);
			}
			else {
				vec2 coor_player = registry.worldObjects.get(registry.players.entities[0]).position;
				int dx = entity_object.position.x - coor_player.x;
				int dy = entity_object.position.y - coor_player.y;
				float angle = atan2(dy, dx) - M_PI;
				if (angle < 0)
					angle += 2 * M_PI;
				createProjectile(renderer, entity_object.position, angle, 350.0f, false, current_room);
				Mix_Volume(Mix_PlayChannel(-1, enemy_shooting_sound, 0), 5);
				set_last_shot_time(entity);
			}
		}
	}
}

void WorldSystem::boss_one_shoot(Entity& entity, WorldObject& entity_object) {
	BossOne& boss = registry.bossOnes.get(entity);

	if (boss.boss_pos != BOSS_ONE_POS::MOTHER) {
		if (boss.boss_state == BOSS_ONE_STATE::FOUR_ALIVE) {
			if (boss.boss_pos == BOSS_ONE_POS::TOP_LEFT ||
				boss.boss_pos == BOSS_ONE_POS::TOP_RIGHT) {
				// 15 degree spread
				/*createProjectile(renderer, entity_object.position, radians(75.f), 350.0f, false, current_room);
				createProjectile(renderer, entity_object.position, radians(90.f), 350.0f, false, current_room);
				createProjectile(renderer, entity_object.position, radians(105.f), 350.0f, false, current_room);*/

				// 10 degree spread
				createProjectile(renderer, entity_object.position, radians(80.f), 350.0f, false, current_room);
				createProjectile(renderer, entity_object.position, radians(90.f), 350.0f, false, current_room);
				createProjectile(renderer, entity_object.position, radians(100.f), 350.0f, false, current_room);

				createProjectile(renderer, entity_object.position, radians(0.f), 350.0f, false, current_room);
				createProjectile(renderer, entity_object.position, radians(180.f), 350.0f, false, current_room);
				//Mix_Volume(Mix_PlayChannel(-1, enemy_shooting_sound, 0), 5);
			}
			else {
				// 15 degree spread
				/*createProjectile(renderer, entity_object.position, radians(255.f), 350.0f, false, current_room);
				createProjectile(renderer, entity_object.position, radians(270.f), 350.0f, false, current_room);
				createProjectile(renderer, entity_object.position, radians(285.f), 350.0f, false, current_room);*/

				// 10 degree spread
				createProjectile(renderer, entity_object.position, radians(260.f), 350.0f, false, current_room);
				createProjectile(renderer, entity_object.position, radians(270.f), 350.0f, false, current_room);
				createProjectile(renderer, entity_object.position, radians(280.f), 350.0f, false, current_room);

				createProjectile(renderer, entity_object.position, radians(0.f), 350.0f, false, current_room);
				createProjectile(renderer, entity_object.position, radians(180.f), 350.0f, false, current_room);
				//Mix_Volume(Mix_PlayChannel(-1, enemy_shooting_sound, 0), 5);
			}
		}
		else if (boss.boss_state == BOSS_ONE_STATE::THREE_ALIVE_ONE_BOT_LEFT
			|| boss.boss_state == BOSS_ONE_STATE::THREE_ALIVE_ONE_BOT_RIGHT
			|| boss.boss_state == BOSS_ONE_STATE::THREE_ALIVE_ONE_TOP_LEFT
			|| boss.boss_state == BOSS_ONE_STATE::THREE_ALIVE_ONE_TOP_RIGHT) {

			createProjectile(renderer, entity_object.position, radians(0.f + boss.bullet_angle), 350.0f, false, current_room);
			createProjectile(renderer, entity_object.position, radians(180.f + boss.bullet_angle), 350.0f, false, current_room);

			boss.bullet_angle += 25;
		}
		else if (boss.boss_state == BOSS_ONE_STATE::TWO_OPPOSITE_SIDE_ALIVE_BL_TR
			|| boss.boss_state == BOSS_ONE_STATE::TWO_SAME_SIDE_ALIVE_BOT
			|| boss.boss_state == BOSS_ONE_STATE::TWO_SAME_SIDE_ALIVE_TOP) {

			if (boss.boss_pos == BOSS_ONE_POS::TOP_LEFT ||
				boss.boss_pos == BOSS_ONE_POS::BOT_LEFT) {
				createProjectile(renderer, entity_object.position, radians(0.f), 350.0f, false, current_room);
				createProjectile(renderer, entity_object.position, radians(45.f), 350.0f, false, current_room);
				createProjectile(renderer, entity_object.position, radians(-45.f), 350.0f, false, current_room);
			}
			else {
				createProjectile(renderer, entity_object.position, radians(180.f), 350.0f, false, current_room);
				createProjectile(renderer, entity_object.position, radians(225.f), 350.0f, false, current_room);
				createProjectile(renderer, entity_object.position, radians(135.f), 350.0f, false, current_room);
			}
		}
		else if (boss.boss_state == BOSS_ONE_STATE::TWO_OPPOSITE_SIDE_ALIVE_R_R) {
			if (boss.boss_pos == BOSS_ONE_POS::TOP_RIGHT) {
				createProjectile(renderer, entity_object.position, radians(180.f), 350.0f, false, current_room);
				createProjectile(renderer, entity_object.position, radians(225.f), 350.0f, false, current_room);
				createProjectile(renderer, entity_object.position, radians(135.f), 350.0f, false, current_room);
			}
			else {
				createProjectile(renderer, entity_object.position, radians(0.f), 350.0f, false, current_room);
				createProjectile(renderer, entity_object.position, radians(45.f), 350.0f, false, current_room);
				createProjectile(renderer, entity_object.position, radians(-45.f), 350.0f, false, current_room);
			}
		}
		else if (boss.boss_state == BOSS_ONE_STATE::TWO_OPPOSITE_SIDE_ALIVE_L_L) {
			if (boss.boss_pos == BOSS_ONE_POS::TOP_LEFT) {
				createProjectile(renderer, entity_object.position, radians(180.f), 350.0f, false, current_room);
				createProjectile(renderer, entity_object.position, radians(225.f), 350.0f, false, current_room);
				createProjectile(renderer, entity_object.position, radians(135.f), 350.0f, false, current_room);
			}
			else {
				createProjectile(renderer, entity_object.position, radians(0.f), 350.0f, false, current_room);
				createProjectile(renderer, entity_object.position, radians(45.f), 350.0f, false, current_room);
				createProjectile(renderer, entity_object.position, radians(-45.f), 350.0f, false, current_room);
			}
		}
		else if (boss.boss_state == BOSS_ONE_STATE::TWO_OPPOSITE_SIDE_ALIVE_BR_TL) {
			if (boss.boss_pos == BOSS_ONE_POS::TOP_LEFT) {
				createProjectile(renderer, entity_object.position, radians(180.f), 350.0f, false, current_room);
				createProjectile(renderer, entity_object.position, radians(225.f), 350.0f, false, current_room);
				createProjectile(renderer, entity_object.position, radians(135.f), 350.0f, false, current_room);
			}
			else {
				createProjectile(renderer, entity_object.position, radians(0.f), 350.0f, false, current_room);
				createProjectile(renderer, entity_object.position, radians(45.f), 350.0f, false, current_room);
				createProjectile(renderer, entity_object.position, radians(-45.f), 350.0f, false, current_room);
			}
		}
	}
	else if (boss.boss_state == BOSS_ONE_STATE::ALL_DEAD) {
		float wave = 15.f;
		if (boss.shot_pattern == 0) {
			boss.shot_pattern++;

			createProjectile(renderer, entity_object.position, radians(0.f), 350.0f, false, current_room);
			createProjectile(renderer, entity_object.position, radians(20.f), 350.0f, false, current_room);
			createProjectile(renderer, entity_object.position, radians(40.f), 350.0f, false, current_room);

			createProjectile(renderer, entity_object.position, radians(60.f), 350.0f, false, current_room);
			createProjectile(renderer, entity_object.position, radians(80.f), 350.0f, false, current_room);
			createProjectile(renderer, entity_object.position, radians(100.f), 350.0f, false, current_room);
			createProjectile(renderer, entity_object.position, radians(120.f), 350.0f, false, current_room);

			createProjectile(renderer, entity_object.position, radians(140.f), 350.0f, false, current_room);
			createProjectile(renderer, entity_object.position, radians(160.f), 350.0f, false, current_room);
			createProjectile(renderer, entity_object.position, radians(180.f), 350.0f, false, current_room);
		}
		else if (boss.shot_pattern == 1) {
			boss.shot_pattern++;

			createProjectile(renderer, entity_object.position, radians(0.f - wave), 350.0f, false, current_room);
			createProjectile(renderer, entity_object.position, radians(20.f - wave), 350.0f, false, current_room);
			createProjectile(renderer, entity_object.position, radians(40.f - wave), 350.0f, false, current_room);

			createProjectile(renderer, entity_object.position, radians(60.f - wave), 350.0f, false, current_room);
			createProjectile(renderer, entity_object.position, radians(80.f - wave), 350.0f, false, current_room);
			createProjectile(renderer, entity_object.position, radians(100.f - wave), 350.0f, false, current_room);
			createProjectile(renderer, entity_object.position, radians(120.f - wave), 350.0f, false, current_room);

			createProjectile(renderer, entity_object.position, radians(140.f - wave), 350.0f, false, current_room);
			createProjectile(renderer, entity_object.position, radians(160.f - wave), 350.0f, false, current_room);
			createProjectile(renderer, entity_object.position, radians(180.f - wave), 350.0f, false, current_room);
		}
		else if (boss.shot_pattern == 2) {
			boss.shot_pattern = 0;

			createProjectile(renderer, entity_object.position, radians(0.f + wave), 350.0f, false, current_room);
			createProjectile(renderer, entity_object.position, radians(20.f + wave), 350.0f, false, current_room);
			createProjectile(renderer, entity_object.position, radians(40.f + wave), 350.0f, false, current_room);

			createProjectile(renderer, entity_object.position, radians(60.f + wave), 350.0f, false, current_room);
			createProjectile(renderer, entity_object.position, radians(80.f + wave), 350.0f, false, current_room);
			createProjectile(renderer, entity_object.position, radians(100.f + wave), 350.0f, false, current_room);
			createProjectile(renderer, entity_object.position, radians(120.f + wave), 350.0f, false, current_room);

			createProjectile(renderer, entity_object.position, radians(140.f + wave), 350.0f, false, current_room);
			createProjectile(renderer, entity_object.position, radians(160.f + wave), 350.0f, false, current_room);
			createProjectile(renderer, entity_object.position, radians(180.f + wave), 350.0f, false, current_room);
		}
	}
	set_last_shot_time(entity);
}

void WorldSystem::change_rooms(ivec2 new_room) {
	auto roomMap = registry.map.components[0].roomMap;
	registry.roomCoords.get(player).position = new_room;
	current_room = new_room;
	if (roomMap[{current_room.x, current_room.y}] == ROOM_TYPE::BOSS_ROOM_ONE
		&& !registry.players.get(player).boss_one_beat) {
		registry.players.get(registry.players.entities[0]).combat_state = COMBAT_STATE::BOSS_ONE_COMBAT;
		update_music();
	}
	else if (roomMap[{current_room.x, current_room.y}] == ROOM_TYPE::BOSS_ROOM_TWO
		&& !registry.players.get(player).boss_two_beat) {
		registry.players.get(registry.players.entities[0]).combat_state = COMBAT_STATE::BOSS_TWO_COMBAT;
		update_music();
	}
	registry.activeComponents.clear();
	for (Entity entity : registry.gameSceneComponents.entities) {
		if (!registry.roomCoords.has(entity)) {
			registry.activeComponents.emplace(entity);
		}
		else if (registry.roomCoords.get(entity).position == current_room) {
			registry.activeComponents.emplace(entity);
		}
	}
}

TEXTURE_ASSET_ID WorldSystem::getItemTexture(ItemStat item) {
	TEXTURE_ASSET_ID item_texture = TEXTURE_ASSET_ID::BATTERY_PACK;
	switch (item.name)
	{
	case ITEM_NAME::BATTERY_PACK:
		item_texture = TEXTURE_ASSET_ID::BATTERY_PACK;
		break;
	case ITEM_NAME::SHATTERED_QUARTZ:
		item_texture = TEXTURE_ASSET_ID::SHATTERED_QUARTZ;
		break;
	case ITEM_NAME::REPEATER:
		item_texture = TEXTURE_ASSET_ID::REPEATER;
		break;
	case ITEM_NAME::CREAKY_WHEEL:
		item_texture = TEXTURE_ASSET_ID::CREAKY_WHEEL;
		break;
	case ITEM_NAME::HEATSINK:
		item_texture = TEXTURE_ASSET_ID::HEATSINK;
		break;
	}
	return item_texture;
}

std::chrono::steady_clock::time_point WorldSystem::get_curr_time() {
	return std::chrono::high_resolution_clock::now();
}

void WorldSystem::set_last_shot_time(Entity& entity) {
	auto& shooter = registry.shooters.get(entity);
	using Clock = std::chrono::high_resolution_clock;
	shooter.t = Clock::now();
}

void WorldSystem::playEnemyAttack(Entity enemy) {

	Animation& enemy_animation = registry.animations.get(enemy);
	Deadly& deadly = registry.deadlys.get(enemy);
	Motion enemy_motion = registry.motions.get(enemy);
	RenderRequest& enemy_render_request = registry.renderRequests.get(enemy);

	if (deadly.type == 0) {
		enemy_render_request.used_texture = TEXTURE_ASSET_ID::ENEMY_ATTACK;
		enemy_animation.cols = 7;
		enemy_animation.frames = 7;
	}
	else if (deadly.type == 2) { // Final mother bodygaurds
		enemy_render_request.used_texture = TEXTURE_ASSET_ID::ENEMY_ATTACK;
		enemy_animation.cols = 7;
		enemy_animation.frames = 7;
	}
	else if (deadly.type == 3) { // Final mother (no attack animation at the moment)
		enemy_render_request.used_texture = TEXTURE_ASSET_ID::MOTHER_FINAL_IDLE;
		enemy_animation.cols = 22;
		enemy_animation.frames = 22;
	}
	else if (deadly.type == 4) {
		enemy_render_request.used_texture = TEXTURE_ASSET_ID::HEAVY_ATTACK;
		enemy_animation.cols = 7;
		enemy_animation.frames = 7;
	}
	deadly.attacking = false;
}

// add a flashing effect when the player does a successful dodge
void playPlayerDodgeEffect(Entity entity) {
	FlashingColor& flashing_color = registry.flashingColors.emplace(entity);
	flashing_color.color = vec3(0.2, 0.2, 1.0);
	flashing_color.flash_rate = 100;
}

// add a flashing effect when the player is damaged
void playPlayerDamagedEffect(Entity entity) {
	FlashingColor& flashing_color = registry.flashingColors.emplace(entity);
	flashing_color.color = vec3(1.0, 0.2, 0.2);
	flashing_color.flash_rate = 100;
}

void remove_item(Entity item) {
	// give this item a flashing effect
	FlashingColor& flashing_color = registry.flashingColors.emplace(item);
	flashing_color.color = vec3(0.2, 0.2, 1.0);
	flashing_color.flash_rate = 100;

	// make the item float upwards a bit
	Motion& item_motion = registry.motions.emplace(item);
	item_motion.velocity = vec2(0.f, -75.f);

	// remove this item after 1s
	Lifetime& item_lifetime = registry.lifetimes.emplace(item);
	item_lifetime.time_remaining_ms = 1000;
}

// used in game manager for pause and dialogue systems - bugfix TODO: make better
void WorldSystem::set_player_velocity(vec2 velocity) {
	registry.motions.get(registry.players.entities[0]).target_velocity = velocity;
	registry.motions.get(registry.players.entities[0]).velocity = velocity;
	updatePlayerAnimation();
}



// ==================== HANDLE FUNCTIONS ====================
void WorldSystem::handle_boss_one_death(Entity& entity) {
	BossOne& boss_part = registry.bossOnes.get(entity);
	if (boss_part.boss_pos == BOSS_ONE_POS::TOP_LEFT) {
		for (BossOne& b : registry.bossOnes.components) {
			b.top_left_alive = false;
		}
	}
	else if (boss_part.boss_pos == BOSS_ONE_POS::TOP_RIGHT) {
		for (BossOne& b : registry.bossOnes.components) {
			b.top_right_alive = false;
		}
	}
	else if (boss_part.boss_pos == BOSS_ONE_POS::BOT_LEFT) {
		for (BossOne& b : registry.bossOnes.components) {
			b.bot_left_alive = false;
		}
	}
	else if (boss_part.boss_pos == BOSS_ONE_POS::BOT_RIGHT) {
		for (BossOne& b : registry.bossOnes.components) {
			b.bot_right_alive = false;
		}
	}
	else if (boss_part.boss_pos == BOSS_ONE_POS::MOTHER) {
		for (BossOne& b : registry.bossOnes.components) {
			b.mother = false;
		}
	}
	if (!text_shown && (((boss_part.top_right_alive ? 1 : 0) + (boss_part.bot_left_alive ? 1 : 0) + (boss_part.bot_right_alive ? 1 : 0) + (boss_part.top_left_alive ? 1 : 0)))==1) {
		final_phase_text = create_self_destruct_text(renderer, current_room);
		text_shown = true;
	}

	if (!boss_part.top_left_alive && !boss_part.top_right_alive
		&& !boss_part.bot_left_alive && !boss_part.bot_right_alive
		&& boss_part.mother) {
		registry.pendingRemoves.emplace_with_duplicates(final_phase_text);
	}

	else if (!boss_part.top_left_alive && !boss_part.top_right_alive
		&& !boss_part.bot_left_alive && !boss_part.bot_right_alive
		&& !boss_part.mother) {
		// DROP ITEMS HERE FOR KILLING BOSS
		createItem(renderer, vec2(CENTER_X - 100, CENTER_Y), vec2(75, 75), uniform_dist, rng, current_room, ITEM_TYPE::HEALTH_PACK);
		createItem(renderer, vec2(CENTER_X + 100, CENTER_Y), vec2(75, 75), uniform_dist, rng, current_room, ITEM_TYPE::RANDOM);
		registry.players.get(player).combat_state = COMBAT_STATE::NO_COMBAT; // might not be necessary
		registry.players.get(player).boss_one_beat = true;
		update_music();
		std::cout << "YAYYYY :3" << std::endl;
	}
}

void WorldSystem::handle_boss_two() {
	int alive = 0;

	BossTwo& bossTwo = registry.bossTwos.components[0];
	if (alive == 0 && bossTwo.curr_wave == BOSS_TWO_WAVE::WAVE_ONE) {
		int left = 0;
		int right = 0;
		for (int i = 0; i < bossTwo.wave_1; i++) {
			std::cout << "WAVE 1" << std::endl;
			int loc = rand() % 2;
			if (loc == 0) {
				createEnemy(renderer, { 342, 234 + 25 * left }, 200, current_room);
				left++;
			}
			else {
				createEnemy(renderer, { 982, 234 + 25 * right }, 200, current_room);
				right++;
			}
		}
		bossTwo.wave_1 = 0;
	}

	if (alive == 0 && bossTwo.curr_wave == BOSS_TWO_WAVE::WAVE_TWO) {
		int left = 0;
		int right = 0;
		for (int i = 0; i < bossTwo.wave_2; i++) {
			std::cout << "WAVE 2" << std::endl;
			int loc = rand() % 2;
			if (loc == 0) {
				createEnemy(renderer, { 342, 234 + 25 * left }, 200, current_room);
				left++;
			}
			else {
				createEnemy(renderer, { 982, 234 + 25 * right }, 200, current_room);
				right++;
			}
		}
		bossTwo.wave_2 = 0;
	}

	if (alive == 0 && bossTwo.curr_wave == BOSS_TWO_WAVE::WAVE_THREE) {
		int left = 0;
		int right = 0;
		for (int i = 0; i < bossTwo.wave_3; i++) {
			std::cout << "WAVE :3" << std::endl;
			int loc = rand() % 2;
			if (loc == 0) {
				createEnemy(renderer, { 342, 234 + 25 * left }, 200, current_room);
				left++;
			}
			else {
				createEnemy(renderer, { 982, 234 + 25 * right }, 200, current_room);
				right++;
			}
		}
		bossTwo.wave_3 = 0;
	}

	if (alive == 0 && bossTwo.curr_wave == BOSS_TWO_WAVE::WAVE_FOUR) {
		int left = 0;
		int right = 0;
		for (int i = 0; i < bossTwo.wave_4; i++) {
			std::cout << "WAVE 4" << std::endl;
			int loc = rand() % 2;
			if (loc == 0) {
				createEnemy(renderer, { 342, 234 + 25 * left }, 200, current_room);
				left++;
			}
			else {
				createEnemy(renderer, { 982, 234 + 25 * right }, 200, current_room);
				right++;
			}
		}
		bossTwo.wave_4 = 0;
	}

	alive = 0;
	for (Entity entity : registry.deadlys.entities) {
		if (registry.activeComponents.has(entity)) {
			alive++;
		}
	}

	if (bossTwo.curr_wave == BOSS_TWO_WAVE::WAVE_ONE && alive == 0) {
		bossTwo.curr_wave = BOSS_TWO_WAVE::WAVE_TWO;
	}
	else if (bossTwo.curr_wave == BOSS_TWO_WAVE::WAVE_TWO && alive == 0) {
		bossTwo.curr_wave = BOSS_TWO_WAVE::WAVE_THREE;
	}
	else if (bossTwo.curr_wave == BOSS_TWO_WAVE::WAVE_THREE && alive == 0) {
		bossTwo.curr_wave = BOSS_TWO_WAVE::WAVE_FOUR;
	}
	else if (bossTwo.curr_wave == BOSS_TWO_WAVE::WAVE_FOUR && alive == 0 && !registry.players.get(player).boss_two_beat) {
		registry.players.get(player).combat_state = COMBAT_STATE::NO_COMBAT; // Don't move or delete this line. It is needed for the boss to function, in particular to return to the non-combat state.
		registry.players.get(player).boss_two_beat = true;
		createItem(renderer, vec2(CENTER_X - 100, CENTER_Y), vec2(75, 75), uniform_dist, rng, current_room, ITEM_TYPE::HEALTH_PACK);
		createItem(renderer, vec2(CENTER_X + 100, CENTER_Y), vec2(75, 75), uniform_dist, rng, current_room, ITEM_TYPE::RANDOM);
		update_music();
	}
}

void WorldSystem::handle_item_pickup(Entity item) {
	// Pick up item and apply effects to the player
	Inventory& player_inventory = registry.inventory.get(player);
	ItemStat new_item = registry.itemStats.get(item);
	if (!registry.lifetimes.has(item)) { // to avoid adding item more than once
		if ((player_inventory.items.size() < player_inventory.size) && (new_item.type != ITEM_TYPE::HEALTH_PACK)) {
			int key = 0;
			while (player_inventory.items.find(key) != player_inventory.items.end()) {
				key++;
			}
			player_inventory.items[key] = new_item;
			update_player_modifier();
			remove_item(item);
		}
		else if (new_item.type == ITEM_TYPE::HEALTH_PACK) {
			// Apply health pack item
			Health& player_health = registry.healthComponents.get(player);
			if (player_health.curr_health + new_item.heal_size <= player_health.max_health) {
				player_health.curr_health = player_health.curr_health + new_item.heal_size;
				remove_item(item);
			}
			else if (player_health.curr_health < player_health.max_health) {
				player_health.curr_health = player_health.max_health;
				remove_item(item);
			}
			
		}
		else {
			std::cout << "Already have " << player_inventory.size << " items" << std::endl;
		}
		updateGameUI();
	}
}

void WorldSystem::handle_item_drop(int item_key) {
	Inventory& player_inventory = registry.inventory.get(player);
	//print all item keys and values
	for (auto const& item : player_inventory.items) {
		std::cout << "Item key: " << item.first << " pressed key: "<<item_key  << std::endl;
	}

	if (player_inventory.items.find(item_key) == player_inventory.items.end()) {
		std::cout << "Item not found" << std::endl;
		return;
	}
	ItemStat dropped_item = player_inventory.items[item_key];
	// create a new item entity
	Entity new_item = createItem(renderer, registry.worldObjects.get(player).position, vec2(75, 75), uniform_dist, rng, current_room, dropped_item.type, &dropped_item);
	// remove the item from the player's inventory
	player_inventory.items.erase(item_key);
	for (Entity entity : registry.uiElements.entities) {
		if (registry.uiElements.get(entity).name == "item_ui_" + std::to_string(item_key)) {
			registry.pendingRemoves.emplace(entity);
		}
	}
	update_player_modifier();
	updateGameUI();
}

void WorldSystem::handle_scrapping(Entity item) {
	ItemStat item_stat = registry.itemStats.get(item);
	registry.players.components[0].scrap += item_stat.scrap_amt;
	ShopSystem::updateScrapLevel(ShopSystem::getScrapLevel()+item_stat.scrap_amt);
	remove_item(item);
	updateGameUI();
}
void WorldSystem::increaseScrap(int amt) {
	registry.players.components[0].scrap += amt;
	updateGameUI();
}

void WorldSystem::handle_interactions(void (WorldSystem::*func)(Entity)) {
	printf("interactable handling triggered\n");
	auto& interactablesRegistry = registry.interactables;
	for (Entity interactableEntity : registry.activeInteractables.entities) {
		Interactable& interactable = interactablesRegistry.get(interactableEntity);

		float range = interactable.range;

		WorldObject& playerWorldObject = registry.worldObjects.get(player);
		WorldObject& interactableObject = registry.worldObjects.get(interactableEntity);

		float dist = distance(playerWorldObject.position, interactableObject.position);
		if (dist < range) {
			if (registry.itemStats.has(interactableEntity)) {
				(this->*func)(interactableEntity);
			}
			else {
				interactable.interaction(interactable.value, interactableEntity);
			}
		}
	}
}

void WorldSystem::handlePlayerDoor(Entity player, Entity door) {
	// Lock door during combat
	if (registry.players.get(player).combat_state == COMBAT_STATE::NO_COMBAT) {
		// change rooms

		ivec2 new_room = registry.doors.get(door).leads_to;
		printf("room switching from %d, %d to %d, %d\n", current_room.x, current_room.y, new_room.x, new_room.y);
		WorldObject& player_worldobject = registry.worldObjects.get(player);

		int dx = new_room.x - current_room.x; // positive if moving to the right
		int dy = new_room.y - current_room.y; // positive if moving up

		if (dx == 1) {
			player_worldobject.position = { WALL_WIDTH + 80, (window_height_px - BASE_UI_HEIGHT) / 2.f + BASE_UI_HEIGHT };
		}
		else if (dx == -1) {
			player_worldobject.position = { window_width_px - (WALL_WIDTH + 80), (window_height_px - BASE_UI_HEIGHT) / 2.f + BASE_UI_HEIGHT };
		}
		else if (dy == 1) {
			player_worldobject.position = { window_width_px / 2, (window_height_px - WALL_WIDTH) - 80 };
		}
		else if (dy == -1) {
			player_worldobject.position = { window_width_px / 2, BASE_UI_HEIGHT + (WALL_WIDTH + 80) };
		}
		change_rooms(new_room);
	}
}

void WorldSystem::handlePlayerBossOne(Entity player, Entity boss) {
	std::array<Entity, 2> entities = { player, boss };
	for (Entity entity : entities) {
		if (!registry.invincibleTimers.has(entity) && !registry.deathTimers.has(player)) {
			registry.invincibleTimers.emplace(entity);
			if (registry.players.has(entity)) {
				registry.healthComponents.get(entity).curr_health -= registry.deadlys.get(boss).melee_damge;

				if (registry.bossOnes.has(boss)) {
					if ((registry.bossOnes.get(boss).boss_state == BOSS_ONE_STATE::ONE_ALIVE_B_L
						|| registry.bossOnes.get(boss).boss_state == BOSS_ONE_STATE::ONE_ALIVE_B_R
						|| registry.bossOnes.get(boss).boss_state == BOSS_ONE_STATE::ONE_ALIVE_T_L
						|| registry.bossOnes.get(boss).boss_state == BOSS_ONE_STATE::ONE_ALIVE_T_R)
						&& registry.bossOnes.get(boss).boss_pos != BOSS_ONE_POS::MOTHER) {
						registry.healthComponents.get(boss).curr_health = 0;
					}
				}
			}
			else if (!registry.deadlys.get(entity).immune) {
				registry.healthComponents.get(entity).curr_health -= 1;
			}
			updateGameUI();
			if (registry.players.has(entity)) {
				createParticles(renderer, registry.worldObjects.get(entity).position, uniform_dist, rng, TEXTURE_ASSET_ID::HIT_PARTICLE_PLAYER, current_room);
				Mix_Volume(Mix_PlayChannel(-1, melee_sound, 0), 10);
			}
			else {
				createParticles(renderer, registry.worldObjects.get(entity).position, uniform_dist, rng, TEXTURE_ASSET_ID::HIT_PARTICLE, current_room);
			}

		}
	}
	return;
}

void WorldSystem::handlePlayerDeadly(Entity player, Entity deadly) {

	std::array<Entity, 2> entities = { player, deadly };
	for (Entity entity : entities) {
		if (!registry.invincibleTimers.has(entity) && !registry.deathTimers.has(player)) {
			registry.invincibleTimers.emplace(entity);
			if (registry.players.has(entity)) {
				Modifier& player_modifier = registry.modifiers.get(player);
				if (uniform_dist(rng) * 100 > (100 - player_modifier.dodge_chance)) {
					// successful dodge; do not remove health
					playPlayerDodgeEffect(player);
					// TODO: a special sound effect would be nice
				}
				else if (registry.deadlys.get(deadly).type != 1) {
					registry.healthComponents.get(entity).curr_health -= 1;
					updateGameUI();
					playPlayerDamagedEffect(player);
					createParticles(renderer, registry.worldObjects.get(entity).position, uniform_dist, rng, TEXTURE_ASSET_ID::HIT_PARTICLE_PLAYER, current_room);
					Mix_Volume(Mix_PlayChannel(-1, melee_sound, 0), 10);
				}
			}
		}
	}
	return;
}

void WorldSystem::handleActorBlocker(Entity actor, Entity blocker) {
	// Get the WorldObject components of both entities
	WorldObject& worldobject_actor = registry.worldObjects.get(actor);
	WorldObject& worldobject_blocker = registry.worldObjects.get(blocker);

	// Get the bounding boxes (sizes) of both entities
	vec2 bbox_actor = get_bounding_box(worldobject_actor);
	vec2 bbox_blocker = get_bounding_box(worldobject_blocker);

	// Compute half sizes for easier calculation
	float half_width_actor = bbox_actor.x / 2.0f;
	float half_height_actor = bbox_actor.y / 2.0f;
	float half_width_blocker = bbox_blocker.x / 2.0f;
	float half_height_blocker = bbox_blocker.y / 2.0f;

	// Compute the difference in positions
	float dx = worldobject_actor.position.x - worldobject_blocker.position.x;
	float dy = worldobject_actor.position.y - worldobject_blocker.position.y;

	// Compute combined half widths and heights
	float combined_half_widths = half_width_actor + half_width_blocker;
	float combined_half_heights = half_height_actor + half_height_blocker;

	// Check for collision on the x and y axes
	if (std::abs(dx) < combined_half_widths && std::abs(dy) < combined_half_heights) {
		// Collision detected
		float overlap_x = combined_half_widths - std::abs(dx);
		float overlap_y = combined_half_heights - std::abs(dy);

		// Determine the direction of maximum overlap
		if (overlap_x > overlap_y) {
			// Push out along the y-axis
			if (dy > 0) {
				// Actor is below the blocker
				worldobject_actor.position.y += overlap_y;
			}
			else {
				// Actor is above the blocker
				worldobject_actor.position.y -= overlap_y;
			}
		}
		else {
			// Push out along the x-axis
			if (dx > 0) {
				// Actor is to the right of the blocker
				worldobject_actor.position.x += overlap_x;
			}
			else {
				// Actor is to the left of the blocker
				worldobject_actor.position.x -= overlap_x;
			}
		}
	}
}

void WorldSystem::handleProjectileBlocker(Entity projectile, Entity blocker) {
	// remove projectile
	registry.pendingRemoves.emplace_with_duplicates(projectile);
	return;
}

void WorldSystem::handleProjectileDeadly(Entity projectile, Entity deadly) {
	// Decrease health of deadly
	if (!registry.deadlys.get(deadly).immune) {
		registry.healthComponents.get(deadly).curr_health -= registry.projectiles.get(projectile).damage;
		createParticles(renderer, registry.worldObjects.get(deadly).position, uniform_dist, rng, TEXTURE_ASSET_ID::HIT_PARTICLE, current_room);
	}

	// Remove projectile
	registry.pendingRemoves.emplace_with_duplicates(projectile);
	return;
}

void WorldSystem::handleProjectilePlayer(Entity projectile, Entity player) {
	// Decrease health of player
	if (!registry.invincibleTimers.has(player) && !registry.deathTimers.has(player)) {
		registry.invincibleTimers.emplace(player);
		registry.healthComponents.get(player).curr_health -= registry.projectiles.get(projectile).damage;
		playPlayerDamagedEffect(player);
		createParticles(renderer, registry.worldObjects.get(player).position, uniform_dist, rng, TEXTURE_ASSET_ID::HIT_PARTICLE_PLAYER, current_room);

		updateGameUI();
		Mix_Volume(Mix_PlayChannel(-1, player_projectile_damage_sound, 0), 5);
	}

	// Remove projectile
	registry.pendingRemoves.emplace_with_duplicates(projectile);
	return;
}



// ==================== UPDATE FUNCTIONS ====================
void WorldSystem::updateGameUI() {
	// this updates health, scrap, and items
	UIElement& health_elt = registry.uiElements.get(health_ui);
	health_elt.value = std::to_string(registry.healthComponents.get(player).curr_health);

	UIElement& scrap_elt = registry.uiElements.get(scrap_ui);
	scrap_elt.value = std::to_string(registry.players.components[0].scrap);

	// re render the items
	// TODO pull this into a helper method later
	Inventory& player_inventory = registry.inventory.get(player);
	cout << player_inventory.items.size() << endl;
	for (auto item : player_inventory.items) {
		UISystem::createTexturedUIElement(
			renderer,
			vec2(window_width_px - ((item.first * ITEM_UI_OFFSET_X) + INITIAL_ITEM_UI_OFFSET_X), INITIAL_ITEM_UI_OFFSET_Y),
			vec2(75.f, 75.f),
			"item_ui_" + std::to_string(item.first),
			getItemTexture(player_inventory.items[item.first]),
			SCENE_TYPE::GAME);
	}
}

void WorldSystem::updatePlayerAnimation() {
	Animation& player_animation = registry.animations.get(player);
	Motion player_motion = registry.motions.get(player);
	if (player_motion.target_velocity.x != 0.f || player_motion.target_velocity.y != 0.f) {
		// player is moving; play walking animation (4 frames)
		player_animation.frames = player_animation.cols * player_animation.rows;
	}
	else {
		// player is still; use only 1 frame
		player_animation.frames = 1;
	}
}

void WorldSystem::updateEnemyAnimation(Entity enemy) {
	Animation& enemy_animation = registry.animations.get(enemy);
	Motion enemy_motion = registry.motions.get(enemy);
	RenderRequest& enemy_render_request = registry.renderRequests.get(enemy);
	Deadly& deadly = registry.deadlys.get(enemy);

	// reset to walking texture
	if (deadly.type == 0) 		// select robot 1
	{
		enemy_render_request.used_texture = TEXTURE_ASSET_ID::ENEMY_WALK;
		enemy_animation.cols = 4;
		enemy_animation.frames = 4;
	}
	else if (deadly.type == 1)						// select robot 2
	{
		enemy_render_request.used_texture = TEXTURE_ASSET_ID::ENEMY_2_WALK;
		enemy_animation.cols = 4;
		enemy_animation.frames = 4;
	}
	else if (deadly.type == 2) // Boss one bodygaurd
	{
		enemy_render_request.used_texture = TEXTURE_ASSET_ID::ENEMY_WALK;
		enemy_animation.cols = 4;
		enemy_animation.frames = 4;
	}
	else if (deadly.type == 3) // Boss one mother 
	{
		enemy_render_request.used_texture = TEXTURE_ASSET_ID::MOTHER_FINAL_IDLE;
		enemy_animation.cols = 22;
		enemy_animation.frames = 22;
	}
	else if (deadly.type == 4)
	{
		enemy_render_request.used_texture = TEXTURE_ASSET_ID::HEAVY_WALK;
		enemy_animation.cols = 4;
		enemy_animation.frames = 4;
	}

	if (enemy_motion.target_velocity.x != 0.f || enemy_motion.target_velocity.y != 0.f) {
		// enemy is moving; play walking animation (4 frames)
		enemy_animation.frames = enemy_animation.cols * enemy_animation.rows;
	}
	else if (deadly.type == 3) {
		enemy_animation.frames = enemy_animation.cols * enemy_animation.rows;
	}
	else {
		// enemy is still; use only 1 frame
		enemy_animation.frames = 1;
	}
}

// Call everytime the player inventory is going to be modified. So when an item is being picked up or dropped
void WorldSystem::update_player_modifier() const {
	int new_damage_flat = 0;

	float new_speed_flat = 0;
	float new_speed_percent = 0;

	float new_fire_rate_flat = 0;
	float new_fire_rate_percent = 0;

	float new_range_flat = 0;
	float new_range_percent = 0;

	float new_accuracy = 0;

	for (auto object : registry.inventory.get(player).items) {
		auto item = object.second;
		new_damage_flat += item.flat_damage_mod;

		new_speed_flat += item.flat_speed_mod;
		new_speed_percent += item.percent_speed_mod;

		new_fire_rate_flat += item.flat_fire_rate;
		new_fire_rate_percent += item.percent_fire_rate;

		new_range_flat += item.flat_range;
		new_range_percent += item.percent_range;

		new_accuracy += item.accuracy;
	}
	Modifier& player_modifier = registry.modifiers.get(player);

	player_modifier.damage_modifier_flat = new_damage_flat;

	player_modifier.speed_modifier_flat = new_speed_flat;
	player_modifier.speed_modifier_percent = new_speed_percent;

	player_modifier.range_modifier_flat = new_range_flat;
	player_modifier.range_modifier_percent = new_range_percent;

	player_modifier.fire_rate_modifier_flat = new_fire_rate_flat;
	player_modifier.fire_rate_modifier_percent = new_fire_rate_percent;

	if (new_accuracy >= 0) {
		player_modifier.accuracy_modifier = new_accuracy;
	}
	else {
		player_modifier.accuracy_modifier = 0;
	}
}