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

void UISystem::init(RenderSystem* renderer_arg, GLFWwindow* window_arg, SCENE_TYPE* scene_arg) {
	this->renderer = renderer_arg;
	this->window = window_arg;
	this->scene = scene_arg;
}

void UISystem::initGameUI(uint scrap, uint level) {
	// Add the base UI
	Entity base_ui = createPanel(
		SCENE_TYPE::GAME,
		vec2(window_width_px / 2, BASE_UI_HEIGHT / 2),
		0.f,
		vec2(window_width_px , BASE_UI_HEIGHT),
		TEXTURE_ASSET_ID::UI
	);

	// grab player health
	// TODO must update this when any UI elements change
	// maybe make a gameUI update fn
	float player_health = registry.healthComponents.get(registry.players.entities[0]).curr_health;

	// create health_ui entity
	health_ui = createUIElement(
		vec2(window_width_px / 10, window_height_px / 11),
		vec2(165.f, 40.f),
		"health_ui",
		static_cast<float>(player_health),
		SCENE_TYPE::GAME);

	// create scrap_ui entity
	scrap_ui = createUIElement(
		vec2(375.f, window_height_px / 20),
		vec2(25.f, 25.f),
		"scrap_ui",
		static_cast<float>(scrap),
		SCENE_TYPE::GAME);

	// create level_ui entity
	level_ui = createUIElement(
		vec2(375.f, window_height_px / 10),
		vec2(25.f, 30.f),
		"level_ui",
		static_cast<float>(level),
		SCENE_TYPE::GAME);

	// create item_ui entities
	// as a placeholder, there is just one item slot for now
	// later, we will want to render all the items and show locked slots too
	Entity item_ui = createTexturedUIElement(
		vec2(window_width_px - window_width_px / 22, window_height_px / 11),
		vec2(75.f, 75.f),
		"item_one_ui",
		TEXTURE_ASSET_ID::ITEM,
		SCENE_TYPE::GAME);
}

void UISystem::initMenuUI()
{
	Entity entity = Entity();

	renderer->updateCursorVisibility(true);

	Entity base_ui = createPanel(
		SCENE_TYPE::MENU,
		vec2(window_width_px / 2, window_height_px / 2),
		0.f,
		vec2(window_width_px , window_height_px),
		TEXTURE_ASSET_ID::START_MENU
	);

	createButton(
		vec2(window_width_px / 2 - 1.f, 275.f),
		vec2(162.f, 42.f),
		[&]() { 
			*(this->scene) = SCENE_TYPE::GAME; 
		},
		"start_button",
		TEXTURE_ASSET_ID::START_BUTTON,
		SCENE_TYPE::MENU
	);

	createButton(
		vec2(window_width_px / 2 + 3.f, 322.f),
		vec2(90.f, 44.f),
		[&]() {
			std::cout << "Help button pressed!" << std::endl;
		},
		"start_button",
		TEXTURE_ASSET_ID::HELP_BUTTON,
		SCENE_TYPE::MENU
	);

	createButton(
		vec2(window_width_px / 2, 372.f),
		vec2(165.f, 44.f),
		[&]() {
			std::cout << "Upgrades button pressed!" << std::endl;
		},
		"start_button",
		TEXTURE_ASSET_ID::SHOP_BUTTON,
		SCENE_TYPE::MENU
	);

	createButton(
		vec2(window_width_px / 2 + 7.f, 420.f),
		vec2(90.f, 44.f),
		[&]() {
			std::cout << "Quit button pressed!" << std::endl;
		},
		"start_button",
		TEXTURE_ASSET_ID::QUIT_BUTTON,
		SCENE_TYPE::MENU
	);
}

// creates a basic panel with the given texture
// used to render base UI components ex. the start menu background
Entity UISystem::createPanel(SCENE_TYPE scene_type, vec2 pos, float angle, vec2 scale, TEXTURE_ASSET_ID texture)
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
	registry.baseUI.emplace(entity);

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

Entity UISystem::createUIElement(vec2 pos, vec2 scale, std::string element_name, float element_value, SCENE_TYPE scene_type) {
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

Entity UISystem::createTexturedUIElement(vec2 pos, vec2 scale, std::string element_name, TEXTURE_ASSET_ID texture_id, SCENE_TYPE scene_type) {
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

void UISystem::on_mouse_button(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		// if mouse position is within button boundaries
		// activate the button's action function
		auto& uiButtonsRegistry = registry.uiButtons;
		auto& worldObjectsRegistry = registry.worldObjects;
		for (uint i = 0; i < uiButtonsRegistry.size(); i++) {
			UIButton button = uiButtonsRegistry.components[i];
			Entity buttonEntity = uiButtonsRegistry.entities[i];
			WorldObject buttonObject = worldObjectsRegistry.get(buttonEntity);
			if (is_mouse_within_button(buttonObject)) {
				button.action();
			}
		}
	}
}

bool UISystem::is_mouse_within_button(WorldObject buttonObject)
{
	bool passesX = (cursor_position.x > buttonObject.position.x - (buttonObject.scale.x / 2)
		&& cursor_position.x < buttonObject.position.x + (buttonObject.scale.x / 2));
	bool passesY = (cursor_position.y > buttonObject.position.y - (buttonObject.scale.y / 2)
		&& cursor_position.y < buttonObject.position.y + (buttonObject.scale.y / 2));
	return passesX && passesY;
}

void UISystem::on_mouse_move(vec2 mouse_position) {
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	cursor_position = vec2(xpos, ypos);
	// change cursor to be hover
}
