#pragma once
#include <vector>

#include "tiny_ecs_registry.hpp"
#include "common.hpp"
using namespace std;

class ReloadabilitySystem
{
public:
    static void saveGame();
    static void loadGame();
};