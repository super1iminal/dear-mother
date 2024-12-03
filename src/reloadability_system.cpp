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
    save[id]["player"] = {
        {"combat_state", registry.players.get(entity).combat_state},
        {"boss_one_dead", registry.players.get(entity).boss_one_dead},
        {"boss_two_beat", registry.players.get(entity).boss_two_beat},
        {"boss_three_beat", registry.players.get(entity).boss_three_beat},
        {"scrap", registry.players.get(entity).scrap},
		{"stage", registry.players.get(entity).stage}
    };
    if (registry.worldObjects.has(entity)) {
        WorldObject& worldObject = registry.worldObjects.get(entity);
        save[id]["worldObject"] = {
            {"position", {worldObject.position.x, worldObject.position.y}}
        };
    }
    if (registry.healthComponents.has(entity)) {
        Health& health = registry.healthComponents.get(entity);
        save[id]["health"] = {
            {"curr_health", health.curr_health},
			{"max_health", health.max_health}
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
        for (auto itemPair : inventory.items) {
            auto item = itemPair.second;
            save[id]["inventory"][to_string(itemPair.first)] ={
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
            };
        }
    }

    //save deadly
    for (Entity entity : registry.deadlys.entities) {
        id = to_string((int)entity);
        Deadly& deadly = registry.deadlys.get(entity);
        save[id];
        save[id]["deadly"];
        save[id]["deadly"] = {
                {"type" , deadly.type},
            {"immune", deadly.immune},
            {"attacking", deadly.attacking}
			};
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
				{"curr_health", health.curr_health},
				{"max_health", health.max_health}
			};
		}

		if (registry.bossOnes.has(entity)) {
		    BossOne& boss = registry.bossOnes.get(entity);
		    cout << "boss one" << endl;
		    save[id]["bossOne"] = {
		        {"BOSS_ONE_POS", boss.boss_pos},
		        {"BOSS_ONE_STATE", boss.boss_state},
		        {"top_left_alive", boss.top_left_alive},
                {"top_right_alive", boss.top_right_alive},
                {"bot_left_alive", boss.bot_left_alive},
                {"bot_right_alive", boss.bot_right_alive},
                {"shot_pattern", boss.shot_pattern},
                {"mother", boss.mother},
		        {"bullet_angle", boss.bullet_angle}
		    };
		}
    }

    // save item on floor
    for (Entity entity : registry.itemStats.entities) {
        if (registry.lifetimes.has(entity)) {
            continue;
        }
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


bool ReloadabilitySystem::loadGame() {
    std::ifstream validity_file(reload_path("save_one_valid.json"));
    if (!validity_file) {
        cout << "failed to open validity file";
        return false;
    }
    json valid;
    try {
        validity_file >> valid;
    }
    catch (const json::parse_error& e) {
        cout << "parse fail (validity): " << e.what() << std::endl;
        return false;
    }

    if (valid["valid"] == 0) {
        cout << "save file not valid." << endl;
        return false;
    }

    while (registry.deadlys.entities.size() > 0) {
        registry.remove_all_components_of(registry.deadlys.entities[0]);
    }

    std::ifstream input_file(reload_path("test.json"));
    if (!input_file) {
        cout << "failed to open file";
        return false;
    }
    json test;
    try {
        input_file >> test;
    }
    catch (const json::parse_error& e) {
        cout << "parse fail" << e.what() << std::endl;
        return false;
    }
    for (auto& item : test.items()) {
        json data = item.value();
        auto key = item.key();
        if (data.contains("player")) {
            vec2 pos = { data["worldObject"]["position"][0], data["worldObject"]["position"][1] };
            int curr_health = data["health"]["curr_health"];
            int max_health = data["health"]["max_health"];
            ivec2 room_coord = { data["roomCoord"]["position"][0], data["roomCoord"]["position"][1] };

            auto player = createPlayer(renderer, pos, curr_health, room_coord);
            registry.players.components[0].combat_state = data["player"]["combat_state"];
            registry.players.components[0].boss_one_dead = data["player"]["boss_one_dead"];
            registry.players.components[0].boss_two_beat = data["player"]["boss_two_beat"];
            registry.players.components[0].boss_three_beat = data["player"]["boss_three_beat"];
            registry.players.components[0].scrap = data["player"]["scrap"];
			registry.players.components[0].stage = data["player"]["stage"];
            registry.healthComponents.get(player).max_health = max_health;
            Inventory& inventory = registry.inventory.get(player);

            for (auto& itemPair : data["inventory"].items()) {
                auto entity = Entity();
                ItemStat& itemstat = registry.itemStats.emplace(entity);
                auto idx = stoi(itemPair.key());
                auto item = itemPair.value();
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
                inventory.items[idx] = itemstat;
            }

        }
        else if (data.contains("deadly")) {
            vec2 pos = { data["worldObject"]["position"][0], data["worldObject"]["position"][1] };
            float speed = data["motion"]["max_speed"];
            ivec2 room_coord = { data["roomCoord"]["position"][0], data["roomCoord"]["position"][1] };
            int curr_health = data["health"]["curr_health"];
            int max_health = data["health"]["max_health"];
            int type = data["deadly"]["type"];
            if (data.contains("bossOne")) {
                BOSS_ONE_POS boss_pos = data["bossOne"]["BOSS_ONE_POS"];
                Entity entity = createBossOne(renderer, pos, boss_pos, room_coord, false);
                BossOne& boss = registry.bossOnes.get(entity);
                boss.boss_state = data["bossOne"]["BOSS_ONE_STATE"];
                boss.top_left_alive = data["bossOne"]["top_left_alive"];
                boss.top_right_alive = data["bossOne"]["top_right_alive"];
                boss.bot_left_alive = data["bossOne"]["bot_left_alive"];
                boss.bot_right_alive = data["bossOne"]["bot_right_alive"];
                boss.shot_pattern = data["bossOne"]["shot_pattern"];
                boss.mother = data["bossOne"]["mother"];
                boss.bullet_angle = data["bossOne"]["bullet_angle"];
                Deadly& deadly = registry.deadlys.get(entity);
                deadly.immune = data["deadly"]["immune"];
                deadly.attacking = data["deadly"]["attacking"];
            }
            else {
                Entity enemy = createEnemy(renderer, pos, speed, room_coord, curr_health, type);
                registry.healthComponents.get(enemy).max_health = max_health;
            }

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
            createItem(renderer, pos, size, uniform_dist, rng, room_coord,ITEM_TYPE::RANDOM, &itemstat);
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

    return true;

    // close input file?
}

void ReloadabilitySystem::setSaveValidity(bool validity) {
    std::ifstream validity_file(reload_path("save_one_valid.json"));
    if (!validity_file) {
        cout << "failed to open validity file";
        return;
    }
    json valid;
    try {
        validity_file >> valid;
    }
    catch (const json::parse_error& e) {
        cout << "parse fail (validity): " << e.what() << std::endl;
        return;
    }
    
    valid["valid"] = static_cast<int>(validity);

    std::ofstream output_file(reload_path("save_one_valid.json"));
    output_file << valid;
}


void ReloadabilitySystem::recordPlayerDeathRoom() {
    Entity entity = registry.players.entities[0];
    if (registry.deathTimers.has(entity)) {
        return;
    }
    std::ifstream input_file(reload_path("death_record.json"));
    if (!input_file) {
        cout << "failed to open file";
        return;
    }
    json save;
    try {
        input_file >> save;
    }
    catch (const json::parse_error& e) {
        cout << "parse fail" << e.what() << std::endl;
        save = json();
    }
    int idx = 0;
    while (save.contains(to_string(idx))) {
        idx++;
    }


    ivec2 room_coord = registry.roomCoords.get(entity).position;
    string id = to_string((int)entity);
    string index = to_string(idx);
    save[index];
    save[index][id]["player"];
    save[index][id]["worldObject"] = {
        {"position", {registry.worldObjects.get(entity).position.x, registry.worldObjects.get(entity).position.y}}
    };

    for (Entity entity : registry.itemStats.entities) {
        if (!registry.roomCoords.has(entity)) {
            continue;
        }

        if (registry.roomCoords.get(entity).position != room_coord) {
            continue;
        }

        string id = to_string((int)entity);
        save[index][id];
        save[index][id]["itemStat"] = {
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
            save[index][id]["worldObject"] = {
                {"position", {worldObject.position.x, worldObject.position.y}},
                {"angle", worldObject.angle},
                {"scale", {worldObject.scale.x, worldObject.scale.y}}
            };
        }

    }


    for (Entity entity : registry.floorItems.entities) {

        if (!registry.roomCoords.has(entity)) {
        continue;
        }
        if (registry.roomCoords.get(entity).position != room_coord) {
            continue;
        }
        id = to_string((int)entity);
        save[index][id];
        save[index][id]["floorItem"] = {
            {"type", registry.floorItems.get(entity).type}
        };
        if (registry.worldObjects.has(entity)) {
            WorldObject& worldObject = registry.worldObjects.get(entity);
            save[index][id]["worldObject"] = {
                {"position", {worldObject.position.x, worldObject.position.y}},
                {"angle", worldObject.angle},
                {"scale", {worldObject.scale.x, worldObject.scale.y}}
            };
        }
    }

    std::ofstream output_file(reload_path("death_record.json"), std::ios::out | std::ios::trunc);
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
