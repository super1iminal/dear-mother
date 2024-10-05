#include "world_init.hpp"
#include "tiny_ecs_registry.hpp"

Entity createPlayer(RenderSystem* renderer, vec2 pos)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial motion value
	Motion& motion = registry.motions.emplace(entity);
	motion.velocity = { 0.f, 0.f };

	// setting position, scale, orientation
	WorldObject& worldobject = registry.worldobjects.emplace(entity);
	worldobject.position = pos;
	worldobject.angle = 0.f;
	worldobject.scale = mesh.original_size * 300.f;
	worldobject.scale.y *= -1; // point front to the right

	// create an empty Player component for our character
	registry.players.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::FISH,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

Entity createEnemy(RenderSystem* renderer, vec2 position)
{
	auto entity = Entity();

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Initialize the motion
	auto& motion = registry.motions.emplace(entity);
	motion.velocity = { 0, 100.f };

	// setting position, scale, orientation
	WorldObject& worldobject = registry.worldobjects.emplace(entity);
	worldobject.position = position;
	worldobject.angle = 0.f;
	worldobject.scale = vec2({ -ENEMY_BB_WIDTH, ENEMY_BB_HEIGHT });

	// create an empty Enemy component to be able to refer to all enemies
	registry.deadlys.emplace(entity);
	registry.renderRequests.insert(
		entity,
		{
			TEXTURE_ASSET_ID::BOUNDBOX,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		});

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
	motion.velocity = { 0, 0 };

	// setting position, scale, orientation
	WorldObject& worldobject = registry.worldobjects.emplace(entity);
	worldobject.position = position;
	worldobject.angle = 0.f;
	worldobject.scale = scale;

	registry.debugComponents.emplace(entity);
	return entity;
}