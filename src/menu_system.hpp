#include <render_system.hpp>
#include <ui_system.hpp>
class MenuSystem
{
public:
	MenuSystem();

	// Releases all associated resources
	~MenuSystem();

	// initialize
	void init(RenderSystem* renderer_arg, GLFWwindow* window_arg, SCENE_TYPE* scene_arg);

	void initStartMenu();

	void on_mouse_button(GLFWwindow* window, int button, int action, int mods);
	void on_mouse_move(vec2 mouse_position);

private:
	RenderSystem* renderer;

	// Window handle
	GLFWwindow* window;

	vec2 cursor_position;

	SCENE_TYPE* scene;

	// open help screen
	void initHelpScreen();

	// close help screen
	void closeHelpScreen();

	bool is_mouse_within_button(WorldObject buttonObject);
};