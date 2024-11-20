#pragma once
#include <vector>

#include "render_system.hpp"
#include "tiny_ecs_registry.hpp"
#include "common.hpp"
using namespace std;

class ReloadabilitySystem
{
public:
    void init(RenderSystem* renderer_arg, GLFWwindow* window_arg);
    static void saveGame();
    static void loadGame();
    static void recordPlayerDeathRoom();
private:
    static RenderSystem* renderer;
    GLFWwindow* window;
};