// internal
#include <queue>
#include <vector>
#include <iostream>
#include "reloadability_system.hpp"
#include <fstream>

#include "json.hpp"
#include "world_system.hpp"
#include "world_init.hpp"
#include "tiny_ecs.hpp"
using namespace std;

using json = nlohmann::json;
void ReloadabilitySystem::saveGame() {
    cout << "Saving game" << endl;
    json save;

    Entity player = registry.players.entities[0];
    string id = to_string((int)player);
    save[id];
    save[id]["player"];
    if (registry.worldObjects.has(player)) {
        WorldObject& worldObject = registry.worldObjects.get(player);
        save[id]["worldObject"] = {
            {"position", {worldObject.position.x, worldObject.position.y}},
            {"angle", worldObject.angle},
            {"scale", {worldObject.scale.x, worldObject.scale.y}}
        };
    }
    if (registry.motions.has(player)) {
        Motion& motion = registry.motions.get(player);
        save[id]["motion"] = {
            {"max_speed", motion.max_speed},
            {"velocity", {motion.velocity.x, motion.velocity.y}},
            {"target_velocity", {motion.target_velocity.x, motion.target_velocity.y}},
            {"acceleration", {motion.acceleration.x, motion.acceleration.y}},
            {"mass", motion.mass}
        };
    }

    for (Entity entity : registry.deadlys.entities) {
        string id = to_string((int)entity);
        save[id];

        save[id]["deadly"];

        if (registry.worldObjects.has(entity)) {
            WorldObject& worldObject = registry.worldObjects.get(entity);
            save[id]["worldObject"] = {
                {"position", {worldObject.position.x, worldObject.position.y}},
                {"angle", worldObject.angle},
                {"scale", {worldObject.scale.x, worldObject.scale.y}}
            };
        }

        if (registry.motions.has(entity)) {
            Motion& motion = registry.motions.get(entity);
            save[id]["motion"] = {
                {"max_speed", motion.max_speed},
                {"velocity", {motion.velocity.x, motion.velocity.y}},
                {"target_velocity", {motion.target_velocity.x, motion.target_velocity.y}},
                {"acceleration", {motion.acceleration.x, motion.acceleration.y}},
                {"mass", motion.mass}
            };
        }

        if (registry.roomCoords.has(entity)) {
            RoomCoordinate& room_coord = registry.roomCoords.get(entity);
            save[id]["roomCoord"] = {
                {"position", {room_coord.position.x, room_coord.position.y}}
            };
        }
    }

    std::ofstream output_file("../data/saved_game/test.json", std::ios::out | std::ios::trunc);
    if (!output_file) {
        cout << "failed to open file" << endl;
    } else {
        try {
            output_file << save.dump(4); // Pretty-print with indentation
            cout << "save successful" << endl;
        } catch (const std::exception& e) {
            cout << "save failed" << e.what() << endl;
        }
        output_file.close();
    }
}

void ReloadabilitySystem::loadGame() {
    std::ifstream input_file("../data/saved_game/test.json");
    if (!input_file) {
        cout << "failed to open file";
        return;
    }
    json test;
    try {
        input_file >> test;
    } catch (const json::parse_error& e) {
        cout << "parse fail" << e.what() << std::endl;
        return;
    }
    int deadly_counter = 0;
    for (auto& [id, data] : test.items()) {
        if (data.contains("player")) {
            auto player = registry.players.entities[0];
            registry.players.emplace(player);
            WorldObject& world = registry.worldObjects.emplace(player);
            world.angle = data["worldObject"]["angle"];
            world.position = {data["worldObject"]["position"][0], data["worldObject"]["position"][1]};
            world.scale = {data["worldObject"]["scale"][0], data["worldObject"]["scale"][1]};
            Motion& motion = registry.motions.emplace(player);
            motion.max_speed = data["motion"]["max_speed"];
            motion.velocity = {data["motion"]["velocity"][0], data["motion"]["velocity"][1]};
            motion.target_velocity = {data["motion"]["target_velocity"][0], data["motion"]["target_velocity"][1]};
            motion.acceleration = {data["motion"]["acceleration"][0], data["motion"]["acceleration"][1]};
            motion.mass = data["motion"]["mass"];
        } else if (data.contains("deadly")) {
            auto entity = registry.deadlys.entities[deadly_counter];
            deadly_counter++;
        registry.deadlys.emplace(entity);
        WorldObject& world = registry.worldObjects.emplace(entity);
        world.angle = data["worldObject"]["angle"];
        world.position = {data["worldObject"]["position"][0], data["worldObject"]["position"][1]};
        world.scale = {data["worldObject"]["scale"][0], data["worldObject"]["scale"][1]};
        Motion& motion = registry.motions.emplace(entity);
        motion.max_speed = data["motion"]["max_speed"];
        motion.velocity = {data["motion"]["velocity"][0], data["motion"]["velocity"][1]};
        motion.target_velocity = {data["motion"]["target_velocity"][0], data["motion"]["target_velocity"][1]};
        motion.acceleration = {data["motion"]["acceleration"][0], data["motion"]["acceleration"][1]};
        motion.mass = data["motion"]["mass"];
        RoomCoordinate& room_coord = registry.roomCoords.emplace(entity, (ivec2){data["roomCoord"]["position"][0], data["roomCoord"]["position"][1]});
        }
    }



}