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

void UISystem::init(GLFWwindow* window_arg) {
	this->window = window_arg;
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
		registry.activeComponents.emplace(entity);
		break;
	case SCENE_TYPE::MENU:
		registry.menuSceneComponents.emplace(entity);
		break;
	case SCENE_TYPE::HELP:
		registry.helpSceneComponents.emplace(entity);
		break;
	case SCENE_TYPE::PAUSE:
		registry.pauseSceneComponents.emplace(entity);
		break;
	case SCENE_TYPE::SHOP:
		registry.shopSceneComponents.emplace(entity);
		break;
	case SCENE_TYPE::DIALOGUE:
		registry.dialogueSceneComponents.emplace(entity);
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

	registry.renderRequests.insert_sorted(
		entity,
		{ texture,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_ORDER::UI_PANEL });

	return entity;
}

// creates a solid colored square; used to cover locked inventory slots
Entity UISystem::createSquareUIElement(
	RenderSystem* renderer,
	vec2 pos,
	vec2 scale,
	std::string element_name,
	SCENE_TYPE scene_type) {
	// Store a reference to the potentially re-used mesh object
	Entity entity = Entity();
	switch (scene_type) {
	case SCENE_TYPE::GAME:
		registry.gameSceneComponents.emplace(entity);
		registry.activeComponents.emplace(entity);
		break;
	case SCENE_TYPE::MENU:
		registry.menuSceneComponents.emplace(entity);
		break;
	case SCENE_TYPE::HELP:
		registry.helpSceneComponents.emplace(entity);
		break;
	case SCENE_TYPE::PAUSE:
		registry.pauseSceneComponents.emplace(entity);
		break;
	case SCENE_TYPE::SHOP:
		registry.shopSceneComponents.emplace(entity);
		break;
	case SCENE_TYPE::DIALOGUE:
		registry.dialogueSceneComponents.emplace(entity);
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

	vec3& ui_color = registry.colors.emplace(entity);
	ui_color = vec3(0.56f, 0.58f, 0.61f);

	registry.renderRequests.insert_sorted(
		entity,
		{ TEXTURE_ASSET_ID::TEXTURE_COUNT,
			EFFECT_ASSET_ID::UI_ELEMENT,
			GEOMETRY_BUFFER_ID::SQUARE,
			RENDER_ORDER::UI_ELEMENT });

	return entity;
}

Entity UISystem::createTextUIElement(
	RenderSystem* renderer, 
	vec2 pos, 
	vec2 scale, 
	std::string element_name, 
	std::string element_value, 
	vec3 color,
	SCENE_TYPE scene_type) {
	// Store a reference to the potentially re-used mesh object
	Entity entity = Entity();
	switch (scene_type) {
	case SCENE_TYPE::GAME:
		registry.gameSceneComponents.emplace(entity);
		registry.activeComponents.emplace(entity);
		break;
	case SCENE_TYPE::MENU:
		registry.menuSceneComponents.emplace(entity);
		break;
	case SCENE_TYPE::HELP:
		registry.helpSceneComponents.emplace(entity);
		break;
	case SCENE_TYPE::PAUSE:
		registry.pauseSceneComponents.emplace(entity);
		break;
	case SCENE_TYPE::SHOP:
		registry.shopSceneComponents.emplace(entity);
		break;
	case SCENE_TYPE::DIALOGUE:
		registry.dialogueSceneComponents.emplace(entity);
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
	ui_elt.value = element_value; // the actual text to be rendered

	vec3& ui_color = registry.colors.emplace(entity);
	ui_color = color;

	registry.renderRequests.insert_sorted(
		entity,
		{ TEXTURE_ASSET_ID::TEXTURE_COUNT,
			EFFECT_ASSET_ID::FONT,
			GEOMETRY_BUFFER_ID::SQUARE,
			RENDER_ORDER::UI_ELEMENT });

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
		registry.activeComponents.emplace(entity);
		break;
	case SCENE_TYPE::MENU:
		registry.menuSceneComponents.emplace(entity);
		break;
	case SCENE_TYPE::HELP:
		registry.helpSceneComponents.emplace(entity);
		break;
	case SCENE_TYPE::PAUSE:
		registry.pauseSceneComponents.emplace(entity);
		break;
	case SCENE_TYPE::SHOP:
		registry.shopSceneComponents.emplace(entity);
		break;
	case SCENE_TYPE::DIALOGUE:
		registry.dialogueSceneComponents.emplace(entity);
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

	registry.renderRequests.insert_sorted(
		entity,
		{ texture_id,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_ORDER::TEXTURED_UI_ELEMENT });

	return entity;
}

Entity UISystem::createButton(
	RenderSystem* renderer,
	vec2 pos,
	vec2 scale,
	std::function<void()> action,
	std::string button_name,
	TEXTURE_ASSET_ID texture_id,
	TEXTURE_ASSET_ID hover_texture_id,
	SCENE_TYPE scene_type) {
	Entity entity = Entity();
	switch (scene_type) {
	case SCENE_TYPE::GAME:
		registry.gameSceneComponents.emplace(entity);
		registry.activeComponents.emplace(entity);
		break;
	case SCENE_TYPE::MENU:
		registry.menuSceneComponents.emplace(entity);
		break;
	case SCENE_TYPE::HELP:
		registry.helpSceneComponents.emplace(entity);
		break;
	case SCENE_TYPE::PAUSE:
		registry.pauseSceneComponents.emplace(entity);
		break;
	case SCENE_TYPE::SHOP:
		registry.shopSceneComponents.emplace(entity);
		break;
	case SCENE_TYPE::DIALOGUE:
		registry.dialogueSceneComponents.emplace(entity);
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
	ui_button.base_texture = texture_id;
	ui_button.hover_texture = hover_texture_id;

	registry.renderRequests.insert_sorted(
		entity,
		{ texture_id,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_ORDER::UI_BUTTON });

	return entity;
}

// make a button, with text instead of a texture
Entity UISystem::createTextButton(
	RenderSystem* renderer,
	vec2 pos,
	vec2 scale,
	std::function<void()> action,
	std::string button_name,
	std::string text,
	vec3 color,
	SCENE_TYPE scene_type
) {
	Entity entity = Entity();
	switch (scene_type) {
	case SCENE_TYPE::GAME:
		registry.gameSceneComponents.emplace(entity);
		registry.activeComponents.emplace(entity);
		break;
	case SCENE_TYPE::MENU:
		registry.menuSceneComponents.emplace(entity);
		break;
	case SCENE_TYPE::HELP:
		registry.helpSceneComponents.emplace(entity);
		break;
	case SCENE_TYPE::PAUSE:
		registry.pauseSceneComponents.emplace(entity);
		break;
	case SCENE_TYPE::SHOP:
		registry.shopSceneComponents.emplace(entity);
		break;
	case SCENE_TYPE::DIALOGUE:
		registry.dialogueSceneComponents.emplace(entity);
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

	UIElement& ui_element = registry.uiElements.emplace(entity);
	ui_element.name = button_name;
	ui_element.value = text;

	vec3& button_color = registry.colors.emplace(entity);
	button_color = color;

	registry.renderRequests.insert_sorted(
		entity,
		{	TEXTURE_ASSET_ID::TEXTURE_COUNT,
			EFFECT_ASSET_ID::FONT,
			GEOMETRY_BUFFER_ID::SQUARE,
			RENDER_ORDER::UI_BUTTON });

	return entity;
}

Entity UISystem::createCrosshair(RenderSystem* renderer, TEXTURE_ASSET_ID texture, SCENE_TYPE scene_type) {
	auto entity = Entity();
	switch (scene_type) {
	case SCENE_TYPE::GAME:
		registry.gameSceneComponents.emplace(entity);
		registry.activeComponents.emplace(entity);
		break;
	case SCENE_TYPE::MENU:
		registry.menuSceneComponents.emplace(entity);
		break;
	case SCENE_TYPE::HELP:
		registry.helpSceneComponents.emplace(entity);
		break;
	case SCENE_TYPE::PAUSE:
		registry.pauseSceneComponents.emplace(entity);
		break;
	case SCENE_TYPE::SHOP:
		registry.shopSceneComponents.emplace(entity);
		break;
	case SCENE_TYPE::DIALOGUE:
		registry.dialogueSceneComponents.emplace(entity);
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

	registry.renderRequests.insert_sorted(
		entity,
		{
			texture,
			EFFECT_ASSET_ID::TEXTURED,
			GEOMETRY_BUFFER_ID::SPRITE,
			RENDER_ORDER::CROSSHAIR
		});

	return entity;
}



// ==================== COMPLEX FUNCTIONS ====================
// WHEN GETTING RID OF A TEXT BOX, GET RID OF ITS CONSTITUENT ENTITIES FIRST!
// linebreaks automatically inserted when text is too long
// size is the size of the text box, not the text (text scale is defined by a constant)
Entity UISystem::createTextBox(RenderSystem * renderer, vec2 pos, vec2 size, vec3 color, SCENE_TYPE scene_type, std::string text) {
	// right now, its not a button, but it could be modified to be one
	Entity entity = Entity();
	switch (scene_type) {
	case SCENE_TYPE::GAME:
		registry.gameSceneComponents.emplace(entity);
		registry.activeComponents.emplace(entity);
		break;
	case SCENE_TYPE::MENU:
		registry.menuSceneComponents.emplace(entity);
		break;
	case SCENE_TYPE::HELP:
		registry.helpSceneComponents.emplace(entity);
		break;
	case SCENE_TYPE::PAUSE:
		registry.pauseSceneComponents.emplace(entity);
		break;
	case SCENE_TYPE::SHOP:
		registry.shopSceneComponents.emplace(entity);
		break;
	case SCENE_TYPE::DIALOGUE:
		registry.dialogueSceneComponents.emplace(entity);
		break;
	case SCENE_TYPE::TEST:
		registry.testSceneComponents.emplace(entity);
		break;
	}
	
	TextBox& textbox = registry.textBoxes.emplace(entity);

	std::vector<float> widths = renderer->getCharacterWidths(text, DIALOGUE_TEXT_SCALE); // a list of the widths of each character in the string.
	float line_height = renderer->get_line_height(); // height of a line of text. if the number of lines exceeds num_lines * line_height * scale
	float max_line_width = size.x - (2.f * DIALOGUE_BOX_MARGINS);

	assert(widths.size() == text.length() && "Text size isn't equal to the number of widths :(");

	// calculate the total height of the text box and insert linebreaks
	std::string formatted_text;
	float curr_width = 0.f;
	float total_height = line_height;
	int last_space_idx = -1; // Index in formatted_text

	for (size_t i = 0, w = 0; i < text.length(); ++i, ++w) {
		char c = text[i];
		float char_width = widths[w];

		// If the current character is a newline, reset variables
		if (c == '\n') {
			formatted_text += c;
			total_height += line_height * LINE_SPACING * 2;
			curr_width = 0.f;
			last_space_idx = -1;
			continue;
		}

		// Check if adding the current character exceeds the maximum line width
		if (curr_width + char_width > max_line_width) {
			if (last_space_idx != -1) {
				// Replace the space at last_space_idx with a newline
				formatted_text[last_space_idx] = '\n';

				// Calculate the width of the substring after the newline
				curr_width = 0.f;
				for (size_t k = last_space_idx + 1; k < formatted_text.length(); ++k) {
					// Find the corresponding width index
					size_t width_idx = k;
					if (width_idx < widths.size()) {
						curr_width += widths[width_idx];
					}
				}

				total_height += line_height * LINE_SPACING * 2;
				last_space_idx = -1;
			}
			else {
				// No space found, insert a newline before the current character
				formatted_text += '\n';
				total_height += line_height * LINE_SPACING*2;
				curr_width = 0.f;
			}
		}

		// Add the current character to the formatted text
		formatted_text += c;
		curr_width += char_width;

		// Update the last space index if the current character is a space
		if (c == ' ') {
			last_space_idx = formatted_text.length() - 1; // Current index in formatted_text
		}
	}


	if (total_height > (size.y - 2.f * DIALOGUE_BOX_MARGINS)) {
		std::cout << "Text box too small for text!" << std::endl;
		printf("total height %f greater than than y-size %f", total_height, size.y - 2.f * DIALOGUE_BOX_MARGINS);
		exit(1);
	}

	vec2 text_pos = { pos.x - size.x / 2.f + DIALOGUE_BOX_MARGINS, pos.y - (size.y / 2.f) + (line_height + DIALOGUE_BOX_MARGINS) };

	textbox.textbox_sprite = createTexturedUIElement(renderer, pos, size, "textbox", TEXTURE_ASSET_ID::TEXT_BOX, scene_type);
	textbox.textbox_text = createTextUIElement(renderer, text_pos, vec2(1.f, DIALOGUE_TEXT_SCALE), "textbox_text", formatted_text, color, scene_type);

	return entity;
}