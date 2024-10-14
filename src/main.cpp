
#define GL3W_IMPLEMENTATION
#include <gl3w.h>

// stlib
#include <chrono>

// internal
#include "physics_system.hpp"
#include "render_system.hpp"
#include "world_system.hpp"
#include "ai_system.hpp"

using Clock = std::chrono::high_resolution_clock;

// Entry point
int main()
{
	// Global systems
	WorldSystem world;
	RenderSystem renderer;
	PhysicsSystem physics;
	AISystem ai;

	// Initializing window
	GLFWwindow* window = world.create_window();
	if (!window) {
		// Time to read the error message
		printf("Press any key to exit");
		getchar();
		return EXIT_FAILURE;
	}

	// initialize the main systems
	renderer.init(window);
	world.init(&renderer);

	// variable timestep loop
	auto t = Clock::now();

	double last_time = glfwGetTime();
	int num_of_frames = 0;
	double fps = 0;

	while (!world.is_over()) {
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

		world.step(elapsed_ms, fps);
		ai.step(elapsed_ms);
		physics.step(elapsed_ms);
		world.handle_collisions();
		world.handle_deaths();
		world.cleanup(); // remove dead entities and entities we want to remove

		renderer.draw();
	}

	return EXIT_SUCCESS;
}
