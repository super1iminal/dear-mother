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
	health.max_health = DEADLY_MAX_HEALTH;
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
				RENDER_ORDER::ENEMY });
	} else {
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
		worldobject.scale = vec2({ 300, 200 });
		registry.renderRequests.insert(
			entity,
			{ TEXTURE_ASSET_ID::MOTHER_FINAL_IDLE,
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

	Animation& enemy_animation = registry.animations.emplace(entity);
	enemy_animation.cols = 5;
	enemy_animation.rows = 1;
	enemy_animation.frames = 5;
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

Entity createItem(RenderSystem* renderer, 
	vec2 position, 
	vec2 size, 
	ITEM_TYPE spec_type, 
	std::uniform_real_distribution<float> uniform_dist, 
	std::default_random_engine& rng, 
	ivec2 room_coord, ItemStat* loaded_item
) {

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
			RENDER_ORDER::ITEM });

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
	createFloor(renderer, { window_width_px / 2, (window_height_px + 120.f) / 2 }, { window_width_px , window_height_px - 120.f }, coord);

	// left wall
	createWall(renderer, { 25.f, (window_height_px / 2) + WALL_WIDTH }, { WALL_WIDTH, 590.f }, 0.f, TEXTURE_ASSET_ID::VERT_WALL, coord);
	// right wall
	createWall(renderer, { window_width_px - 25.f, (window_height_px / 2) + WALL_WIDTH }, { WALL_WIDTH, 590.f }, M_PI, TEXTURE_ASSET_ID::VERT_WALL, coord);
	// top wall
	createWall(renderer, { window_width_px / 2, 25.f + 120.f }, { window_width_px, WALL_WIDTH }, 0.f, TEXTURE_ASSET_ID::HORZ_WALL, coord);
	// bottom wall
	createWall(renderer, { window_width_px / 2, window_height_px - 25.f }, { window_width_px, WALL_WIDTH }, M_PI, TEXTURE_ASSET_ID::HORZ_WALL, coord);


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
void createEnemyRoom(RenderSystem* renderer, ivec2 coord, std::uniform_real_distribution<float> uniform_dist, std::default_random_engine& rng) {
	
	// create a floor entity
	createFloor(renderer, { window_width_px / 2, (window_height_px + 120.f) / 2 }, { window_width_px , window_height_px - 120.f }, coord);

	//createInteractable(renderer, { window_width_px / 2, window_height_px - 200 }, { 75.f, 75.f }, bound_interactable_fn, 1, { 0, 0 });

	//// top wall
	//createWall(renderer, { window_width_px / 2, 25.f + 120.f }, { window_width_px, WALL_WIDTH }, 0.f, TEXTURE_ASSET_ID::HORZ_WALL, coord);
	//// bottom wall
	//createWall(renderer, { window_width_px / 2, window_height_px - 25.f }, { window_width_px, WALL_WIDTH }, M_PI, TEXTURE_ASSET_ID::HORZ_WALL, coord);
	//// left wall
	//createWall(renderer, { 25.f, (window_height_px / 2) + 60.f }, { WALL_WIDTH, window_height_px - 120.f }, 0.f, TEXTURE_ASSET_ID::VERT_WALL, coord);
	//// right wall
	//createWall(renderer, { window_width_px - 25.f, (window_height_px / 2) + 60.f }, { WALL_WIDTH, window_height_px - 120.f }, M_PI, TEXTURE_ASSET_ID::VERT_WALL, coord);

	// left wall
	createWall(renderer, { 25.f, (window_height_px / 2) + WALL_WIDTH }, { WALL_WIDTH, 590.f }, 0.f, TEXTURE_ASSET_ID::VERT_WALL, coord);
	// right wall
	createWall(renderer, { window_width_px - 25.f, (window_height_px / 2) + WALL_WIDTH }, { WALL_WIDTH, 590.f }, M_PI, TEXTURE_ASSET_ID::VERT_WALL, coord);
	// top wall
	createWall(renderer, { window_width_px / 2, 25.f + 120.f }, { window_width_px, WALL_WIDTH }, 0.f, TEXTURE_ASSET_ID::HORZ_WALL, coord);
	// bottom wall
	createWall(renderer, { window_width_px / 2, window_height_px - 25.f }, { window_width_px, WALL_WIDTH }, M_PI, TEXTURE_ASSET_ID::HORZ_WALL, coord);

	if (!(coord.x == 0 && coord.y == 0)) { // no enemies in base room
		float scalingFactor = sqrt(coord.x * coord.x + coord.y + coord.y); // gets harder as you move further from spawn
		int numEnemies = ((int)rand() % 2) + 1;
		if (scalingFactor > 4)
			numEnemies += 1;
		else if (scalingFactor > 2)
			numEnemies += 2;

		if (registry.gameLoadingHelper.components.size() == 0 || registry.gameLoadingHelper.components[0].savedGame == false) {
			for (int i = 0; i < numEnemies; i++) { // create a variable number of enemies of random type
				createEnemy(renderer, vec2((uniform_dist(rng) * (window_width_px - (2 * WALL_WIDTH))) + WALL_WIDTH, ((uniform_dist(rng) * (window_height_px - (2 * WALL_WIDTH) - BASE_UI_HEIGHT))) + WALL_WIDTH + BASE_UI_HEIGHT), ENEMY_SPEED, coord);
			}
		}

	}
	if (registry.gameLoadingHelper.components.size() == 0 || registry.gameLoadingHelper.components[0].savedGame == false) {
		// create numFloorItems floor items (functionally a wall)
	// need to be cautious of spawn location, not near doors (euclidean distance) or player/enemies (overlap) or on top of each other (overlap)
		int numFloorItems = (int)rand() % 3;

		float minX = WALL_WIDTH + FLOOR_ITEM_SIZE / 2;
		float minY = WALL_WIDTH + BASE_UI_HEIGHT + FLOOR_ITEM_SIZE / 2;


		float xRange = window_width_px - (WALL_WIDTH * 2) - FLOOR_ITEM_SIZE;
		float yRange = window_height_px - (WALL_WIDTH * 2) - BASE_UI_HEIGHT - FLOOR_ITEM_SIZE;

		float xPos;
		float yPos;
		while (numFloorItems > 0) {
			do {
				xPos = minX + rand() % (int)xRange;
				yPos = minY + rand() % (int)yRange;
			} while (notSafe({ xPos, yPos }));

			TEXTURE_ASSET_ID floortexture;
			int seed = rand() % 7;
			switch (seed) {
			case 0:
				floortexture = TEXTURE_ASSET_ID::FURNACE;
			case 1:
				floortexture = TEXTURE_ASSET_ID::BROKEN_GENERATOR;
			case 2:
				floortexture = TEXTURE_ASSET_ID::DEAD_ROBOT;
			case 3:
				floortexture = TEXTURE_ASSET_ID::BROKEN_CONTROL_PANEL;
			case 4:
				floortexture = TEXTURE_ASSET_ID::FLOOR_HOLE;
			case 5:
				floortexture = TEXTURE_ASSET_ID::RUSTY_PIPES;
			default:
				floortexture = TEXTURE_ASSET_ID::SLAG_PIT;
			}

			createWall(renderer, { xPos, yPos }, { FLOOR_ITEM_SIZE, FLOOR_ITEM_SIZE }, 0.f, floortexture, coord);
			numFloorItems--;
		}
	}


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
}

void createNPCRoom(RenderSystem* renderer, ivec2 coord, NPC_TYPE npc_type) {
	createFloor(renderer, { window_width_px / 2, (window_height_px + 120.f) / 2 }, { window_width_px , window_height_px - 120.f }, coord);

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
	createFloor(renderer, { window_width_px / 2, (window_height_px + 120.f) / 2 }, { window_width_px , window_height_px - 120.f }, coord, FLOOR_TYPE::BOSS_ROOM_ONE);

	// left wall
	createWall(renderer, { 25.f, (window_height_px / 2) + 30.f }, { WALL_WIDTH, window_height_px - 70.f }, 0.f, TEXTURE_ASSET_ID::BOSS_ONE_VERT_WALL, coord);
	// right wall
	createWall(renderer, { window_width_px - 25.f, (window_height_px / 2) + 30.f }, { WALL_WIDTH, window_height_px - 70.f }, M_PI, TEXTURE_ASSET_ID::BOSS_ONE_VERT_WALL, coord);
	// top wall
	createWall(renderer, { window_width_px / 2, /*25.f + 120.f*/ 90 }, { window_width_px, WALL_WIDTH }, 0.f, TEXTURE_ASSET_ID::BOSS_ONE_HORZ_WALL, coord);
	// bottom wall
	createWall(renderer, { window_width_px / 2, window_height_px - 25.f }, { window_width_px, WALL_WIDTH }, M_PI, TEXTURE_ASSET_ID::BOSS_ONE_HORZ_WALL, coord);


	float minX = WALL_WIDTH + FLOOR_ITEM_SIZE / 2;
	float minY = WALL_WIDTH + BASE_UI_HEIGHT + FLOOR_ITEM_SIZE / 2;

	float xRange = window_width_px - (WALL_WIDTH * 2) - FLOOR_ITEM_SIZE;
	float yRange = window_height_px - (WALL_WIDTH * 2) - BASE_UI_HEIGHT - FLOOR_ITEM_SIZE;
	auto roomMap = registry.map.components[0].roomMap;
	if (roomMap.find({ coord.x + 1, coord.y }) != roomMap.end()) {
		createDoor(renderer, coord, { coord.x + 1, coord.y }, DIRECTION::RIGHT);
	}
	if (roomMap.find({ coord.x - 1, coord.y }) != roomMap.end()) {
		createDoor(renderer, coord, { coord.x - 1, coord.y }, DIRECTION::LEFT);
	}
	/*if (roomMap.find({ coord.x, coord.y + 1 }) != roomMap.end()) {
		createDoor(renderer, coord, { coord.x, coord.y + 1 }, DIRECTION::UP);
	}*/
	if (roomMap.find({ coord.x, coord.y - 1 }) != roomMap.end()) {
		createDoor(renderer, coord, { coord.x, coord.y - 1 }, DIRECTION::DOWN);
	}

	createBossOne(renderer, { WALL_WIDTH + 500, WALL_WIDTH + BASE_UI_HEIGHT + 75 + 50 }, BOSS_ONE_POS::TOP_LEFT, coord);
	createBossOne(renderer, { WALL_WIDTH + 700, WALL_WIDTH + BASE_UI_HEIGHT + 75 + 50 }, BOSS_ONE_POS::TOP_RIGHT, coord);

	createBossOne(renderer, { WALL_WIDTH + 500, WALL_WIDTH + BASE_UI_HEIGHT + 375 + 30 }, BOSS_ONE_POS::BOT_LEFT, coord);
	createBossOne(renderer, { WALL_WIDTH + 700, WALL_WIDTH + BASE_UI_HEIGHT + 375 + 30 }, BOSS_ONE_POS::BOT_RIGHT, coord);

	createBossOne(renderer, { CENTER_X, 225 }, BOSS_ONE_POS::MOTHER, coord);

	// Decoration
	createWall(renderer, { WALL_WIDTH + 300, WALL_WIDTH + BASE_UI_HEIGHT - 20 }, { ENEMY_BB_WIDTH - 30, ENEMY_BB_HEIGHT - 30 }, 0.f, TEXTURE_ASSET_ID::ENEMY_ROBOT_OFF, coord);
	createWall(renderer, { WALL_WIDTH + 200, WALL_WIDTH + BASE_UI_HEIGHT - 20 }, { ENEMY_BB_WIDTH - 30, ENEMY_BB_HEIGHT - 30 }, 0.f, TEXTURE_ASSET_ID::ENEMY_ROBOT_OFF, coord);
	createWall(renderer, { WALL_WIDTH + 100, WALL_WIDTH + BASE_UI_HEIGHT - 20 }, { ENEMY_BB_WIDTH - 30, ENEMY_BB_HEIGHT - 30 }, 0.f, TEXTURE_ASSET_ID::ENEMY_ROBOT_OFF, coord);

	createWall(renderer, { WALL_WIDTH + 800, WALL_WIDTH + BASE_UI_HEIGHT - 20 }, { FLOOR_ITEM_SIZE, FLOOR_ITEM_SIZE }, 0.f, TEXTURE_ASSET_ID::DEAD_ROBOT, coord);
	createWall(renderer, { WALL_WIDTH + 900, WALL_WIDTH + BASE_UI_HEIGHT - 20 }, { FLOOR_ITEM_SIZE, FLOOR_ITEM_SIZE }, 0.f, TEXTURE_ASSET_ID::DEAD_ROBOT, coord);
	createWall(renderer, { WALL_WIDTH + 1000, WALL_WIDTH + BASE_UI_HEIGHT - 20 }, { FLOOR_ITEM_SIZE, FLOOR_ITEM_SIZE }, 0.f, TEXTURE_ASSET_ID::DEAD_ROBOT, coord);
}

void createBossRoomTwo(RenderSystem* renderer, ivec2 coord) {
	// create a floor entity
	createFloor(renderer, { window_width_px / 2, (window_height_px + 120.f) / 2 }, { window_width_px , window_height_px - 120.f }, coord);


	createWall(renderer, { window_width_px / 2, 25.f + 120.f }, { window_width_px, WALL_WIDTH }, 0.f, TEXTURE_ASSET_ID::HORZ_WALL, coord);
	// Boss Two
	createBossTwo(renderer, { window_width_px / 2, 60.f + 120.f }, coord);
	// left wall
	createWall(renderer, { 25.f, (window_height_px / 2) + WALL_WIDTH }, { WALL_WIDTH, 590.f }, 0.f, TEXTURE_ASSET_ID::VERT_WALL, coord);
	// right wall
	createWall(renderer, { window_width_px - 25.f, (window_height_px / 2) + WALL_WIDTH }, { WALL_WIDTH, 590.f }, M_PI, TEXTURE_ASSET_ID::VERT_WALL, coord);
	// bottom wall
	createWall(renderer, { window_width_px / 2, window_height_px - 25.f }, { window_width_px, WALL_WIDTH }, M_PI, TEXTURE_ASSET_ID::HORZ_WALL, coord);

	float minX = WALL_WIDTH + FLOOR_ITEM_SIZE / 2;
	float minY = WALL_WIDTH + BASE_UI_HEIGHT + FLOOR_ITEM_SIZE / 2;

	float xRange = window_width_px - (WALL_WIDTH * 2) - FLOOR_ITEM_SIZE;
	float yRange = window_height_px - (WALL_WIDTH * 2) - BASE_UI_HEIGHT - FLOOR_ITEM_SIZE;

	auto roomMap = registry.map.components[0].roomMap;
	if (roomMap.find({ coord.x + 1, coord.y }) != roomMap.end()) {
		createDoor(renderer, coord, { coord.x + 1, coord.y }, DIRECTION::RIGHT);
	}
	if (roomMap.find({ coord.x - 1, coord.y }) != roomMap.end()) {
		createDoor(renderer, coord, { coord.x - 1, coord.y }, DIRECTION::LEFT);
	}
	/*if (roomMap.find({ coord.x, coord.y + 1 }) != roomMap.end()) {
		createDoor(renderer, coord, { coord.x, coord.y + 1 }, DIRECTION::UP);
	}*/
	if (roomMap.find({ coord.x, coord.y - 1 }) != roomMap.end()) {
		createDoor(renderer, coord, { coord.x, coord.y - 1 }, DIRECTION::DOWN);
	}

}

void generate_map() {
	if (registry.map.components.size() > 0) {
		registry.remove_all_components_of(registry.map.entities[0]);
	}
	auto entity = Entity();
	registry.map.emplace(entity);
	std::map<std::pair<int, int>, ROOM_TYPE>& roomMap = registry.map.get(entity).roomMap;
	roomMap[{0, 0}] = ROOM_TYPE::ENEMY_ROOM;
	roomMap[{1, 0}] = ROOM_TYPE::OLD_ROBOT_ROOM;
	roomMap[{1, 1}] = ROOM_TYPE::SCARECROW_ROOM;
	//roomMap[{1, 0}] = ROOM_TYPE::ENEMY_ROOM;
	//roomMap[{0, 1}] = ROOM_TYPE::ENEMY_ROOM;
	roomMap[{0, 1}] = ROOM_TYPE::EMPTY;
	//roomMap[{1, 1}] = ROOM_TYPE::ENEMY_ROOM;
	roomMap[{1, 2}] = ROOM_TYPE::EMPTY;
	roomMap[{1, 3}] = ROOM_TYPE::EMPTY;
	//roomMap[{0, 2}] = ROOM_TYPE::ENEMY_ROOM;
	roomMap[{0, 2}] = ROOM_TYPE::EMPTY;
	//roomMap[{0, 3}] = ROOM_TYPE::ENEMY_ROOM;
	roomMap[{0, -1}] = ROOM_TYPE::ENEMY_ROOM;
	roomMap[{1, -1}] = ROOM_TYPE::ENEMY_ROOM;
	roomMap[{2, 0}] = ROOM_TYPE::BOSS_ROOM_ONE;
	roomMap[{2, 2}] = ROOM_TYPE::BOSS_ROOM_TWO;
	roomMap[{2, 1}] = ROOM_TYPE::EMPTY;
	roomMap[{2, 3}] = ROOM_TYPE::EMPTY;
	roomMap[{-1, -1}] = ROOM_TYPE::ENEMY_ROOM;
	roomMap[{-1, -2}] = ROOM_TYPE::ENEMY_ROOM;
	roomMap[{-1, 3}] = ROOM_TYPE::ENEMY_ROOM;
	roomMap[{-2, 3}] = ROOM_TYPE::ENEMY_ROOM;
	roomMap[{-2, 2}] = ROOM_TYPE::ENEMY_ROOM;
	roomMap[{-2, 4}] = ROOM_TYPE::ENEMY_ROOM;
}

void generate_rooms(RenderSystem* renderer, ivec2 current_room, std::uniform_real_distribution<float> uniform_dist, std::default_random_engine& rng) {

	if (registry.gameLoadingHelper.components.size() == 0 || registry.gameLoadingHelper.components[0].savedGame == false) {
		generate_map();
	}
	// Iterating using structured bindings
	auto roomMap = registry.map.components[0].roomMap;
	for (const auto& room : roomMap) {
		const ivec2 coord = { room.first.first, room.first.second };
		printf("creating room at %d, %d\n", coord.x, coord.y);
		ROOM_TYPE type = room.second;

		switch (type) {
		case ROOM_TYPE::ENEMY_ROOM:
			createEnemyRoom(renderer, coord, uniform_dist, rng);
			break;
		case ROOM_TYPE::EMPTY:
			createEmptyRoom(renderer, coord);
			break;
		case ROOM_TYPE::BOSS_ROOM_ONE:
			createBossRoomOne(renderer, coord);
			break;
		case ROOM_TYPE::BOSS_ROOM_TWO:
			createBossRoomTwo(renderer, coord);
			break;
		case ROOM_TYPE::OLD_ROBOT_ROOM:
			createNPCRoom(renderer, coord, NPC_TYPE::OLD_ROBOT_NPC);
			break;
		case ROOM_TYPE::SCARECROW_ROOM:
			createNPCRoom(renderer, coord, NPC_TYPE::SCARECROW_NPC);
			break;
		}
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
	health.max_health = PLAYER_MAX_HEALTH;
	health.curr_health = PLAYER_MAX_HEALTH; // TODO: why doesn't this load the player's health?
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
			GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_ORDER::PLAYER });

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
		health.max_health = DEADLY_MAX_HEALTH;
		health.curr_health = DEADLY_MAX_HEALTH;
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
				GEOMETRY_BUFFER_ID::SPRITE,
				RENDER_ORDER::ENEMY
			});
	}
}