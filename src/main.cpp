
#define GL3W_IMPLEMENTATION
#include <gl3w.h>

// stlib
#include <chrono>

// internal
#include "physics_system.hpp"
#include "render_system.hpp"
#include "world_system.hpp"
#include "ai_system.hpp"
#include "collision_system.hpp"
#include "game_manager.hpp"
#include "common.hpp"

using Clock = std::chrono::high_resolution_clock;

// Entry point
int main()
{
	GameManager game;

	// initialize the main systems
	if (!game.init()) {
		// Time to read the error message
		printf("Press any key to exit");
		getchar();
		return EXIT_FAILURE;
	}

	// variable timestep loop
	auto t = Clock::now();
	// FPS Counter based on this tutorial: http://www.opengl-tutorial.org/miscellaneous/an-fps-counter/
	double last_time = glfwGetTime();
	int num_of_frames = 0;
	double fps = 0;
	int num_of_frames_debug = 0;
	while (!game.is_over()) {
		// Processes system messages, if this wasn't present the window would become unresponsive
		glfwPollEvents();

		// Calculating elapsed times in milliseconds from the previous iteration
		auto now = Clock::now();

		double curr_time = glfwGetTime();
		num_of_frames++;

		num_of_frames_debug++;
		if (curr_time - last_time >= 1.0) {
			fps = num_of_frames;
			num_of_frames = 0;
			last_time += 1.0;
		}
		if (num_of_frames_debug > 1000) {
			num_of_frames_debug = 0;
			std::cout << registry.players.entities.size() << std::endl;
		}
		float elapsed_ms =
			(float)(std::chrono::duration_cast<std::chrono::microseconds>(now - t)).count() / 1000;
		t = now;

		// Updating the game state
		game.step(elapsed_ms, fps);
	}

	return EXIT_SUCCESS;
}
