// internal
#include <common.hpp>
#include <ui_system.hpp>
#include <render_system.hpp>
#include <tiny_ecs_registry.hpp>

UISystem::UISystem()
{

}

UISystem::~UISystem() {

}

void UISystem::init(GLFWwindow* window_arg, SCENE_TYPE* scene_arg) {
	this->window = window_arg;
	this->scene = scene_arg;
}

// creates a basic panel with the given texture
// used to render base UI components ex. the start menu background
Entity UISystem::createPanel(
	RenderSystem* renderer, 
	SCENE_TYPE scene_type, 
	vec2 pos, 
	float angle, 
	vec2 scale, 
	std::string name, 
	TEXTURE_ASSET_ID texture)
{
	Entity entity = Entity();
	switch (scene_type) {
	case SCENE_TYPE::GAME:
		registry.gameSceneComponents.emplace(entity);
		break;
	case SCENE_TYPE::MENU:
		registry.menuSceneComponents.emplace(entity);
		break;
	case SCENE_TYPE::PAUSE:
		registry.pauseSceneComponents.emplace(entity);
		break;
	case SCENE_TYPE::TEST:
		registry.testSceneComponents.emplace(entity);
		break;
	}

	// Store a reference to the potentially re-used mesh object
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);
	BaseUI& baseUI = registry.baseUI.emplace(entity);
	baseUI.name = name;

	// Setting initial position, scale, and orientation values
	WorldObject& worldobject = registry.worldObjects.emplace(entity);
	worldobject.position = pos;
	worldobject.angle = angle;
	worldobject.scale = scale;

	registry.renderRequests.insert(
		entity,
		{ texture,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

Entity UISystem::createUIElement(
	RenderSystem* renderer, 
	vec2 pos, 
	vec2 scale, 
	std::string element_name, 
	float element_value, 
	SCENE_TYPE scene_type) {
	// Store a reference to the potentially re-used mesh object
	Entity entity = Entity();
	switch (scene_type) {
	case SCENE_TYPE::GAME:
		registry.gameSceneComponents.emplace(entity);
		break;
	case SCENE_TYPE::MENU:
		registry.menuSceneComponents.emplace(entity);
		break;
	case SCENE_TYPE::PAUSE:
		registry.pauseSceneComponents.emplace(entity);
		break;
	case SCENE_TYPE::TEST:
		registry.testSceneComponents.emplace(entity);
		break;
	}
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SQUARE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial position, scale, and orientation values
	WorldObject& worldobject = registry.worldObjects.emplace(entity);
	worldobject.position = pos;
	worldobject.scale = scale;

	// setting value for UI
	UIElement& ui_elt = registry.uiElements.emplace(entity);
	ui_elt.name = element_name;
	ui_elt.value = static_cast<float>(element_value);

	vec3& ui_color = registry.colors.emplace(entity);
	ui_color = vec3(1.0f, 1.0f, 1.0f);

	registry.renderRequests.insert(
		entity,
		{ TEXTURE_ASSET_ID::TEXTURE_COUNT,
			EFFECT_ASSET_ID::FONT,
			GEOMETRY_BUFFER_ID::SQUARE });

	return entity;
}

Entity UISystem::createTexturedUIElement(
	RenderSystem* renderer, 
	vec2 pos, 
	vec2 scale, 
	std::string element_name, 
	TEXTURE_ASSET_ID texture_id, 
	SCENE_TYPE scene_type) {
	// Store a reference to the potentially re-used mesh object
	Entity entity = Entity();
	switch (scene_type) {
	case SCENE_TYPE::GAME:
		registry.gameSceneComponents.emplace(entity);
		break;
	case SCENE_TYPE::MENU:
		registry.menuSceneComponents.emplace(entity);
		break;
	case SCENE_TYPE::PAUSE:
		registry.pauseSceneComponents.emplace(entity);
		break;
	case SCENE_TYPE::TEST:
		registry.testSceneComponents.emplace(entity);
		break;
	}

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

Entity UISystem::createButton(
	RenderSystem* renderer,
	vec2 pos, 
	vec2 scale, 
	std::function<void()> action, 
	std::string button_name, 
	TEXTURE_ASSET_ID texture_id, 
	SCENE_TYPE scene_type) {
	// Store a reference to the potentially re-used mesh object
	Entity entity = Entity();
	switch (scene_type) {
	case SCENE_TYPE::GAME:
		registry.gameSceneComponents.emplace(entity);
		break;
	case SCENE_TYPE::MENU:
		registry.menuSceneComponents.emplace(entity);
		break;
	case SCENE_TYPE::PAUSE:
		registry.pauseSceneComponents.emplace(entity);
		break;
	case SCENE_TYPE::TEST:
		registry.testSceneComponents.emplace(entity);
		break;
	}

	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SQUARE);
	registry.meshPtrs.emplace(entity, &mesh);

	// Setting initial position, scale, and orientation values
	WorldObject& worldobject = registry.worldObjects.emplace(entity);
	worldobject.position = pos;
	worldobject.scale = scale;

	// setting value for UI
	UIButton& ui_button = registry.uiButtons.emplace(entity);
	ui_button.name = button_name;
	ui_button.action = action;

	registry.renderRequests.insert(
		entity,
		{ texture_id,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE });

	return entity;
}

Entity UISystem::createCrosshair(RenderSystem* renderer, TEXTURE_ASSET_ID texture, SCENE_TYPE scene_type) {
	auto entity = Entity();
	switch (scene_type) {
	case SCENE_TYPE::GAME:
		registry.gameSceneComponents.emplace(entity);
		break;
	case SCENE_TYPE::MENU:
		registry.menuSceneComponents.emplace(entity);
		break;
	case SCENE_TYPE::PAUSE:
		registry.pauseSceneComponents.emplace(entity);
		break;
	case SCENE_TYPE::TEST:
		registry.testSceneComponents.emplace(entity);
		break;
	}

	// Store a reference to the potentially re-used mesh object (the value is stored in the resource cache)
	Mesh& mesh = renderer->getMesh(GEOMETRY_BUFFER_ID::SPRITE);
	registry.meshPtrs.emplace(entity, &mesh);
	registry.crosshairs.emplace(entity);

	// setting position, scale, orientation
	WorldObject& worldobject = registry.worldObjects.emplace(entity);
	worldobject.position = { -1.f, -1.f }; // initializing position to off screen
	worldobject.angle = 0.f;
	worldobject.scale = vec2({ CROSSHAIR_SIZE, CROSSHAIR_SIZE });

	registry.renderRequests.insert(
		entity,
		{
			texture,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE
		});

	return entity;
}

