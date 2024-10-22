// Header
#include "world_system.hpp"
#include "world_init.hpp"

// stlib
#include <cassert>
#include <sstream>
#include <chrono>
#include <iostream>
#include <ui_system.hpp>

// Game configuration
// add variables here

// create the underwater world
WorldSystem::WorldSystem()
	: points(0)
	, player_health(0)
	, current_speed(1.0) {
	// Seeding rng with random device
	rng = std::default_random_engine(std::random_device()());
}

WorldSystem::~WorldSystem() {

	// destroy music components
	if (background_music != nullptr)
		Mix_FreeMusic(background_music);
	if (salmon_dead_sound != nullptr)
		Mix_FreeChunk(salmon_dead_sound);
	if (salmon_eat_sound != nullptr)
		Mix_FreeChunk(salmon_eat_sound);

	Mix_CloseAudio();
}

void WorldSystem::set_last_shot_time(Entity& entity) {
	using Clock = std::chrono::high_resolution_clock;
	registry.shooters.get(entity).t = Clock::now();
}

std::chrono::steady_clock::time_point WorldSystem::get_last_shot_time(Entity& entity) {
	return registry.shooters.get(entity).t;
}

std::chrono::steady_clock::time_point WorldSystem::get_curr_time() {
	using Clock = std::chrono::high_resolution_clock;
	return Clock::now();
}

void WorldSystem::init(RenderSystem* renderer_arg, GLFWwindow* window, UISystem* ui) {
	this->renderer = renderer_arg;
	this->window = window;
	this->ui = ui;
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

	background_music = Mix_LoadMUS(audio_path("music.wav").c_str());
	salmon_dead_sound = Mix_LoadWAV(audio_path("death_sound.wav").c_str());
	salmon_eat_sound = Mix_LoadWAV(audio_path("eat_sound.wav").c_str());

	if (background_music == nullptr || salmon_dead_sound == nullptr || salmon_eat_sound == nullptr) {
		fprintf(stderr, "Failed to load sounds\n %s\n %s\n %s\n make sure the data directory is present",
			audio_path("music.wav").c_str(),
			audio_path("death_sound.wav").c_str(),
			audio_path("eat_sound.wav").c_str());
		exit(1);
	}


	// Playing background music indefinitely
	Mix_PlayMusic(background_music, -1);
	fprintf(stderr, "Loaded music\n");

	// Set all states to default
	restart_game();
}

// Update our game world
bool WorldSystem::step(float elapsed_ms_since_last_update) {
	// Get Player
	Entity player = registry.players.entities[0];

	// Updating window title with points
	std::stringstream title_ss;
	player_health = registry.healthComponents.get(player).curr_health;
	title_ss << "Points: " << points;
	title_ss << " Health: " << player_health;
	glfwSetWindowTitle(window, title_ss.str().c_str());

	// Remove debug info from the last step
	for (Entity entity : registry.debugComponents.entities) {
		registry.pendingRemoves.emplace_with_duplicates(entity);
	}
	// cleanup();


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

	// Shoot if LMB is clicked
	for (Entity entity : registry.shooters.entities) {
		shoot(entity);
	}

	// spawn two enemies
	if (registry.deadlys.components.size() < 2) {
		createEnemy(renderer, vec2((uniform_dist(rng) * (window_width_px - (2 * WALL_WIDTH))) + WALL_WIDTH, ((uniform_dist(rng) * (window_height_px - (2 * WALL_WIDTH) - BASE_UI_HEIGHT))) + WALL_WIDTH + BASE_UI_HEIGHT), 100.f);
	}

	// Processing the salmon state
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
			restart_game();
			return true;
		}
	}

	auto& invincibleTimerRegistry = registry.invincibleTimers;
	for (Entity entity : invincibleTimerRegistry.entities) {
		InvincibleTimer& counter = invincibleTimerRegistry.get(entity);
		counter.counter_ms -= elapsed_ms_since_last_update;
		if (counter.counter_ms < 0) {
			invincibleTimerRegistry.remove(entity);
		}
	}

	// reduce window brightness if the salmon is dying
	screen.darken_screen_factor = 1 - min_counter_ms / 3000;

	return true;
}

// Reset the world state to its initial state
void WorldSystem::restart_game() {
	// Debugging for memory/component leaks
	registry.list_all_components();
	printf("Restarting\n");

	// Reset the game speed
	current_speed = 1.f;

	// Remove all entities that we created
	// i.e. All world objects
	for (Entity entity : registry.worldObjects.entities) {
		registry.pendingRemoves.emplace_with_duplicates(entity);
	}
	// cleanup();

	// Debugging for memory/component leaks
	registry.list_all_components();

	// create a floor entity
	createFloor(renderer, { window_width_px / 2, (window_height_px + 120.f) / 2 }, { window_width_px , window_height_px - 120.f });

	// create a new Player entity
	player = createPlayer(renderer, { window_width_px / 2, window_height_px - 200 });

	createInteractable(renderer, { window_width_px / 2, window_height_px - 200 }, { 75.f, 75.f },
		[](int a) {std::cout << "Player interacted with interactable! Int passed in: " << a << std::endl; }
	);
	// top wall
	createWall(renderer, { window_width_px / 2, 25.f + 120.f }, { window_width_px, WALL_WIDTH }, 0.f, TEXTURE_ASSET_ID::HORZ_WALL);
	// bottom wall
	createWall(renderer, { window_width_px / 2, window_height_px - 25.f }, { window_width_px, WALL_WIDTH }, M_PI, TEXTURE_ASSET_ID::HORZ_WALL);
	// left wall
	createWall(renderer, { 25.f, (window_height_px / 2) + 60.f }, { WALL_WIDTH, window_height_px - 120.f }, 0.f, TEXTURE_ASSET_ID::VERT_WALL);
	// right wall
	createWall(renderer, { window_width_px - 25.f, (window_height_px / 2) + 60.f }, { WALL_WIDTH, window_height_px - 120.f }, M_PI, TEXTURE_ASSET_ID::VERT_WALL);

	// Set initial cooldown time
	for (Entity entity : registry.shooters.entities) {
		set_last_shot_time(entity);
	}
	
	initGameUI();
	createCrosshair(renderer);
}

void WorldSystem::initGameUI() {
	// Add the base UI
	Entity base_ui = ui->createPanel(
		SCENE_TYPE::GAME,
		vec2(window_width_px / 2, BASE_UI_HEIGHT / 2),
		0.f,
		vec2(window_width_px, BASE_UI_HEIGHT),
		"game_HUD",
		TEXTURE_ASSET_ID::UI
	);

	// grab player health
	// TODO must update this when any UI elements change
	// maybe make a gameUI update fn
	float player_health = registry.healthComponents.get(registry.players.entities[0]).curr_health;

	// create health_ui entity
	this->health_ui = ui->createUIElement(
		vec2(window_width_px / 10, window_height_px / 11),
		vec2(165.f, 40.f),
		"health_ui",
		static_cast<float>(player_health),
		SCENE_TYPE::GAME);

	// create scrap_ui entity
	scrap_ui = ui->createUIElement(
		vec2(375.f, window_height_px / 20),
		vec2(25.f, 25.f),
		"scrap_ui",
		static_cast<float>(scrap),
		SCENE_TYPE::GAME);

	// create level_ui entity
	level_ui = ui->createUIElement(
		vec2(375.f, window_height_px / 10),
		vec2(25.f, 30.f),
		"level_ui",
		static_cast<float>(level),
		SCENE_TYPE::GAME);

	// create item_ui entities
	// as a placeholder, there is just one item slot for now
	// later, we will want to render all the items and show locked slots too
	item_ui = ui->createTexturedUIElement(
		vec2(window_width_px - window_width_px / 22, window_height_px / 11),
		vec2(75.f, 75.f),
		"item_one_ui",
		TEXTURE_ASSET_ID::ITEM,
		SCENE_TYPE::GAME);
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
			default:
				printf("Unhandled collision\n");
				break;
			}
		}

	}

	// Remove all collisions from this simulation step
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

void WorldSystem::handlePlayerDeadly(Entity player, Entity deadly) {

	std::array<Entity, 2> entities = { player, deadly };
	for (Entity entity : entities) {
		if (!registry.invincibleTimers.has(entity)) {
			registry.healthComponents.get(entity).curr_health -= 1;
			if (registry.players.has(entity)) {
				createParticles(renderer, registry.worldObjects.get(entity).position, uniform_dist, rng, TEXTURE_ASSET_ID::HIT_PARTICLE_PLAYER);
			}
			else {
				createParticles(renderer, registry.worldObjects.get(entity).position, uniform_dist, rng, TEXTURE_ASSET_ID::HIT_PARTICLE);
			}
			
		}
		registry.invincibleTimers.emplace(entity);
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
	registry.healthComponents.get(deadly).curr_health -= registry.projectiles.get(projectile).damage;
	createParticles(renderer, registry.worldObjects.get(deadly).position, uniform_dist, rng, TEXTURE_ASSET_ID::HIT_PARTICLE);

	// Remove projectile
	registry.pendingRemoves.emplace_with_duplicates(projectile);
	return;
}

void WorldSystem::handleProjectilePlayer(Entity projectile, Entity player) {
	// Decrease health of player
	registry.healthComponents.get(player).curr_health -= registry.projectiles.get(projectile).damage;
	createParticles(renderer, registry.worldObjects.get(player).position, uniform_dist, rng, TEXTURE_ASSET_ID::HIT_PARTICLE_PLAYER);

	// Remove projectile
	registry.pendingRemoves.emplace_with_duplicates(projectile);
	return;
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
			interactable.interaction(6);
		}
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
				if (!registry.deathTimers.has(entity)) {
					registry.deathTimers.emplace(entity);
					Mix_PlayChannel(-1, salmon_dead_sound, 0);
				}
			}
			else {
				if (registry.deadlys.has(entity)) {
					// TODO: drop item on death
				}
				registry.pendingRemoves.emplace_with_duplicates(entity);
			}
		}
	}
}

void WorldSystem::shoot(Entity& entity) {
	auto now = get_curr_time();
	float elapsed_ms = (float)(std::chrono::duration_cast<std::chrono::microseconds>(now - get_last_shot_time(entity))).count() / 1000;
	Motion& entity_motion = registry.motions.get(entity);
	WorldObject& entity_object = registry.worldObjects.get(entity);

	if (registry.players.has(entity)) {
		if (left_mouse_button) {
			if ((elapsed_ms >= registry.shooters.get(entity).fire_rate) || first_shot) {
				double xpos, ypos;
				glfwGetCursorPos(window, &xpos, &ypos);
				float angle = atan2(ypos - entity_object.position.y, xpos - entity_object.position.x);
				createProjectile(renderer, entity_object.position, angle, 350.0f, true);
				first_shot = false;

				set_last_shot_time(entity);
			}
		}
	}
	else if (registry.deadlys.has(entity)) {
		if ((elapsed_ms >= registry.shooters.get(entity).fire_rate)) {
			vec2 coor_player = registry.worldObjects.get(registry.players.entities[0]).position;
			int dx = entity_object.position.x - coor_player.x;
			int dy = entity_object.position.y - coor_player.y;
			float angle = atan2(dy, dx) - M_PI;
			if (angle < 0)
				angle += 2 * M_PI;
			createProjectile(renderer, entity_object.position, angle, 350.0f, false);
			set_last_shot_time(entity);
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
			player_motion.target_velocity = v_from_sa(player_motion.max_speed, angle);
		}
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

