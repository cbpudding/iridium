// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <stdlib.h>
#include <string.h>

#include "matrix.h"
#include "model.h"

#define MAT4_ALLOC() aligned_alloc(alignof(mat4), sizeof(mat4))

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

	if (lua_gettop(L) != 1) {
		lua_pop(L, lua_gettop(L));
		return 0;
	}

	if (!lua_isuserdata(L, index)) {
		lua_pop(L, 1);
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

void ir_matrix_pushmatrix(lua_State *L, mat4 *victim) {
	mat4 **userdata = lua_newuserdata(L, sizeof(mat4 *));
	// No hardware accelerated magic here. We can't guarantee that memory
	// allocated by Lua is aligned! ~ahill
	// memcpy(userdata, victim, sizeof(mat4));
    *userdata = victim;

	lua_createtable(L, 0, 3);

	lua_pushcfunction(L, ir_matrix_index_lua);
	lua_setfield(L, -2, "__index");

	lua_pushcfunction(L, ir_matrix_multiply_lua);
	lua_setfield(L, -2, "__mul");

    lua_pushcfunction(L, ir_matrix_free_lua);
    lua_setfield(L, -2, "__gc");

	lua_setmetatable(L, -2);
}

mat4 *ir_matrix_tomatrix(lua_State *L, int index) {
    // Assumes you checked if it's a matrix ~FabricatorZayac
    mat4 **userdata = lua_touserdata(L, index);
    return *userdata;
}

// Matrix Operations

int ir_matrix_free_lua(lua_State *L) {
    mat4 **userdata = lua_touserdata(L, -1);
    free(*userdata);
    lua_pop(L, 1);

    return 0;
}

int ir_matrix_from_lua(lua_State *L) {
    int argc;
	float temp;
	// mat4 result;
    // NOTE: For potential windows support we'd need _aligned_malloc instead ~FabricatorZayac
    mat4 *result = MAT4_ALLOC();

    argc = lua_gettop(L);
	if (argc != 1) {
		lua_pop(L, argc);
        return ir_push_error_lua(L, "Expected 1 argument, provided %d", argc);
	}

	if (!lua_istable(L, -1)) {
        const char *typename = lua_typename(L, lua_type(L, -1));
		lua_pop(L, 1);
        return ir_push_error_lua(L, "Expected table, provided %s", typename);
	}

	if (lua_objlen(L, -1) != 16) {
		lua_pop(L, 1);
        return ir_push_error_lua(L, "Invalid matrix dimensions");
	}

	for (int i = 1; i <= 16; i++) {
		lua_rawgeti(L, -1, i);
		if (!lua_isnumber(L, -1)) {
            const char *typename = lua_typename(L, lua_type(L, -1));
			lua_pop(L, 2);
            return ir_push_error_lua(L, "Expected number, provided %s", typename);
		}
		temp = lua_tonumber(L, -1);
		lua_pop(L, 1);
		*result[(i - 1) % 4][(i - 1) / 4] = temp;
	}

	lua_pop(L, 1);

	ir_matrix_pushmatrix(L, result);

	return 1;
}

int ir_matrix_identity_lua(lua_State *L) {
    mat4 *identity = MAT4_ALLOC();
	glm_mat4_identity(*identity);
	ir_matrix_pushmatrix(L, identity);
	return 1;
}

int ir_matrix_index_lua(lua_State *L) {
	const char *key;

    // I think this should not be done like this and should be implemented
    // through the kernel internals, to avoid cascading strcmp in favor of
    // hash tables.
    //
    // I'm gonna leave it like this for now tho ~FabricatorZayac

	// TODO: Include the ability to directly manipulate the matrix from Lua ~ahill
	if(lua_isstring(L, -1)) {
		key = lua_tostring(L, -1);
		lua_pop(L, 2);
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
	}
	return 0;
}

int ir_matrix_inverse_lua(lua_State *L) {
    int argc;
	mat4 *input;
	mat4 *output;

    argc = lua_gettop(L);
    if (argc != 1) {
        lua_pop(L, argc);
        return ir_push_error_lua(L, "Expected 1 argument, provided %d", argc);
    }

	if (!ir_matrix_ismatrix(L, -1)) {
        // We might want to make a custom typename function that would
        // additionally check the __type field in userdata ~FabricatorZayac
        const char *typename = lua_typename(L, lua_type(L, -1));
		lua_pop(L, 1);
        return ir_push_error_lua(L, "Expected matrix, provided %s", typename);
	}

    input = ir_matrix_tomatrix(L, -1);
    lua_pop(L, 1);

    output = MAT4_ALLOC();
    glm_mat4_inv(*input, *output);
    ir_matrix_pushmatrix(L, output);

    return 1;
}

int ir_matrix_multiply_lua(lua_State *L) {
	mat4 *a;
    int argc;
	mat4 *b;
	mat4 *result;

    argc = lua_gettop(L);
	if (argc != 2) {
        lua_pop(L, argc);
        return ir_push_error_lua(L, "Expected 2 arguments, provided %d", argc); 
	}

	if (!ir_matrix_ismatrix(L, -2)) {
        const char *typename = lua_typename(L, lua_type(L, -2));
		lua_pop(L, argc);
        return ir_push_error_lua(L, "Expected matrix, provided %s", typename);
	}
    if (!ir_matrix_ismatrix(L, -1)) {
        const char *typename = lua_typename(L, lua_type(L, -1));
		lua_pop(L, argc);
        return ir_push_error_lua(L, "Expected matrix, provided %s", typename);
    }

	a = ir_matrix_tomatrix(L, -2);
    b = ir_matrix_tomatrix(L, -1);
    lua_pop(L, 2);

    result = MAT4_ALLOC();
    glm_mat4_mul(*a, *b, *result);
    ir_matrix_pushmatrix(L, result);
    return 1;
}

int ir_matrix_transpose_lua(lua_State *L) {
    int argc;
	mat4 *input;
	mat4 *output;

    argc = lua_gettop(L);
    if (argc != 1) {
        lua_pop(L, argc);
        return ir_push_error_lua(L, "Expected 1 argument, provided %d", argc);
    }

    if (!ir_matrix_ismatrix(L, -1)) {
        const char *typename = lua_typename(L, lua_type(L, -1));
		lua_pop(L, argc);
        return ir_push_error_lua(L, "Expected matrix, provided %s", typename);
    }

    input = ir_matrix_tomatrix(L, -1);
    lua_pop(L, 1);

    output = MAT4_ALLOC();
    glm_mat4_transpose_to(*input, *output);
    ir_matrix_pushmatrix(L, output);
    return 1;
}

int ir_matrix_zero_lua(lua_State *L) {
    mat4 *identity = MAT4_ALLOC();
	glm_mat4_zero(*identity);
	ir_matrix_pushmatrix(L, identity);
	return 1;
}

// Vector Interface

void ir_vector_init_lua(lua_State *L) {
	lua_createtable(L, 0, 0);
	// ...
	lua_setfield(L, -2, "vec");
}

// TODO: Vector interface
