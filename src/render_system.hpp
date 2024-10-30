#pragma once

// stlib
#include <array>
#include <utility>
#include <cassert>
#include <sstream>
#include <chrono>
#include <iostream>

#include "common.hpp"
#include "components.hpp"
#include "tiny_ecs.hpp"

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_mixer.h>
#include <map>

// System responsible for setting up OpenGL and for rendering all the
// visual entities in the game
class RenderSystem {
	/**
	 * The following arrays store the assets the game will use. They are loaded
	 * at initialization and are assumed to not be modified by the render loop.
	 *
	 * Whenever possible, add to these lists instead of creating dynamic state
	 * it is easier to debug and faster to execute for the computer.
	 */
	std::array<GLuint, texture_count> texture_gl_handles;
	std::array<ivec2, texture_count> texture_dimensions;

	// Make sure these paths remain in sync with the associated enumerators.
	// Associated id with .obj path
	const std::vector < std::pair<GEOMETRY_BUFFER_ID, std::string>> mesh_paths =
	{
		  std::pair<GEOMETRY_BUFFER_ID, std::string>(GEOMETRY_BUFFER_ID::SALMON, mesh_path("salmon.obj")),
		  std::pair<GEOMETRY_BUFFER_ID, std::string>(GEOMETRY_BUFFER_ID::SQUARE, mesh_path("square.obj"))
		  // specify meshes of other assets here
	};

	// Make sure these paths remain in sync with the associated enumerators.
	const std::array<std::string, texture_count> texture_paths = {
			textures_path("player_robot.png"),
			textures_path("bounding_box.png"),
			textures_path("floor_1.png"),
			textures_path("bounding_box_blue.png"),
			textures_path("bullet_friendly.png"),
			textures_path("bullet_enemy.png"),
			textures_path("/crosshairs/crosshair003.png"),
			textures_path("/crosshairs/crosshair193.png"),
			textures_path("/crosshairs/crosshair070.png"),
			textures_path("base_UI.png"),
			textures_path("enemy_robot.png"),
			textures_path("horz_wall.png"),
			textures_path("vert_wall.png"),
			textures_path("battery.png"),
			textures_path("battery.png"), // Repeated for other items for now, should be SHATTERED_QUARTZ. Do double check.
			textures_path("battery.png"), // Repeated for other items for now, should be CREAKY_WHEEL
			textures_path("battery.png"), // Repeated for other items for now, should be HEATSINK
			textures_path("battery.png"), // Repeated for other items for now, should be REPEATER
			textures_path("gear.png"),
			textures_path("gear_player.png"),
			textures_path("start_screen_bg.png"),
			textures_path("help_screen.png"),
			textures_path("/buttons/start_button.png"),
			textures_path("/buttons/help_button.png"),
			textures_path("/buttons/upgrade_button.png"),
			textures_path("/buttons/quit_button.png"),
			textures_path("/buttons/back_button.png"),
      //textures_path("alert.png")
	};

	std::array<GLuint, effect_count> effects;
	// Make sure these paths remain in sync with the associated enumerators.
	const std::array<std::string, effect_count> effect_paths = {
		shader_path("coloured"),
		shader_path("egg"),
		shader_path("ui_element"),
		shader_path("font"),
		shader_path("salmon"),
		shader_path("textured"),
		shader_path("water") };

	std::array<GLuint, geometry_count> vertex_buffers;
	std::array<GLuint, geometry_count> index_buffers;
	std::array<Mesh, geometry_count> meshes;

public:

	// Creates a window
	GLFWwindow* create_window();

	// Initialize the window
	bool init(GLFWwindow* window);

	template <class T>
	void bindVBOandIBO(GEOMETRY_BUFFER_ID gid, std::vector<T> vertices, std::vector<uint16_t> indices);

	void initializeGlTextures();

	void initializeGlEffects();

	void initializeGlMeshes();

	void initFont(const std::string& font_filename, unsigned int font_default_size);

	Mesh& getMesh(GEOMETRY_BUFFER_ID id) { return meshes[(int)id]; };

	void initializeGlGeometryBuffers();
	// Initialize the screen texture used as intermediate render target
	// The draw loop first renders to this texture, then it is used for the wind
	// shader
	bool initScreenTexture();

	// Destroy resources associated to one or all entities created by the system
	~RenderSystem();

	// Draw all entities
	void draw(SCENE_TYPE scene);

	mat3 createProjectionMatrix();

private:
	// Internal drawing functions for each entity type
	void drawTexturedMesh(Entity entity, const mat3& projection);
	void drawToScreen();

	// Window handle
	GLFWwindow* window;

	// Screen texture handles
	GLuint frame_buffer;
	GLuint off_screen_render_buffer_color;
	GLuint off_screen_render_buffer_depth;

	Entity screen_state_entity;

	// font characters
	std::map<char, Character> m_ftCharacters;
};

bool loadEffectFromFile(
	const std::string& vs_path, const std::string& fs_path, GLuint& out_program);
