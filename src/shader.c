// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "log.h"
#include "lua.h"
#include "shader.h"

// TODO: Should I switch over to using Allegro's shader functions at some point?
// ~ahill

void ir_shader_drop(ir_shader *shader) {
	glDetachShader(shader->program, shader->fragment);
	glDetachShader(shader->program, shader->vertex);
	glDeleteProgram(shader->program);
	glDeleteShader(shader->fragment);
	glDeleteShader(shader->vertex);
}

int ir_shader_new(
	ir_shader *shader,
	size_t vert_len,
	char *vert_src,
	size_t frag_len,
	char *frag_src
) {
	char buffer[BUFSIZ];
	GLint status;

	// TODO: Is this actually safe enough or should we zero the entire buffer
	// out? ~ahill
	buffer[0] = 0;

	shader->vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(
		shader->vertex, 1, (const GLchar **)&vert_src, (const int *)&vert_len
	);
	glCompileShader(shader->vertex);
	glGetShaderiv(shader->vertex, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE) {
		glGetShaderInfoLog(shader->vertex, BUFSIZ, NULL, buffer);
		ir_error("ir_shader_new: %s", buffer);
		glDeleteShader(shader->vertex);
		return 1;
	}

	shader->fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(
		shader->fragment, 1, (const GLchar **)&frag_src, (const int *)&frag_len
	);
	glCompileShader(shader->fragment);
	glGetShaderiv(shader->fragment, GL_COMPILE_STATUS, &status);
	if (status != GL_TRUE) {
		glGetShaderInfoLog(shader->fragment, BUFSIZ, NULL, buffer);
		ir_error("ir_shader_new: %s", buffer);
		glDeleteShader(shader->fragment);
		glDeleteShader(shader->vertex);
		return 1;
	}

	shader->program = glCreateProgram();
	glAttachShader(shader->program, shader->fragment);
	glAttachShader(shader->program, shader->vertex);
	glLinkProgram(shader->program);
	glGetProgramiv(shader->program, GL_LINK_STATUS, &status);
	if (status != GL_TRUE) {
		glGetProgramInfoLog(shader->program, BUFSIZ, NULL, buffer);
		ir_error("ir_shader_new: %s", buffer);
		// The entire structure has been initialized anyways so why not? ~ahill
		ir_shader_drop(shader);
		return 1;
	}

	shader->position = glGetAttribLocation(shader->program, "position");
	glVertexAttribPointer(shader->position, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 0);
	glEnableVertexAttribArray(shader->position);

	shader->texcoord = glGetAttribLocation(shader->program, "texcoord");
	glVertexAttribPointer(shader->texcoord, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(4 * sizeof(float)));
	glEnableVertexAttribArray(shader->texcoord);

	shader->texture_id = glGetAttribLocation(shader->program, "textureid");
	glVertexAttribPointer(shader->texture_id, 1, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void *)(3 * sizeof(float)));
	glEnableVertexAttribArray(shader->texture_id);

	shader->camera = glGetUniformLocation(shader->program, "camera");
	shader->texturemap = glGetUniformLocation(shader->program, "texturemap");

	return 0;
}

void ir_shader_use(ir_shader *shader) {
	glUseProgram(shader->program);
}

// Lua interface

int ir_shader_isshader(lua_State *L, int index) {
	const char *type;

	if (!lua_isuserdata(L, index)) {
		return 0;
	}

	lua_getfield(L, index, "__type");
	if (!lua_isstring(L, -1)) {
		lua_pop(L, 1);
		return 0;
	}

	type = lua_tostring(L, -1);
	if (strcmp(type, "shader")) {
		lua_pop(L, 1);
		return 0;
	}

	lua_pop(L, 1);
	return 1;
}

int ir_shader_drop_lua(lua_State *L) {
	ir_shader *shader;

	if (ir_shader_isshader(L, -1)) {
		shader = lua_touserdata(L, -1);
		ir_shader_drop(shader);
	}

	lua_pop(L, 1);
	return 0;
}

int ir_shader_new_lua(lua_State *L) {
	const char *fragment;
	size_t fragment_len;
	ir_shader *shader;
	const char *vertex;
	size_t vertex_len;

	if (lua_gettop(L) != 2) {
		return 0;
	}

	if (lua_isstring(L, -2) && lua_isstring(L, -1)) {
		lua_pop(L, 2);
		return 0;
	}

	fragment = lua_tostring(L, -1);
	fragment_len = strlen(fragment);
	vertex = lua_tostring(L, -2);
	vertex_len = strlen(vertex);

	lua_pop(L, 2);

	shader = lua_newuserdata(L, sizeof(ir_shader));

	if (ir_shader_new(
			shader, vertex_len, (char *)vertex, fragment_len, (char *)fragment
		)) {
		lua_pop(L, 1);
		return 0;
	}

    lua_getglobal(L, "ir");
    lua_getfield(L, -1,  "internal");
    lua_replace(L, -2);
    lua_getfield(L, -1,  "meta");
    lua_replace(L, -2);
    lua_getfield(L, -1,  "shader");
    lua_replace(L, -2);

	lua_setmetatable(L, -2);

	return 1;
}

int ir_shader_use_lua(lua_State *L) {
	ir_shader *shader;

	if (ir_shader_isshader(L, -1)) {
		shader = lua_touserdata(L, -1);
		ir_shader_use(shader);
	}

	lua_pop(L, 1);
	return 0;
}
