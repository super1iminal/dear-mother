#include "scene_manager.hpp"
#include "scene.hpp"
#include "fstream"

SceneManager::SceneManager(GLFWwindow* gl_window) : window(gl_window)
{
	// Setting callbacks to member functions (that's why the redirect is needed)
	// Input is handled using GLFW, for more info see
	// http://www.glfw.org/docs/latest/input_guide.html
	glfwSetWindowUserPointer(window, this);
	auto on_key_redirect = [](GLFWwindow* wnd, int _0, int _1, int _2, int _3) { ((SceneManager*)glfwGetWindowUserPointer(wnd))->on_key(_0, _1, _2, _3); };
	auto on_mouse_move_redirect = [](GLFWwindow* wnd, double _0, double _1) { ((SceneManager*)glfwGetWindowUserPointer(wnd))->on_mouse_move({ _0, _1 }); };
	auto on_mouse_button_redirect = [](GLFWwindow* wnd, int _0, int _1, int _2) { ((SceneManager*)glfwGetWindowUserPointer(wnd))->on_mouse_button(wnd, _0, _1, _2); };
	glfwSetKeyCallback(window, on_key_redirect);
	glfwSetCursorPosCallback(window, on_mouse_move_redirect);
	glfwSetMouseButtonCallback(window, on_mouse_button_redirect);
}

SceneManager::~SceneManager()
{
}

void SceneManager::set_scene(std::unique_ptr<Scene> scene)
{
	if (current_scene) {
		current_scene->exit();
	}

	current_scene = std::move(scene);

	if (current_scene) {
		current_scene->enter();
	}
	else {
		printf("SceneManager::change_scene: current_scene is null\n");
	}
}

void SceneManager::step(float elapsed_ms)
{
	if (current_scene) {
		current_scene->step(elapsed_ms);
	}
	else {
		printf("SceneManager::update: current_scene is null\n");
	}
}

void SceneManager::render(float elapsed_ms)
{
	if (current_scene) {
		current_scene->render(elapsed_ms);
	}
	else {
		printf("SceneManager::render: current_scene is null\n");
	}
}

void SceneManager::on_key(int key, int sc, int action, int mod) {
	if (current_scene) {
		current_scene->on_key(key, sc, action, mod);
	}
	else {
		printf("SceneManager::on_key: current_scene is null\n");
	}
}

void SceneManager::on_mouse_button(GLFWwindow* window, int button, int action, int mods) {
	if (current_scene) {
		current_scene->on_mouse_button(window, button, action, mods);
	}
	else {
		printf("SceneManager::on_mouse_button: current_scene is null\n");
	}
}

void SceneManager::on_mouse_move(vec2 mouse_position) {
	for (Entity crosshair : registry.crosshairs.entities) {
		WorldObject& crosshair_object = registry.worldObjects.get(crosshair);
		if (mouse_position.x > 0 && mouse_position.x < window_width_px && mouse_position.y > 0 && mouse_position.y < window_height_px)
			crosshair_object.position = mouse_position;
	}
	if (current_scene) {
		current_scene->on_mouse_move(mouse_position);
	}
	else {
		printf("SceneManager::on_mouse_move: current_scene is null\n");
	}
}

SceneManager scene_manager;