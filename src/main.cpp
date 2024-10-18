
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
#include "scene_system.hpp"
#include "common.hpp"

using Clock = std::chrono::high_resolution_clock;

// Entry point
int main()
{
	SceneSystem scene;

	// initialize the main systems
	if (!scene.init()) {
		// Time to read the error message
		printf("Press any key to exit");
		getchar();
		return EXIT_FAILURE;
	}

	// variable timestep loop
	auto t = Clock::now();
	while (!scene.is_over()) {
		// Processes system messages, if this wasn't present the window would become unresponsive
		glfwPollEvents();

		// Calculating elapsed times in milliseconds from the previous iteration
		auto now = Clock::now();

		double curr_time = glfwGetTime();
		num_of_frames++;

		if (curr_time - last_time >= 1.0) {
			fps = num_of_frames;
			num_of_frames = 0;
			last_time += 1.0;
		}

		float elapsed_ms =
			(float)(std::chrono::duration_cast<std::chrono::microseconds>(now - t)).count() / 1000;
		t = now;

		// Updating the game state
		scene.step(elapsed_ms);
	}

	return EXIT_SUCCESS;
}
