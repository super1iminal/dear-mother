#include "world_init.hpp"
#include "tiny_ecs_registry.hpp"

#include <iostream>
#include <ui_system.hpp>
#include <world_system.hpp>

// WHEN ADDING A CREATE FUNCTION:
// 1. Add the function prototype to world_init.hpp
// 2. Add the function definition to world_init.cpp
// 3. Add to gameSceneComponents
// 4. Add meshPtrs
// 5. Add the position of the room it's in (specified in parameters)
// 6. Assign a component (wall/player/enemy/etc) to the entity
// 7. Add to renderRequests ordering
// 



// ==================== CREATE FUNCTIONS ====================

Entity createParticle(RenderSystem* renderer, vec2 pos, std::uniform_real_distribution<float> uniform_dist, std::default_random_engine& rng, TEXTURE_ASSET_ID type, ivec2 room_coord) {
	auto entity = Entity();
	registry.gameSceneComponents.emplace(entity);
	registry.roomCoords.emplace(entity, room_coord);
	registry.activeComponents.emplace(entity);

	// Store a reference to the potentially re-used mesh object
	// Adding meshptr component
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// adding particles component
	registry.particles.emplace(entity);

	//float mass_size_rand = uniform_dist(rng);

	// setting initial position, scale, and orientation values
	WorldObject& worldobject = registry.worldObjects.emplace(entity);
	worldobject.position = pos;
	worldobject.angle = uniform_dist(rng) * 2 * M_PI;
	float size = uniform_dist(rng) * MAX_PARTICLE_SIZE + PARTICLE_SIZE_OFFSET;
	worldobject.scale = vec2({ size, size });

	// adding motion component
	Motion& motion = registry.motions.emplace(entity);
	float angle = uniform_dist(rng) * 2 * M_PI;
	float speed = uniform_dist(rng) * MAX_PARTICLE_SPEED + PARTICLE_SPEED_OFFSET;
	speed = speed == 0.f ? 0.02f : speed;
	motion.velocity = v_from_sa(speed, angle);
	motion.mass = MAX_PARTICLE_MASS;
	motion.acceleration = { 0.0, 0.0 };
	
	// adding friction component CALCULATING AIR FRICTION
	Friction& friction = registry.frictions.emplace(entity);
	float velocity_magnitude_sq = glm::dot(motion.velocity, motion.velocity);

	friction.force = 0.5f * M_RHO * velocity_magnitude_sq * PARTICLE_DRAG_COEF;	
	// equation from https://www.ck12.org/flexi/physics/uniform-acceleration/how-can-air-resistance-be-calculated-using-mass-and-acceleration/#:~:text=It%20is%20typically%20calculated%20using,is%20the%20cross%2Dsectional%20area.

	// adding lifetimne component
	Lifetime& lifetime = registry.lifetimes.emplace(entity);
	//lifetime.time_remaining_ms = uniform_dist(rng) * MAX_PARTICLE_LIFETIME;
	lifetime.time_remaining_ms = (((1.0f / VELOCITY_THRESHOLD) - (1.0f / speed)) * (2.0f * motion.mass) / (M_RHO * PARTICLE_DRAG_COEF)) + 500;

	// adding renderRequest component
	registry.renderRequests.insert_sorted(
		entity,
		{ type,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_ORDER::PARTICLE
		});

	return entity;
}

// NOTE: when creating a wall, then angle represents the normal. it is necessary for collision handling
Entity createWall(RenderSystem* renderer, vec2 pos, vec2 size, float angle, TEXTURE_ASSET_ID type, ivec2 room_coord) {
	auto entity = Entity();
	registry.gameSceneComponents.emplace(entity);
	registry.roomCoords.emplace(entity, room_coord);
	registry.activeComponents.emplace(entity);

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);
	if (type == TEXTURE_ASSET_ID::VERT_WALL || type == TEXTURE_ASSET_ID::HORZ_WALL
		|| type == TEXTURE_ASSET_ID::BOSS_ONE_VERT_WALL || type == TEXTURE_ASSET_ID::BOSS_ONE_HORZ_WALL) {
		registry.walls.emplace(entity);
	}
	else {
		FloorItem& floor = registry.floorItems.emplace(entity);
		floor.type = type;
	}

	// Setting initial position, scale, and orientation values
	WorldObject& worldobject = registry.worldObjects.emplace(entity);
	worldobject.position = pos;
	worldobject.angle = angle;
	worldobject.scale = size;

	registry.blockers.emplace(entity);
	// don't be fooled, type is type
	registry.renderRequests.insert_sorted(
		entity,
		{ type,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_ORDER::WALL });

	return entity;

}

Entity createPlayer(
	RenderSystem* renderer,
	vec2 pos,
	int curr_health,
	ivec2 room_coord
)
{
	Entity entity = Entity();
	
	registry.gameSceneComponents.emplace(entity);
	registry.roomCoords.emplace(entity, room_coord);
	registry.activeComponents.emplace(entity);

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::GUY);
	registry.meshFlags.emplace(entity);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion value
	Motion& motion = registry.motions.emplace(entity);
	motion.max_speed = PLAYER_MAX_SPEED;
	motion.velocity = { 0.f, 0.f };
	motion.acceleration = { 0.f, 0.f };

	// setting position, scale, orientation
	WorldObject& worldobject = registry.worldObjects.emplace(entity);
	worldobject.position = pos;
	worldobject.angle = 0.f;
	worldobject.scale = vec2(mesh.original_size.x * 4.f * PLAYER_SIZE, mesh.original_size.y * 4.f * PLAYER_SIZE);

	// create an empty Player component for our character
	registry.players.emplace(entity);
	auto& shooter = registry.shooters.emplace(entity);
	shooter.fire_rate = 500.0f;
	auto& health = registry.healthComponents.emplace(entity);
	health.max_health = PLAYER_MAX_HEALTH;
	health.curr_health = curr_health;

	registry.inventory.emplace(entity);
	registry.modifiers.emplace(entity);
	Animation& player_animation = registry.animations.emplace(entity);
	player_animation.cols = 4;
	player_animation.rows = 1;
	player_animation.frames = 1;
	player_animation.current_frame = 0;
	player_animation.time_since_last_frame = 0;

	
	registry.renderRequests.insert_sorted(
		entity,
		{ TEXTURE_ASSET_ID::PLAYER_WALK,
			EFFECT_ASSET_ID::ANIM,
			GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_ORDER::PLAYER });
	
	/* MESH
	registry.renderRequests.insert_sorted(
		entity,
		{ TEXTURE_ASSET_ID::TEXTURE_COUNT,
			EFFECT_ASSET_ID::SALMON,
			GEOMETRY_BUFFER_ID::GUY,
			RENDER_ORDER::PLAYER });
	*/
	return entity;
}

Entity createEnemy(
	RenderSystem* renderer, 
	vec2 position, 
	float speed, 
	ivec2 room_coord,
	int curr_health,
	int type
)
{
	auto entity = Entity();
	registry.gameSceneComponents.emplace(entity);
	registry.roomCoords.emplace(entity, room_coord);
	registry.activeComponents.emplace(entity);

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the motion
	auto& motion = registry.motions.emplace(entity);
	motion.max_speed = speed;
	if (type == HEAVY_TYPE || type == FLY_TYPE)
		motion.max_speed = ENEMY_SPEED * .6;
	motion.velocity = { 0.f, 0.f };
	motion.acceleration = { 0.f, 0.f };

	// create an empty Enemy component to be able to refer to all enemies
	auto& deadly = registry.deadlys.emplace(entity);
	deadly.t = std::chrono::high_resolution_clock::now();
	deadly.t_patrol = std::chrono::high_resolution_clock::now();
  
	deadly.immune = false;

	deadly.type = type;  // 0 for grey melee, 1 for slower yellow projectile, 4 for heavy, 5 for flying

	if (registry.deadlys.get(entity).type == 1) {
		registry.motions.get(entity).max_speed = 0.7 * speed;
		auto& shooter = registry.shooters.emplace(entity);
		shooter.fire_rate = std::numeric_limits<int>::max();
	}
  	auto& health = registry.healthComponents.emplace(entity);
	health.max_health = curr_health;
	health.curr_health = curr_health;

	// setting position, scale, orientation
	WorldObject& worldobject = registry.worldObjects.emplace(entity);
	worldobject.position = position;
	worldobject.angle = 0.f;
	worldobject.scale = vec2({ -ENEMY_BB_WIDTH, ENEMY_BB_HEIGHT });
	if (type == 4)
		worldobject.scale = vec2({ -4.5 / 2.f * ENEMY_BB_WIDTH, 1.5 * ENEMY_BB_HEIGHT });


	Animation& enemy_animation = registry.animations.emplace(entity);
	enemy_animation.cols = 4;
	enemy_animation.rows = 1;
	enemy_animation.frames = 1;
	enemy_animation.current_frame = 0;
	
	if (registry.deadlys.get(entity).type == 0) {
		registry.renderRequests.insert_sorted(
			entity,
			{ TEXTURE_ASSET_ID::ENEMY_WALK,
				EFFECT_ASSET_ID::ANIM,
				GEOMETRY_BUFFER_ID::SPRITE,
				RENDER_ORDER::ENEMY });
	}
	else if (registry.deadlys.get(entity).type == HEAVY_TYPE) {
		registry.renderRequests.insert_sorted(
			entity,
			{ TEXTURE_ASSET_ID::HEAVY_WALK,
				EFFECT_ASSET_ID::ANIM,
				GEOMETRY_BUFFER_ID::SPRITE,
				RENDER_ORDER::ENEMY });
	}
	else if (registry.deadlys.get(entity).type == FLY_TYPE) {
		registry.renderRequests.insert_sorted(
			entity,
			{ TEXTURE_ASSET_ID::ENEMY_FLY_WALK,
			EFFECT_ASSET_ID::ANIM,
			GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_ORDER::ENEMY });
	}
	else {
		registry.renderRequests.insert_sorted(
			entity,
			{ TEXTURE_ASSET_ID::ENEMY_2_WALK,
				EFFECT_ASSET_ID::ANIM,
				GEOMETRY_BUFFER_ID::SPRITE,
				RENDER_ORDER::ENEMY });
	}

	return entity;
}

Entity createFloorText(RenderSystem* renderer, std::string text, vec2 pos, vec2 scale, ivec2 room_coord) {
	// Store a reference to the potentially re-used mesh object
	Entity entity = Entity();
	registry.gameSceneComponents.emplace(entity);
	registry.activeComponents.emplace(entity);
	registry.roomCoords.emplace(entity, room_coord);

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SQUARE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial position, scale, and orientation values
	WorldObject& worldobject = registry.worldObjects.emplace(entity);
	worldobject.position = pos;
	worldobject.scale = scale;

	vec3& text_color = registry.colors.emplace(entity);
	text_color = vec3(1.f, 0.f, 0.f);

	registry.floorTexts.emplace(entity).text = text;

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::TEXTURE_COUNT,
			EFFECT_ASSET_ID::FLOOR_TEXT,
			GEOMETRY_BUFFER_ID::SQUARE });

	return entity;
}

Entity createBossOne(RenderSystem* renderer, vec2 pos, BOSS_ONE_POS boss_pos, ivec2 room_coord) {
	auto entity = Entity();
	registry.gameSceneComponents.emplace(entity);
	registry.roomCoords.emplace(entity, room_coord);
	registry.activeComponents.emplace(entity);

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the motion
	auto& motion = registry.motions.emplace(entity);

	WorldObject& worldobject = registry.worldObjects.emplace(entity);
	worldobject.position = pos;
	worldobject.angle = 0.f;

	auto& health = registry.healthComponents.emplace(entity);
	health.max_health = 20;
	health.curr_health = 10;

	auto& deadly = registry.deadlys.emplace(entity);
	deadly.t = std::chrono::high_resolution_clock::now();
	deadly.t_patrol = std::chrono::high_resolution_clock::now();

	// Manage boss states
	registry.bossOnes.emplace(entity).boss_pos = boss_pos;
	
	registry.shooters.emplace(entity).fire_rate = 1500.f;

	Animation& enemy_animation = registry.animations.emplace(entity);
	enemy_animation.cols = 4;
	enemy_animation.rows = 1;
	enemy_animation.frames = 1;
	enemy_animation.current_frame = 0;
	
	if (boss_pos == BOSS_ONE_POS::MOTHER) {
		deadly.type = BOSS_ONE;
		deadly.immune = true;
		health.curr_health = 20;
		worldobject.scale = vec2({ 450, 300 });
		registry.renderRequests.insert(
			entity,
			{ TEXTURE_ASSET_ID::MOTHER_FINAL_IDLE,
				EFFECT_ASSET_ID::ANIM,
				GEOMETRY_BUFFER_ID::SPRITE });
	}
	else {
		deadly.type = 2;
		deadly.immune = false;
		worldobject.scale = vec2({ ENEMY_BB_WIDTH, ENEMY_BB_HEIGHT });
		registry.renderRequests.insert(
			entity,
			{ TEXTURE_ASSET_ID::ENEMY_WALK,
				EFFECT_ASSET_ID::ANIM,
				GEOMETRY_BUFFER_ID::SPRITE });
	}

	return entity;
}

Entity createBossTwo(RenderSystem* renderer, vec2 pos, ivec2 room_coord) {
	auto entity = Entity();
	registry.gameSceneComponents.emplace(entity);
	registry.roomCoords.emplace(entity, room_coord);
	registry.activeComponents.emplace(entity);

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial position, scale, and orientation values
	WorldObject& worldobject = registry.worldObjects.emplace(entity);
	worldobject.position = pos;
	worldobject.angle = 0;
	worldobject.scale = { window_width_px, WALL_WIDTH * 3 };

	registry.bossTwos.emplace(entity).curr_wave = BOSS_TWO_WAVE::WAVE_ONE;

	registry.walls.emplace(entity);

	Animation& enemy_animation = registry.animations.emplace(entity);
	enemy_animation.cols = 20;
	enemy_animation.rows = 1;
	enemy_animation.frames = 20;
	enemy_animation.current_frame = 0;

	registry.blockers.emplace(entity);
	// don't be fooled, type is type
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::BOSS_ONE_IDLE,
			EFFECT_ASSET_ID::ANIM,
			GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

Entity createBossThree(RenderSystem* renderer, vec2 pos, ivec2 room_coord) {
	auto entity = Entity();
	registry.gameSceneComponents.emplace(entity);
	registry.roomCoords.emplace(entity, room_coord);
	registry.activeComponents.emplace(entity);

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial position, scale, and orientation values
	WorldObject& worldobject = registry.worldObjects.emplace(entity);
	worldobject.position = pos;
	worldobject.angle = 0;
	worldobject.scale = { 175 + 125, 119 + 97};

	auto& motion = registry.motions.emplace(entity);
	motion.max_speed = 100.f; // Slow it down maybe
	motion.velocity = { 0.f, 0.f };
	motion.acceleration = { 0.f, 0.f };

	registry.bossThrees.emplace(entity).boss_phase = BOSS_THREE_PHASE::PHASE_ONE;

	auto& health = registry.healthComponents.emplace(entity);
	health.max_health = 60;
	health.curr_health = 60;

	auto& deadly = registry.deadlys.emplace(entity);
	deadly.t = std::chrono::high_resolution_clock::now();
	deadly.t_patrol = std::chrono::high_resolution_clock::now();
	deadly.type = BOSS_THREE;
	deadly.immune = false;

	auto& shooter = registry.shooters.emplace(entity);
	shooter.fire_rate = 1000;

	Animation& enemy_animation = registry.animations.emplace(entity);
	enemy_animation.cols = 4;
	enemy_animation.rows = 1;
	enemy_animation.frames = 4;
	enemy_animation.current_frame = 0;

	// don't be fooled, type is type
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::BOSS_TWO_WALK,
			EFFECT_ASSET_ID::ANIM,
			GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

Entity createFloor(RenderSystem* renderer, vec2 position, vec2 size, ivec2 room_coord, FLOOR_TYPE floor_type) {
	// create an entity in order to render the floor background
	auto floor = Entity();
	registry.gameSceneComponents.emplace(floor);
	registry.roomCoords.emplace(floor, room_coord);
	registry.activeComponents.emplace(floor);

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(floor, &mesh);
	registry.floors.emplace(floor);

	// Setting initial position, scale, and orientation values
	WorldObject& worldobject = registry.worldObjects.emplace(floor);
	worldobject.position = position;
	worldobject.angle = 0.f;
	worldobject.scale = size;

	if (floor_type == FLOOR_TYPE::DEFAULT) {
		registry.renderRequests.insert_sorted(
			floor,
			{ TEXTURE_ASSET_ID::FLOOR,
				EFFECT_ASSET_ID::TEXTURED,
				GEOMETRY_BUFFER_ID::SPRITE,
				RENDER_ORDER::FLOOR });
	}
	else if (floor_type == FLOOR_TYPE::BOSS_ROOM_ONE) {
		registry.renderRequests.insert_sorted(
			floor,
			{ TEXTURE_ASSET_ID::BOSS_ONE_FLOOR,
				EFFECT_ASSET_ID::TEXTURED,
				GEOMETRY_BUFFER_ID::SPRITE,
				RENDER_ORDER::FLOOR });
	}

	return floor;
}

// need to add to collisions
Entity createDoor(RenderSystem* renderer, ivec2 room_coord, ivec2 leads_to, DIRECTION orientation) {
	// create an entity in order to render the floor background
	printf("creating door at %d, %d\n", room_coord.x, room_coord.y);
	auto door = Entity();
	registry.gameSceneComponents.emplace(door);
	registry.roomCoords.emplace(door, room_coord);
	registry.activeComponents.emplace(door);

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(door, &mesh);
	registry.doors.emplace(door, leads_to, orientation);

	// Setting initial position, scale, and orientation values
	WorldObject& worldobject = registry.worldObjects.emplace(door);
	TEXTURE_ASSET_ID texture_id = TEXTURE_ASSET_ID::DOOR_LEFT_RIGHT;
	switch (orientation) {
	case DIRECTION::UP:
		worldobject.position = { window_width_px/2.f, BASE_UI_HEIGHT + ((WALL_WIDTH / 2.f) * 1.25f) };
		worldobject.angle = 0.f;
		worldobject.scale = vec2({ 255.f, WALL_WIDTH + 6 });
		texture_id = TEXTURE_ASSET_ID::DOOR_UP_DOWN;
		break;
	case DIRECTION::DOWN:
		worldobject.position = { window_width_px / 2.f, window_height_px - ((WALL_WIDTH / 2.f) * 1.25f) };
		worldobject.angle = M_PI;
		worldobject.scale = vec2({ 255.f, WALL_WIDTH + 6 });
		texture_id = TEXTURE_ASSET_ID::DOOR_UP_DOWN;
		break;
	case DIRECTION::LEFT:
		worldobject.position = { ((WALL_WIDTH / 2.f) * 1.25f), (window_height_px-BASE_UI_HEIGHT)/2.f + BASE_UI_HEIGHT};
		worldobject.angle = 0.f;
		worldobject.scale = vec2({ WALL_WIDTH + 6, 255.f });
		break;
	case DIRECTION::RIGHT:
		worldobject.position = { window_width_px - ((WALL_WIDTH / 2.f) * 1.25f), (window_height_px - BASE_UI_HEIGHT) / 2.f + BASE_UI_HEIGHT };
		worldobject.angle = M_PI;
		worldobject.scale = vec2({ WALL_WIDTH + 6, 255.f });
		break;
	}

	registry.renderRequests.insert_sorted(
		door,
		{ texture_id,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_ORDER::DOOR });

	return door;
}

Entity createInteractable(RenderSystem* renderer, vec2 position, vec2 size, std::function<bool(int, Entity)> function, int value, ivec2 room_coord) {
	// create an interactable entity
	Entity interactable_entity = Entity();
	registry.gameSceneComponents.emplace(interactable_entity);
	registry.roomCoords.emplace(interactable_entity, room_coord);
	registry.activeComponents.emplace(interactable_entity);

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(interactable_entity, &mesh);

	Interactable& interactable = registry.interactables.emplace(interactable_entity);
	interactable.range = 50.f;
	interactable.interaction = function;
	interactable.value = value;

	// Setting initial position, scale, and orientation values
	WorldObject& interactable_object = registry.worldObjects.emplace(interactable_entity);
	interactable_object.position = position;
	interactable_object.angle = 0.f;
	interactable_object.scale = size;

	registry.renderRequests.insert_sorted(
		interactable_entity,
		{ TEXTURE_ASSET_ID::BATTERY_PACK,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_ORDER::INTERACTABLE });

	return interactable_entity;
}



Entity createSparkles(RenderSystem* renderer, vec2 position, vec2 size, ivec2 room_coord) {
	Entity entity = Entity();

	registry.gameSceneComponents.emplace(entity);
	registry.roomCoords.emplace(entity, room_coord);
	registry.activeComponents.emplace(entity);

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	WorldObject& sparkle_world_object = registry.worldObjects.emplace(entity);
	sparkle_world_object.position = position;
	sparkle_world_object.angle = 0.f;
	sparkle_world_object.scale = size;

	Animation& sparkle_animation = registry.animations.emplace(entity);
	sparkle_animation.cols = 4;
	sparkle_animation.rows = 1;
	sparkle_animation.frames = 4;
	sparkle_animation.current_frame = 0;
	sparkle_animation.time_since_last_frame = 0;

	registry.renderRequests.insert_sorted(
		entity,
		{ TEXTURE_ASSET_ID::ITEM_SPARKLES,
			EFFECT_ASSET_ID::ANIM,
			GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_ORDER::PARTICLE });

	return entity;
}

Entity createItem(RenderSystem* renderer, vec2 position, vec2 size, std::uniform_real_distribution<float> uniform_dist, std::default_random_engine& rng, ivec2 room_coord,ITEM_TYPE spec_type, ItemStat* loaded_item) {

	// create an interactable entity
	Entity entity = Entity();
	registry.gameSceneComponents.emplace(entity);
	registry.roomCoords.emplace(entity, room_coord);
	registry.activeComponents.emplace(entity);

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	ItemStat& item = registry.itemStats.emplace(entity);

	if (loaded_item == nullptr) {
		int roll_type = uniform_dist(rng) * 100;

	if (spec_type == ITEM_TYPE::HEALTH_PACK) {
		roll_type = 1;
	}
	else if (spec_type == ITEM_TYPE::RANGE) {
		roll_type = 21;
	}
	else if (spec_type == ITEM_TYPE::FIRE_RATE) {
		roll_type = 41;
	}
	else if (spec_type == ITEM_TYPE::SPEED) {
		roll_type = 61;
	}
	else if (spec_type == ITEM_TYPE::DAMAGE) {
		roll_type = 81;
	}

	int roll_item;
	if (roll_type > 80) {
		roll_item = (rand() % registry.damage_items.size());
		item = registry.damage_items.at(roll_item);
	}
	else if (roll_type > 60) {
		roll_item = (rand() % registry.speed_items.size());
		item = registry.speed_items.at(roll_item);
	}
	else if (roll_type > 40) {
		roll_item = (rand() % registry.fire_rate_items.size());
		item = registry.fire_rate_items.at(roll_item);
	}
	else if (roll_type > 20) {
		roll_item = (rand() % registry.range_items.size());
		item = registry.range_items.at(roll_item);
	}
	else {
		roll_item = (rand() % registry.healing_items.size());
		item = registry.healing_items.at(roll_item);
	}
	} else {
		item = *loaded_item;
	}


	Interactable& interactable = registry.interactables.emplace(entity);
	interactable.range = 50.f;

	// Setting initial position, scale, and orientation values
	WorldObject& interactable_object = registry.worldObjects.emplace(entity);
	interactable_object.position = position;
	interactable_object.angle = 0.f;
	interactable_object.scale = size;

	TEXTURE_ASSET_ID item_texture = TEXTURE_ASSET_ID::BATTERY_PACK;
	switch (item.name)
	{
	case ITEM_NAME::BATTERY_PACK:
		item_texture = TEXTURE_ASSET_ID::BATTERY_PACK;
		break;
	case ITEM_NAME::SHATTERED_QUARTZ:
		item_texture = TEXTURE_ASSET_ID::SHATTERED_QUARTZ;
		break;
	case ITEM_NAME::REPEATER:
		item_texture = TEXTURE_ASSET_ID::REPEATER;
		break;
	case ITEM_NAME::CREAKY_WHEEL:
		item_texture = TEXTURE_ASSET_ID::CREAKY_WHEEL;
		break;
	case ITEM_NAME::HEATSINK:
		item_texture = TEXTURE_ASSET_ID::HEATSINK;
		break;
	case ITEM_NAME::WD4000:
		item_texture = TEXTURE_ASSET_ID::WD4000;
		break;
	case ITEM_NAME::VOLITILE_BLASTER:
		item_texture = TEXTURE_ASSET_ID::VOLITILE_BLASTER;
		break;
	case ITEM_NAME::OPTICAL_SENSOR:
		item_texture = TEXTURE_ASSET_ID::OPTICAL_SENSOR;
		break;
	case ITEM_NAME::HOT_DIESEL:
		item_texture = TEXTURE_ASSET_ID::HOT_DIESEL;
		break;
	case ITEM_NAME::SUPERCHARGED_BATTERY_PACK:
		item_texture = TEXTURE_ASSET_ID::SUPERCHARGED_BATTERY_PACK;
		break;
	}

	registry.renderRequests.insert_sorted(
		entity,
		{ item_texture,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_ORDER::ITEM });

	item.particles = createSparkles(renderer, position, size, room_coord);

	return entity;
}

Entity createProjectile(RenderSystem* renderer, vec2 pos, float angle, float speed, bool is_friendly, ivec2 room_coord, int lifespan)
{
	auto entity = Entity();
	registry.gameSceneComponents.emplace(entity);
	registry.roomCoords.emplace(entity, room_coord);
	registry.activeComponents.emplace(entity);

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::BULLET_ENEMY);
	registry.meshFlags.emplace(entity);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion values
	Motion& motion = registry.motions.emplace(entity);
	motion.velocity = v_from_sa(speed, angle);
	motion.acceleration = { 0.f, 0.f };

	// Set position, angle, scale
	WorldObject& worldObject = registry.worldObjects.emplace(entity);
	worldObject.position = pos;
	worldObject.angle = angle;
	worldObject.scale = mesh.original_size * 30.f;
	worldObject.scale.x *= -1; // point front to the right

	Lifetime& lifetime = registry.lifetimes.emplace(entity);
	lifetime.time_remaining_ms = lifespan;

	Projectile& projectile = registry.projectiles.emplace(entity);
	projectile.friendly = is_friendly;
	if (is_friendly) 
	{
		registry.renderRequests.insert_sorted
		(
			entity,
			{ TEXTURE_ASSET_ID::TEXTURE_COUNT,
				EFFECT_ASSET_ID::SALMON,
				GEOMETRY_BUFFER_ID::BULLET_FRIENDLY,
				RENDER_ORDER::FRIENDLY_PROJECTILE
			}
		);
	}
	else 
	{
		registry.renderRequests.insert_sorted
		(
			entity,
			{ TEXTURE_ASSET_ID::TEXTURE_COUNT,
				EFFECT_ASSET_ID::SALMON,
				GEOMETRY_BUFFER_ID::BULLET_ENEMY,
				RENDER_ORDER::DEADLY_PROJECTILE
			}
		);
	}


	return entity;
}

Entity createLine(vec2 position, vec2 scale)
{
	Entity entity = Entity();
	registry.gameSceneComponents.emplace(entity);
	registry.activeComponents.emplace(entity);

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	registry.renderRequests.insert_sorted(
		entity, {
			TEXTURE_ASSET_ID::TEXTURE_COUNT,
			EFFECT_ASSET_ID::EGG,
			GEOMETRY_BUFFER_ID::DEBUG_LINE,
			RENDER_ORDER::LINE
		});

	// Create motion
	Motion& motion = registry.motions.emplace(entity);
	motion.velocity = { 0.0, 0.0 };

	// setting position, scale, orientation
	WorldObject& worldobject = registry.worldObjects.emplace(entity);
	worldobject.position = position;
	worldobject.angle = 0.f;
	worldobject.scale = scale;

	registry.debugComponents.emplace(entity);
	return entity;
}

Entity createNPC(RenderSystem* renderer, vec2 pos, vec2 size, ivec2 room_coord, NPC_TYPE npc_type) {
	auto entity = Entity();
	registry.gameSceneComponents.emplace(entity);
	registry.roomCoords.emplace(entity, room_coord);
	registry.activeComponents.emplace(entity);

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial position, scale, and orientation values
	WorldObject& worldobject = registry.worldObjects.emplace(entity);
	worldobject.position = pos;
	worldobject.angle = 0.f;
	worldobject.scale = size;

	NPC& npc = registry.NPCs.emplace(entity, "", npc_type);

	TEXTURE_ASSET_ID texture_id;
	switch (npc_type) {
		case (NPC_TYPE::OLD_ROBOT_NPC):
			texture_id = TEXTURE_ASSET_ID::OLD_MAN;
			npc.dialogue_path = dialogue_path("old_robot.json");
			break;
		case (NPC_TYPE::SCARECROW_NPC):
			texture_id = TEXTURE_ASSET_ID::SCARECROW;
			npc.dialogue_path = dialogue_path("scarecrow.json");
			break;
		case (NPC_TYPE::FINAL_DEATH):
			texture_id = TEXTURE_ASSET_ID::FINAL_BOSS_DEATH;
			npc.dialogue_path = dialogue_path("scarecrow.json");
			break;
		default:
			printf("NPC type not recognized\n");
			exit(1);
	}

	Interactable& interactable =  registry.interactables.emplace(entity);
	interactable.range = 200.f;
	interactable.interaction = [](int arg1, Entity arg2) {
		if (registry.dialogueStates.size() == 0) {
			Entity dialogueEntity = Entity();
			DialogueState& dialogueState = registry.dialogueStates.emplace(dialogueEntity);
			dialogueState.talking_to = arg2;
		}
		else if (registry.dialogueStates.size() == 1) {
			registry.dialogueStates.components[0].talking_to = arg2;
		}
		else {
			printf("Too many dialogue states\n");
			exit(1);
		}
		scene_manager.set_scene(SCENE_TYPE::DIALOGUE);
		return true;
	};
	// TODO: call or somehow call the dialogue system using interactable.parent as the NPC, perhaps change functionality to enum?
	// also, need to switch scenes to dialogue scene
	interactable.value = 0;

	registry.blockers.emplace(entity);

	registry.renderRequests.insert_sorted(
		entity,
		{ texture_id,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_ORDER::ENEMY });

	return entity;
}


// ==================== COMPLEX CREATE FUNCTIONS ====================

void createParticles(RenderSystem* renderer, 
	vec2 pos, 
	std::uniform_real_distribution<float> uniform_dist, 
	std::default_random_engine& rng, 
	TEXTURE_ASSET_ID type, 
	ivec2 room_coord
) {
	float num_particles = ceil(uniform_dist(rng) * MAX_NUM_PARTICLES) + NUM_PARTICLES_OFFSET;
	for (int i = 0; i < num_particles; i++) {
		createParticle(renderer, pos, uniform_dist, rng, type, room_coord);
	}
}

void createEmptyRoom(RenderSystem* renderer, ivec2 coord) {
	// create a floor entity
	createFloor(renderer, { window_width_px / 2, (window_height_px + BASE_UI_HEIGHT) / 2 }, { (window_width_px - (2 * WALL_WIDTH)) * 1.01 , (window_height_px - (BASE_UI_HEIGHT + (2 * WALL_WIDTH))) * 1.01 }, coord);

	// left wall
	createWall(renderer, { 43.f, (window_height_px / 2) + WALL_WIDTH }, { WALL_WIDTH, ((window_height_px - BASE_UI_HEIGHT) * 0.96) }, 0.f, TEXTURE_ASSET_ID::VERT_WALL, coord);
	// right wall
	createWall(renderer, { window_width_px - 43.f, (window_height_px / 2) + WALL_WIDTH }, { WALL_WIDTH, ((window_height_px - BASE_UI_HEIGHT) * 0.96) }, M_PI, TEXTURE_ASSET_ID::VERT_WALL, coord);
	// top wall
	createWall(renderer, { window_width_px / 2, 45.f + BASE_UI_HEIGHT }, { window_width_px, WALL_WIDTH }, 0.f, TEXTURE_ASSET_ID::HORZ_WALL, coord);
	// bottom wall
	createWall(renderer, { window_width_px / 2, window_height_px - 45.f }, { window_width_px, WALL_WIDTH }, M_PI, TEXTURE_ASSET_ID::HORZ_WALL, coord);


	auto roomMap = registry.map.components[0].roomMap;
	if (roomMap.find({ coord.x + 1, coord.y }) != roomMap.end()) {
		createDoor(renderer, coord, { coord.x + 1, coord.y }, DIRECTION::RIGHT);
	}
	if (roomMap.find({ coord.x - 1, coord.y }) != roomMap.end()) {
		createDoor(renderer, coord, { coord.x - 1, coord.y }, DIRECTION::LEFT);
	}
	if (roomMap.find({ coord.x, coord.y + 1 }) != roomMap.end()) {
		createDoor(renderer, coord, { coord.x, coord.y + 1 }, DIRECTION::UP);
	}
	if (roomMap.find({ coord.x, coord.y - 1 }) != roomMap.end()) {
		if (roomMap[{ coord.x, coord.y - 1 }] != ROOM_TYPE::BOSS_ROOM_ONE
			&& roomMap[{ coord.x, coord.y - 1 }] != ROOM_TYPE::BOSS_ROOM_TWO) {
			createDoor(renderer, coord, { coord.x, coord.y - 1 }, DIRECTION::DOWN);
		}
	}
}

bool notSafe(vec2 pos) {
	// check proximity to doors
	float leftRightDoorY = ((window_height_px - WALL_WIDTH - BASE_UI_HEIGHT) + WALL_WIDTH + BASE_UI_HEIGHT) / 2.f;
	float upDoorY = WALL_WIDTH + BASE_UI_HEIGHT;
	float leftDoorX = WALL_WIDTH;
	float rightDoorX = window_width_px - WALL_WIDTH;
	float upDownDoorX = window_height_px / 2.f;
	float downDoorY = window_height_px - WALL_WIDTH;
	if (sqrt(((pos.x - leftDoorX) * (pos.x - leftDoorX)) + ((pos.y - leftRightDoorY) * (pos.y - leftRightDoorY))) < 400) // left door
		return true;
	if (sqrt(((pos.x - rightDoorX) * (pos.x - rightDoorX)) + ((pos.y - leftRightDoorY) * (pos.y - leftRightDoorY))) < 400) // right door
		return true;
	if (sqrt(((pos.x - upDownDoorX) * (pos.x - upDownDoorX)) + ((pos.y - upDoorY) * (pos.y - upDoorY))) < 400) // top door
		return true;
	if (sqrt(((pos.x - upDownDoorX) * (pos.x - upDownDoorX)) + ((pos.y - downDoorY) * (pos.y - downDoorY))) < 400) // bottom door
		return true;

	return false;
}
void createEnemyRoom(RenderSystem* renderer, ivec2 coord, ROOM_TYPE type) {
	
	// create a floor entity
	createFloor(renderer, { window_width_px / 2, (window_height_px + BASE_UI_HEIGHT) / 2 }, { (window_width_px - (2 * WALL_WIDTH)) * 1.01 , (window_height_px - (BASE_UI_HEIGHT + (2 * WALL_WIDTH))) * 1.01 }, coord);

	//createInteractable(renderer, { window_width_px / 2, window_height_px - 200 }, { 75.f, 75.f }, bound_interactable_fn, 1, { 0, 0 });

	// left wall
	createWall(renderer, { 42.f, (window_height_px / 2) + WALL_WIDTH }, { WALL_WIDTH, ((window_height_px - BASE_UI_HEIGHT) * 0.96) }, 0.f, TEXTURE_ASSET_ID::VERT_WALL, coord);
	// right wall
	createWall(renderer, { window_width_px - 42.f, (window_height_px / 2) + WALL_WIDTH }, { WALL_WIDTH, ((window_height_px - BASE_UI_HEIGHT) * 0.96) }, M_PI, TEXTURE_ASSET_ID::VERT_WALL, coord);
	// top wall
	createWall(renderer, { window_width_px / 2, 45.f + BASE_UI_HEIGHT }, { window_width_px, WALL_WIDTH }, 0.f, TEXTURE_ASSET_ID::HORZ_WALL, coord);
	// bottom wall
	createWall(renderer, { window_width_px / 2, window_height_px - 45.f }, { window_width_px, WALL_WIDTH }, M_PI, TEXTURE_ASSET_ID::HORZ_WALL, coord);

	std::map<std::pair<int, int>, ROOM_TYPE>& roomMap = registry.map.components[0].roomMap;
	if (roomMap.find({ coord.x + 1, coord.y }) != roomMap.end()) {
		createDoor(renderer, coord, { coord.x + 1, coord.y }, DIRECTION::RIGHT);
	}
	if (roomMap.find({ coord.x - 1, coord.y }) != roomMap.end()) {
		createDoor(renderer, coord, { coord.x - 1, coord.y }, DIRECTION::LEFT);
	}
	if (roomMap.find({ coord.x, coord.y + 1 }) != roomMap.end()) {
		createDoor(renderer, coord, { coord.x, coord.y + 1 }, DIRECTION::UP);
	}
	if (roomMap.find({ coord.x, coord.y - 1 }) != roomMap.end()) {
		if (roomMap[{ coord.x, coord.y - 1 }] != ROOM_TYPE::BOSS_ROOM_ONE
			&& roomMap[{ coord.x, coord.y - 1 }] != ROOM_TYPE::BOSS_ROOM_TWO) {
			createDoor(renderer, coord, { coord.x, coord.y - 1 }, DIRECTION::DOWN);
		}
	}

	if (registry.gameLoadingHelper.components.size() == 0 || registry.gameLoadingHelper.components[0].savedGame == false) {
		// Generate enemies
		enemyRoomGenerateEnemies(renderer, coord, type);
		// Generate floor items
		enemyRoomGenerateFloorItems(renderer, coord, type);
	}
}

void enemyRoomGenerateEnemies(RenderSystem* renderer, ivec2 coord, ROOM_TYPE type)
{
	// simplify enemy positioning
	float middleY = ((window_height_px - BASE_UI_HEIGHT - (2.f * WALL_WIDTH)) / 2.f) + BASE_UI_HEIGHT + WALL_WIDTH;
	float topY = window_height_px - WALL_WIDTH - ENEMY_BB_HEIGHT / 2.f;
	float bottomY = BASE_UI_HEIGHT + WALL_WIDTH + ENEMY_BB_HEIGHT / 2.f;

	// enemy generation by room type
	switch (type) {
	case ROOM_TYPE::TWO_SIMPLE:
		createEnemy(renderer, vec2(window_width_px / 3.f, middleY), ENEMY_SPEED, coord);
		createEnemy(renderer, vec2(window_width_px * 2.f / 3.f, middleY), ENEMY_SPEED, coord);
		break;
	case ROOM_TYPE::MIDLINE_PROJ:
		createEnemy(renderer, vec2(window_width_px / 3.f, middleY - (ENEMY_BB_HEIGHT + FLOOR_ITEM_SIZE) / 2.f), ENEMY_SPEED, coord);
		createEnemy(renderer, vec2(window_width_px * 2.f / 3.f, middleY + (ENEMY_BB_HEIGHT + FLOOR_ITEM_SIZE) / 2.f), ENEMY_SPEED, coord);
		break;
	case ROOM_TYPE::CORNER_MIX:
		createEnemy(renderer, vec2(WALL_WIDTH + ENEMY_BB_WIDTH / 2.f, topY), ENEMY_SPEED, coord);
		createEnemy(renderer, vec2(WALL_WIDTH + ENEMY_BB_WIDTH / 2.f, bottomY), ENEMY_SPEED, coord);
		createEnemy(renderer, vec2(window_width_px - (WALL_WIDTH + ENEMY_BB_WIDTH / 2.f), topY), ENEMY_SPEED, coord);
		createEnemy(renderer, vec2(window_width_px - (WALL_WIDTH + ENEMY_BB_WIDTH / 2.f), bottomY), ENEMY_SPEED, coord);
		break;
	case ROOM_TYPE::LAPS:
		createEnemy(renderer, vec2(window_width_px / 2.f, middleY - ENEMY_BB_HEIGHT), ENEMY_SPEED, coord);
		createEnemy(renderer, vec2(window_width_px / 2.f, middleY + ENEMY_BB_HEIGHT), ENEMY_SPEED, coord);
		createEnemy(renderer, vec2(window_width_px / 2.f - ENEMY_BB_HEIGHT, middleY), ENEMY_SPEED, coord);
		createEnemy(renderer, vec2(window_width_px / 2.f + ENEMY_BB_HEIGHT, middleY), ENEMY_SPEED, coord);
		break;
	case ROOM_TYPE::CHECKERBOARD:
		createEnemy(renderer, vec2(window_width_px / 2.f, middleY), ENEMY_SPEED, coord, 5, FLY_TYPE);
		createEnemy(renderer, vec2(window_width_px / 2.f - 3.5f * ENEMY_BB_WIDTH, middleY), ENEMY_SPEED, coord);
		createEnemy(renderer, vec2(window_width_px / 2.f + 3.5f * ENEMY_BB_WIDTH, middleY), ENEMY_SPEED, coord);
		break;
	case ROOM_TYPE::BIG_X:
		createEnemy(renderer, vec2(window_width_px / 2.f - 1.5 * ENEMY_BB_WIDTH, middleY), ENEMY_SPEED, coord);
		createEnemy(renderer, vec2(window_width_px / 2.f + 1.5 * ENEMY_BB_WIDTH, middleY), ENEMY_SPEED, coord);
		createEnemy(renderer, vec2(window_width_px / 2.f, middleY - 0.5 * (ENEMY_BB_HEIGHT + FLOOR_ITEM_SIZE)), ENEMY_SPEED, coord);
		createEnemy(renderer, vec2(window_width_px / 2.f, middleY + 0.5 * (ENEMY_BB_HEIGHT + FLOOR_ITEM_SIZE)), ENEMY_SPEED, coord);
		break;
	case ROOM_TYPE::TUNNELS:
		createEnemy(renderer, vec2(window_width_px * 23.f / 36.f, middleY), ENEMY_SPEED, coord);
		createEnemy(renderer, vec2(window_width_px * 13.f / 36.f, middleY), ENEMY_SPEED, coord);
		createEnemy(renderer, vec2(window_width_px * 2.f / 9.f, middleY), ENEMY_SPEED, coord);
		createEnemy(renderer, vec2(window_width_px * 7.f / 9.f, middleY), ENEMY_SPEED, coord);
		break;
	case ROOM_TYPE::SCATTER:
		createEnemy(renderer, vec2(window_width_px / 2.f + ENEMY_BB_WIDTH * 3.7, topY), ENEMY_SPEED, coord);
		createEnemy(renderer, vec2(window_width_px / 2.f - ENEMY_BB_WIDTH * 2.1, bottomY + ENEMY_BB_HEIGHT * 0.7), ENEMY_SPEED, coord);
		createEnemy(renderer, vec2(window_width_px / 2.f - ENEMY_BB_WIDTH * 4.2, topY - FLOOR_ITEM_SIZE * 0.7), ENEMY_SPEED, coord);
		break;
	case ROOM_TYPE::ONE_HEAVY:
		createEnemy(renderer, vec2(window_width_px / 2.f, middleY), ENEMY_SPEED, coord, HEAVY_HEALTH, HEAVY_TYPE);
		break;
	case ROOM_TYPE::TWO_HEAVY:
		createEnemy(renderer, vec2(window_width_px / 3.f, middleY), ENEMY_SPEED, coord, HEAVY_HEALTH, HEAVY_TYPE);
		createEnemy(renderer, vec2(window_width_px * 2.f / 3.f, middleY), ENEMY_SPEED, coord, HEAVY_HEALTH, HEAVY_TYPE);
		break;
	case ROOM_TYPE::ENEMY_SOCIAL:
		createEnemy(renderer, vec2(window_width_px / 5.f * 2.f - ENEMY_BB_WIDTH, middleY - ENEMY_BB_HEIGHT), ENEMY_SPEED, coord, HEAVY_HEALTH, HEAVY_TYPE);
		createEnemy(renderer, vec2(window_width_px / 5.f * 2.f - ENEMY_BB_WIDTH * 0.5, middleY + ENEMY_BB_HEIGHT), ENEMY_SPEED, coord);
		createEnemy(renderer, vec2(window_width_px / 5.f * 3.f + ENEMY_BB_WIDTH * 0.5, middleY - ENEMY_BB_HEIGHT), ENEMY_SPEED, coord);
		createEnemy(renderer, vec2(window_width_px / 5.f * 3.f + ENEMY_BB_WIDTH * 0.5, middleY + ENEMY_BB_HEIGHT), ENEMY_SPEED, coord, 5, FLY_TYPE);
		break;
	case ROOM_TYPE::FLY_TUNNELS:
		createEnemy(renderer, vec2(window_width_px * 23.f / 36.f, middleY), ENEMY_SPEED, coord, 5, FLY_TYPE);
		createEnemy(renderer, vec2(window_width_px * 13.f / 36.f, middleY), ENEMY_SPEED, coord);
		createEnemy(renderer, vec2(window_width_px * 2.f / 9.f, middleY), ENEMY_SPEED, coord, 5, FLY_TYPE);
		createEnemy(renderer, vec2(window_width_px * 7.f / 9.f, middleY), ENEMY_SPEED, coord);
		break;
	case ROOM_TYPE::FLY_LAPS:
		createEnemy(renderer, vec2(window_width_px / 2.f, middleY - ENEMY_BB_HEIGHT), ENEMY_SPEED, coord, 5, FLY_TYPE);
		createEnemy(renderer, vec2(window_width_px / 2.f, middleY + ENEMY_BB_HEIGHT), ENEMY_SPEED, coord, 5, FLY_TYPE);
		createEnemy(renderer, vec2(window_width_px / 2.f - ENEMY_BB_HEIGHT, middleY), ENEMY_SPEED, coord);
		createEnemy(renderer, vec2(window_width_px / 2.f + ENEMY_BB_HEIGHT, middleY), ENEMY_SPEED, coord);
		break;
	}
}

TEXTURE_ASSET_ID randomFloorItem()
{
	int seed = rand() % 7;
	switch (seed) {
	case 0:
		return TEXTURE_ASSET_ID::FURNACE;
	case 1:
		return TEXTURE_ASSET_ID::BROKEN_GENERATOR;
	case 2:
		return TEXTURE_ASSET_ID::DEAD_ROBOT;
	case 3:
		return TEXTURE_ASSET_ID::BROKEN_CONTROL_PANEL;
	case 4:
		return TEXTURE_ASSET_ID::FLOOR_HOLE;
	case 5:
		return TEXTURE_ASSET_ID::RUSTY_PIPES;
	default:
		return TEXTURE_ASSET_ID::SLAG_PIT;
	}
}

void enemyRoomGenerateFloorItems(RenderSystem* renderer, ivec2 coord, ROOM_TYPE type)
{
	// SIMPLIFY positioning
	const float middleY = ((window_height_px - BASE_UI_HEIGHT - (2.f * WALL_WIDTH)) / 2.f) + BASE_UI_HEIGHT + WALL_WIDTH;
	const float topY = window_height_px - WALL_WIDTH - FLOOR_ITEM_SIZE / 2.f;
	const float bottomY = BASE_UI_HEIGHT + WALL_WIDTH + FLOOR_ITEM_SIZE / 2.f;
	const vec2 scale = { FLOOR_ITEM_SIZE , FLOOR_ITEM_SIZE };

	switch (type) {
	case ROOM_TYPE::MIDLINE_PROJ:
		createWall(renderer, { window_width_px / 2.f, middleY }, scale, 0.f, randomFloorItem(), coord);
		createWall(renderer, { window_width_px / 2.f - FLOOR_ITEM_SIZE - FLOOR_ITEM_BUFFER, middleY }, scale, 0.f, randomFloorItem(), coord);
		createWall(renderer, { window_width_px / 2.f - (FLOOR_ITEM_SIZE * 2.f) - (FLOOR_ITEM_BUFFER * 2.f), middleY }, scale, 0.f, randomFloorItem(), coord);
		createWall(renderer, { window_width_px / 2.f - (FLOOR_ITEM_SIZE * 3.f) - (FLOOR_ITEM_BUFFER * 3.f), middleY }, scale, 0.f, randomFloorItem(), coord);
		createWall(renderer, { window_width_px / 2.f - (FLOOR_ITEM_SIZE * 4.f) - (FLOOR_ITEM_BUFFER * 4.f), middleY }, scale, 0.f, randomFloorItem(), coord);
		createWall(renderer, { window_width_px / 2.f + FLOOR_ITEM_SIZE + FLOOR_ITEM_BUFFER, middleY }, scale, 0.f, randomFloorItem(), coord);
		createWall(renderer, { window_width_px / 2.f + (FLOOR_ITEM_SIZE * 2.f) + (FLOOR_ITEM_BUFFER * 2.f), middleY }, scale, 0.f, randomFloorItem(), coord);
		createWall(renderer, { window_width_px / 2.f + (FLOOR_ITEM_SIZE * 3.f) + (FLOOR_ITEM_BUFFER * 3.f), middleY }, scale, 0.f, randomFloorItem(), coord);
		createWall(renderer, { window_width_px / 2.f + (FLOOR_ITEM_SIZE * 4.f) + (FLOOR_ITEM_BUFFER * 4.f), middleY }, scale, 0.f, randomFloorItem(), coord);
		break;
	case ROOM_TYPE::CORNER_MIX:
		createWall(renderer, { WALL_WIDTH + 1.6 * FLOOR_ITEM_SIZE, topY - 1.3 * FLOOR_ITEM_SIZE }, scale, 0.f, randomFloorItem(), coord);
		createWall(renderer, { WALL_WIDTH + 1.6 * FLOOR_ITEM_SIZE, bottomY + 1.3 * FLOOR_ITEM_SIZE }, scale, 0.f, randomFloorItem(), coord);
		createWall(renderer, { window_width_px - WALL_WIDTH - 1.6 * FLOOR_ITEM_SIZE, topY - 1.3 * FLOOR_ITEM_SIZE }, scale, 0.f, randomFloorItem(), coord);
		createWall(renderer, { window_width_px - WALL_WIDTH - 1.6 * FLOOR_ITEM_SIZE, bottomY + 1.3 * FLOOR_ITEM_SIZE }, scale, 0.f, randomFloorItem(), coord);
		break;
	case ROOM_TYPE::FLY_LAPS:
	case ROOM_TYPE::LAPS:
		createWall(renderer, { window_width_px / 4.f, middleY }, scale, 0.f, randomFloorItem(), coord);
		createWall(renderer, { window_width_px / 4.f, middleY + FLOOR_ITEM_SIZE + FLOOR_ITEM_BUFFER }, scale, 0.f, randomFloorItem(), coord);
		createWall(renderer, { window_width_px / 4.f, middleY - FLOOR_ITEM_SIZE - FLOOR_ITEM_BUFFER }, scale, 0.f, randomFloorItem(), coord);
		createWall(renderer, { window_width_px * 3.f / 4.f, middleY }, scale, 0.f, randomFloorItem(), coord);
		createWall(renderer, { window_width_px * 3.f / 4.f, middleY - FLOOR_ITEM_SIZE - FLOOR_ITEM_BUFFER }, scale, 0.f, randomFloorItem(), coord);
		createWall(renderer, { window_width_px * 3.f / 4.f, middleY + FLOOR_ITEM_SIZE + FLOOR_ITEM_BUFFER }, scale, 0.f, randomFloorItem(), coord);
		break;
	case ROOM_TYPE::CHECKERBOARD:
		createWall(renderer, { WALL_WIDTH + 1.5 * FLOOR_ITEM_SIZE, topY - 1.3 * FLOOR_ITEM_SIZE }, scale, 0.f, randomFloorItem(), coord);
		createWall(renderer, { WALL_WIDTH + 1.5 * FLOOR_ITEM_SIZE, bottomY + 1.3 * FLOOR_ITEM_SIZE }, scale, 0.f, randomFloorItem(), coord);
		createWall(renderer, { window_width_px - WALL_WIDTH - 1.5 * FLOOR_ITEM_SIZE, topY - 1.3 * FLOOR_ITEM_SIZE }, scale, 0.f, randomFloorItem(), coord);
		createWall(renderer, { window_width_px - WALL_WIDTH - 1.5 * FLOOR_ITEM_SIZE, bottomY + 1.3 * FLOOR_ITEM_SIZE }, scale, 0.f, randomFloorItem(), coord);

		createWall(renderer, { WALL_WIDTH + 4.5 * FLOOR_ITEM_SIZE, topY - 1.3 * FLOOR_ITEM_SIZE }, scale, 0.f, randomFloorItem(), coord);
		createWall(renderer, { WALL_WIDTH + 4.5 * FLOOR_ITEM_SIZE, bottomY + 1.3 * FLOOR_ITEM_SIZE }, scale, 0.f, randomFloorItem(), coord);
		createWall(renderer, { window_width_px - WALL_WIDTH - 4.5 * FLOOR_ITEM_SIZE, topY - 1.3 * FLOOR_ITEM_SIZE }, scale, 0.f, randomFloorItem(), coord);
		createWall(renderer, { window_width_px - WALL_WIDTH - 4.5 * FLOOR_ITEM_SIZE, bottomY + 1.3 * FLOOR_ITEM_SIZE }, scale, 0.f, randomFloorItem(), coord);

		createWall(renderer, { window_width_px / 2.f, topY - 1.3 * FLOOR_ITEM_SIZE }, scale, 0.f, randomFloorItem(), coord);
		createWall(renderer, { window_width_px / 2.f, bottomY + 1.3 * FLOOR_ITEM_SIZE }, scale, 0.f, randomFloorItem(), coord);
		break;
	case ROOM_TYPE::BIG_X:
		createWall(renderer, { window_width_px / 2.f, middleY, }, scale, 0.f, randomFloorItem(), coord);

		createWall(renderer, { window_width_px / 2.f - (FLOOR_ITEM_SIZE + FLOOR_ITEM_BUFFER), middleY - (FLOOR_ITEM_SIZE + FLOOR_ITEM_BUFFER) }, scale, 0.f, randomFloorItem(), coord);
		createWall(renderer, { window_width_px / 2.f - (FLOOR_ITEM_SIZE + FLOOR_ITEM_BUFFER), middleY + (FLOOR_ITEM_SIZE + FLOOR_ITEM_BUFFER) }, scale, 0.f, randomFloorItem(), coord);
		createWall(renderer, { window_width_px / 2.f + (FLOOR_ITEM_SIZE + FLOOR_ITEM_BUFFER), middleY - (FLOOR_ITEM_SIZE + FLOOR_ITEM_BUFFER) }, scale, 0.f, randomFloorItem(), coord);
		createWall(renderer, { window_width_px / 2.f + (FLOOR_ITEM_SIZE + FLOOR_ITEM_BUFFER), middleY + (FLOOR_ITEM_SIZE + FLOOR_ITEM_BUFFER) }, scale, 0.f, randomFloorItem(), coord);

		createWall(renderer, { window_width_px / 2.f - 2.f * (FLOOR_ITEM_SIZE + FLOOR_ITEM_BUFFER), middleY - 1.2 * (FLOOR_ITEM_SIZE + FLOOR_ITEM_BUFFER) }, scale, 0.f, randomFloorItem(), coord);
		createWall(renderer, { window_width_px / 2.f - 2.f * (FLOOR_ITEM_SIZE + FLOOR_ITEM_BUFFER), middleY + 1.2 * (FLOOR_ITEM_SIZE + FLOOR_ITEM_BUFFER) }, scale, 0.f, randomFloorItem(), coord);
		createWall(renderer, { window_width_px / 2.f + 2.f * (FLOOR_ITEM_SIZE + FLOOR_ITEM_BUFFER), middleY - 1.2 * (FLOOR_ITEM_SIZE + FLOOR_ITEM_BUFFER) }, scale, 0.f, randomFloorItem(), coord);
		createWall(renderer, { window_width_px / 2.f + 2.f * (FLOOR_ITEM_SIZE + FLOOR_ITEM_BUFFER), middleY + 1.2 * (FLOOR_ITEM_SIZE + FLOOR_ITEM_BUFFER) }, scale, 0.f, randomFloorItem(), coord);
		break;
	case ROOM_TYPE::FLY_TUNNELS:
	case ROOM_TYPE::TUNNELS:
		createWall(renderer, { (window_width_px - 2 * WALL_WIDTH) * 8.f / 13.f + WALL_WIDTH / 2.f, topY }, scale, 0.f, randomFloorItem(), coord);
		createWall(renderer, { (window_width_px - 2 * WALL_WIDTH) * 8.f / 13.f + WALL_WIDTH / 2.f, topY - (FLOOR_ITEM_BUFFER + FLOOR_ITEM_SIZE) }, scale, 0.f, randomFloorItem(), coord);
		createWall(renderer, { (window_width_px - 2 * WALL_WIDTH) * 8.f / 13.f + WALL_WIDTH / 2.f, topY - 2.f * (FLOOR_ITEM_BUFFER + FLOOR_ITEM_SIZE) }, scale, 0.f, randomFloorItem(), coord);
		createWall(renderer, { (window_width_px - 2 * WALL_WIDTH) * 8.f / 13.f + WALL_WIDTH / 2.f, topY - 3.f * (FLOOR_ITEM_BUFFER + FLOOR_ITEM_SIZE) }, scale, 0.f, randomFloorItem(), coord);

		createWall(renderer, { (window_width_px - 2 * WALL_WIDTH) * 10.f / 13.f + WALL_WIDTH / 2.f, bottomY }, scale, 0.f, randomFloorItem(), coord);
		createWall(renderer, { (window_width_px - 2 * WALL_WIDTH) * 10.f / 13.f + WALL_WIDTH / 2.f, bottomY + (FLOOR_ITEM_BUFFER + FLOOR_ITEM_SIZE) }, scale, 0.f, randomFloorItem(), coord);
		createWall(renderer, { (window_width_px - 2 * WALL_WIDTH) * 10.f / 13.f + WALL_WIDTH / 2.f, bottomY + 2.f * (FLOOR_ITEM_BUFFER + FLOOR_ITEM_SIZE) }, scale, 0.f, randomFloorItem(), coord);
		createWall(renderer, { (window_width_px - 2 * WALL_WIDTH) * 10.f / 13.f + WALL_WIDTH / 2.f, bottomY + 3.f * (FLOOR_ITEM_BUFFER + FLOOR_ITEM_SIZE) }, scale, 0.f, randomFloorItem(), coord);

		createWall(renderer, { (window_width_px - 2 * WALL_WIDTH) * 12.f / 13.f + WALL_WIDTH / 2.f, topY }, scale, 0.f, randomFloorItem(), coord);
		createWall(renderer, { (window_width_px - 2 * WALL_WIDTH) * 12.f / 13.f + WALL_WIDTH / 2.f, topY - (FLOOR_ITEM_BUFFER + FLOOR_ITEM_SIZE) }, scale, 0.f, randomFloorItem(), coord);
		createWall(renderer, { (window_width_px - 2 * WALL_WIDTH) * 12.f / 13.f + WALL_WIDTH / 2.f, topY - 2.f * (FLOOR_ITEM_BUFFER + FLOOR_ITEM_SIZE) }, scale, 0.f, randomFloorItem(), coord);
		createWall(renderer, { (window_width_px - 2 * WALL_WIDTH) * 12.f / 13.f + WALL_WIDTH / 2.f, topY - 3.f * (FLOOR_ITEM_BUFFER + FLOOR_ITEM_SIZE) }, scale, 0.f, randomFloorItem(), coord);

		createWall(renderer, { (window_width_px - 2 * WALL_WIDTH) * 2.f / 13.f + WALL_WIDTH / 3.f, bottomY }, scale, 0.f, randomFloorItem(), coord);
		createWall(renderer, { (window_width_px - 2 * WALL_WIDTH) * 2.f / 13.f + WALL_WIDTH / 3.f, bottomY + (FLOOR_ITEM_BUFFER + FLOOR_ITEM_SIZE) }, scale, 0.f, randomFloorItem(), coord);
		createWall(renderer, { (window_width_px - 2 * WALL_WIDTH) * 2.f / 13.f + WALL_WIDTH / 3.f, bottomY + 2.f * (FLOOR_ITEM_BUFFER + FLOOR_ITEM_SIZE) }, scale, 0.f, randomFloorItem(), coord);
		createWall(renderer, { (window_width_px - 2 * WALL_WIDTH) * 2.f / 13.f + WALL_WIDTH / 3.f, bottomY + 3.f * (FLOOR_ITEM_BUFFER + FLOOR_ITEM_SIZE) }, scale, 0.f, randomFloorItem(), coord);

		createWall(renderer, { (window_width_px - 2 * WALL_WIDTH) * 4.f / 13.f + WALL_WIDTH / 3.f, topY }, scale, 0.f, randomFloorItem(), coord);
		createWall(renderer, { (window_width_px - 2 * WALL_WIDTH) * 4.f / 13.f + WALL_WIDTH / 3.f, topY - (FLOOR_ITEM_BUFFER + FLOOR_ITEM_SIZE) }, scale, 0.f, randomFloorItem(), coord);
		createWall(renderer, { (window_width_px - 2 * WALL_WIDTH) * 4.f / 13.f + WALL_WIDTH / 3.f, topY - 2.f * (FLOOR_ITEM_BUFFER + FLOOR_ITEM_SIZE) }, scale, 0.f, randomFloorItem(), coord);
		createWall(renderer, { (window_width_px - 2 * WALL_WIDTH) * 4.f / 13.f + WALL_WIDTH / 3.f, topY - 3.f * (FLOOR_ITEM_BUFFER + FLOOR_ITEM_SIZE) }, scale, 0.f, randomFloorItem(), coord);

		createWall(renderer, { (window_width_px - 2 * WALL_WIDTH) * 6.f / 13.f + WALL_WIDTH / 3.f, bottomY }, scale, 0.f, randomFloorItem(), coord);
		createWall(renderer, { (window_width_px - 2 * WALL_WIDTH) * 6.f / 13.f + WALL_WIDTH / 3.f, bottomY + (FLOOR_ITEM_BUFFER + FLOOR_ITEM_SIZE) }, scale, 0.f, randomFloorItem(), coord);
		createWall(renderer, { (window_width_px - 2 * WALL_WIDTH) * 6.f / 13.f + WALL_WIDTH / 3.f, bottomY + 2.f * (FLOOR_ITEM_BUFFER + FLOOR_ITEM_SIZE) }, scale, 0.f, randomFloorItem(), coord);
		createWall(renderer, { (window_width_px - 2 * WALL_WIDTH) * 6.f / 13.f + WALL_WIDTH / 3.f, bottomY + 3.f * (FLOOR_ITEM_BUFFER + FLOOR_ITEM_SIZE) }, scale, 0.f, randomFloorItem(), coord);
		break;
	case ROOM_TYPE::SCATTER:
		createWall(renderer, { window_width_px / 2.f - 37.f, middleY + 61.f }, scale, 0.f, randomFloorItem(), coord);
		createWall(renderer, { WALL_WIDTH * 4.f, topY - FLOOR_ITEM_SIZE * 2.7f }, scale, 0.f, randomFloorItem(), coord);
		createWall(renderer, { window_width_px - FLOOR_ITEM_SIZE * 3.8, middleY }, scale, 0.f, randomFloorItem(), coord);
		createWall(renderer, { window_width_px - WALL_WIDTH - .5 * FLOOR_ITEM_SIZE, topY }, scale, 0.f, randomFloorItem(), coord);
		createWall(renderer, { window_width_px - WALL_WIDTH - 5.5 * FLOOR_ITEM_SIZE, bottomY }, scale, 0.f, randomFloorItem(), coord);
		break;
	case ROOM_TYPE::TWO_HEAVY:
		createWall(renderer, { window_width_px / 2.f, middleY }, scale, 0.f, randomFloorItem(), coord);
		break;
	case ROOM_TYPE::ENEMY_SOCIAL:
		createWall(renderer, { WALL_WIDTH + 1.5 * FLOOR_ITEM_SIZE, topY - 1.2 * FLOOR_ITEM_SIZE }, scale, 0.f, randomFloorItem(), coord);
		createWall(renderer, { WALL_WIDTH + 1.5 * FLOOR_ITEM_SIZE, bottomY + 1.2 * FLOOR_ITEM_SIZE }, scale, 0.f, randomFloorItem(), coord);
		createWall(renderer, { window_width_px - WALL_WIDTH - 1.5 * FLOOR_ITEM_SIZE, topY - 1.2 * FLOOR_ITEM_SIZE }, scale, 0.f, randomFloorItem(), coord);
		createWall(renderer, { window_width_px - WALL_WIDTH - 1.5 * FLOOR_ITEM_SIZE, bottomY + 1.2 * FLOOR_ITEM_SIZE }, scale, 0.f, randomFloorItem(), coord);
		break;
	}
}

void createNPCRoom(RenderSystem* renderer, ivec2 coord, NPC_TYPE npc_type) {
	createFloor(renderer, { window_width_px / 2, (window_height_px + BASE_UI_HEIGHT) / 2 }, { (window_width_px - (2 * WALL_WIDTH)) * 1.01 , (window_height_px - (BASE_UI_HEIGHT + (2 * WALL_WIDTH))) * 1.01 }, coord);

	createEmptyRoom(renderer, coord);

	switch (npc_type) {
	case NPC_TYPE::OLD_ROBOT_NPC:
		createNPC(renderer,
			vec2(WALL_WIDTH + OLD_ROBOT_WIDTH / 2.f, WALL_WIDTH + BASE_UI_HEIGHT + OLD_ROBOT_HEIGHT / 2.f),
			vec2({ OLD_ROBOT_WIDTH, OLD_ROBOT_HEIGHT }),
			coord,
			npc_type);
		break;
	case NPC_TYPE::SCARECROW_NPC:
		createNPC(renderer, 
			vec2(window_width_px - WALL_WIDTH - SCARECROW_WIDTH / 2.f, WALL_WIDTH + BASE_UI_HEIGHT + SCARECROW_HEIGHT / 2.f),
			vec2({ SCARECROW_WIDTH, SCARECROW_HEIGHT }),
			coord,
			npc_type);
		break;
	}
	
}

void createBossRoomOne(RenderSystem* renderer, ivec2 coord) {
	// create a floor entity
	createFloor(renderer, { window_width_px / 2, (window_height_px + BASE_UI_HEIGHT) / 2 }, { (window_width_px - (2 * WALL_WIDTH)) * 1.05 , (window_height_px - (BASE_UI_HEIGHT + (2 * WALL_WIDTH))) * 1.4}, coord, FLOOR_TYPE::BOSS_ROOM_ONE);

	// left wall
	createWall(renderer, { 42.f, (window_height_px / 2) + 30.f }, { WALL_WIDTH, window_height_px - 70.f }, 0.f, TEXTURE_ASSET_ID::BOSS_ONE_VERT_WALL, coord);
	// right wall
	createWall(renderer, { window_width_px - 42.f, (window_height_px / 2) + 30.f }, { WALL_WIDTH, window_height_px - 70.f }, M_PI, TEXTURE_ASSET_ID::BOSS_ONE_VERT_WALL, coord);
	// top wall
	createWall(renderer, { window_width_px / 2, 90.f }, { window_width_px, WALL_WIDTH }, 0.f, TEXTURE_ASSET_ID::BOSS_ONE_HORZ_WALL, coord);
	// bottom wall
	createWall(renderer, { window_width_px / 2, window_height_px - 42.f }, { window_width_px, WALL_WIDTH }, M_PI, TEXTURE_ASSET_ID::BOSS_ONE_HORZ_WALL, coord);

	auto roomMap = registry.map.components[0].roomMap;
	if (roomMap.find({ coord.x + 1, coord.y }) != roomMap.end()) {
		createDoor(renderer, coord, { coord.x + 1, coord.y }, DIRECTION::RIGHT);
	}
	if (roomMap.find({coord.x - 1, coord.y}) != roomMap.end()) {
		createDoor(renderer, coord, { coord.x - 1, coord.y }, DIRECTION::LEFT);
	}
	if (roomMap.find({ coord.x, coord.y - 1 }) != roomMap.end()) {
		createDoor(renderer, coord, { coord.x, coord.y - 1 }, DIRECTION::DOWN);
	}
	if (registry.gameLoadingHelper.components.size() == 0 || registry.gameLoadingHelper.components[0].savedGame == false) {
		createBossOne(renderer, { WALL_WIDTH + 750, window_height_px * 0.4 }, BOSS_ONE_POS::TOP_LEFT, coord);
		createBossOne(renderer, { WALL_WIDTH + 1050, window_height_px * 0.4 }, BOSS_ONE_POS::TOP_RIGHT, coord);

		createBossOne(renderer, { WALL_WIDTH + 750, WALL_WIDTH + BASE_UI_HEIGHT + 563 + 45 }, BOSS_ONE_POS::BOT_LEFT, coord);
		createBossOne(renderer, { WALL_WIDTH + 1050, WALL_WIDTH + BASE_UI_HEIGHT + 563 + 45 }, BOSS_ONE_POS::BOT_RIGHT, coord);

		createBossOne(renderer, { CENTER_X, 300 }, BOSS_ONE_POS::MOTHER, coord);
	}

	// Decoration
	//createWall(renderer, { WALL_WIDTH + 640, WALL_WIDTH + BASE_UI_HEIGHT - 30 }, { ENEMY_BB_WIDTH, ENEMY_BB_HEIGHT }, 0.f, TEXTURE_ASSET_ID::ENEMY_ROBOT_OFF, coord);
	createWall(renderer, { WALL_WIDTH + 550, WALL_WIDTH + BASE_UI_HEIGHT - 30 }, { ENEMY_BB_WIDTH, ENEMY_BB_HEIGHT }, 0.f, TEXTURE_ASSET_ID::ENEMY_ROBOT_OFF, coord);
	createWall(renderer, { WALL_WIDTH + 450, WALL_WIDTH + BASE_UI_HEIGHT - 30 }, { ENEMY_BB_WIDTH, ENEMY_BB_HEIGHT }, 0.f, TEXTURE_ASSET_ID::ENEMY_ROBOT_OFF, coord);
	createWall(renderer, { WALL_WIDTH + 350, WALL_WIDTH + BASE_UI_HEIGHT - 30 }, { ENEMY_BB_WIDTH, ENEMY_BB_HEIGHT }, 0.f, TEXTURE_ASSET_ID::ENEMY_ROBOT_OFF, coord);
	createWall(renderer, { WALL_WIDTH + 250, WALL_WIDTH + BASE_UI_HEIGHT - 30 }, { ENEMY_BB_WIDTH, ENEMY_BB_HEIGHT }, 0.f, TEXTURE_ASSET_ID::ENEMY_ROBOT_OFF, coord);
	createWall(renderer, { WALL_WIDTH + 150, WALL_WIDTH + BASE_UI_HEIGHT - 30 }, { ENEMY_BB_WIDTH, ENEMY_BB_HEIGHT }, 0.f, TEXTURE_ASSET_ID::ENEMY_ROBOT_OFF, coord);
	createWall(renderer, { WALL_WIDTH + 50, WALL_WIDTH + BASE_UI_HEIGHT - 30 }, { ENEMY_BB_WIDTH, ENEMY_BB_HEIGHT }, 0.f, TEXTURE_ASSET_ID::ENEMY_ROBOT_OFF, coord);

	createWall(renderer, { WALL_WIDTH + 1200, WALL_WIDTH + BASE_UI_HEIGHT - 30 }, { FLOOR_ITEM_SIZE, FLOOR_ITEM_SIZE }, 0.f, TEXTURE_ASSET_ID::DEAD_ROBOT, coord);
	createWall(renderer, { WALL_WIDTH + 1350, WALL_WIDTH + BASE_UI_HEIGHT - 30 }, { FLOOR_ITEM_SIZE, FLOOR_ITEM_SIZE }, 0.f, TEXTURE_ASSET_ID::DEAD_ROBOT, coord);
	createWall(renderer, { WALL_WIDTH + 1500, WALL_WIDTH + BASE_UI_HEIGHT - 30 }, { FLOOR_ITEM_SIZE, FLOOR_ITEM_SIZE }, 0.f, TEXTURE_ASSET_ID::DEAD_ROBOT, coord);
	createWall(renderer, { WALL_WIDTH + 1650, WALL_WIDTH + BASE_UI_HEIGHT - 30 }, { FLOOR_ITEM_SIZE, FLOOR_ITEM_SIZE }, 0.f, TEXTURE_ASSET_ID::DEAD_ROBOT, coord);
}

void createBossRoomTwo(RenderSystem* renderer, ivec2 coord) {
	// create a floor entity
	createFloor(renderer, { window_width_px / 2, (window_height_px + BASE_UI_HEIGHT) / 2 }, { (window_width_px - (2 * WALL_WIDTH)) * 1.01 , (window_height_px - (BASE_UI_HEIGHT + (2 * WALL_WIDTH))) * 1.01 }, coord);

	createWall(renderer, { window_width_px / 2, 45.f + BASE_UI_HEIGHT }, { window_width_px, WALL_WIDTH }, 0.f, TEXTURE_ASSET_ID::HORZ_WALL, coord);
	// Boss Two
	createBossTwo(renderer, { window_width_px / 2, (WALL_WIDTH * 1.65) + BASE_UI_HEIGHT }, coord);
	// left wall
	createWall(renderer, { 43.f, (window_height_px / 2) + WALL_WIDTH }, { WALL_WIDTH, ((window_height_px - BASE_UI_HEIGHT) * 0.96) }, 0.f, TEXTURE_ASSET_ID::VERT_WALL, coord);
	// right wall
	createWall(renderer, { window_width_px - 43.f, (window_height_px / 2) + WALL_WIDTH }, { WALL_WIDTH, ((window_height_px - BASE_UI_HEIGHT) * 0.96) }, M_PI, TEXTURE_ASSET_ID::VERT_WALL, coord);
	// bottom wall
	createWall(renderer, { window_width_px / 2, window_height_px - 45.f }, { window_width_px, WALL_WIDTH }, M_PI, TEXTURE_ASSET_ID::HORZ_WALL, coord);

	auto roomMap = registry.map.components[0].roomMap;
	if (roomMap.find({ coord.x + 1, coord.y }) != roomMap.end()) {
		createDoor(renderer, coord, { coord.x + 1, coord.y }, DIRECTION::RIGHT);
	}
	if (roomMap.find({ coord.x - 1, coord.y }) != roomMap.end()) {
		createDoor(renderer, coord, { coord.x - 1, coord.y }, DIRECTION::LEFT);
	}
	if (roomMap.find({ coord.x, coord.y - 1 }) != roomMap.end()) {
		createDoor(renderer, coord, { coord.x, coord.y - 1 }, DIRECTION::DOWN);
	}

}

void createBossRoomThree(RenderSystem* renderer, ivec2 coord) {

	// create a floor entity
	createFloor(renderer, { window_width_px / 2, (window_height_px + 120.f) / 2 }, { window_width_px , window_height_px - 120.f }, coord);

	// left wall
	createWall(renderer, { 42.f, (window_height_px / 2) + WALL_WIDTH }, { WALL_WIDTH, ((window_height_px - BASE_UI_HEIGHT) * 0.96) }, 0.f, TEXTURE_ASSET_ID::VERT_WALL, coord);
	// right wall
	createWall(renderer, { window_width_px - 42.f, (window_height_px / 2) + WALL_WIDTH }, { WALL_WIDTH, ((window_height_px - BASE_UI_HEIGHT) * 0.96) }, M_PI, TEXTURE_ASSET_ID::VERT_WALL, coord);
	// top wall
	createWall(renderer, { window_width_px / 2, 45.f + BASE_UI_HEIGHT }, { window_width_px, WALL_WIDTH }, 0.f, TEXTURE_ASSET_ID::HORZ_WALL, coord);
	// bottom wall
	createWall(renderer, { window_width_px / 2, window_height_px - 45.f }, { window_width_px, WALL_WIDTH }, M_PI, TEXTURE_ASSET_ID::HORZ_WALL, coord);

	std::map<std::pair<int, int>, ROOM_TYPE>& roomMap = registry.map.components[0].roomMap;
	if (roomMap.find({ coord.x + 1, coord.y }) != roomMap.end()) {
		createDoor(renderer, coord, { coord.x + 1, coord.y }, DIRECTION::RIGHT);
	}
	if (roomMap.find({ coord.x - 1, coord.y }) != roomMap.end()) {
		createDoor(renderer, coord, { coord.x - 1, coord.y }, DIRECTION::LEFT);
	}
	if (roomMap.find({ coord.x, coord.y + 1 }) != roomMap.end()) {
		createDoor(renderer, coord, { coord.x, coord.y + 1 }, DIRECTION::UP);
	}
	if (roomMap.find({ coord.x, coord.y - 1 }) != roomMap.end()) {
		if (roomMap[{ coord.x, coord.y - 1 }] != ROOM_TYPE::BOSS_ROOM_ONE
			&& roomMap[{ coord.x, coord.y - 1 }] != ROOM_TYPE::BOSS_ROOM_TWO) {
			createDoor(renderer, coord, { coord.x, coord.y - 1 }, DIRECTION::DOWN);
		}
	}

	createBossThree(renderer, {500,500}, coord);
}

void generate_map() {
	if (registry.map.components.size() > 0) {
		registry.remove_all_components_of(registry.map.entities[0]);
	}
	auto entity = Entity();
	registry.map.emplace(entity);
	std::map<std::pair<int, int>, ROOM_TYPE>& roomMap = registry.map.get(entity).roomMap;
	// Test room
	roomMap[{-1, 0}] = ROOM_TYPE::BOSS_ROOM_ONE;

	// FLOOR ONE
	roomMap[{ 0,  0 }] = ROOM_TYPE::EMPTY;
	roomMap[{ 1, 0 }] = ROOM_TYPE::TWO_SIMPLE;
	roomMap[{ 1, -1 }] = ROOM_TYPE::MIDLINE_PROJ;
	roomMap[{ 1, -2 }] = ROOM_TYPE::CORNER_MIX;
	roomMap[{ 0, -2 }] = ROOM_TYPE::LAPS;
	roomMap[{ 0, -3 }] = ROOM_TYPE::CHECKERBOARD;
	roomMap[{-1, -3 }] = ROOM_TYPE::BIG_X;
	roomMap[{-1, -4 }] = ROOM_TYPE::TUNNELS;
	roomMap[{-1, -5 }] = ROOM_TYPE::SCATTER;
	roomMap[{-2, -3 }] = ROOM_TYPE::MIDLINE_PROJ;
	roomMap[{-3, -3 }] = ROOM_TYPE::OLD_ROBOT_ROOM;
	roomMap[{ 1,  1 }] = ROOM_TYPE::CORNER_MIX;
	roomMap[{ 1,  2 }] = ROOM_TYPE::TWO_SIMPLE;
	roomMap[{ 2,  2 }] = ROOM_TYPE::BIG_X;
	roomMap[{ 2,  3 }] = ROOM_TYPE::CORNER_MIX;
	roomMap[{ 2,  5 }] = ROOM_TYPE::TUNNELS;
	roomMap[{ 3, -1 }] = ROOM_TYPE::CHECKERBOARD;
	roomMap[{ 3,  0 }] = ROOM_TYPE::SCATTER;
	roomMap[{ 3,  1 }] = ROOM_TYPE::BIG_X;
	roomMap[{ 3,  3 }] = ROOM_TYPE::FLY_LAPS;
	roomMap[{ 3,  4 }] = ROOM_TYPE::MIDLINE_PROJ;
	roomMap[{ 3,  5 }] = ROOM_TYPE::LAPS;
	roomMap[{ 3,  6 }] = ROOM_TYPE::MIDLINE_PROJ;
	roomMap[{ 4,  6 }] = ROOM_TYPE::CHECKERBOARD;
	roomMap[{ 4,  3 }] = ROOM_TYPE::CORNER_MIX;
	roomMap[{ 4,  2 }] = ROOM_TYPE::MIDLINE_PROJ;
	roomMap[{ 4,  1 }] = ROOM_TYPE::TWO_SIMPLE;
	roomMap[{ 4, -1 }] = ROOM_TYPE::FLY_TUNNELS;
	roomMap[{ 5, -1 }] = ROOM_TYPE::SCATTER;

	// BOSS ONE (6, -1)
	roomMap[{ 6, -1 }] = ROOM_TYPE::BOSS_ROOM_ONE;

	// FLOOR TWO MAP
	roomMap[{7, -1}] = ROOM_TYPE::ONE_HEAVY;
	roomMap[{7, -2}] = ROOM_TYPE::CHECKERBOARD;
	roomMap[{7, -3}] = ROOM_TYPE::MIDLINE_PROJ;
	roomMap[{6, -3}] = ROOM_TYPE::ENEMY_SOCIAL;
	roomMap[{8, -3}] = ROOM_TYPE::SCATTER;
	roomMap[{8, -4}] = ROOM_TYPE::TUNNELS;
	roomMap[{8, -5}] = ROOM_TYPE::TWO_SIMPLE;
	roomMap[{8, -6}] = ROOM_TYPE::BIG_X;
	roomMap[{9, -5}] = ROOM_TYPE::SCARECROW_ROOM;
	roomMap[{8, -1}] = ROOM_TYPE::CORNER_MIX;
	roomMap[{8, -2}] = ROOM_TYPE::FLY_TUNNELS;
	roomMap[{8, 0}] = ROOM_TYPE::LAPS;
	roomMap[{9, -1}] = ROOM_TYPE::TWO_HEAVY;
	roomMap[{9, 0}] = ROOM_TYPE::FLY_TUNNELS;
	roomMap[{10, 0}] = ROOM_TYPE::BIG_X;
	roomMap[{10, 1}] = ROOM_TYPE::ONE_HEAVY;
	roomMap[{10, 2}] = ROOM_TYPE::TWO_SIMPLE;
	roomMap[{9, 2}] = ROOM_TYPE::ENEMY_SOCIAL;
	roomMap[{11, 2}] = ROOM_TYPE::CHECKERBOARD;
	roomMap[{9, 3}] = ROOM_TYPE::TWO_HEAVY;
	roomMap[{8, 3}] = ROOM_TYPE::CORNER_MIX;
	roomMap[{8, 4}] = ROOM_TYPE::MIDLINE_PROJ;

	// BOSS TWO
	roomMap[{8, 5}] = ROOM_TYPE::BOSS_ROOM_TWO;

}

void createRoomByType(const std::pair<const std::pair<int, int>, ROOM_TYPE>& room, RenderSystem* renderer)
{
	const ivec2 coord = { room.first.first, room.first.second };
	ROOM_TYPE type = room.second;

	switch (type) {
	case ROOM_TYPE::EMPTY:
		createEmptyRoom(renderer, coord);
		break;
	case ROOM_TYPE::BOSS_ROOM_ONE:
		createBossRoomOne(renderer, coord);
		break;
	case ROOM_TYPE::BOSS_ROOM_TWO:
		createBossRoomTwo(renderer, coord);
		break;
	case ROOM_TYPE::BOSS_ROOM_THREE:
		createBossRoomThree(renderer, coord);
		break;
	case ROOM_TYPE::OLD_ROBOT_ROOM:
		createNPCRoom(renderer, coord, NPC_TYPE::OLD_ROBOT_NPC);
		break;
	case ROOM_TYPE::SCARECROW_ROOM:
		createNPCRoom(renderer, coord, NPC_TYPE::SCARECROW_NPC);
		break;
	default: // ALL ENEMY ROOMS
		createEnemyRoom(renderer, coord, type);
		break;
	}
}

void generate_rooms(RenderSystem* renderer, ivec2 current_room, std::uniform_real_distribution<float> uniform_dist, std::default_random_engine& rng) {

	if (registry.gameLoadingHelper.components.size() == 0 || registry.gameLoadingHelper.components[0].savedGame == false) {
		generate_map();
	}
	// Iterating using structured bindings
	auto roomMap = registry.map.components[0].roomMap;
	for (const auto& room : roomMap) {
		createRoomByType(room, renderer);
	}

	// need to only have the current entities as active.
	// some entities, such as UI elements and the player, do not have room coords, and must always be rendered
	registry.activeComponents.clear();
	for (Entity entity : registry.gameSceneComponents.entities) {
		if (!registry.roomCoords.has(entity)) {
			registry.activeComponents.emplace(entity);
		}
		else if (registry.roomCoords.get(entity).position == current_room) {
			registry.activeComponents.emplace(entity);
		}
	}
}

// useless lol
Entity create_self_destruct_text(RenderSystem* renderer, ivec2 current_room) {
	return createFloorText(renderer, "SELF DESTRUCT ACTIVE", { window_width_px / 2 - 350, window_height_px / 2 - 100 }, { 6, 2 }, current_room);
} 


void buildItemSet() {
	ItemStat shattered_quartz;
	shattered_quartz.name = ITEM_NAME::SHATTERED_QUARTZ;
	shattered_quartz.type = ITEM_TYPE::DAMAGE;
	shattered_quartz.flat_damage_mod = 1;
	shattered_quartz.flat_range = -200;	
	shattered_quartz.accuracy = 0.3;
	shattered_quartz.percent_fire_rate = -0.3;
	registry.all_items.push_back(shattered_quartz);
	registry.damage_items.push_back(shattered_quartz);

	ItemStat creaky_wheel;
	creaky_wheel.name = ITEM_NAME::CREAKY_WHEEL;
	creaky_wheel.type = ITEM_TYPE::SPEED;
	creaky_wheel.percent_speed_mod = 0.2;
	registry.all_items.push_back(creaky_wheel);
	registry.speed_items.push_back(creaky_wheel);

	ItemStat heatsink;
	heatsink.name = ITEM_NAME::HEATSINK;
	heatsink.type = ITEM_TYPE::FIRE_RATE;
	heatsink.percent_fire_rate = 0.1;
	registry.all_items.push_back(heatsink);
	registry.fire_rate_items.push_back(heatsink);

	ItemStat repeater;
	repeater.name = ITEM_NAME::REPEATER;
	repeater.type = ITEM_TYPE::RANGE;
	repeater.flat_range = 100;
	registry.all_items.push_back(repeater);
	registry.range_items.push_back(repeater);

	ItemStat battery_pack;
	battery_pack.name = ITEM_NAME::BATTERY_PACK;
	battery_pack.type = ITEM_TYPE::HEALTH_PACK;
	battery_pack.heal_size = 1;
	registry.all_items.push_back(battery_pack);
	registry.healing_items.push_back(battery_pack);

	ItemStat wd_4000;
	wd_4000.name = ITEM_NAME::WD4000;
	wd_4000.type = ITEM_TYPE::FIRE_RATE;
	wd_4000.percent_fire_rate = 0.1;
	wd_4000.accuracy = 0.1;
	registry.all_items.push_back(wd_4000);
	registry.fire_rate_items.push_back(wd_4000);

	ItemStat super_battery_pack;
	super_battery_pack.name = ITEM_NAME::SUPERCHARGED_BATTERY_PACK;
	super_battery_pack.type = ITEM_TYPE::HEALTH_PACK;
	super_battery_pack.heal_size = 2;
	registry.all_items.push_back(super_battery_pack);
	registry.healing_items.push_back(super_battery_pack);

	ItemStat volitile_blaster;
	volitile_blaster.name = ITEM_NAME::VOLITILE_BLASTER;
	volitile_blaster.type = ITEM_TYPE::DAMAGE;
	volitile_blaster.crit_chance = 3;
	registry.all_items.push_back(volitile_blaster);
	registry.damage_items.push_back(volitile_blaster);

	ItemStat hot_diesel;
	hot_diesel.name = ITEM_NAME::HOT_DIESEL;
	hot_diesel.type = ITEM_TYPE::SPEED;
	hot_diesel.percent_speed_mod = 0.1;
	hot_diesel.dodge_chance = 2;
	registry.all_items.push_back(hot_diesel);
	registry.speed_items.push_back(hot_diesel);

	ItemStat optical_sensor;
	optical_sensor.name = ITEM_NAME::OPTICAL_SENSOR;
	optical_sensor.type = ITEM_TYPE::RANGE;
	optical_sensor.flat_range = 75;
	optical_sensor.dodge_chance = 1;
	registry.all_items.push_back(optical_sensor);
	registry.range_items.push_back(optical_sensor);
}
