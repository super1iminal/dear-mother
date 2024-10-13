#include "world_init.hpp"
#include "tiny_ecs_registry.hpp"

#include <iostream>

void createParticle(RenderSystem* renderer, vec2 pos, std::uniform_real_distribution<float> uniform_dist, std::default_random_engine& rng, TEXTURE_ASSET_ID type) {
	auto entity = Entity();

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
	registry.renderRequests.insert(
		entity,
		{ type,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE });
}

void createParticles(RenderSystem* renderer, vec2 pos, std::uniform_real_distribution<float> uniform_dist, std::default_random_engine& rng, TEXTURE_ASSET_ID type) {
	float num_particles = ceil(uniform_dist(rng) * MAX_NUM_PARTICLES) + NUM_PARTICLES_OFFSET;
	for (int i = 0; i < num_particles; i++) {
		createParticle(renderer, pos, uniform_dist, rng, type);
	}
}

// NOTE: when creating a wall, then angle represents the normal. it is necessary for collision handling
Entity createWall(RenderSystem* renderer, vec2 pos, vec2 size, float angle, TEXTURE_ASSET_ID type) {
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);
	registry.walls.emplace(entity);

	// Setting initial position, scale, and orientation values
	WorldObject& worldobject = registry.worldObjects.emplace(entity);
	worldobject.position = pos;
	worldobject.angle = angle;
	worldobject.scale = size;

	registry.blockers.emplace(entity);
	// don't be fooled, type is type
	registry.renderRequests.insert(
		entity,
		{ type,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE });

	return entity;

}

Entity createPlayer(RenderSystem* renderer, vec2 pos)
{
	auto entity = Entity();

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
	health.max_health = 5;
	health.curr_health = 5;
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::PLAYER,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

Entity createEnemy(RenderSystem* renderer, vec2 position, float speed)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the motion
	auto& motion = registry.motions.emplace(entity);
	motion.max_speed = speed;
	motion.velocity = { 0.f, 0.f };
	motion.acceleration = { 0.f, 0.f };

	// setting position, scale, orientation
	WorldObject& worldobject = registry.worldObjects.emplace(entity);
	worldobject.position = position;
	worldobject.angle = 0.f;
	worldobject.scale = vec2({ -ENEMY_BB_WIDTH, ENEMY_BB_HEIGHT });

	// create an empty Enemy component to be able to refer to all enemies
	registry.deadlys.emplace(entity);
	auto& shooter = registry.shooters.emplace(entity);
	shooter.fire_rate = 10000.0f;
	auto& health = registry.healthComponents.emplace(entity);
	health.max_health = 5;
	health.curr_health = 5;
	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::ENEMY,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		});

	return entity;
}

Entity createFloor(RenderSystem* renderer, vec2 position, vec2 size) {
	// create an entity in order to render the floor background
	auto floor = Entity();
	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(floor, &mesh);
	registry.floors.emplace(floor);

	// Setting initial position, scale, and orientation values
	WorldObject& worldobject = registry.worldObjects.emplace(floor);
	worldobject.position = position;
	worldobject.angle = 0.f;
	worldobject.scale = size;

	registry.renderRequests.insert(
		floor,
		{ TEXTURE_ASSET_ID::FLOOR,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE });

	return floor;
}

Entity createInteractable(RenderSystem* renderer, vec2 position, vec2 size, void (*a)(int)) {
	// create an interactable entity
	Entity interactable_entity = Entity();

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(interactable_entity, &mesh);

	Interactable& interactable = registry.interactables.emplace(interactable_entity);
	interactable.range = 50.f;
	interactable.interaction = a;

	// Setting initial position, scale, and orientation values
	WorldObject& interactable_object = registry.worldObjects.emplace(interactable_entity);
	interactable_object.position = position;
	interactable_object.angle = 0.f;
	interactable_object.scale = size;

	registry.renderRequests.insert(
		interactable_entity,
		{ TEXTURE_ASSET_ID::ITEM,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE });

	return interactable_entity;
}

Entity createCrosshair(RenderSystem* renderer) {
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);
	registry.crosshair.emplace(entity);

	// setting position, scale, orientation
	WorldObject& worldobject = registry.worldObjects.emplace(entity);
	worldobject.position = { -1.f, -1.f }; // initializing position to off screen
	worldobject.angle = 0.f; 
	worldobject.scale = vec2({ CROSSHAIR_SIZE, CROSSHAIR_SIZE });

	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::CROSSHAIR,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		});

	return entity;
}

Entity createProjectile(RenderSystem* renderer, vec2 pos, float angle, float speed, bool is_friendly)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
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
	worldObject.scale.y *= -1; // point front to the right

	Lifetime& lifetime = registry.lifetimes.emplace(entity);
	lifetime.time_remaining_ms = PROJECTILE_LIFESPAN;

	Projectile& projectile = registry.projectiles.emplace(entity);
	projectile.friendly = is_friendly;
	if (is_friendly) 
	{
		registry.renderRequests.insert
		(
			entity,
			{ TEXTURE_ASSET_ID::BULLET_FRIENDLY,
				EFFECT_ASSET_ID::TEXTURED,
				GEOMETRY_BUFFER_ID::SPRITE 
			}
		);
	}
	else 
	{
		registry.renderRequests.insert
		(
			entity,
			{ TEXTURE_ASSET_ID::BULLET_ENEMY,
				EFFECT_ASSET_ID::TEXTURED,
				GEOMETRY_BUFFER_ID::SPRITE 
			}
		);
	}


	return entity;
}

Entity createLine(vec2 position, vec2 scale)
{
	Entity entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	registry.renderRequests.insert(
		entity, {
			TEXTURE_ASSET_ID::TEXTURE_COUNT,
			EFFECT_ASSET_ID::EGG,
			GEOMETRY_BUFFER_ID::DEBUG_LINE
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

Entity createBaseUI(RenderSystem* renderer)
{
	Entity entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);
	registry.baseUI.emplace(entity);

	// Setting initial position, scale, and orientation values
	WorldObject& worldobject = registry.worldObjects.emplace(entity);
	worldobject.position = vec2(window_width_px / 2, BASE_UI_HEIGHT/2); // should be based on texture size later
	worldobject.angle = 0.f;
	worldobject.scale.x = window_width_px;
	worldobject.scale.y = BASE_UI_HEIGHT; // later this should be based on texture height

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::UI,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

Entity createTexturedUIElement(RenderSystem* renderer, vec2 pos, vec2 scale, std::string element_name, float element_value) {
	// Store a reference to the potentially re-used mesh object
	Entity entity = Entity();
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SQUARE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial position, scale, and orientation values
	WorldObject& worldobject = registry.worldObjects.emplace(entity);
	worldobject.position = pos;
	worldobject.scale = scale;

	// setting health value for UI
	UIElement& health_ui_elt = registry.uiElements.emplace(entity);
	health_ui_elt.name = element_name;
	health_ui_elt.value = static_cast<float>(element_value);

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::TEXTURE_COUNT,
			EFFECT_ASSET_ID::UI_ELEMENT,
			GEOMETRY_BUFFER_ID::SQUARE });

	return entity;
}

Entity createTexturedUIElement(RenderSystem* renderer, vec2 pos, vec2 scale, std::string element_name, TEXTURE_ASSET_ID texture_id) {
	// Store a reference to the potentially re-used mesh object
	Entity entity = Entity();
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SQUARE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial position, scale, and orientation values
	WorldObject& worldobject = registry.worldObjects.emplace(entity);
	worldobject.position = pos;
	worldobject.scale = scale;

	// setting value for UI
	UIElement& ui_element = registry.uiElements.emplace(entity);
	ui_element.name = element_name;
	ui_element.value = static_cast<float>(texture_id);

	registry.renderRequests.insert(
		entity,
		{ texture_id,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}