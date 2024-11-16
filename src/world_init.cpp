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

void createParticle(RenderSystem* renderer, vec2 pos, std::uniform_real_distribution<float> uniform_dist, std::default_random_engine& rng, TEXTURE_ASSET_ID type, ivec2 room_coord) {
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
			5
		});
}

void createParticles(RenderSystem* renderer, vec2 pos, std::uniform_real_distribution<float> uniform_dist, std::default_random_engine& rng, TEXTURE_ASSET_ID type, ivec2 room_coord) {
	float num_particles = ceil(uniform_dist(rng) * MAX_NUM_PARTICLES) + NUM_PARTICLES_OFFSET;
	for (int i = 0; i < num_particles; i++) {
		createParticle(renderer, pos, uniform_dist, rng, type, room_coord);
	}
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
	if (type == TEXTURE_ASSET_ID::VERT_WALL || type == TEXTURE_ASSET_ID::HORZ_WALL) {
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
			3 });

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
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
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
	worldobject.scale = vec2(mesh.original_size.x * PLAYER_SIZE, mesh.original_size.y * PLAYER_SIZE * 1.3);

	// create an empty Player component for our character
	registry.players.emplace(entity);
	auto& shooter = registry.shooters.emplace(entity);
	shooter.fire_rate = 500.0f;
	auto& health = registry.healthComponents.emplace(entity);
	health.max_health = 10;
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
			10 });

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
	motion.velocity = { 0.f, 0.f };
	motion.acceleration = { 0.f, 0.f };

	// create an empty Enemy component to be able to refer to all enemies
	auto& deadly = registry.deadlys.emplace(entity);
	deadly.t = std::chrono::high_resolution_clock::now();
	deadly.t_patrol = std::chrono::high_resolution_clock::now();
	deadly.immune = false;

	deadly.type = type;  // 0 for grey melee, 1 for slower yellow projectile

	if (registry.deadlys.get(entity).type == 1) {
		registry.motions.get(entity).max_speed = 0.7 * speed;
		auto& shooter = registry.shooters.emplace(entity);
		shooter.fire_rate = std::numeric_limits<int>::max();
	}
  	auto& health = registry.healthComponents.emplace(entity);
	health.max_health = 5;
	health.curr_health = curr_health;

	// setting position, scale, orientation
	WorldObject& worldobject = registry.worldObjects.emplace(entity);
	worldobject.position = position;
	worldobject.angle = 0.f;
	worldobject.scale = vec2({ -ENEMY_BB_WIDTH, ENEMY_BB_HEIGHT });

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
				9 });
	} else {
		registry.renderRequests.insert_sorted(
			entity,
			{ TEXTURE_ASSET_ID::ENEMY_2_WALK,
				EFFECT_ASSET_ID::ANIM,
				GEOMETRY_BUFFER_ID::SPRITE,
				9 });
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

	registry.shooters.emplace(entity).fire_rate = 1000.f;

	Animation& enemy_animation = registry.animations.emplace(entity);
	enemy_animation.cols = 4;
	enemy_animation.rows = 1;
	enemy_animation.frames = 1;
	enemy_animation.current_frame = 0;

	if (boss_pos == BOSS_ONE_POS::MOTHER) {
		deadly.type = 3;
		deadly.immune = true;
		health.curr_health = 20;
		worldobject.scale = vec2({ 1000, 50 });
		registry.renderRequests.insert(
			entity,
			{ TEXTURE_ASSET_ID::ENEMY_2_WALK,
				EFFECT_ASSET_ID::ANIM,
				GEOMETRY_BUFFER_ID::SPRITE });
	}
	else {
		deadly.type = 2;
		deadly.immune = false;
		worldobject.scale = vec2({ 70, 100 });
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
	worldobject.scale = { window_width_px, WALL_WIDTH };

	registry.bossTwos.emplace(entity).curr_wave = BOSS_TWO_WAVE::WAVE_ONE;

	registry.walls.emplace(entity);

	registry.blockers.emplace(entity);
	// don't be fooled, type is type
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::HORZ_WALL,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

Entity createFloor(RenderSystem* renderer, vec2 position, vec2 size, ivec2 room_coord) {
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

	registry.renderRequests.insert_sorted(
		floor,
		{ TEXTURE_ASSET_ID::FLOOR,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			2 });

	return floor;
}

// need to add to collisions
Entity createDoor(RenderSystem* renderer, ivec2 room_coord, ivec2 leads_to, DIRECTION orientation) {
	// create an entity in order to render the floor background
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
		worldobject.position = { window_width_px/2.f, BASE_UI_HEIGHT + WALL_WIDTH/2.f };
		worldobject.angle = 0.f;
		worldobject.scale = vec2({ 170.f, WALL_WIDTH + 4 });
		texture_id = TEXTURE_ASSET_ID::DOOR_UP_DOWN;
		break;
	case DIRECTION::DOWN:
		worldobject.position = { window_width_px / 2.f, window_height_px - WALL_WIDTH/2.f};
		worldobject.angle = M_PI;
		worldobject.scale = vec2({ 170.f, WALL_WIDTH + 4 });
		texture_id = TEXTURE_ASSET_ID::DOOR_UP_DOWN;
		break;
	case DIRECTION::LEFT:
		worldobject.position = { WALL_WIDTH / 2.f, (window_height_px-BASE_UI_HEIGHT)/2.f + BASE_UI_HEIGHT};
		worldobject.angle = 0.f;
		worldobject.scale = vec2({ WALL_WIDTH + 4, 170.f });
		break;
	case DIRECTION::RIGHT:
		worldobject.position = { window_width_px - WALL_WIDTH / 2.f, (window_height_px - BASE_UI_HEIGHT) / 2.f + BASE_UI_HEIGHT };
		worldobject.angle = M_PI;
		worldobject.scale = vec2({ WALL_WIDTH + 4, 170.f });
		break;
	}

	registry.renderRequests.insert_sorted(
		door,
		{ texture_id,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			4 });

	return door;
}

Entity createInteractable(RenderSystem* renderer, vec2 position, vec2 size, std::function<void(int)> function, int value, ivec2 room_coord) {
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
			5 });

	return interactable_entity;
}


Entity createItem(RenderSystem* renderer, vec2 position, vec2 size, ITEM_TYPE spec_type, std::uniform_real_distribution<float> uniform_dist, std::default_random_engine& rng, ivec2 room_coord, ItemStat* loaded_item) {

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
	}

	registry.renderRequests.insert_sorted(
		entity,
		{ item_texture,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			6 });

	return entity;
}

Entity createProjectile(RenderSystem* renderer, vec2 pos, float angle, float speed, bool is_friendly, ivec2 room_coord)
{
	auto entity = Entity();
	registry.gameSceneComponents.emplace(entity);
	registry.roomCoords.emplace(entity, room_coord);
	registry.activeComponents.emplace(entity);

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SALMON);
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
	lifetime.time_remaining_ms = PROJECTILE_LIFESPAN;

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
				11
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
				12
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
			20
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

void buildItemSet() {
	ItemStat shattered_quartz;
	shattered_quartz.name = ITEM_NAME::SHATTERED_QUARTZ;
	shattered_quartz.type = ITEM_TYPE::DAMAGE;
	shattered_quartz.flat_damage_mod = 1;
	shattered_quartz.flat_range = -100;	// May change debuff to just accuracy 
	shattered_quartz.accuracy = 0.05;
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
}


void createLoadedGame(RenderSystem *renderer) {
	auto entity = registry.players.entities[0];
	registry.gameSceneComponents.emplace(entity);
		registry.activeComponents.emplace(entity);
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	auto& shooter = registry.shooters.emplace(entity);
	shooter.fire_rate = 500.0f;
	auto& health = registry.healthComponents.emplace(entity);
	health.max_health = 5;
	health.curr_health = 5;
	registry.inventory.emplace(entity);
	registry.modifiers.emplace(entity);
		Animation& player_animation = registry.animations.emplace(entity);
		player_animation.cols = 4;
		player_animation.rows = 1;
		player_animation.frames = 1;
		player_animation.current_frame = 0;
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::PLAYER_WALK,
			EFFECT_ASSET_ID::ANIM,
			GEOMETRY_BUFFER_ID::SPRITE });

	for (Entity entity : registry.deadlys.entities) {
		registry.gameSceneComponents.emplace(entity);
		registry.activeComponents.emplace(entity);
		Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
		registry.meshPtrs.emplace(entity, &mesh);
		auto& deadly = registry.deadlys.emplace(entity);
		deadly.t = std::chrono::high_resolution_clock::now();
		deadly.t_patrol = std::chrono::high_resolution_clock::now();
		auto& shooter = registry.shooters.emplace(entity);
		shooter.fire_rate = std::numeric_limits<int>::max();
		auto& health = registry.healthComponents.emplace(entity);
		health.max_health = 5;
		health.curr_health = 5;
		Animation& enemy_animation = registry.animations.emplace(entity);
		enemy_animation.cols = 4;
		enemy_animation.rows = 1;
		enemy_animation.frames = 1;
		enemy_animation.current_frame = 0;
		registry.renderRequests.insert(
			entity,
			{
				TEXTURE_ASSET_ID::ENEMY,
				EFFECT_ASSET_ID::ANIM,
				GEOMETRY_BUFFER_ID::SPRITE
			});
	}
}
