#include "game_manager.hpp"

GameManager::GameManager()
{
}

GameManager::~GameManager()
{
	// Destroy all created components
	registry.clear_all_components();

	// renderer will be destroyed before glfwDestroyWindow, which is not the correct order. we want the other order.

	// Close the window
	// commenting this out prevents an error. im not sure why
	// glfwDestroyWindow(window);
}

bool GameManager::init()
{
	// Initialize the window
	window = renderer.create_window();
	if (!window) {
		// Time to read the error message
		printf("Press any key to exit");
		getchar();
		return false;
	}
	// Setting callbacks to member functions (that's why the redirect is needed)
	// Input is handled using GLFW, for more info see
	// http://www.glfw.org/docs/latest/input_guide.html
	glfwSetWindowUserPointer(window, this);
	auto key_redirect = [](GLFWwindow* wnd, int _0, int _1, int _2, int _3) { ((GameManager*)glfwGetWindowUserPointer(wnd))->on_key(_0, _1, _2, _3); };
	auto cursor_pos_redirect = [](GLFWwindow* wnd, double _0, double _1) { ((GameManager*)glfwGetWindowUserPointer(wnd))->on_mouse_move({ _0, _1 }); };
	auto on_mouse_button = [](GLFWwindow* wnd, int _0, int _1, int _2) { ((GameManager*)glfwGetWindowUserPointer(wnd))->on_mouse_button(wnd, _0, _1, _2); };
	glfwSetKeyCallback(window, key_redirect);
	glfwSetCursorPosCallback(window, cursor_pos_redirect);
	glfwSetMouseButtonCallback(window, on_mouse_button);

	// initialize SDL, MIX and music!
	// Loading music and sounds with SDL
	if (SDL_Init(SDL_INIT_AUDIO) < 0) {
		fprintf(stderr, "Failed to initialize SDL Audio");
		exit(1);
	}
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) == -1) {
		fprintf(stderr, "Failed to open audio device");
		exit(1);
	}

	// Initialize renderer
	renderer.init(window);

	// Intialize ui
	ui.init(window);

	// initialize menu and world
	menu.init(&renderer, window);
	help.init(&renderer, window);
	pause.init(&renderer, window);
	shop.init(&renderer, window);
	dialogue.init(&renderer, window);
	world.init(&renderer, window);
	reload.init(&renderer, window);
	
	reset_dialogue_run_status();

	menu.update_music(); // play main menu music (hardcoded)
	return true;
}

void GameManager::on_key(int key, int sc, int action, int mod) {
	// Close window
	//if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE) {
	//	glfwSetWindowShouldClose(window, GL_TRUE);
	//}
	// Debugging mode toggle
	if (key == GLFW_KEY_X) {
		debugging.in_debug_mode = (action != GLFW_RELEASE);
	}
	// List all components
	else if (action == GLFW_PRESS && key == GLFW_KEY_L) {
		registry.list_all_components();
		printf("current scene: %d", scene_manager.get_scene());
	}
	else {
		switch (scene_manager.get_scene()) {
		case SCENE_TYPE::GAME:
		{
			world.on_key(key, sc, action, mod);
			break;
		}
		case SCENE_TYPE::MENU:
		{
			// Update the menu screen
			menu.on_key(key, sc, action, mod);
			break;
		}
		case SCENE_TYPE::HELP:
		{
			// Update the help screen
			break;
		}
		case SCENE_TYPE::PAUSE:
		{
			pause.on_key(key, sc, action, mod);
			break;
		}
		case SCENE_TYPE::SHOP:
		{
			// Update the shop screen
			break;
		}
		case SCENE_TYPE::TEST:
		{
			// Update the test screen
			break;
		}
		case SCENE_TYPE::DIALOGUE:
		{
			dialogue.on_key(key, sc, action, mod);
			break;
		}
		case SCENE_TYPE::SCENE_COUNT:
		{
			// This should never happen
			break;
		}
		}
	}

}

void GameManager::on_mouse_move(vec2 pos) {
	// Update the position of the crosshair
	for (Entity crosshair : registry.crosshairs.entities) {
		WorldObject& crosshair_object = registry.worldObjects.get(crosshair);
		if (scene_manager.get_scene() == SCENE_TYPE::GAME) {
			if (pos.x > 0 && pos.x < window_width_px && pos.y > BASE_UI_HEIGHT && pos.y < window_height_px)
				crosshair_object.position = pos;
		}
		else {
			if (pos.x > 0 && pos.x < window_width_px && pos.y > 0 && pos.y < window_height_px)
				crosshair_object.position = pos;
		}
		
	}
	switch (scene_manager.get_scene()) {
	case SCENE_TYPE::GAME:
	{
		world.on_mouse_move(pos);
		break;
	}
	case SCENE_TYPE::MENU:
	{
		// Update the menu screen
		menu.on_mouse_move(pos);
		break;
	}
	case SCENE_TYPE::HELP:
	{
		// Update the help screen
		help.on_mouse_move(pos);
		break;
	}
	case SCENE_TYPE::PAUSE:
	{
		pause.on_mouse_move(pos);
		break;
	}
	case SCENE_TYPE::SHOP:
	{
		// Update the shop screen
		shop.on_mouse_move(pos);
		break;
	}
	case SCENE_TYPE::TEST:
	{
		// Update the test screen
		break;
	}
	case SCENE_TYPE::DIALOGUE:
	{

		break;
	}
	case SCENE_TYPE::SCENE_COUNT:
	{
		// This should never happen
		break;
	}
	}
}

void GameManager::on_mouse_button(GLFWwindow* window, int button, int action, int mods) {
	switch (scene_manager.get_scene()) {
	case SCENE_TYPE::GAME:
	{
		world.on_mouse_button(window, button, action, mods);
		break;
	}
	case SCENE_TYPE::MENU:
	{
		// Update the menu screen
		menu.on_mouse_button(window, button, action, mods);
		break;
	}
	case SCENE_TYPE::HELP:
	{
		// Update the help screen
		help.on_mouse_button(window, button, action, mods);
		break;
	}
	case SCENE_TYPE::PAUSE:
	{
		pause.on_mouse_button(window, button, action, mods);
		break;
	}
	case SCENE_TYPE::SHOP:
	{
		// Update the shop screen
		shop.on_mouse_button(window, button, action, mods);
		break;
	}
	case SCENE_TYPE::TEST:
	{
		// Update the test screen
		break;
	}
	case SCENE_TYPE::DIALOGUE:
	{

	}
	case SCENE_TYPE::SCENE_COUNT:
	{
		// This should never happen
		break;
	}
	}
}


bool GameManager::step(float elapsed_ms, double fps)
{
	std::stringstream title_ss;
	title_ss << "Dear Mother FPS: " << fps;
	glfwSetWindowTitle(window, title_ss.str().c_str());

	SCENE_TYPE scene = scene_manager.get_scene();
	switch (scene) {
	case SCENE_TYPE::GAME:
	{
		// one if statement shouldn't affect performance too much
		// this should change once saving has been implemented but i dont know how
		// this is the only location where has_just_changed is used and can be removed safely
		if (scene_manager.has_just_changed()) {
			if (scene_manager.get_previous_scene() == SCENE_TYPE::MENU) {
				world.restart_game();
			} if (scene_manager.get_previous_scene() != SCENE_TYPE::DIALOGUE) {
				world.update_music();
			}
		}
		world.step(elapsed_ms);
		ai.step(elapsed_ms);
		physics.step(elapsed_ms);
		collisions.add_collisions();
		world.update_animations();
		world.handle_collisions();
		world.handle_deaths();
		break;
	}
	case SCENE_TYPE::MENU:
	{
		// Update the menu screen
		if (scene_manager.has_just_changed()) {
			printf("updating menu music\n");
			menu.update_music();
		}
		break;
	}
	case SCENE_TYPE::HELP:
	{
		// Update the help screen
		break;
	}
	case SCENE_TYPE::PAUSE:
	{
		// Update the pause screen
		if (scene_manager.has_just_changed()) {
			pause.update_music();
		}
		break;
	}
	case SCENE_TYPE::SHOP:
	{
		// Update the shop screen
		break;
	}
	case SCENE_TYPE::TEST:
	{
		// Update the test screen
		break;
	}
	case SCENE_TYPE::DIALOGUE:
	{
		if (scene_manager.has_just_changed()) {
			if (!dialogue.load_dialogue()) {
				scene_manager.set_scene(SCENE_TYPE::GAME);
			}
			else {
				dialogue.continue_dialogue(); // show the first box
			}
		}
		break;
	}
	case SCENE_TYPE::SCENE_COUNT:
	{
		// This should never happen
		break;
	}
	}
	renderer.draw(elapsed_ms);
	cleanup(); // remove dead entities and entities we want to remove
	scene_manager.set_just_changed(false);
	return true;
}


// Should the game be over ?
bool GameManager::is_over() const {
	return bool(glfwWindowShouldClose(window));
}

// cleanup all world entities to be removed. will prevent so many errors. 
// works fine with duplicate entries in removes
void GameManager::cleanup() {
	// Make a copy of the entities to remove
	auto entities_to_remove = registry.pendingRemoves.entities; // Copy the list

	// Remove components of each entity
	for (Entity entity : entities_to_remove) {
		registry.remove_all_components_of(entity);
	}

	// Clear the removes container
	registry.pendingRemoves.clear();
}