#include "scene_system.hpp"

SceneSystem::SceneSystem() :
	scene(SCENE_TYPE::MENU)
{

}

SceneSystem::~SceneSystem()
{
	// Destroy all created components
	registry.clear_all_components();

	// renderer will be destroyed before glfwDestroyWindow, which is not the correct order. we want the other order.

	// Close the window
	// commenting this out prevents an error. im not sure why
	// glfwDestroyWindow(window);
}

bool SceneSystem::init()
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
	auto key_redirect = [](GLFWwindow* wnd, int _0, int _1, int _2, int _3) { ((SceneSystem*)glfwGetWindowUserPointer(wnd))->on_key(_0, _1, _2, _3); };
	auto cursor_pos_redirect = [](GLFWwindow* wnd, double _0, double _1) { ((SceneSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_move({ _0, _1 }); };
	auto on_mouse_button = [](GLFWwindow* wnd, int _0, int _1, int _2) { ((SceneSystem*)glfwGetWindowUserPointer(wnd))->on_mouse_button(wnd, _0, _1, _2); };
	glfwSetKeyCallback(window, key_redirect);
	glfwSetCursorPosCallback(window, cursor_pos_redirect);
	glfwSetMouseButtonCallback(window, on_mouse_button);

	// Initialize renderer
	renderer.init(window);

	// Intialize ui
	ui.init(&renderer, window);
	
	if (scene == SCENE_TYPE::MENU) {
		ui.initMenuUI();
	}
	else if (scene == SCENE_TYPE::GAME) {
		// Initialize world
		world.init(&renderer, window, &ui);
	}

	return true;
}

void SceneSystem::on_key(int key, int sc, int action, int mod) {
	// Close window
	if (action == GLFW_PRESS && key == GLFW_KEY_ESCAPE) {
		glfwSetWindowShouldClose(window, GL_TRUE);
	}
	// Debugging mode toggle
	else if (key == GLFW_KEY_X) {
		debugging.in_debug_mode = (action != GLFW_RELEASE);
	}
	// List all components
	else if (action == GLFW_PRESS && key == GLFW_KEY_L) {
		registry.list_all_components();
	} 
	else {
		switch (scene) {
			case SCENE_TYPE::GAME:
			{
				world.on_key(key, sc, action, mod);
				break;
			}
			case SCENE_TYPE::MENU:
			{
				// Update the menu screen
				break;
			}
			case SCENE_TYPE::PAUSE:
			{
				// Update the pause screen
				break;
			}
			case SCENE_TYPE::TEST:
			{
				// Update the test screen
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

void SceneSystem::on_mouse_move(vec2 pos) {
	// Update the position of the crosshair
	for (Entity crosshair : registry.crosshairs.entities) {
		WorldObject& crosshair_object = registry.worldObjects.get(crosshair);
		if (pos.x > 0 && pos.x < window_width_px && pos.y > BASE_UI_HEIGHT && pos.y < window_height_px)
			crosshair_object.position = pos;
	}
	switch (scene) {
		case SCENE_TYPE::GAME: 
		{
			world.on_mouse_move(pos);
			break;
		}
		case SCENE_TYPE::MENU: 
		{
			// Update the menu screen
			ui.on_mouse_move(pos);
			break;
		}
		case SCENE_TYPE::PAUSE: 
		{
			// Update the pause screen
			break;
		}
		case SCENE_TYPE::TEST: 
		{
			// Update the test screen
			break;
		}
		case SCENE_TYPE::SCENE_COUNT: 
		{
			// This should never happen
			break;
		}
	}
}

void SceneSystem::on_mouse_button(GLFWwindow* window, int button, int action, int mods) {
	switch (scene) {
		case SCENE_TYPE::GAME: 
		{
			world.on_mouse_button(window, button, action, mods);
			break;
		}
		case SCENE_TYPE::MENU: 
		{
			// Update the menu screen
			ui.on_mouse_button(window, button, action, mods);
			break;
		}
		case SCENE_TYPE::PAUSE: 
		{
			// Update the pause screen
			break;
		}
		case SCENE_TYPE::TEST: 
		{
			// Update the test screen
			break;
		}
		case SCENE_TYPE::SCENE_COUNT: 
		{
			// This should never happen
			break;
		}
	}
}


bool SceneSystem::step(float elapsed_ms)
{
	switch (scene) {
		case SCENE_TYPE::GAME: 
		{
			// Update the game
			world.step(elapsed_ms);
			ai.step(elapsed_ms);
			physics.step(elapsed_ms);
			collisions.add_collisions();
			world.handle_collisions();
			world.handle_deaths();
			renderer.drawGame();
			break;
		}
		case SCENE_TYPE::MENU: 
		{
			// Update the menu screen
			renderer.drawMenu();
			break;
		}
		case SCENE_TYPE::PAUSE: 
		{
			// Update the pause screen
			break;
		}
		case SCENE_TYPE::TEST: 
		{
			// Update the test screen
			break;
		}
		case SCENE_TYPE::SCENE_COUNT: 
		{
			// This should never happen
			break;
		}
	}
	cleanup(); // remove dead entities and entities we want to remove
	return true;
}


// Should the game be over ?
bool SceneSystem::is_over() const {
	return bool(glfwWindowShouldClose(window));
}

// cleanup all world entities to be removed. will prevent so many errors. 
// works fine with duplicate entries in removes
void SceneSystem::cleanup() {
	// Make a copy of the entities to remove
	auto entities_to_remove = registry.pendingRemoves.entities; // Copy the list

	// Remove components of each entity
	for (Entity entity : entities_to_remove) {
		registry.remove_all_components_of(entity);
	}

	// Clear the removes container
	registry.pendingRemoves.clear();
}