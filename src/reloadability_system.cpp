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
RenderSystem* ReloadabilitySystem::renderer = nullptr;
void ReloadabilitySystem::init(RenderSystem* renderer_arg, GLFWwindow* window_arg) {
    this->renderer = renderer_arg;
    this->window = window_arg;
    ReloadabilitySystem::renderer = renderer_arg;
}

void ReloadabilitySystem::saveGame() {
    cout << "Saving game" << endl;
    json save;

    //save map
    Entity map = registry.map.entities[0];
    string id = to_string((int)map);
    save[id];
    save[id]["map"];
    for (auto room : registry.map.get(map).roomMap) {
        save[id]["map"][to_string(room.first.first) + "," + to_string(room.first.second)] = room.second;
    }

    //save player
    Entity entity = registry.players.entities[0];
    id = to_string((int)entity);
    save[id];
    save[id]["player"];
    if (registry.worldObjects.has(entity)) {
        WorldObject& worldObject = registry.worldObjects.get(entity);
        save[id]["worldObject"] = {
            {"position", {worldObject.position.x, worldObject.position.y}}
        };
    }
    if (registry.healthComponents.has(entity)) {
        Health& health = registry.healthComponents.get(entity);
        save[id]["health"] = {
            {"curr_health", health.curr_health}
        };
    }
	if (registry.roomCoords.has(entity)) {
		RoomCoordinate& room_coord = registry.roomCoords.get(entity);
		save[id]["roomCoord"] = {
			{"position", {room_coord.position.x, room_coord.position.y}}
		};
	}
    if (registry.inventory.has(entity)) {
        Inventory& inventory = registry.inventory.get(entity);
        save[id]["inventory"];
        for (ItemStat item : inventory.items) {
            save[id]["inventory"].push_back({
                {"name", item.name},
                {"type", item.type},
                {"flat_damage_mod", item.flat_damage_mod},
                {"flat_speed_mod", item.flat_speed_mod},
                {"percent_speed_mod", item.percent_speed_mod},
                {"flat_fire_rate", item.flat_fire_rate},
                {"percent_fire_rate", item.percent_fire_rate},
                {"flat_range", item.flat_range},
                {"percent_range", item.percent_range},
                {"accuracy", item.accuracy},
                {"heal_size", item.heal_size}
            });
        }
    }

    //save deadly
    for (Entity entity : registry.deadlys.entities) {
        id = to_string((int)entity);
        save[id];

        save[id]["deadly"];

        if (registry.worldObjects.has(entity)) {
            WorldObject& worldObject = registry.worldObjects.get(entity);
            save[id]["worldObject"] = {
                {"position", {worldObject.position.x, worldObject.position.y}}
            };
        }

        if (registry.motions.has(entity)) {
            Motion& motion = registry.motions.get(entity);
            save[id]["motion"] = {
                {"max_speed", motion.max_speed}
            };
        }

        if (registry.roomCoords.has(entity)) {
            RoomCoordinate& room_coord = registry.roomCoords.get(entity);
            save[id]["roomCoord"] = {
                {"position", {room_coord.position.x, room_coord.position.y}}
            };
        }

		if (registry.healthComponents.has(entity)) {
			Health& health = registry.healthComponents.get(entity);
			save[id]["health"] = {
				{"curr_health", health.curr_health}
			};
		}

		if (registry.deadlys.has(entity)) {
			Deadly& deadly = registry.deadlys.get(entity);
			save[id]["deadly"] = {
                {"type" , deadly.type}
			};
		}
    }

    // save item on floor
    for (Entity entity : registry.itemStats.entities) {
        id = to_string((int)entity);
        save[id];

        save[id]["itemStat"] = {
			{"name", registry.itemStats.get(entity).name},
			{"type", registry.itemStats.get(entity).type},
            {"flat_damage_mod", registry.itemStats.get(entity).flat_damage_mod},
            {"flat_speed_mod", registry.itemStats.get(entity).flat_speed_mod},
            {"percent_speed_mod", registry.itemStats.get(entity).percent_speed_mod},
            {"flat_fire_rate", registry.itemStats.get(entity).flat_fire_rate},
            {"percent_fire_rate", registry.itemStats.get(entity).percent_fire_rate},
            {"flat_range", registry.itemStats.get(entity).flat_range},
            {"percent_range", registry.itemStats.get(entity).percent_range},
            {"accuracy", registry.itemStats.get(entity).accuracy},
            {"heal_size", registry.itemStats.get(entity).heal_size}
        };
        if (registry.worldObjects.has(entity)) {
            WorldObject& worldObject = registry.worldObjects.get(entity);
            save[id]["worldObject"] = {
                {"position", {worldObject.position.x, worldObject.position.y}},
                {"angle", worldObject.angle},
                {"scale", {worldObject.scale.x, worldObject.scale.y}}
            };
        }
        if (registry.roomCoords.has(entity)) {
            RoomCoordinate& room_coord = registry.roomCoords.get(entity);
            save[id]["roomCoord"] = {
                {"position", {room_coord.position.x, room_coord.position.y}}
            };
        }
    }

    // save flooritem
    for (Entity entity : registry.floorItems.entities) {
        id = to_string((int)entity);
        save[id];
        save[id]["floorItem"] = {
            {"type", registry.floorItems.get(entity).type}
        };
        if (registry.worldObjects.has(entity)) {
            WorldObject& worldObject = registry.worldObjects.get(entity);
            save[id]["worldObject"] = {
                {"position", {worldObject.position.x, worldObject.position.y}},
                {"angle", worldObject.angle},
                {"scale", {worldObject.scale.x, worldObject.scale.y}}
            };
        }
        if (registry.roomCoords.has(entity)) {
            RoomCoordinate& room_coord = registry.roomCoords.get(entity);
            save[id]["roomCoord"] = {
                {"position", {room_coord.position.x, room_coord.position.y}}
            };
        }
    }

    //save file
    std::ofstream output_file(reload_path("test.json"), std::ios::out | std::ios::trunc);
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
    for (Entity entity : registry.deadlys.entities) {
        registry.remove_all_components_of(entity);
    }
    std::ifstream input_file(reload_path("test.json"));
    if (!input_file) {
        cout << "failed to open file";
        return;
    }
    json test;
    try {
        input_file >> test;
    }
    catch (const json::parse_error& e) {
        cout << "parse fail" << e.what() << std::endl;
        return;
    }
    for (auto& item : test.items()) {
        json data = item.value();
        auto key = item.key();
        if (data.contains("player")) {
            vec2 pos = { data["worldObject"]["position"][0], data["worldObject"]["position"][1] };
            int curr_health = data["health"]["curr_health"];
            ivec2 room_coord = { data["roomCoord"]["position"][0], data["roomCoord"]["position"][1] };

            auto player = createPlayer(renderer, pos, curr_health, room_coord);
            Inventory& inventory = registry.inventory.get(player);
            for (auto& item : data["inventory"]) {
                auto entity = Entity();
                ItemStat& itemstat = registry.itemStats.emplace(entity);
                itemstat = {
                    item["name"],
                    item["type"],
                    item["flat_damage_mod"],
                    item["flat_speed_mod"],
                    item["percent_speed_mod"],
                    item["flat_fire_rate"],
                    item["percent_fire_rate"],
                    item["flat_range"],
                    item["percent_range"],
                    item["accuracy"],
                    item["heal_size"]
                };
                inventory.items.push_back(itemstat);
            }

        }
        else if (data.contains("deadly")) {
            vec2 pos = { data["worldObject"]["position"][0], data["worldObject"]["position"][1] };
            float speed = data["motion"]["max_speed"];
            ivec2 room_coord = { data["roomCoord"]["position"][0], data["roomCoord"]["position"][1] };
            int curr_health = data["health"]["curr_health"];
            int type = data["deadly"]["type"];

            createEnemy(renderer, pos, speed, room_coord, curr_health, type);
        }
        else if (data.contains("floorItem")) {
            if (!data.contains("worldObject") || !data.contains("roomCoord")) {
            cout << "missing worldObject or roomCoord" << endl;
            continue;
            }
            vec2 pos = { data["worldObject"]["position"][0], data["worldObject"]["position"][1] };
            vec2 size = { data["worldObject"]["scale"][0], data["worldObject"]["scale"][1] };
            float angle = data["worldObject"]["angle"];
            ivec2 room_coord = { data["roomCoord"]["position"][0], data["roomCoord"]["position"][1] };
            createWall(renderer, pos, size, angle, data["floorItem"]["type"], room_coord);
        }
        else if (data.contains("itemStat")) {
            if (!data.contains("worldObject") || !data.contains("roomCoord")) {
                cout << "missing worldObject or roomCoord" << endl;
                continue;
            }
            ItemStat itemstat;
            auto dat = data["itemStat"];
            itemstat = {
                dat["name"],
                dat["type"],
                dat["flat_damage_mod"],
                dat["flat_speed_mod"],
                dat["percent_speed_mod"],
                dat["flat_fire_rate"],
                dat["percent_fire_rate"],
                dat["flat_range"],
                dat["percent_range"],
                dat["accuracy"],
                dat["heal_size"]
            };
            vec2 pos = { data["worldObject"]["position"][0], data["worldObject"]["position"][1] };
            vec2 size = { data["worldObject"]["scale"][0], data["worldObject"]["scale"][1] };
            ivec2 room_coord = { data["roomCoord"]["position"][0], data["roomCoord"]["position"][1] };
            std::uniform_real_distribution<float> uniform_dist;
            std::default_random_engine rng = std::default_random_engine(std::random_device()());
            //createItem(renderer, pos, size, uniform_dist, rng, room_coord, &itemstat);
        }
        else if (data.contains("map")) {
            auto entity = registry.map.entities[0];
            std::map<std::pair<int, int>, ROOM_TYPE>& roomMap = registry.map.get(entity).roomMap;
            for (auto& item : data["map"].items()) {
                auto key = item.key();
                roomMap[{stoi(key.substr(0, key.find(','))), stoi(key.substr(key.find(',') + 1))}] = item.value();
            }
        }
    }
}
