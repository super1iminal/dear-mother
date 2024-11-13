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

// Game configuration
// add variables here
bool player_seen = false;

// create the underwater world
WorldSystem::WorldSystem()
	: points(0)
	, player_health(0)
	, current_speed(1.0)
	, current_room({0, 0}) {
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


std::chrono::steady_clock::time_point WorldSystem::get_last_shot_time(Entity& entity) {
	return registry.shooters.get(entity).t;
}

std::chrono::steady_clock::time_point WorldSystem::get_curr_time() {
	using Clock = std::chrono::high_resolution_clock;
	return Clock::now();
}

void WorldSystem::init(RenderSystem* renderer_arg, GLFWwindow* window) {
	this->renderer = renderer_arg;
	this->window = window;
	//////////////////////////////////////
	// Loading music and sounds with SDL
	if (SDL_Init(SDL_INIT_AUDIO) < 0) {
		fprintf(stderr, "Failed to initialize SDL Audio");
		exit(1);
	}
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1) {
		fprintf(stderr, "Failed to open audio device");
		exit(1);
	}

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


	// Playing background music indefinitely
	if (enable_music)
		Mix_FadeInMusic(post_combat_music, -1, 5000);
	Mix_VolumeMusic(16);
	fprintf(stderr, "Loaded music\n");

	// Create Item Set
	srand(time(0));
	buildItemSet();
	// Set all states to default
	restart_game();
}


// Update our game world
bool WorldSystem::step(float elapsed_ms_since_last_update) {
	// Get Player
	Entity player = registry.players.entities[0];

	// Updating window title with points
	/*std::stringstream title_ss;
	player_health = registry.healthComponents.get(player).curr_health;
	title_ss << "Points: " << points;
	title_ss << " Health: " << player_health;
	title_ss << " FPS: " << fps;
	glfwSetWindowTitle(window, title_ss.str().c_str());*/

	// Remove debug info from the last step
	//for (Entity entity : registry.debugComponents.entities) {
	//	registry.pendingRemoves.emplace_with_duplicates(entity);
	//}
	// cleanup();
	 


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
	for (Entity entity : registry.shooters.entities) {
		if (!registry.activeComponents.has(entity))
			continue;
		shoot(entity);
	}

	// Set music based on whether or not there are enemies alive
	int activeDeadlyCounter = 0;
	for (Entity entity : registry.deadlys.entities) {
		if (registry.activeComponents.has(entity)) {
			activeDeadlyCounter++;
		}
	}
	if (activeDeadlyCounter > 0) {
		if (!registry.players.get(player).in_combat) {
			Mix_VolumeMusic(8);
			if (enable_music) {
				if (registry.players.get(player).combat_boss_one) {
					Mix_FadeInMusic(combat_music, -1, 2500);
				}
				else {
					Mix_FadeInMusic(combat_music, -1, 2500);
				}
			}
			registry.players.get(player).in_combat = true;
		}
	}
	else {
		if (registry.players.get(player).in_combat) {
			Mix_VolumeMusic(16);
			if (enable_music)
				Mix_FadeInMusic(post_combat_music, -1, 5000);
			registry.players.get(player).in_combat = false;
		}
	}

	if (registry.deadlys.components.size() <= 0) {
		registry.players.get(player).in_combat = false;
	}

	// Processing the player state
	assert(registry.screenStates.components.size() <= 1);
	ScreenState& screen = registry.screenStates.components[0];
	screen.health_status = player_health;

	float min_counter_ms = 3000.f;
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
			Mix_VolumeMusic(16);
			if (enable_music)
				Mix_FadeInMusic(post_combat_music, -1, 5000);
			registry.players.get(player).in_combat = false;
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
		}
	}

	if (registry.players.get(player).combat_boss_two) {
		handle_boss_two();
	}

	// reduce window brightness if the salmon is dying
	screen.darken_screen_factor = 1 - min_counter_ms / 3000;

	return true;
}

void WorldSystem::createEnemyRoom(ivec2 coord) {
	// create a floor entity
	createFloor(renderer, { window_width_px / 2, (window_height_px + 120.f) / 2 }, { window_width_px , window_height_px - 120.f }, coord);

	//createInteractable(renderer, { window_width_px / 2, window_height_px - 200 }, { 75.f, 75.f }, bound_interactable_fn, 1, { 0, 0 });

	// top wall
	createWall(renderer, { window_width_px / 2, 25.f + 120.f }, { window_width_px, WALL_WIDTH }, 0.f, TEXTURE_ASSET_ID::HORZ_WALL, coord);
	// bottom wall
	createWall(renderer, { window_width_px / 2, window_height_px - 25.f }, { window_width_px, WALL_WIDTH }, M_PI, TEXTURE_ASSET_ID::HORZ_WALL, coord);
	// left wall
	createWall(renderer, { 25.f, (window_height_px / 2) + 60.f }, { WALL_WIDTH, window_height_px - 120.f }, 0.f, TEXTURE_ASSET_ID::VERT_WALL, coord);
	// right wall
	createWall(renderer, { window_width_px - 25.f, (window_height_px / 2) + 60.f }, { WALL_WIDTH, window_height_px - 120.f }, M_PI, TEXTURE_ASSET_ID::VERT_WALL, coord);

	if (!(coord.x == 0 && coord.y == 0)) { // no enemies in base room
		float scalingFactor = sqrt(coord.x * coord.x + coord.y + coord.y); // gets harder as you move further from spawn
		int numEnemies = ((int)rand() % 2) + 1;
		if (scalingFactor > 4)
			numEnemies += 1;
		else if (scalingFactor > 2)
			numEnemies += 2;

		for (int i = 0; i < numEnemies; i++) { // create a variable number of enemies of random type
			createEnemy(renderer, vec2((uniform_dist(rng) * (window_width_px - (2 * WALL_WIDTH))) + WALL_WIDTH, ((uniform_dist(rng) * (window_height_px - (2 * WALL_WIDTH) - BASE_UI_HEIGHT))) + WALL_WIDTH + BASE_UI_HEIGHT), ENEMY_SPEED, coord);
		}
	}

	// create numFloorItems floor items (functionally a wall)
	// need to be cautious of spawn location, not near doors (euclidean distance) or player/enemies (overlap) or on top of each other (overlap)
	int numFloorItems = (int)rand() % 3;
	
	float minX = WALL_WIDTH + FLOOR_ITEM_SIZE / 2;
	float minY = WALL_WIDTH + BASE_UI_HEIGHT + FLOOR_ITEM_SIZE / 2;

	float xRange = window_width_px - (WALL_WIDTH * 2) - FLOOR_ITEM_SIZE;
	float yRange = window_height_px - (WALL_WIDTH * 2) - BASE_UI_HEIGHT - FLOOR_ITEM_SIZE;

	float xPos;
	float yPos;
	while (numFloorItems > 0) {
		do {
			xPos = minX + rand() % (int) xRange;
			yPos = minY + rand() % (int) yRange;
		} while (notSafe({ xPos, yPos }));

		createWall(renderer, { xPos, yPos }, { FLOOR_ITEM_SIZE, FLOOR_ITEM_SIZE }, 0.f, randomFloorItem(), coord);
		numFloorItems--;
	}
	
	if (roomMap.find({ coord.x + 1, coord.y }) != roomMap.end()) {
		createDoor(renderer, coord, { coord.x + 1, coord.y }, DIRECTION::RIGHT);
	}
	if (roomMap.find({ coord.x - 1, coord.y }) != roomMap.end()) {
		createDoor(renderer, coord, { coord.x - 1, coord.y }, DIRECTION::LEFT);
	}
	if (roomMap.find({ coord.x, coord.y + 1 }) != roomMap.end()) {
		createDoor(renderer, coord, { coord.x, coord.y + 1 }, DIRECTION::UP);
	}
	if (roomMap.find({ coord.x, coord.y - 1 }) != roomMap.end()) {
		createDoor(renderer, coord, { coord.x, coord.y - 1 }, DIRECTION::DOWN);
	}
}

void WorldSystem::createBossRoomOne(ivec2 coord) {
	// create a floor entity
	createFloor(renderer, { window_width_px / 2, (window_height_px + 120.f) / 2 }, { window_width_px , window_height_px - 120.f }, coord);

	// top wall
	createWall(renderer, { window_width_px / 2, 25.f + 120.f }, { window_width_px, WALL_WIDTH }, 0.f, TEXTURE_ASSET_ID::HORZ_WALL, coord);
	// bottom wall
	createWall(renderer, { window_width_px / 2, window_height_px - 25.f }, { window_width_px, WALL_WIDTH }, M_PI, TEXTURE_ASSET_ID::HORZ_WALL, coord);
	// left wall
	createWall(renderer, { 25.f, (window_height_px / 2) + 60.f }, { WALL_WIDTH, window_height_px - 120.f }, 0.f, TEXTURE_ASSET_ID::VERT_WALL, coord);
	// right wall
	createWall(renderer, { window_width_px - 25.f, (window_height_px / 2) + 60.f }, { WALL_WIDTH, window_height_px - 120.f }, M_PI, TEXTURE_ASSET_ID::VERT_WALL, coord);

	float minX = WALL_WIDTH + FLOOR_ITEM_SIZE / 2;
	float minY = WALL_WIDTH + BASE_UI_HEIGHT + FLOOR_ITEM_SIZE / 2;

	float xRange = window_width_px - (WALL_WIDTH * 2) - FLOOR_ITEM_SIZE;
	float yRange = window_height_px - (WALL_WIDTH * 2) - BASE_UI_HEIGHT - FLOOR_ITEM_SIZE;

	if (roomMap.find({ coord.x + 1, coord.y }) != roomMap.end()) {
		createDoor(renderer, coord, { coord.x + 1, coord.y }, DIRECTION::RIGHT);
	}
	if (roomMap.find({ coord.x - 1, coord.y }) != roomMap.end()) {
		createDoor(renderer, coord, { coord.x - 1, coord.y }, DIRECTION::LEFT);
	}
	if (roomMap.find({ coord.x, coord.y + 1 }) != roomMap.end()) {
		createDoor(renderer, coord, { coord.x, coord.y + 1 }, DIRECTION::UP);
	}
	if (roomMap.find({ coord.x, coord.y - 1 }) != roomMap.end()) {
		createDoor(renderer, coord, { coord.x, coord.y - 1 }, DIRECTION::DOWN);
	}

	createBossOne(renderer, { WALL_WIDTH + 500, WALL_WIDTH + BASE_UI_HEIGHT + 75 +40}, BOSS_ONE_POS::TOP_LEFT, coord);
	createBossOne(renderer, { WALL_WIDTH + 700, WALL_WIDTH + BASE_UI_HEIGHT + 75 +40}, BOSS_ONE_POS::TOP_RIGHT, coord);

	createBossOne(renderer, { WALL_WIDTH + 500, WALL_WIDTH + BASE_UI_HEIGHT + 375 +30}, BOSS_ONE_POS::BOT_LEFT, coord);
	createBossOne(renderer, { WALL_WIDTH + 700, WALL_WIDTH + BASE_UI_HEIGHT + 375 +30}, BOSS_ONE_POS::BOT_RIGHT, coord);

	createBossOne(renderer, { CENTER_X, 234.5 }, BOSS_ONE_POS::MOTHER, coord);
}

void WorldSystem::createBossRoomTwo(ivec2 coord) {
	// create a floor entity
	createFloor(renderer, { window_width_px / 2, (window_height_px + 120.f) / 2 }, { window_width_px , window_height_px - 120.f }, coord);

	// Boss Two
	boss_two =  createBossTwo(renderer, { window_width_px / 2, 25.f + 120.f }, coord);
	// bottom wall
	createWall(renderer, { window_width_px / 2, window_height_px - 25.f }, { window_width_px, WALL_WIDTH }, M_PI, TEXTURE_ASSET_ID::HORZ_WALL, coord);
	// left wall
	createWall(renderer, { 25.f, (window_height_px / 2) + 60.f }, { WALL_WIDTH, window_height_px - 120.f }, 0.f, TEXTURE_ASSET_ID::VERT_WALL, coord);
	// right wall
	createWall(renderer, { window_width_px - 25.f, (window_height_px / 2) + 60.f }, { WALL_WIDTH, window_height_px - 120.f }, M_PI, TEXTURE_ASSET_ID::VERT_WALL, coord);

	float minX = WALL_WIDTH + FLOOR_ITEM_SIZE / 2;
	float minY = WALL_WIDTH + BASE_UI_HEIGHT + FLOOR_ITEM_SIZE / 2;

	float xRange = window_width_px - (WALL_WIDTH * 2) - FLOOR_ITEM_SIZE;
	float yRange = window_height_px - (WALL_WIDTH * 2) - BASE_UI_HEIGHT - FLOOR_ITEM_SIZE;

	if (roomMap.find({ coord.x + 1, coord.y }) != roomMap.end()) {
		createDoor(renderer, coord, { coord.x + 1, coord.y }, DIRECTION::RIGHT);
	}
	if (roomMap.find({ coord.x - 1, coord.y }) != roomMap.end()) {
		createDoor(renderer, coord, { coord.x - 1, coord.y }, DIRECTION::LEFT);
	}
	if (roomMap.find({ coord.x, coord.y + 1 }) != roomMap.end()) {
		createDoor(renderer, coord, { coord.x, coord.y + 1 }, DIRECTION::UP);
	}
	if (roomMap.find({ coord.x, coord.y - 1 }) != roomMap.end()) {
		createDoor(renderer, coord, { coord.x, coord.y - 1 }, DIRECTION::DOWN);
	}

}

bool WorldSystem::notSafe(vec2 pos) {
	// check proximity to doors
	float leftRightDoorY = ((window_height_px - WALL_WIDTH - BASE_UI_HEIGHT) + WALL_WIDTH + BASE_UI_HEIGHT) / 2.f;
	float upDoorY = WALL_WIDTH + BASE_UI_HEIGHT;
	float leftDoorX = WALL_WIDTH;
	float rightDoorX = window_width_px - WALL_WIDTH;
	float upDownDoorX = window_height_px / 2.f;
	float downDoorY = window_height_px - WALL_WIDTH;
	if (sqrt(((pos.x - leftDoorX) * (pos.x - leftDoorX)) + ((pos.y - leftRightDoorY) * (pos.y - leftRightDoorY))) < 400) // left door
		return true;
	if (sqrt(((pos.x - rightDoorX) * (pos.x - rightDoorX)) + ((pos.y - leftRightDoorY) * (pos.y - leftRightDoorY))) < 400) // right door
		return true;
	if (sqrt(((pos.x - upDownDoorX) * (pos.x - upDownDoorX)) + ((pos.y - upDoorY) * (pos.y - upDoorY))) < 400) // top door
		return true;
	if (sqrt(((pos.x - upDownDoorX) * (pos.x - upDownDoorX)) + ((pos.y - downDoorY) * (pos.y - downDoorY))) < 400) // bottom door
		return true;

	return false;
}

void WorldSystem::createEmptyRoom(ivec2 coord) {
	// create a floor entity
	createFloor(renderer, { window_width_px / 2, (window_height_px + 120.f) / 2 }, { window_width_px , window_height_px - 120.f }, coord);

	// top wall
	createWall(renderer, { window_width_px / 2, 25.f + 120.f }, { window_width_px, WALL_WIDTH }, 0.f, TEXTURE_ASSET_ID::HORZ_WALL, coord);
	// bottom wall
	createWall(renderer, { window_width_px / 2, window_height_px - 25.f }, { window_width_px, WALL_WIDTH }, M_PI, TEXTURE_ASSET_ID::HORZ_WALL, coord);
	// left wall
	createWall(renderer, { 25.f, (window_height_px / 2) + 60.f }, { WALL_WIDTH, window_height_px - 120.f }, 0.f, TEXTURE_ASSET_ID::VERT_WALL, coord);
	// right wall
	createWall(renderer, { window_width_px - 25.f, (window_height_px / 2) + 60.f }, { WALL_WIDTH, window_height_px - 120.f }, M_PI, TEXTURE_ASSET_ID::VERT_WALL, coord);


	if (roomMap.find({ coord.x + 1, coord.y }) != roomMap.end()) {
		createDoor(renderer, coord, { coord.x + 1, coord.y }, DIRECTION::RIGHT);
	}
	if (roomMap.find({ coord.x - 1, coord.y }) != roomMap.end()) {
		createDoor(renderer, coord, { coord.x - 1, coord.y }, DIRECTION::LEFT);
	}
	if (roomMap.find({ coord.x, coord.y + 1 }) != roomMap.end()) {
		createDoor(renderer, coord, { coord.x, coord.y + 1 }, DIRECTION::UP);
	}
	if (roomMap.find({ coord.x, coord.y - 1 }) != roomMap.end()) {
		createDoor(renderer, coord, { coord.x, coord.y - 1 }, DIRECTION::DOWN);
	}
}

void WorldSystem::generate_map() {
	roomMap[{0, 0}] = ROOM_TYPE::ENEMY_ROOM;
	roomMap[{1, 0}] = ROOM_TYPE::EMPTY;
	//roomMap[{1, 0}] = ROOM_TYPE::ENEMY_ROOM;
	//roomMap[{0, 1}] = ROOM_TYPE::ENEMY_ROOM;
	roomMap[{0, 1}] = ROOM_TYPE::EMPTY;
	//roomMap[{1, 1}] = ROOM_TYPE::ENEMY_ROOM;
	roomMap[{1, 2}] = ROOM_TYPE::EMPTY;
	//roomMap[{0, 2}] = ROOM_TYPE::ENEMY_ROOM;
	roomMap[{0, 2}] = ROOM_TYPE::EMPTY;
	//roomMap[{0, 3}] = ROOM_TYPE::ENEMY_ROOM;
	roomMap[{0, -1}] = ROOM_TYPE::ENEMY_ROOM;
	roomMap[{1, -1}] = ROOM_TYPE::ENEMY_ROOM;
	roomMap[{2, 0}] = ROOM_TYPE::BOSS_ROOM_ONE;
	roomMap[{2, 2}] = ROOM_TYPE::BOSS_ROOM_TWO;

	roomMap[{-1, -1}] = ROOM_TYPE::ENEMY_ROOM;
	roomMap[{-1, -2}] = ROOM_TYPE::ENEMY_ROOM;
	roomMap[{-1, 3}] = ROOM_TYPE::ENEMY_ROOM;
	roomMap[{-2, 3}] = ROOM_TYPE::ENEMY_ROOM;
	roomMap[{-2, 2}] = ROOM_TYPE::ENEMY_ROOM;
	roomMap[{-2, 4}] = ROOM_TYPE::ENEMY_ROOM;
}

// 
void WorldSystem::generate_rooms() {
	generate_map();
	// Iterating using structured bindings
	for (const auto& room : roomMap) {
		const ivec2 coord = { room.first.first, room.first.second };
		ROOM_TYPE type = room.second;

		switch (type) {
		case ROOM_TYPE::ENEMY_ROOM:
			createEnemyRoom(coord);
			break;
		case ROOM_TYPE::EMPTY:
			createEmptyRoom(coord);
			break;
		case ROOM_TYPE::BOSS_ROOM_ONE:
			createBossRoomOne(coord);
			break;
		case ROOM_TYPE::BOSS_ROOM_TWO:
			createBossRoomTwo(coord);
			break;
		}
	}

	// need to only have the current entities as active. 
	// some entities, such as UI elements and the player, do not have room coords, and must always be rendered
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

void WorldSystem::change_rooms(ivec2 new_room) {
	current_room = new_room;
	if (roomMap[{new_room.x, new_room.y}] == ROOM_TYPE::BOSS_ROOM_ONE
		&& !registry.players.get(player).boss_one_beat) {
		registry.players.get(registry.players.entities[0]).combat_boss_one = true;
	}
	else if (roomMap[{new_room.x, new_room.y}] == ROOM_TYPE::BOSS_ROOM_TWO 
		&& !registry.players.get(player).boss_two_beat) {
		registry.players.get(registry.players.entities[0]).combat_boss_two = true;
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

// Reset the world state to its initial state
void WorldSystem::restart_game() {
	// Debugging for memory/component leaks
	registry.list_all_components();
	printf("Restarting\n");

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
	player = createPlayer(renderer, { window_width_px / 2, window_height_px - 200 });

	// function to use for interactable
	auto bound_interactable_fn = std::bind(&WorldSystem::increaseScrap, this, std::placeholders::_1);

	generate_rooms();

	// Set initial cooldown time
	for (Entity entity : registry.shooters.entities) {
		set_last_shot_time(entity);
	}
	
	initGameUI();
}

void WorldSystem::increaseScrap(int amt) 
{
	scrap += amt;
	updateGameUI();
	std::cout <<  "scrap: " << scrap << std::endl;
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
	float player_health = registry.healthComponents.get(registry.players.entities[0]).curr_health;

	// create health_ui entity
	// TODO see if we can fix how text is rendered
	// so that the position and scale vectors aren't so funky
	health_ui = UISystem::createUIElement(
		renderer,
		vec2(60.f, 324.f),
		vec2(2.f, 40.f),
		"health_ui",
		static_cast<int>(player_health),
		SCENE_TYPE::GAME);

	// create scrap_ui entity
	scrap_ui = UISystem::createUIElement(
		renderer,
		vec2(180.f, 336.f),
		vec2(2.f, 0.f),
		"scrap_ui",
		scrap,
		SCENE_TYPE::GAME);

	// create level_ui entity
	level_ui = UISystem::createUIElement(
		renderer,
		vec2(184.f, 316.f),
		vec2(2.f, 0.f),
		"level_ui",
		level,
		SCENE_TYPE::GAME);

	// create item_ui entities
	// later, we will want to render all the items and show locked slots too
	Inventory& player_inventory = registry.inventory.get(player);
	for (uint i = 0; i < player_inventory.items.size(); i++) {
		UISystem::createTexturedUIElement(
			renderer,
			vec2(window_width_px - ((i * ITEM_UI_OFFSET_X) + INITIAL_ITEM_UI_OFFSET_X), INITIAL_ITEM_UI_OFFSET_Y),
			vec2(75.f, 75.f),
			"item_ui_" + std::to_string(i),
			getItemTexture(player_inventory.items[i]),
			SCENE_TYPE::GAME);
	}

	UISystem::createCrosshair(renderer, TEXTURE_ASSET_ID::GAME_CROSSHAIR, SCENE_TYPE::GAME);
}

void WorldSystem::updateGameUI() {
	// this updates health, scrap, and items
	UIElement& health_elt = registry.uiElements.get(health_ui);
	health_elt.value = registry.healthComponents.get(player).curr_health;

	UIElement& scrap_elt = registry.uiElements.get(scrap_ui);
	scrap_elt.value = static_cast<float>(scrap);

	// re render the items
	// TODO pull this into a helper method later
	Inventory& player_inventory = registry.inventory.get(player);
	for (uint i = 0; i < player_inventory.items.size(); i++) {
		UISystem::createTexturedUIElement(
			renderer,
			vec2(window_width_px - ((i * ITEM_UI_OFFSET_X) + INITIAL_ITEM_UI_OFFSET_X), INITIAL_ITEM_UI_OFFSET_Y),
			vec2(75.f, 75.f),
			"item_ui_" + std::to_string(i),
			getItemTexture(player_inventory.items[i]),
			SCENE_TYPE::GAME);
	}
}

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
		enemy_render_request.used_texture = TEXTURE_ASSET_ID::ENEMY_WALK;
	else if (deadly.type == 1)						// select robot 2
		enemy_render_request.used_texture = TEXTURE_ASSET_ID::ENEMY_2_WALK;
	else if (deadly.type == 2) // Boss one bodygaurd
		enemy_render_request.used_texture = TEXTURE_ASSET_ID::ENEMY_WALK;
	else if (deadly.type == 3) // Boss one mother
		enemy_render_request.used_texture = TEXTURE_ASSET_ID::ENEMY_2_WALK;

	enemy_animation.cols = 4;
	enemy_animation.frames = 4;

	if (enemy_motion.target_velocity.x != 0.f || enemy_motion.target_velocity.y != 0.f) {
		// enemy is moving; play walking animation (4 frames)
		enemy_animation.frames = enemy_animation.cols * enemy_animation.rows;
	}
	else {
		// enemy is still; use only 1 frame
		enemy_animation.frames = 1;
	}
}

void WorldSystem::playEnemyAttack(Entity enemy) {

	Animation& enemy_animation = registry.animations.get(enemy);
	Deadly& deadly = registry.deadlys.get(enemy);
	Motion enemy_motion = registry.motions.get(enemy);
	RenderRequest& enemy_render_request = registry.renderRequests.get(enemy);

	if (deadly.type == 0) {
		enemy_render_request.used_texture = TEXTURE_ASSET_ID::ENEMY_ATTACK;
	}
	else if (deadly.type == 1) {
		enemy_render_request.used_texture = TEXTURE_ASSET_ID::ENEMY_2_ATTACK;
	}
	else if (deadly.type == 2) { // Boss one bodygaurds
		enemy_render_request.used_texture = TEXTURE_ASSET_ID::ENEMY_ATTACK;
	}
	else if (deadly.type == 3) { // Boss one mother
		enemy_render_request.used_texture = TEXTURE_ASSET_ID::ENEMY_2_ATTACK;
	}
	enemy_animation.cols = 7;
	enemy_animation.frames = 7;

	deadly.attacking = false;
}

// Compute collisions between entities, called after physics_system::step which checks for collisions
void WorldSystem::handle_collisions() {
	// god damn. 
	for (Entity entity : registry.collisions.entities) {
		std::vector<Collision*> collisions = registry.collisions.get_all(entity);

		for (int j = 0; j < collisions.size(); j++) {
			Collision collision = *collisions[j];
			Entity entity_other = collision.other;
			COLLISION_TYPE type = collision.type;
			if (!registry.activeComponents.has(entity) || !registry.activeComponents.has(entity_other))
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
	player_seen = false;
	registry.collisions.clear();
}

//void WorldSystem::handlePlayerDeadly(Entity entity, Entity entity_other) {
//	if (!registry.deathTimers.has(entity)) {
//		// Scream, reset timer, and make the salmon sink
//		registry.deathTimers.emplace(entity);
//		Mix_PlayChannel(-1, salmon_dead_sound, 0);
//	}
//	return;
//}

void WorldSystem::handlePlayerDoor(Entity player, Entity door) {
	// Lock door during combat
	if (!registry.players.get(player).in_combat && !registry.players.get(player).combat_boss_one
		&& !registry.players.get(player).combat_boss_two) {
		// change rooms
		printf("room switching\n");
		ivec2 new_room = registry.doors.get(door).leads_to;
		WorldObject& player_worldobject = registry.worldObjects.get(player);

		if (player_seen == false) {
			if (registry.worldObjects.get(player).position.x < (WALL_WIDTH + PLAYER_SIZE * 1.5))
				player_worldobject.position = { window_width_px - (WALL_WIDTH + 80), (window_height_px - BASE_UI_HEIGHT) / 2.f + BASE_UI_HEIGHT };
			else if (registry.worldObjects.get(player).position.x > (window_width_px - (WALL_WIDTH + PLAYER_SIZE * 1.5)))
				player_worldobject.position = { WALL_WIDTH + 80, (window_height_px - BASE_UI_HEIGHT) / 2.f + BASE_UI_HEIGHT };
			else if (registry.worldObjects.get(player).position.y > window_height_px - (WALL_WIDTH + PLAYER_SIZE * 1.5))
				player_worldobject.position = { window_width_px / 2, BASE_UI_HEIGHT + (WALL_WIDTH + 80) };
			else
				player_worldobject.position = { window_width_px / 2, window_height_px - (WALL_WIDTH + 80) };

			change_rooms(new_room);
		}
		player_seen = true;
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
				registry.healthComponents.get(entity).curr_health -= registry.deadlys.get(deadly).melee_damge;
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
		createParticles(renderer, registry.worldObjects.get(player).position, uniform_dist, rng, TEXTURE_ASSET_ID::HIT_PARTICLE_PLAYER, current_room);

		updateGameUI();
		Mix_Volume(Mix_PlayChannel(-1, player_projectile_damage_sound, 0), 5);
	}


	// Remove projectile
	registry.pendingRemoves.emplace_with_duplicates(projectile);
	return;
}

TEXTURE_ASSET_ID WorldSystem::randomFloorItem()
{
	int seed = rand() % 7;
	switch (seed) {
	case 0:
		return TEXTURE_ASSET_ID::FURNACE;
	case 1: 
		return TEXTURE_ASSET_ID::BROKEN_GENERATOR;
	case 2:
		return TEXTURE_ASSET_ID::DEAD_ROBOT;
	case 3:
		return TEXTURE_ASSET_ID::BROKEN_CONTROL_PANEL;
	case 4:
		return TEXTURE_ASSET_ID::FLOOR_HOLE;
	case 5:
		return TEXTURE_ASSET_ID::RUSTY_PIPES;
	default:
		return TEXTURE_ASSET_ID::SLAG_PIT;
	}
}

void WorldSystem::handle_item_pickup(Entity item) {
	// Pick up item and apply effects to the player
	Inventory& player_inventory = registry.inventory.get(player);
	ItemStat new_item = registry.itemStats.get(item);
	if ((player_inventory.items.size() < 8) && (new_item.type != ITEM_TYPE::HEALTH_PACK)) {
		player_inventory.items.push_back(new_item);
		update_player_modifier();
		registry.pendingRemoves.emplace_with_duplicates(item);
	}
	else if (new_item.type == ITEM_TYPE::HEALTH_PACK) {
		// Apply health pack item
		Health& player_health = registry.healthComponents.get(player);
		if (player_health.curr_health + new_item.heal_size <= player_health.max_health) {
			player_health.curr_health = player_health.curr_health + new_item.heal_size;
			registry.pendingRemoves.emplace_with_duplicates(item);
		}
		else if (player_health.curr_health < player_health.max_health) {
			player_health.curr_health = player_health.max_health;
			registry.pendingRemoves.emplace_with_duplicates(item);
		}
	}
	else {
		std::cout << "Already have 8 items" << std::endl;
	}

	updateGameUI();
}

void WorldSystem::handle_interactions() {
	auto& interactablesRegistry = registry.interactables;
	for (Entity interactableEntity : interactablesRegistry.entities) {
		Interactable& interactable = interactablesRegistry.get(interactableEntity);

		float range = interactable.range;

		WorldObject& playerWorldObject = registry.worldObjects.get(player);
		WorldObject& interactableObject = registry.worldObjects.get(interactableEntity);

		float dist = distance(playerWorldObject.position, interactableObject.position);
		if (dist < range) {
			if (registry.itemStats.has(interactableEntity)) {
				handle_item_pickup(interactableEntity);
			}
			else {
				interactable.interaction(interactable.value);
			}
		}
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

	for (ItemStat& item : registry.inventory.get(player).items) {
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

void WorldSystem::handle_boss_two() {
	int alive = 0;

	BossTwo& bossTwo = registry.bossTwos.get(boss_two);
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
		registry.players.get(player).combat_boss_two = false;
		registry.players.get(player).boss_two_beat = true;
		createItem(renderer, vec2(CENTER_X - 100, CENTER_Y), vec2(75, 75), ITEM_TYPE::HEALTH_PACK, uniform_dist, rng, current_room);
		createItem(renderer, vec2(CENTER_X + 100, CENTER_Y), vec2(75, 75), ITEM_TYPE::RANDOM, uniform_dist, rng, current_room);
	}
}

Entity WorldSystem::create_self_destruct_text(RenderSystem* renderer, ivec2 current_room) {
	return createFloorText(renderer, "SELF DESTRUCT ACTIVE", { window_width_px / 2 - 250, window_height_px / 2 - 100 }, { 2, 2 }, current_room);
}

void WorldSystem::handle_boss_one_death(Entity& entity) {
	BossOne& bose_part = registry.bossOnes.get(entity);
	if (bose_part.boss_pos == BOSS_ONE_POS::TOP_LEFT) {
		for (BossOne& b : registry.bossOnes.components) {
			b.top_left_alive = false;
		}
	}
	else if (bose_part.boss_pos == BOSS_ONE_POS::TOP_RIGHT) {
		for (BossOne& b : registry.bossOnes.components) {
			b.top_right_alive = false;
		}
	}
	else if (bose_part.boss_pos == BOSS_ONE_POS::BOT_LEFT) {
		for (BossOne& b : registry.bossOnes.components) {
			b.bot_left_alive = false;
		}
	}
	else if (bose_part.boss_pos == BOSS_ONE_POS::BOT_RIGHT) {
		for (BossOne& b : registry.bossOnes.components) {
			b.bot_right_alive = false;
		}
	}
	else if (bose_part.boss_pos == BOSS_ONE_POS::MOTHER) {
		for (BossOne& b : registry.bossOnes.components) {
			b.mother = false;
		}
	}
	if (!bose_part.top_left_alive && !bose_part.top_right_alive
		&& !bose_part.bot_left_alive && bose_part.bot_right_alive && !text_shown) {
		final_phase_text = create_self_destruct_text(renderer, current_room);
		text_shown = true;
	}

	if (!bose_part.top_left_alive && !bose_part.top_right_alive
		&& bose_part.bot_left_alive && !bose_part.bot_right_alive && !text_shown) {
		final_phase_text = create_self_destruct_text(renderer, current_room);
		text_shown = true;
	}

	if (!bose_part.top_left_alive && bose_part.top_right_alive
		&& !bose_part.bot_left_alive && !bose_part.bot_right_alive && !text_shown) {
		final_phase_text = create_self_destruct_text(renderer, current_room);
		text_shown = true;
	}

	if (bose_part.top_left_alive && !bose_part.top_right_alive
		&& !bose_part.bot_left_alive && !bose_part.bot_right_alive && !text_shown) {
		final_phase_text = create_self_destruct_text(renderer, current_room);
		text_shown = true;
	}

	if (!bose_part.top_left_alive && !bose_part.top_right_alive
		&& !bose_part.bot_left_alive && !bose_part.bot_right_alive
		&& bose_part.mother) {
		registry.pendingRemoves.emplace_with_duplicates(final_phase_text);
	}
	else if (!bose_part.top_left_alive && !bose_part.top_right_alive
		&& !bose_part.bot_left_alive && !bose_part.bot_right_alive
		&& !bose_part.mother) {
		// DROP ITEMS HERE FOR KILLING BOSS
		createItem(renderer, vec2(CENTER_X - 100, CENTER_Y), vec2(75, 75), ITEM_TYPE::HEALTH_PACK, uniform_dist, rng, current_room);
		createItem(renderer, vec2(CENTER_X + 100, CENTER_Y), vec2(75, 75), ITEM_TYPE::RANDOM, uniform_dist, rng, current_room);
		registry.players.get(player).combat_boss_one = false;
		registry.players.get(player).boss_one_beat = true;
		std::cout << "YAYYYY :3" << std::endl;
	}
}

void WorldSystem::handle_deaths() {
	for (Entity entity : registry.healthComponents.entities)
	{
		Health& health = registry.healthComponents.get(entity);
		if (health.curr_health <= 0)
		{
			// Scream, reset timer, and make the salmon sink
			if (registry.players.has(entity)) {
				updateGameUI();
				if (!registry.deathTimers.has(entity)) {
					registry.deathTimers.emplace(entity);
					//Mix_PlayChannel(-1, salmon_dead_sound, 0);
				}
			}
			else {
				if (registry.deadlys.has(entity) && !registry.bossOnes.has(entity) && !registry.players.get(player).combat_boss_two) {
					// TODO: drop item on death
					if (uniform_dist(rng) * 100 > (100 - DROP_CHANCE)) {
						createItem(renderer, registry.worldObjects.get(entity).position, vec2(75, 75), ITEM_TYPE::RANDOM, uniform_dist, rng, current_room);
					}
				}
				else if (registry.deadlys.has(entity) && registry.bossOnes.has(entity)) {
					handle_boss_one_death(entity);
				}
				registry.pendingRemoves.emplace_with_duplicates(entity);
			}
		}
	}
}

void WorldSystem::set_last_shot_time(Entity& entity) {
	auto& shooter = registry.shooters.get(entity);
	using Clock = std::chrono::high_resolution_clock;
	shooter.t = Clock::now();
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
		if (boss.shot_pattern) {
			boss.shot_pattern = !boss.shot_pattern;
			createProjectile(renderer, { 100, entity_object.position.y }, radians(90.f), 350.0f, false, current_room);
			createProjectile(renderer, { 225, entity_object.position.y }, radians(90.f), 350.0f, false, current_room);
			createProjectile(renderer, { 350, entity_object.position.y }, radians(90.f), 350.0f, false, current_room);

			createProjectile(renderer, { 475, entity_object.position.y }, radians(90.f), 350.0f, false, current_room);
			createProjectile(renderer, { 600, entity_object.position.y }, radians(90.f), 350.0f, false, current_room);
			createProjectile(renderer, { 725, entity_object.position.y }, radians(90.f), 350.0f, false, current_room);

			createProjectile(renderer, { 850, entity_object.position.y }, radians(90.f), 350.0f, false, current_room);
			createProjectile(renderer, { 975, entity_object.position.y }, radians(90.f), 350.0f, false, current_room);
			createProjectile(renderer, { 1100, entity_object.position.y }, radians(90.f), 350.0f, false, current_room);
		}
		else {
			boss.shot_pattern = !boss.shot_pattern;
			createProjectile(renderer, { 150, entity_object.position.y }, radians(90.f), 350.0f, false, current_room);
			createProjectile(renderer, { 275, entity_object.position.y }, radians(90.f), 350.0f, false, current_room);
			createProjectile(renderer, { 400, entity_object.position.y }, radians(90.f), 350.0f, false, current_room);

			createProjectile(renderer, { 525, entity_object.position.y }, radians(90.f), 350.0f, false, current_room);
			createProjectile(renderer, { 650, entity_object.position.y }, radians(90.f), 350.0f, false, current_room);
			createProjectile(renderer, { 775, entity_object.position.y }, radians(90.f), 350.0f, false, current_room);

			createProjectile(renderer, { 900, entity_object.position.y }, radians(90.f), 350.0f, false, current_room);
			createProjectile(renderer, { 1025, entity_object.position.y }, radians(90.f), 350.0f, false, current_room);
			createProjectile(renderer, { 1150, entity_object.position.y }, radians(90.f), 350.0f, false, current_room);
		}
	}
	set_last_shot_time(entity);
}

void WorldSystem::shoot(Entity& entity) {
	auto now = get_curr_time();
	float elapsed_ms = (float)(std::chrono::duration_cast<std::chrono::microseconds>(now - get_last_shot_time(entity))).count() / 1000;
	Motion& entity_motion = registry.motions.get(entity);
	WorldObject& entity_object = registry.worldObjects.get(entity);

	if (registry.players.has(entity)) {
		if (left_mouse_button) {
			// Apply fire rate modifier 
			float old_fire_rate = registry.shooters.get(entity).fire_rate;
			Modifier player_modifier = registry.modifiers.get(entity);
			float new_fire_rate = old_fire_rate - player_modifier.fire_rate_modifier_flat - (old_fire_rate * player_modifier.fire_rate_modifier_percent);
			if ((elapsed_ms >= new_fire_rate) || first_shot) {
				double xpos, ypos;
				glfwGetCursorPos(window, &xpos, &ypos);
				float angle = atan2(ypos - entity_object.position.y, xpos - entity_object.position.x);

				// Apply modifiers to player bullets
				Modifier projectile_mod = registry.modifiers.get(player);
				angle += (2 * (uniform_dist(rng) - 0.5)) * projectile_mod.accuracy_modifier;
				Entity projectile = createProjectile(renderer, entity_object.position, angle, 350.0f, true, current_room);
				float bullet_range = registry.lifetimes.get(projectile).time_remaining_ms;
				registry.lifetimes.get(projectile).time_remaining_ms += projectile_mod.range_modifier_flat + (bullet_range * projectile_mod.range_modifier_percent);
				registry.projectiles.get(projectile).damage += projectile_mod.damage_modifier_flat;

				Mix_Volume(Mix_PlayChannel(-1, player_shooting_sound, 0), 5);
				
				first_shot = false;
				set_last_shot_time(entity);
			}
		}
	} 
	else if (registry.deadlys.has(entity)) {
		if ((elapsed_ms >= registry.shooters.get(entity).fire_rate)) {
			if (registry.players.get(player).combat_boss_one) {
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
		Mix_VolumeMusic(16);
		if (enable_music)
			Mix_FadeInMusic(post_combat_music, -1, 5000);
		registry.players.get(player).in_combat = false;
	}

	// Interaction
	if (action == GLFW_PRESS && key == GLFW_KEY_E) {
		handle_interactions();
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
}

void WorldSystem::on_mouse_move(vec2 mouse_position) {
	// nothing yet
}

