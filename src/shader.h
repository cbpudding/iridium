// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef SHADER_H
#define SHADER_H

#define GL_GLEXT_PROTOTYPES

#include <allegro5/allegro_opengl.h>
#include <GL/glext.h>
#include <luajit-2.1/lua.h>

typedef struct {
	GLuint fragment;
	GLuint program;
	GLuint vertex;
} ir_shader;

void ir_shader_drop(ir_shader *shader);
int ir_shader_new(
	ir_shader *shader,
	size_t vert_len,
	char *vert_src,
	size_t frag_len,
	char *frag_src
);

void ir_shader_use(ir_shader *shader);

// Lua interface

void ir_shader_init_lua(lua_State *L);
int ir_shader_isshader(lua_State *L, int index);

int ir_shader_drop_lua(lua_State *L);
int ir_shader_new_lua(lua_State *L);
int ir_shader_use_lua(lua_State *L);

#endif
