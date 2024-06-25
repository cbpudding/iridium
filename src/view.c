// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <allegro5/allegro_opengl.h>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_PSD // fuck you adobe ~FabricatorZayac
#define STBI_NO_STDIO
#include <stb_image.h>

#include "log.h"
#include "main.h"
#include "matrix.h"
#include "view.h"

// When will #embed be a thing? ~ahill
#include "default.frag.glsl.h"
#include "default.vert.glsl.h"

void ir_view_drop(ir_view *view) {
	// ...
    glDeleteTextures(1, &view->texturemap);
	ir_shader_drop(&view->shader);
	glDeleteBuffers(1, &view->vbo);
	if (al_is_audio_installed()) {
		al_uninstall_audio();
	}
	al_destroy_display(view->display);
}

int ir_view_new(ir_view *view) {
	al_set_new_display_flags(ALLEGRO_OPENGL);
	al_set_new_display_option(ALLEGRO_OPENGL_MAJOR_VERSION, 2, ALLEGRO_REQUIRE);
	al_set_new_display_option(ALLEGRO_OPENGL_MINOR_VERSION, 0, ALLEGRO_REQUIRE);
	al_set_new_display_option(
		ALLEGRO_DEFAULT_SHADER_PLATFORM, ALLEGRO_SHADER_GLSL, ALLEGRO_REQUIRE
	);
	al_set_new_display_option(ALLEGRO_VSYNC, 1, ALLEGRO_SUGGEST);
	al_set_new_window_title("Iridium");

	view->display = al_create_display(640, 480);
	if (!view->display) {
		ir_error("ir_view_new: Failed to create display");
		return 1;
	}

	if (!al_install_audio()) {
		ir_warn("ir_view_new: Failed to initialize the audio subsystem");
	}

    glEnable(GL_TEXTURE_2D_ARRAY_EXT);
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
	glVertexAttribPointer(view->position, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 0);
	glEnableVertexAttribArray(view->position);

	view->texcoord = glGetAttribLocation(view->shader.program, "texcoord");
	glVertexAttribPointer(view->texcoord, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(4 * sizeof(float)));
	glEnableVertexAttribArray(view->texcoord);

	view->texture_id = glGetAttribLocation(view->shader.program, "textureid");
	glVertexAttribPointer(view->texture_id, 1, GL_UNSIGNED_INT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
	glEnableVertexAttribArray(view->texture_id);

	view->camera = glGetUniformLocation(view->shader.program, "camera");
	view->textures = glGetUniformLocation(view->shader.program, "textures");

    glGenTextures(1, &view->texturemap);
    glBindTexture(GL_TEXTURE_2D_ARRAY, view->texturemap);

    // TODO: Configurable filtering ~FabricatorZayac
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

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

int ir_view_fullscreen_lua(lua_State *L) {
	bool state = lua_toboolean(L, -1);
	lua_pop(L, 1);
	al_set_display_flag(ENGINE.view.display, ALLEGRO_FULLSCREEN_WINDOW, state);
	return 0;
}

int ir_view_height_lua(lua_State *L) {
	int height;
	int width;

	if(lua_gettop(L) > 1) {
		lua_pop(L, lua_gettop(L));
		return 0;
	}

	if(lua_gettop(L) == 0) {
		height = al_get_display_height(ENGINE.view.display);
		lua_pushnumber(L, height);
		return 1;
	} else {
		height = lua_tonumber(L, -1);
		lua_pop(L, 1);
		width = al_get_display_width(ENGINE.view.display);
		al_resize_display(ENGINE.view.display, width, height);
		return 0;
	}
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

	camera = ir_matrix_tomatrix(L, -1);

	glUniformMatrix4fv(ENGINE.view.camera, 1, GL_FALSE, (float *) *camera);

	lua_pop(L, 1);
	return 0;
}

int ir_view_texturemap_lua(lua_State *L) {
    const unsigned char *buffer;
    const unsigned char *data;
    int height;
    size_t len;
    int width;

    if (lua_gettop(L) != 1) {
        ir_error(
                "ir_view_texturemap_lua: 1 argument expected, %d received",
                lua_gettop(L)
        );
        lua_pop(L, lua_gettop(L));

        lua_pushboolean(L, false);
        lua_pushstring(L, "Expected 1 argument");

        return 2;
    }

    if (!lua_isstring(L, -1)) {
        ir_error(
                "ir_view_texturemap_lua: string expected, %s received",
                lua_typename(L, lua_type(L, -1))
        );
        lua_pop(L, 1);

        lua_pushboolean(L, false);
        lua_pushstring(L, "Expected string (filebuffer)");

        return 2;
    }

    buffer = (const unsigned char *) lua_tolstring(L, -1, &len);
    lua_pop(L, 1);

    data = stbi_load_from_memory(
        buffer,
        len,
        &width,
        &height,
        NULL,
        STBI_rgb_alpha
    );

	if(!data) {
		ir_error("ir_view_texturemap_lua: Failed to decode image: %s", stbi_failure_reason());

		lua_pushboolean(L, false);
		lua_pushstring(L, stbi_failure_reason());

		return 2;
	}

    // TODO: Mipmap??? ~FabricatorZayac
	// TODO: Is glTexStorage3D even required if we're using glTexImage3D? ~ahill
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, width, width, height / width);
	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGB8, width, width, height / width, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

	stbi_image_free((void *) data);

    lua_pushboolean(L, true);
  	return 1;
}

int ir_view_width_lua(lua_State *L) {
	int height;
	int width;

	if(lua_gettop(L) > 1) {
		lua_pop(L, lua_gettop(L));
		return 0;
	}

	if(lua_gettop(L) == 0) {
		width = al_get_display_width(ENGINE.view.display);
		lua_pushnumber(L, width);
		return 1;
	} else {
		width = lua_tonumber(L, -1);
		lua_pop(L, 1);
		height = al_get_display_height(ENGINE.view.display);
		al_resize_display(ENGINE.view.display, width, height);
		return 0;
	}
}
