// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <string.h>

#include "log.h"
#include "matrix.h"

// Matrix Interface

void ir_matrix_init_lua(lua_State *L) {
	lua_createtable(L, 0, 3);

	lua_pushcfunction(L, ir_matrix_from_lua);
	lua_setfield(L, -2, "from");

	lua_pushcfunction(L, ir_matrix_identity_lua);
	lua_setfield(L, -2, "identity");

	lua_pushcfunction(L, ir_matrix_zero_lua);
	lua_setfield(L, -2, "zero");

	lua_setfield(L, -2, "mat");
}

int ir_matrix_ismatrix(lua_State *L, int index) {
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
	if (strcmp(type, "matrix")) {
		lua_pop(L, 1);
		return 0;
	}

	lua_pop(L, 1);
	return 1;
}

void ir_matrix_metatable(lua_State *L, int index) {
	lua_createtable(L, 0, 2);

	lua_pushcfunction(L, ir_matrix_metatable_index);
	lua_setfield(L, -2, "__index");

	lua_pushcfunction(L, ir_matrix_multiply_lua);
	lua_setfield(L, -2, "__mul");

	lua_setmetatable(L, index - 1);
}

int ir_matrix_metatable_index(lua_State *L) {
	int idx;
	const char *key;
	mat4 *userdata;

	if(!ir_matrix_ismatrix(L, -2)) {
		return 0;
	}

	userdata = lua_touserdata(L, -2);

	if(lua_isstring(L, -1)) {
		key = lua_tostring(L, -1);
		lua_pop(L, 2);
		ir_debug("ir_matrix_metatable_index: key %s", key);
		if(!strcmp(key, "__metatable")) {
			lua_pushstring(L, "My God, what are you doing?");
			return 1;
		} else if(!strcmp(key, "__type")) {
			lua_pushstring(L, "matrix");
			return 1;
		} else if(!strcmp(key, "inverse")) {
			lua_pushcfunction(L, ir_matrix_inverse_lua);
			return 1;
		} else if(!strcmp(key, "transpose")) {
			lua_pushcfunction(L, ir_matrix_transpose_lua);
			return 1;
		}
	} else if(lua_isnumber(L, -1)) {
		idx = lua_tointeger(L, -1);
		lua_pop(L, 2);
		ir_debug("ir_matrix_metatable_index: index %u", idx);
		if(idx >= 1 && idx <= 4) {
			// ...
		}
	}
	return 0;
}

void ir_matrix_pushmatrix(lua_State *L, mat4 *victim) {
	mat4 *userdata = lua_newuserdata(L, sizeof(mat4));
	glm_mat4_copy(*victim, *userdata);
	ir_matrix_metatable(L, -1);
}

void ir_matrix_tomatrix(lua_State *L, int index, mat4 *dest) {
	mat4 *userdata;

	if (ir_matrix_ismatrix(L, index)) {
		userdata = lua_touserdata(L, index);
		glm_mat4_copy(*userdata, *dest);
	}
}

// Matrix Operations

int ir_matrix_from_lua(lua_State *L) {
	float temp;
	mat4 *result;

	if (!lua_istable(L, -1)) {
		return 0;
	}

	if (lua_objlen(L, -1) != 16) {
		return 0;
	}

	result = lua_newuserdata(L, sizeof(mat4));

	for (int i = 1; i <= 16; i++) {
		lua_rawgeti(L, -2, i);
		if (!lua_isnumber(L, -1)) {
			lua_pop(L, 2);
			return 0;
		}
		temp = lua_tonumber(L, -1);
		lua_pop(L, 1);
		*result[(i - 1) % 4][(i - 1) / 4] = temp;
	}

	ir_matrix_metatable(L, -1);

	return 1;
}

int ir_matrix_identity_lua(lua_State *L) {
	mat4 *userdata = lua_newuserdata(L, sizeof(mat4));
	glm_mat4_identity(*userdata);
	ir_matrix_metatable(L, -1);
	return 1;
}

int ir_matrix_inverse_lua(lua_State *L) {
	mat4 *input;
	mat4 *output;

	if (ir_matrix_ismatrix(L, -1)) {
		input = lua_touserdata(L, -1);
		lua_pop(L, 1);
		output = lua_newuserdata(L, sizeof(mat4));
		glm_mat4_inv(*input, *output);
		ir_matrix_metatable(L, -1);
		return 1;
	}

	return 0;
}

int ir_matrix_multiply_lua(lua_State *L) {
	// This function makes the assumption that the first argument is always a
	// mat4. If that is not the case, it *will* return nil! ~ahill
	void *a;
	void *b;
	void *result;

	if (lua_gettop(L) != 2) {
		return 0;
	}

	if (!ir_matrix_ismatrix(L, -2)) {
		return 0;
	}

	a = lua_touserdata(L, -2);

	if (ir_matrix_ismatrix(L, -1)) {
		b = lua_touserdata(L, -1);
		lua_pop(L, 2);
		result = lua_newuserdata(L, sizeof(mat4));
		glm_mat4_mul(a, b, result);
		ir_matrix_metatable(L, -1);
		return 1;
	} else if (ir_vector_isvector(L, -1)) {
		b = lua_touserdata(L, -1);
		lua_pop(L, 2);
		result = lua_newuserdata(L, sizeof(vec4));
		glm_mat4_mulv(a, b, result);
		ir_vector_metatable(L, -1);
		return 1;
	} else {
		return 0;
	}
}

int ir_matrix_peek_lua(lua_State *L) {
	int col;
	int row;
	mat4 *userdata;

	if(lua_gettop(L) == 3) {
		return 0;
	}

	if(!ir_matrix_ismatrix(L, -3)) {
		return 0;
	}

	if(!lua_isnumber(L, -2)) {
		return 0;
	}

	if(!lua_isnumber(L, -1)) {
		return 0;
	}

	col = lua_tointeger(L, -2);
	row = lua_tointeger(L, -1);
	userdata = lua_touserdata(L, -3);

	lua_pop(L, 3);
	lua_pushnumber(L, *userdata[col][row]);
	return 1;
}

int ir_matrix_poke_lua(lua_State *L) {
	int col;
	int row;
	mat4 *userdata;

	if(lua_gettop(L) == 4) {
		return 0;
	}

	if(!ir_matrix_ismatrix(L, -4)) {
		return 0;
	}

	if(!lua_isnumber(L, -3)) {
		return 0;
	}

	if(!lua_isnumber(L, -2)) {
		return 0;
	}

	if(!lua_isnumber(L, -1)) {
		return 0;
	}

	col = lua_tointeger(L, -2);
	row = lua_tointeger(L, -1);
	userdata = lua_touserdata(L, -3);
	*userdata[col][row] = lua_tonumber(L, -1);

	lua_pop(L, 4);
	return 1;
}

int ir_matrix_transpose_lua(lua_State *L) {
	mat4 *input;
	mat4 *output;

	if (ir_matrix_ismatrix(L, -1)) {
		input = lua_touserdata(L, -1);
		lua_pop(L, 1);
		output = lua_newuserdata(L, sizeof(mat4));
		glm_mat4_transpose_to(*input, *output);
		ir_matrix_metatable(L, -1);
		return 1;
	}

	return 0;
}

int ir_matrix_zero_lua(lua_State *L) {
	mat4 *userdata = lua_newuserdata(L, sizeof(mat4));
	glm_mat4_zero(*userdata);
	ir_matrix_metatable(L, -1);
	return 1;
}

// Vector Interface

void ir_vector_init_lua(lua_State *L) {
	lua_createtable(L, 0, 0);
	// ...
	lua_setfield(L, -2, "vec");
}

int ir_vector_isvector(lua_State *L, int index) {
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
	if (strcmp(type, "vector")) {
		lua_pop(L, 1);
		return 0;
	}

	lua_pop(L, 1);
	return 1;
}

void ir_vector_metatable(lua_State *L, int index) {
	lua_createtable(L, 0, 1);
	lua_createtable(L, 0, 1);
	lua_pushstring(L, "vector");
	lua_setfield(L, -2, "__type");
	lua_setfield(L, -2, "__index");
	lua_setmetatable(L, index - 1);
}

void ir_vector_pushvector(lua_State *L, vec4 *victim) {
	vec4 *userdata = lua_newuserdata(L, sizeof(vec4));
	glm_vec4_copy((float *)victim, (float *)userdata);
	ir_vector_metatable(L, -1);
}

void ir_vector_tovector(lua_State *L, int index, vec4 *dest) {
	vec4 *userdata;

	if (ir_matrix_ismatrix(L, index)) {
		userdata = lua_touserdata(L, index);
		glm_vec4_copy((float *)userdata, (float *)dest);
	}
}

// Vector Operations

int ir_vector_add_lua(lua_State *L); // glm_vec4_add, glm_vec4_adds

int ir_vector_clamp_lua(lua_State *L);

int ir_vector_distance_lua(lua_State *L);

int ir_vector_divide_lua(lua_State *L); // glm_vec4_div, glm_vec4_divs

int ir_vector_dot_lua(lua_State *L);

int ir_vector_from_lua(lua_State *L);

int ir_vector_lerp_lua(lua_State *L);

int ir_vector_magnitude_lua(lua_State *L);

int ir_vector_max_lua(lua_State *L);

int ir_vector_min_lua(lua_State *L);

int ir_vector_multiply_lua(lua_State *L); // glm_vec4_mul, glm_vec4_scale

int ir_vector_negate_lua(lua_State *L) {
	vec4 *input;
	vec4 *output;

	if (ir_vector_isvector(L, -1)) {
		input = lua_touserdata(L, -1);
		lua_pop(L, 1);
		output = lua_newuserdata(L, sizeof(vec4));
		glm_vec4_negate_to((float *)input, (float *)output);
		ir_vector_metatable(L, -1);
		return 1;
	}

	return 0;
}

int ir_vector_normalize_lua(lua_State *L) {
	vec4 *input;
	vec4 *output;

	if (ir_vector_isvector(L, -1)) {
		input = lua_touserdata(L, -1);
		lua_pop(L, 1);
		output = lua_newuserdata(L, sizeof(vec4));
		glm_vec4_normalize_to((float *)input, (float *)output);
		ir_vector_metatable(L, -1);
		return 1;
	}

	return 0;
}

int ir_vector_one_lua(lua_State *L) {
	vec4 *userdata = lua_newuserdata(L, sizeof(vec4));
	glm_vec4_one((float *)userdata);
	ir_vector_metatable(L, -1);
	return 1;
}

int ir_vector_peek_lua(lua_State *L);

int ir_vector_poke_lua(lua_State *L);

int ir_vector_reflect_lua(lua_State *L);

int ir_vector_subtract_lua(lua_State *L); // glm_vec4_sub, glm_vec4_subs

int ir_vector_zero_lua(lua_State *L) {
	vec4 *userdata = lua_newuserdata(L, sizeof(vec4));
	glm_vec4_zero((float *)userdata);
	ir_vector_metatable(L, -1);
	return 1;
}
