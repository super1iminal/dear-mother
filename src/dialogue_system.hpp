#pragma once
#include "common.hpp"
#include "render_system.hpp"
#include "ui_system.hpp"
#include "tiny_ecs_registry.hpp"
#include "scene_manager.hpp"
#include "json.hpp"
#include <iostream>
#include <fstream>
#include <string>


// okay! so, basically, the dialogue system will run ON TOP of the world system, but is counted as a seperate system so that the world system is paused!
// this means that the render system's rendering of the dialogue system needs to include the rendering of the world system
// yipee!

const float DIALOGUE_BOX_WIDTH = 384.f;
const float DIALOGUE_BOX_HEIGHT = 128.f;

const vec3 DIALOGUE_TEXT_COLOR = vec3(1.f, 1.f, 1.f);

using json = nlohmann::json;
class DialogueSystem {
public:
	DialogueSystem();

	// Releases all associated resources
	~DialogueSystem();

	bool load_dialogue();

	// initialize
	void init(RenderSystem* renderer_arg, GLFWwindow* window_arg);
	bool continue_dialogue();
	void on_mouse_button(GLFWwindow* window, int button, int action, int mods);
	void on_mouse_move(vec2 mouse_position);
	void on_key(int key, int sc, int action, int mod);
private:
	RenderSystem* renderer;

	// Window handle
	GLFWwindow* window;

	vec2 cursor_position;

	SCENE_TYPE* scene;

	void end_dialogue();

	// given from outside
	Entity current_NPC;

	// fully internal
	Entity current_textbox;
	bool valid_textbox;

	int current_run;
	int current_interaction;
	int current_sent;

	json dialogue;

	void delete_previous_textbox();

	bool is_mouse_within_button(WorldObject buttonObject);
};

bool reset_dialogue_run_status();