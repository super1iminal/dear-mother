// Header
#include "world_system.hpp"
#include "world_init.hpp"

// stlib
#include <cassert>
#include <sstream>
#include <chrono>

#include "physics_system.hpp"
#include <iostream>

using Clock = std::chrono::high_resolution_clock;
auto t = Clock::now();
bool first_shot = true;

// Game configuration
// add variables here

// create the underwater world
WorldSystem::WorldSystem()
	: points(0)
	, player_health(0)
	, next_eel_spawn(0.f)
	, next_fish_spawn(0.f) {
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

	// Destroy all created components
	registry.clear_all_components();

	// Close the window
	glfwDestroyWindow(window);
}

// Debugging
namespace {
	void glfw_err_cb(int error, const char *desc) {
		fprintf(stderr, "%d: %s", error, desc);
	}
}

// World initialization
// Note, this has a lot of OpenGL specific things, could be moved to the renderer
GLFWwindow* WorldSystem::create_window() {
	///////////////////////////////////////
	// Initialize GLFW
	glfwSetErrorCallback(glfw_err_cb);
	if (!glfwInit()) {
		fprintf(stderr, "Failed to initialize GLFW");
		return nullptr;
	}

	//-------------------------------------------------------------------------
	// If you are on Linux or Windows, you can change these 2 numbers to 4 and 3 and
	// enable the glDebugMessageCallback to have OpenGL catch your mistakes for you.
	// GLFW / OGL Initialization
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#if __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
	glfwWindowHint(GLFW_RESIZABLE, 0);

	// Create the main window (for rendering, keyboard, and mouse input)
	window = glfwCreateWindow(window_width_px, window_height_px, "Salmon Game Assignment", nullptr, nullptr);
	if (window == nullptr) {
		fprintf(stderr, "Failed to glfwCreateWindow");
		return nullptr;
	}

	// Setting callbacks to member functions (that's why the redirect is needed)
	// Input is handled using GLFW, for more info see
	// http://www.glfw.org/docs/latest/input_guide.html
	glfwSetWindowUserPointer(window, this);
	auto key_redirect = [](GLFWwindow* wnd, int _0, int _1, int _2, int _3) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_key(_0, _1, _2, _3); };
	auto cursor_pos_redirect = [](GLFWwindow* wnd, double _0, double _1) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_move({ _0, _1 }); };
	auto on_mouse_button = [](GLFWwindow* wnd, int _0, int _1, int _2) { ((WorldSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_button(wnd, _0, _1, _2); };
	glfwSetKeyCallback(window, key_redirect);
	glfwSetCursorPosCallback(window, cursor_pos_redirect);
	glfwSetMouseButtonCallback(window, on_mouse_button);

	//////////////////////////////////////
	// Loading music and sounds with SDL
	if (SDL_Init(SDL_INIT_AUDIO) < 0) {
		fprintf(stderr, "Failed to initialize SDL Audio");
		return nullptr;
	}
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1) {
		fprintf(stderr, "Failed to open audio device");
		return nullptr;
	}

	background_music = Mix_LoadMUS(audio_path("music.wav").c_str());
	salmon_dead_sound = Mix_LoadWAV(audio_path("death_sound.wav").c_str());
	salmon_eat_sound = Mix_LoadWAV(audio_path("eat_sound.wav").c_str());

	if (background_music == nullptr || salmon_dead_sound == nullptr || salmon_eat_sound == nullptr) {
		fprintf(stderr, "Failed to load sounds\n %s\n %s\n %s\n make sure the data directory is present",
			audio_path("music.wav").c_str(),
			audio_path("death_sound.wav").c_str(),
			audio_path("eat_sound.wav").c_str());
		return nullptr;
	}

	return window;
}

void WorldSystem::init(RenderSystem* renderer_arg) {
	this->renderer = renderer_arg;
	// Playing background music indefinitely
	Mix_PlayMusic(background_music, -1);
	fprintf(stderr, "Loaded music\n");

	// Set all states to default
    restart_game();
}

// Update our game world
bool WorldSystem::step(float elapsed_ms_since_last_update) {
	// Updating window title with points
	std::stringstream title_ss;
	player_health = registry.healthComponents.get(registry.players.entities[0]).curr_health;
	title_ss << "Points: " << points;
	title_ss << " Health: " << player_health;
	glfwSetWindowTitle(window, title_ss.str().c_str());

	// Remove debug info from the last step
	while (registry.debugComponents.entities.size() > 0)
	    registry.remove_all_components_of(registry.debugComponents.entities.back());

	// Removing out of screen entities
	auto& worldobjects_registry = registry.worldObjects;

	// Remove entities that leave the screen on the left side
	// Iterate backwards to be able to remove without unterfering with the next object to visit
	// (the containers exchange the last element with the current)
	for (Entity entity : worldobjects_registry.entities) {
		WorldObject& worldobject = worldobjects_registry.get(entity);
		if ((worldobject.position.x + abs(worldobject.scale.x) < 0.f ||
			worldobject.position.x - abs(worldobject.scale.x) > window_width_px ||
			worldobject.position.y + abs(worldobject.scale.y) < 0.f ||
			worldobject.position.y - abs(worldobject.scale.y) > window_height_px) &&
			!registry.players.has(entity)) {
			registry.remove_all_components_of(entity);
		}
	}

	// spawn two enemies
	next_eel_spawn -= elapsed_ms_since_last_update * current_speed;
	if (registry.deadlys.components.size() <= 2 && next_eel_spawn < 0.f) {
		createEnemy(renderer, vec2(window_width_px - 200.f, 250.f), 100);
		createEnemy(renderer, vec2(200.f, 250.f), 100);
	}

	// Processing the salmon state
	assert(registry.screenStates.components.size() <= 1);
    ScreenState &screen = registry.screenStates.components[0];
	screen.health_status = player_health;

    float min_counter_ms = 3000.f;
	for (Entity entity : registry.deathTimers.entities) {
		// progress timer
		DeathTimer& counter = registry.deathTimers.get(entity);
		counter.counter_ms -= elapsed_ms_since_last_update;
		if(counter.counter_ms < min_counter_ms){
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
	while (registry.worldObjects.entities.size() > 0)
	    registry.remove_all_components_of(registry.worldObjects.entities.back());

	// Debugging for memory/component leaks
	registry.list_all_components();

	// create a new Player entity
	player = createPlayer(renderer, { window_width_px / 2, window_height_px - 200 });
	crosshair = createCrosshair(renderer);

	// SHOULD MOVE FLOOR AND INTERACTABLE INITIALISATION TO WORLD_INIT.CPP PLEASE
	// create a floor entity
	createFloor(renderer,{ window_width_px / 2, window_height_px / 2 }, { 150.f , 150.f });
	createInteractable(renderer, { window_width_px / 2, window_height_px - 200 }, { 50.f, 50.f },
		[](int a) {std::cout << "Player interacted with interactable! Int passed in: " << a << std::endl;}
	);

	// Add the base UI
	Entity base_ui = createBaseUI(renderer);

	// set initial player health
	player_health = registry.healthComponents.get(registry.players.entities[0]).curr_health;

	// create health_ui entity
	health_ui = createTexturedUIElement(renderer,
		vec2(window_width_px / 10, window_height_px / 11),
		vec2(165.f, 40.f),
		"health_ui",
		static_cast<float>(player_health));

	// create scrap_ui entity
	scrap_ui = createTexturedUIElement(renderer,
		vec2(window_width_px / 4, window_height_px / 13),
		vec2(50.f, 25.f),
		"scrap_ui",
		static_cast<float>(scrap));

	// create level_ui entity
	level_ui = createTexturedUIElement(renderer,
		vec2(window_width_px / 4, window_height_px / 8),
		vec2(50.f, 30.f),
		"level_ui",
		static_cast<float>(level));

	// create item_ui entities
	// as a placeholder, there is just one item slot for now
	// later, we will want to render all the items and show locked slots too
	Entity item_ui = createTexturedUIElement(renderer,
		vec2(window_width_px - window_width_px / 7, window_height_px / 11),
		vec2(75.f, 75.f),
		"item_one_ui",
		TEXTURE_ASSET_ID::FISH);
}

// Compute collisions between entities
void WorldSystem::handle_collisions() {
	// Loop over all collisions detected by the physics system
	auto& collisionsRegistry = registry.collisions;
	for (uint i = 0; i < collisionsRegistry.components.size(); i++) {
		// The entity and its collider
		Entity entity = collisionsRegistry.entities[i];
		Entity entity_other = collisionsRegistry.components[i].other;

		// for now, we are only interested in collisions that involve the salmon
		if (registry.players.has(entity)) {
			//Player& player = registry.players.get(entity);

			// Checking Player - Deadly collisions
			if (registry.deadlys.has(entity_other)) {
				// initiate death unless already dying
				if (!registry.deathTimers.has(entity)) {
					// Scream, reset timer, and make the salmon sink
					registry.deathTimers.emplace(entity);
					Mix_PlayChannel(-1, salmon_dead_sound, 0);
				}
			}
		}
	}

	// Remove all collisions from this simulation step
	registry.collisions.clear();
}

void WorldSystem::handle_interactions() {
	auto& interactablesRegistry = registry.interactables;
	for (uint i = 0; i < interactablesRegistry.components.size(); i++) {
		Interactable& interactable = interactablesRegistry.components[i];
		Entity interactableEntity = interactablesRegistry.entities[i];

		float range = interactable.range;

		WorldObject& playerWorldObject = registry.worldObjects.get(player);
		WorldObject& interactableObject = registry.worldObjects.get(interactableEntity);

		float distance = sqrt(pow(playerWorldObject.position.x - interactableObject.position.x, 2)
			+ pow(playerWorldObject.position.y - interactableObject.position.y, 2));
		if (distance < range) {
			interactable.interaction(6);
		}
	}
}

// Should the game be over ?
bool WorldSystem::is_over() const {
	return bool(glfwWindowShouldClose(window));
}
bool left_mouse_button;
void WorldSystem::on_mouse_button(GLFWwindow* window, int button, int action, int mods)
{
	left_mouse_button = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
	
	Entity player = registry.players.entities[0];
	Motion& player_motion = registry.motions.get(player);
	WorldObject& player_object = registry.worldObjects.get(player);

	if (left_mouse_button) {
		auto now = Clock::now();
		float elapsed_ms =
			(float)(std::chrono::duration_cast<std::chrono::microseconds>(now - t)).count() / 1000;
		if ((elapsed_ms > registry.players.get(player).fire_rate) || first_shot) {
			double xpos, ypos;
			glfwGetCursorPos(window, &xpos, &ypos);
			float angle = atan2(ypos - player_object.position.y, xpos - player_object.position.x);
			createProjectile(renderer, player_object.position, angle, 150.0f, true);
			first_shot = false;
			t = Clock::now();
		}
	}
}

void WorldSystem::on_key(int key, int, int action, int mod) {
	// Resetting game
	if (action == GLFW_RELEASE && key == GLFW_KEY_R) {
		int w, h;
		glfwGetWindowSize(window, &w, &h);
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
			player_motion.speed = 0.f;
		}
		else {
			player_motion.speed = player_motion.max_speed;
			float angle = atan2(dy, dx);
			if (angle < 0)
				angle += 2 * M_PI;
			player_motion.motion_angle = angle;
		}
	}

	// List all components
	if (action == GLFW_PRESS && key == GLFW_KEY_L) {
		registry.list_all_components();
	}

	// Interaction
	if (action == GLFW_PRESS && key == GLFW_KEY_E) {
		handle_interactions();
	}

	// Close window
	if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}

	// Debugging mode toggle
	if (key == GLFW_KEY_X) {
		debugging.in_debug_mode = (action != GLFW_RELEASE);
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
	// Update the position of the crosshair
	WorldObject& crosshair_object = registry.worldObjects.get(crosshair);
	if (mouse_position.x > 0 && mouse_position.x < window_width_px && mouse_position.y > 0 && mouse_position.y < window_height_px) 
		crosshair_object.position = mouse_position;
}
