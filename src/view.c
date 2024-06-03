// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "log.h"
#include "main.h"
#include "matrix.h"
#include "view.h"

// When will #embed be a thing? ~ahill
#include "default.frag.glsl.h"
#include "default.vert.glsl.h"

void ir_view_drop(ir_view *view) {
	// ...
	ir_shader_drop(&view->shader);
	glDeleteBuffers(1, &view->vbo);
	if (al_is_audio_installed()) {
		al_uninstall_audio();
	}
	al_destroy_display(view->display);
}

int ir_view_new(ir_view *view) {
	al_set_new_display_flags(ALLEGRO_FULLSCREEN | ALLEGRO_OPENGL);
	al_set_new_display_option(ALLEGRO_OPENGL_MAJOR_VERSION, 2, ALLEGRO_REQUIRE);
	al_set_new_display_option(ALLEGRO_OPENGL_MINOR_VERSION, 0, ALLEGRO_REQUIRE);
	al_set_new_display_option(
		ALLEGRO_DEFAULT_SHADER_PLATFORM, ALLEGRO_SHADER_GLSL, ALLEGRO_REQUIRE
	);
	al_set_new_display_option(ALLEGRO_VSYNC, 1, ALLEGRO_SUGGEST);
	al_set_new_window_title("Iridium");

	view->display = al_create_display(1920, 1080);
	if (!view->display) {
		ir_error("ir_view_new: Failed to create display");
		return 1;
	}

	if (!al_install_audio()) {
		ir_warn("ir_view_new: Failed to initialize the audio subsystem");
	}

	glGenBuffers(1, &view->vbo);
	glBindBuffer(GL_ARRAY_BUFFER, view->vbo);

	if (ir_shader_new(
			&view->shader,
			src_default_vert_glsl_len,
			(char *)src_default_vert_glsl,
			src_default_frag_glsl_len,
			(char *)src_default_frag_glsl
		)) {
		ir_error("ir_view_new: Failed to compile default shader");
		glDeleteBuffers(1, &view->vbo);
		if (al_is_audio_installed()) {
			al_uninstall_audio();
		}
		al_destroy_display(view->display);
		return 1;
	}

	ir_shader_use(&view->shader);

	view->position = glGetAttribLocation(view->shader.program, "position");
	glVertexAttribPointer(view->position, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(view->position);

	view->camera = glGetUniformLocation(view->shader.program, "camera");

	// ...

	return 0;
}

int ir_view_clear_lua(lua_State *L) {
	// We don't actually use the Lua state in this case, it's just here to match
	// the function signature for Lua. ~ahill
	(void)L;

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	return 0;
}

int ir_view_present_lua(lua_State *L) {
	// We don't actually use the Lua state in this case, it's just here to match
	// the function signature for Lua. ~ahill
	(void)L;

	al_flip_display();
	return 0;
}

int ir_view_render_lua(lua_State *L) {
	// TODO: Is there a better way to do this? ~ahill
	size_t length = lua_objlen(L, -1);
	float *buffer = malloc(length * sizeof(float));

	for (size_t i = 0; i < length; i++) {
		lua_pushinteger(L, i + 1);
		lua_gettable(L, -2);
		buffer[i] = lua_tonumber(L, -1);
		lua_pop(L, 1);
	}

	// TODO: Look into glBufferSubData to prevent the unnecessary reallocation
	// of VRAM ~ahill
	glBufferData(
		GL_ARRAY_BUFFER, length * sizeof(float), buffer, GL_STREAM_DRAW
	);
	glDrawArrays(GL_TRIANGLES, 0, length / 3);

	free(buffer);
	lua_pop(L, 1);
	return 0;
}

int ir_view_setcamera_lua(lua_State *L) {
	mat4 *camera;

	if(!ir_matrix_ismatrix(L, -1)) {
		return 0;
	}

	camera = lua_touserdata(L, -1);

	glUniformMatrix4fv(ENGINE.view.camera, 1, GL_FALSE, (GLfloat *) camera);

	lua_pop(L, 1);
	return 0;
}