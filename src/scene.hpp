#pragma once
#include "common.hpp"

class SceneManager;

class Scene {
public:
	virtual ~Scene() = default;
	virtual void enter() = 0;
	virtual void exit() = 0;
	virtual void step(float elapsed_ms) = 0;
	virtual void render(float elapsed_ms) = 0;
	virtual void on_key(int key, int sc, int action, int mod) = 0;
	virtual void on_mouse_button(GLFWwindow* window, int button, int action, int mods) = 0;
	virtual void on_mouse_move(vec2 mouse_position) = 0;
	Scene(SceneManager& manager) : sceneManager(manager) {};

protected:
	SceneManager& sceneManager;
};