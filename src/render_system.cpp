// internal
#include "render_system.hpp"
#include <SDL.h>

#include "tiny_ecs_registry.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

// Debugging
namespace {
	void glfw_err_cb(int error, const char* desc) {
		fprintf(stderr, "%d: %s", error, desc);
	}
}

// World initialization
// Note, this has a lot of OpenGL specific things, could be moved to the renderer
GLFWwindow* RenderSystem::create_window() {
	///////////////////////////////////////
	// Initialize GLFW
	glfwSetErrorCallback(glfw_err_cb);
	if (!glfwInit()) {
		fprintf(stderr, "Failed to initialize GLFW");
		return nullptr;
	}

	//-------------------------------------------------------------------------
	// If you are on Linux or Windows, you can change these 2 numbers to 4 and 3 and
	// enable the glDebugMessageCallback to have OpenGL catch your mistakes for you.
	// GLFW / OGL Initialization
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#if __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
	glfwWindowHint(GLFW_RESIZABLE, 0);

	// Create the main window (for rendering, keyboard, and mouse input)
	GLFWwindow* window = glfwCreateWindow(window_width_px, window_height_px, "Dear Mother", nullptr, nullptr);
	if (window == nullptr) {
		fprintf(stderr, "Failed to glfwCreateWindow");
		return nullptr;
	}

	return window;
}

void RenderSystem::drawTexturedMesh(Entity entity,
									const mat3 &projection,
									float elapsed_ms)
{
	WorldObject &worldobject = registry.worldObjects.get(entity);
	// Transformation code, see Rendering and Transformation in the template
	// specification for more info Incrementally updates transformation matrix,
	// thus ORDER IS IMPORTANT
	Transform transform;
	transform.translate(worldobject.position);
	transform.rotate(worldobject.angle);
	transform.scale(worldobject.scale);

	assert(registry.renderRequests.has(entity));
	const RenderRequest &render_request = registry.renderRequests.get(entity);

	const GLuint used_effect_enum = (GLuint)render_request.used_effect;
	assert(used_effect_enum != (GLuint)EFFECT_ASSET_ID::EFFECT_COUNT);
	const GLuint program = (GLuint)effects[used_effect_enum];

	// Setting shaders
	glUseProgram(program);
	gl_has_errors();

	glBindVertexArray(mainVAO);
	gl_has_errors();

	assert(render_request.used_geometry != GEOMETRY_BUFFER_ID::GEOMETRY_COUNT);
	const GLuint vbo = vertex_buffers[(GLuint)render_request.used_geometry];
	const GLuint ibo = index_buffers[(GLuint)render_request.used_geometry];

	// Setting vertex and index buffers
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	gl_has_errors();

	// Input data location as in the vertex buffer
	if (render_request.used_effect == EFFECT_ASSET_ID::TEXTURED)
	{
		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		GLint in_texcoord_loc = glGetAttribLocation(program, "in_texcoord");
		gl_has_errors();
		assert(in_texcoord_loc >= 0);

		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
							  sizeof(TexturedVertex), (void *)0);
		gl_has_errors();

		glEnableVertexAttribArray(in_texcoord_loc);
		glVertexAttribPointer(
			in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex),
			(void *)sizeof(
				vec3)); // note the stride to skip the preceeding vertex position

		// Enabling and binding texture to slot 0
		glActiveTexture(GL_TEXTURE0);
		gl_has_errors();

		assert(registry.renderRequests.has(entity));
		GLuint texture_id =
			texture_gl_handles[(GLuint)registry.renderRequests.get(entity).used_texture];

		glBindTexture(GL_TEXTURE_2D, texture_id);
		gl_has_errors();
	}
	else if (render_request.used_effect == EFFECT_ASSET_ID::ANIM) {
		int rows = 1;
		int cols = 1;
		int frames = 1;
		int current_frame = 0;

		if (registry.animations.has(entity)) {
			Animation& entity_animation = registry.animations.get(entity);
			rows = entity_animation.rows;
			cols = entity_animation.cols;
			frames = entity_animation.frames;

			entity_animation.time_since_last_frame += elapsed_ms;

			if (entity_animation.time_since_last_frame > frame_duration) {
				entity_animation.current_frame = (entity_animation.current_frame + 1) % (frames);
				entity_animation.time_since_last_frame = 0;
			}

			current_frame = entity_animation.current_frame;
		}
		
		float frame_width = 1.0f / cols;
		float frame_height = 1.0f / rows;
		
		int col = current_frame % cols;
		int row = current_frame / rows;

		float u_offset = col * frame_width;
		float v_offset = row * frame_height;

		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		GLint in_texcoord_loc = glGetAttribLocation(program, "in_texcoord");
		gl_has_errors();
		assert(in_texcoord_loc >= 0);

		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
			sizeof(TexturedVertex), (void*)0);
		gl_has_errors();

		glEnableVertexAttribArray(in_texcoord_loc);
		glVertexAttribPointer(
			in_texcoord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(TexturedVertex),
			(void*)sizeof(
				vec3)); // note the stride to skip the preceeding vertex position

		unsigned int uvOffsetLoc =
			glGetUniformLocation(program, "uvOffset");
		glUniform2f(uvOffsetLoc, u_offset, v_offset);

		unsigned int uvScaleLoc =
			glGetUniformLocation(program, "uvScale");
		glUniform2f(uvScaleLoc, frame_width, frame_height);

		// Enabling and binding texture to slot 0
		glActiveTexture(GL_TEXTURE0);
		gl_has_errors();

		assert(registry.renderRequests.has(entity));
		GLuint texture_id =
			texture_gl_handles[(GLuint)registry.renderRequests.get(entity).used_texture];

		glBindTexture(GL_TEXTURE_2D, texture_id);
		gl_has_errors();
	}
	else if (render_request.used_effect == EFFECT_ASSET_ID::SALMON || render_request.used_effect == EFFECT_ASSET_ID::EGG)
	{
		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		GLint in_color_loc = glGetAttribLocation(program, "in_color");
		
		gl_has_errors();

		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
							  sizeof(ColoredVertex), (void *)0);
		gl_has_errors();

		glEnableVertexAttribArray(in_color_loc);
		glVertexAttribPointer(in_color_loc, 3, GL_FLOAT, GL_FALSE,
							  sizeof(ColoredVertex), (void *)sizeof(vec3));
		gl_has_errors();
	}
	else if (render_request.used_effect == EFFECT_ASSET_ID::UI_ELEMENT) {
		GLint in_position_loc = glGetAttribLocation(program, "in_position");
		GLint in_color_loc = glGetAttribLocation(program, "in_color");
		gl_has_errors();

		glEnableVertexAttribArray(in_position_loc);
		glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE,
								sizeof(ColoredVertex), (void*)0);
		gl_has_errors();

		glEnableVertexAttribArray(in_color_loc);
		glVertexAttribPointer(in_color_loc, 3, GL_FLOAT, GL_FALSE,
								sizeof(ColoredVertex), (void*)sizeof(vec3));
		gl_has_errors();
	}
	else if (render_request.used_effect == EFFECT_ASSET_ID::FONT) {
		// let render_text() handle this

		UIElement ui_elt = registry.uiElements.get(entity);
		WorldObject world_object = registry.worldObjects.get(entity);

		vec3 color = vec3(1.0f, 1.0f, 1.0f);

		if (registry.colors.has(entity)) {
			color = registry.colors.get(entity);
		}

		glm::mat4 trans = glm::mat4(1.0f);
		trans = glm::rotate(trans, world_object.angle, glm::vec3(0.0, 0.0, 1.0));
		render_text(
			ui_elt.value,
			world_object.position.x,
			window_height_px - world_object.position.y,
			world_object.scale.y,
			color,
			trans
		);
		
		glBindVertexArray(0);
		glUseProgram(program);
		return;
	}
	else if (render_request.used_effect == EFFECT_ASSET_ID::FLOOR_TEXT) {
		floor_text_to_render.push_back(entity);

		glBindVertexArray(0);
		return;
	}
	else if (render_request.used_effect == EFFECT_ASSET_ID::POP_UP_TEXT) {
		pop_up_text_to_render.push_back(entity);

		glBindVertexArray(0);
		return;
	}
	else
	{
		assert(false && "Type of render request not supported");
	}

	// Getting uniform locations for glUniform* calls
	GLint color_uloc = glGetUniformLocation(program, "fcolor");
	const vec3 color = registry.colors.has(entity) ? registry.colors.get(entity) : vec3(1);
	glUniform3fv(color_uloc, 1, (float *)&color);
	gl_has_errors();

	if (registry.flashingColors.has(entity)) {
		FlashingColor& flashing_color = registry.flashingColors.get(entity);
		flashing_color.time_since_last_flash += elapsed_ms;
		vec3 color = vec3(1);
		if (flashing_color.time_since_last_flash > flashing_color.flash_rate && !flashing_color.flashing) {
			vec3 color = flashing_color.color;
			glUniform3fv(color_uloc, 1, (float*)&color);
			flashing_color.flashing = true;
			flashing_color.time_since_last_flash = 0;
		}
		else {
			glUniform3fv(color_uloc, 1, (float*)&color);
			flashing_color.flashing = false;
		}
	}
	gl_has_errors();

	// Get number of indices from index buffer, which has elements uint16_t
	GLint size = 0;
	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
	gl_has_errors();

	GLsizei num_indices = size / sizeof(uint16_t);
	// GLsizei num_triangles = num_indices / 3;

	GLint currProgram;
	glGetIntegerv(GL_CURRENT_PROGRAM, &currProgram);
	// Setting uniform values to the currently bound program
	GLuint transform_loc = glGetUniformLocation(currProgram, "transform");
	glUniformMatrix3fv(transform_loc, 1, GL_FALSE, (float *)&transform.mat);
	GLuint projection_loc = glGetUniformLocation(currProgram, "projection");
	glUniformMatrix3fv(projection_loc, 1, GL_FALSE, (float *)&projection);
	gl_has_errors();
	// Drawing of num_indices/3 triangles specified in the index buffer
	glDrawElements(GL_TRIANGLES, num_indices, GL_UNSIGNED_SHORT, nullptr);
	gl_has_errors();
	glBindVertexArray(0);
	gl_has_errors();
}

// draw the intermediate texture to the screen, with some distortion to simulate
// water
void RenderSystem::drawToScreen()
{
	// Setting shaders
	// get the water texture, sprite mesh, and program
	glUseProgram(effects[(GLuint)EFFECT_ASSET_ID::WATER]);
	gl_has_errors();

	glBindVertexArray(mainVAO);
	gl_has_errors();
	// Clearing backbuffer
	int w, h;
	glfwGetFramebufferSize(window, &w, &h); // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, w, h);
	glDepthRange(0, 10);
	glClearColor(1.f, 0, 0, 1.0);
	glClearDepth(1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	gl_has_errors();
	// Disabling alpha channel for textures
	glDisable(GL_BLEND);
	// glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);
	gl_has_errors();

	// Draw the screen texture on the quad geometry
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffers[(GLuint)GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE]);
	glBindBuffer(
		GL_ELEMENT_ARRAY_BUFFER,
		index_buffers[(GLuint)GEOMETRY_BUFFER_ID::SCREEN_TRIANGLE]); // Note, GL_ELEMENT_ARRAY_BUFFER associates
																	 // indices to the bound GL_ARRAY_BUFFER
	gl_has_errors();
	const GLuint water_program = effects[(GLuint)EFFECT_ASSET_ID::WATER];
	// Set clock
	// fixed bug here where pause scene had dimmed screen
	GLuint time_uloc = glGetUniformLocation(water_program, "time");
	GLuint dead_timer_uloc = glGetUniformLocation(water_program, "darken_screen_factor");
	glUniform1f(time_uloc, (float)(glfwGetTime() * 10.0f));
	ScreenState &screen = registry.screenStates.get(screen_state_entity);
	float darken_screen_factor = -1;
	if (scene_manager.get_scene() == SCENE_TYPE::GAME) {
		darken_screen_factor = screen.darken_screen_factor;
	}
	glUniform1f(dead_timer_uloc, darken_screen_factor);
	gl_has_errors();


	// Set the vertex position and vertex texture coordinates (both stored in the
	// same VBO)
	GLint in_position_loc = glGetAttribLocation(water_program, "in_position");
	glEnableVertexAttribArray(in_position_loc);
	glVertexAttribPointer(in_position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), (void *)0);
	gl_has_errors();

	// Bind our texture in Texture Unit 0
	glActiveTexture(GL_TEXTURE0);

	glBindTexture(GL_TEXTURE_2D, off_screen_render_buffer_color);
	gl_has_errors();
	// Draw
	glDrawElements(
		GL_TRIANGLES, 3, GL_UNSIGNED_SHORT,
		nullptr); // one triangle = 3 vertices; nullptr indicates that there is
				  // no offset from the bound index buffer
	gl_has_errors();

	// Disabling alpha channel for textures
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	gl_has_errors();
}

// Render our game world
// http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-14-render-to-texture/
void RenderSystem::draw(float elapsed_ms)
{
	// Getting size of window
	int w, h;
	glfwGetFramebufferSize(window, &w, &h); // Note, this will be 2x the resolution given to glfwCreateWindow on retina displays

	// First render to the custom framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);
	gl_has_errors();
	// Clearing backbuffer
	glViewport(0, 0, w, h);
	glDepthRange(0.00001, 10);
	glClearColor(GLfloat(172 / 255), GLfloat(216 / 255), GLfloat(255 / 255), 1.0);
	glClearDepth(10.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST); // native OpenGL does not work with a depth buffer
							  // and alpha blending, one would have to sort
							  // sprites back to front
	gl_has_errors();

	mat3 projection_2D = createProjectionMatrix();
	std::vector<Entity> render_list = {};

	switch (scene_manager.get_scene()) {
	case SCENE_TYPE::GAME: {
		// draw game
		// Draw all textured meshes that have a position and size component
		for (Entity entity : registry.renderRequests.entities)
		{
			// note that activeComponents are ONLY USED for game entities
			if (!registry.worldObjects.has(entity) || !registry.gameSceneComponents.has(entity))
				continue;
			if (registry.roomCoords.has(entity)) {
				if (!registry.activeComponents.has(entity))
					continue;
			}
			if (!on_screen(registry.worldObjects.get(entity).position))
				continue;
			drawTexturedMesh(entity, projection_2D, elapsed_ms);
		}
		break;
	}
	case SCENE_TYPE::MENU: {
		// draw menu

		for (Entity entity : registry.renderRequests.entities)
		{
			if (!registry.worldObjects.has(entity) || !registry.menuSceneComponents.has(entity))
				continue;
			drawTexturedMesh(entity, projection_2D, elapsed_ms);
		}
		break;
	}
	case SCENE_TYPE::HELP: {
		// draw help screen
		for (Entity entity : registry.renderRequests.entities)
		{
			if (!registry.worldObjects.has(entity) || !registry.helpSceneComponents.has(entity))
				continue;
			drawTexturedMesh(entity, projection_2D, elapsed_ms);
		}
		break;
	}
	case SCENE_TYPE::PAUSE: {
		// draw pause screen
		for (Entity entity : registry.renderRequests.entities)
		{
			if (!registry.worldObjects.has(entity) || !registry.pauseSceneComponents.has(entity))
				continue;
			drawTexturedMesh(entity, projection_2D, elapsed_ms);
		}
		break;
	}
	case SCENE_TYPE::SHOP: {
		// draw shop screen
		for (Entity entity : registry.renderRequests.entities)
		{
			if (!registry.worldObjects.has(entity) || !registry.shopSceneComponents.has(entity))
				continue;
			drawTexturedMesh(entity, projection_2D, elapsed_ms);
		}
		break;
	}
	case SCENE_TYPE::DIALOGUE: {
		// draw game entities:
		for (Entity entity : registry.renderRequests.entities)
		{
			// note that activeComponents are ONLY USED for game entities
			if (!registry.worldObjects.has(entity) || !registry.gameSceneComponents.has(entity))
				continue;
			if (registry.roomCoords.has(entity)) {
				if (!registry.activeComponents.has(entity))
					continue;
			}
			if (!on_screen(registry.worldObjects.get(entity).position))
				continue;
			drawTexturedMesh(entity, projection_2D, elapsed_ms);
		}

		// then draw dialogue on top of everything. simple, yipeeeeee!:
		for (Entity entity : registry.renderRequests.entities)
		{
			if (!registry.worldObjects.has(entity) || !registry.dialogueSceneComponents.has(entity))
				continue;
			drawTexturedMesh(entity, projection_2D, elapsed_ms);
		}
	}
	case SCENE_TYPE::TEST: {
		// draw test scene
		break;
	}
	}

	// render all text
	drawText(); // Mabye up here

	// Truely render to the screen
	drawToScreen();

	drawPopUpText();
	
	// flicker-free display with a double buffer
	glfwSwapBuffers(window);
	gl_has_errors();
}


void RenderSystem::drawPopUpText() {
	for (uint i = 0; i < pop_up_text_to_render.size(); i++) {
		UIElement ui_elt = registry.uiElements.get(pop_up_text_to_render[i]);
		WorldObject world_object = registry.worldObjects.get(pop_up_text_to_render[i]);

		vec3 color = vec3(1.0f, 1.0f, 1.0f);

		if (registry.colors.has(pop_up_text_to_render[i])) {
			color = registry.colors.get(pop_up_text_to_render[i]);
		}

		glm::mat4 trans = glm::mat4(1.0f);
		trans = glm::rotate(trans, world_object.angle, glm::vec3(0.0, 0.0, 1.0));
		render_text(
			ui_elt.value,
			world_object.position.x,
			window_height_px - world_object.position.y,
			world_object.scale.y,
			color,
			trans
		);
	}
	pop_up_text_to_render.clear();
}

// Doesn't look like this is used anymore
void RenderSystem::drawFloorText() {
	for (Entity text : floor_text_to_render) {
		WorldObject world_object = registry.worldObjects.get(text);
		FloorText floor_text = registry.floorTexts.get(text);

		glm::mat4 trans = glm::mat4(1.0f);
		/*trans = glm::rotate(trans, world_object.angle, glm::vec3(0.0, 0.0, 1.0));
		trans = glm::translate(trans, glm::vec3(world_object.position, 0.0f));*/
		render_text(
			floor_text.text,
			world_object.position.x,
			world_object.position.y,
			world_object.scale.x,
			//vec3(1.0f, 1.0f, 1.0f),
			registry.colors.get(text),
			trans
		);
	}
	floor_text_to_render.clear();
}

void RenderSystem::drawText() {
	for (uint i = 0; i < text_to_render.size(); i++) {
		UIElement ui_elt = registry.uiElements.get(text_to_render[i]);
		WorldObject world_object = registry.worldObjects.get(text_to_render[i]);

		vec3 color = vec3(1.0f, 1.0f, 1.0f);

		if (registry.colors.has(text_to_render[i])) {
			color = registry.colors.get(text_to_render[i]);
		}

		glm::mat4 trans = glm::mat4(1.0f);
		trans = glm::rotate(trans, world_object.angle, glm::vec3(0.0, 0.0, 1.0));
		render_text(
			ui_elt.value, 
			world_object.position.x, 
			window_height_px - world_object.position.y,
			world_object.scale.y,
			color,
			trans
		);
	}
	text_to_render.clear();
}


void RenderSystem::render_text(std::string text, float x, float y, float scale, const glm::vec3& color, const glm::mat4& trans) {
	// activate the shader program
	glUseProgram(fontShaderProgram);
	gl_has_errors();

	// enable blending or you will just get solid boxes instead of text
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	gl_has_errors();

	// get shader uniforms
	GLint textColor_location =
		glGetUniformLocation(fontShaderProgram, "textColor");
	glUniform3f(textColor_location, color.x, color.y, color.z);
	gl_has_errors();

	GLint transformLoc =
		glGetUniformLocation(fontShaderProgram, "transform");
	glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(trans));
	gl_has_errors();
	float origX = x; // Save original x position to reset on new lines

	glBindVertexArray(fontVAO);
	gl_has_errors();

	for (char c : text) {
		if (c == '\n') {
			x = origX;             // Reset x to original x position
			y -= lineHeight * scale* LINE_SPACING; // Move y position down by line height*1.5
			continue;
		}

		Character ch = m_ftCharacters[c];

		float xpos = x + ch.Bearing.x * scale;
		float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

		float w = ch.Size.x * scale;
		float h = ch.Size.y * scale;

		// Update VBO for each character
		float vertices[6][4] = {
			{ xpos,     ypos + h,   0.0f, 0.0f },
			{ xpos,     ypos,       0.0f, 1.0f },
			{ xpos + w, ypos,       1.0f, 1.0f },

			{ xpos,     ypos + h,   0.0f, 0.0f },
			{ xpos + w, ypos,       1.0f, 1.0f },
			{ xpos + w, ypos + h,   1.0f, 0.0f }
		};

		// Render glyph texture over quad
		glBindTexture(GL_TEXTURE_2D, ch.TextureID);
		gl_has_errors();

		// Update content of VBO memory
		glBindBuffer(GL_ARRAY_BUFFER, fontVBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		gl_has_errors();

		// Render quad
		glDrawArrays(GL_TRIANGLES, 0, 6);
		gl_has_errors();

		// Advance cursor for next glyph
		x += (ch.Advance >> 6) * scale;
		gl_has_errors();
	}

	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

mat3 RenderSystem::createProjectionMatrix()
{
	// Fake projection matrix, scales with respect to window coordinates
	float left = 0.f;
	float top = 0.f;

	gl_has_errors();
	float right = (float) window_width_px;
	float bottom = (float) window_height_px;

	float sx = 2.f / (right - left);
	float sy = 2.f / (top - bottom);
	float tx = -(right + left) / (right - left);
	float ty = -(top + bottom) / (top - bottom);
	return {{sx, 0.f, 0.f}, {0.f, sy, 0.f}, {tx, ty, 1.f}};
}

std::vector<float> RenderSystem::getCharacterWidths(const std::string& text, const float scale) const {
	std::vector<float> widths;
	for (char c : text) {
		auto it = m_ftCharacters.find(c);
		if (it != m_ftCharacters.end()) {
			// Use Advance to get the character width
			float width = (it->second.Advance >> 6) * scale;
			widths.push_back(width);
		}
		else {
			// If the character isn't found, append a default width (e.g., 0)
			widths.push_back(0.0f);
		}
	}
	return widths;
}
