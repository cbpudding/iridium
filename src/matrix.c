// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <string.h>

#include "matrix.h"

// Matrix Interface

int ir_matrix_ismatrix(lua_State *L, int index) {
    const char *type;
    void *userdata;

    if(!lua_isuserdata(L, index)) {
        return 0;
    }

    userdata = lua_touserdata(L, index);
    lua_getfield(L, index, "__type");
    if(!lua_isstring(L, -1)) {
        lua_pop(L, 1);
        return 0;
    }

    type = lua_tostring(L, -1);
    if(strcmp(type, "matrix")) {
        lua_pop(L, 1);
        return 0;
    }

    lua_pop(L, 1);
    return 1;
}

void ir_matrix_pushmatrix(lua_State *L, mat4 *victim) {
    mat4 *userdata = lua_newuserdata(L, sizeof(mat4));
    glm_mat4_copy((float (*)[4]) victim, (float (*)[4]) userdata);
    lua_createtable(L, 0, 1);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "matrix");
    lua_setfield(L, -2, "__type");
    lua_setfield(L, -2, "__index");
    lua_setmetatable(L, -2);
}

void ir_matrix_tomatrix(lua_State *L, int index, mat4 *dest) {
    mat4 *userdata;

    if(ir_matrix_ismatrix(L, index)) {
        userdata = lua_touserdata(L, index);
        glm_mat4_copy((float (*)[4]) userdata, (float (*)[4]) dest);
    }
}

// Matrix Operations

int ir_matrix_identity_lua(lua_State *L) {
    mat4 *userdata;

    if(ir_matrix_ismatrix(L, -1)) {
        userdata = lua_touserdata(L, -1);
        glm_mat4_identity((float (*)[4]) userdata);
    }

    lua_pop(L, 1);
    return 0;
}

int ir_matrix_inverse_lua(lua_State *L) {
    mat4 temp;
    mat4 *userdata;

    if(ir_matrix_ismatrix(L, -1)) {
        userdata = lua_touserdata(L, -1);
        glm_mat4_inv((float (*)[4]) userdata, (float (*)[4]) &temp);
        glm_mat4_copy((float (*)[4]) &temp, (float (*)[4]) userdata);
    }

    lua_pop(L, 1);
    return 0;
}

int ir_matrix_multiply_lua(lua_State *L); // glm_mat4_mul, glm_mat4_mulv, glm_mat4_scale

int ir_matrix_peek_lua(lua_State *L);

int ir_matrix_poke_lua(lua_State *L);

int ir_matrix_transpose_lua(lua_State *L) {
    mat4 *userdata;

    if(ir_matrix_ismatrix(L, -1)) {
        userdata = lua_touserdata(L, -1);
        glm_mat4_transpose((float (*)[4]) userdata);
    }

    lua_pop(L, 1);
    return 0;
}

int ir_matrix_zero_lua(lua_State *L) {
    mat4 *userdata;

    if(ir_matrix_ismatrix(L, -1)) {
        userdata = lua_touserdata(L, -1);
        glm_mat4_zero((float (*)[4]) userdata);
    }

    lua_pop(L, 1);
    return 0;
}

// Vector Interface

int ir_vector_isvector(lua_State *L, int index) {
    const char *type;
    void *userdata;

    if(!lua_isuserdata(L, index)) {
        return 0;
    }

    userdata = lua_touserdata(L, index);
    lua_getfield(L, index, "__type");
    if(!lua_isstring(L, -1)) {
        lua_pop(L, 1);
        return 0;
    }

    type = lua_tostring(L, -1);
    if(strcmp(type, "vector")) {
        lua_pop(L, 1);
        return 0;
    }

    lua_pop(L, 1);
    return 1;
}

void ir_vector_pushvector(lua_State *L, vec4 *victim) {
    vec4 *userdata = lua_newuserdata(L, sizeof(vec4));
    glm_vec4_copy((float *) victim, (float *) userdata);
    lua_createtable(L, 0, 1);
    lua_createtable(L, 0, 1);
    lua_pushstring(L, "vector");
    lua_setfield(L, -2, "__type");
    lua_setfield(L, -2, "__index");
    lua_setmetatable(L, -2);
}

void ir_vector_tovector(lua_State *L, int index, vec4 *dest) {
    vec4 *userdata;

    if(ir_matrix_ismatrix(L, index)) {
        userdata = lua_touserdata(L, index);
        glm_vec4_copy((float *) userdata, (float *) dest);
    }
}

// Vector Operations

int ir_vector_add_lua(lua_State *L); // glm_vec4_add, glm_vec4_adds

int ir_vector_clamp_lua(lua_State *L);

int ir_vector_distance_lua(lua_State *L);

int ir_vector_divide_lua(lua_State *L); // glm_vec4_div, glm_vec4_divs

int ir_vector_dot_lua(lua_State *L);

int ir_vector_lerp_lua(lua_State *L);

int ir_vector_magnitude_lua(lua_State *L);

int ir_vector_max_lua(lua_State *L);

int ir_vector_min_lua(lua_State *L);

int ir_vector_multiply_lua(lua_State *L); // glm_vec4_mul, glm_vec4_scale

int ir_vector_negate_lua(lua_State *L) {
    vec4 *userdata;

    if(ir_vector_isvector(L, -1)) {
        userdata = lua_touserdata(L, -1);
        glm_vec4_negate((float *) userdata);
    }

    lua_pop(L, 1);
    return 0;
}

int ir_vector_normalize_lua(lua_State *L) {
    vec4 *userdata;

    if(ir_vector_isvector(L, -1)) {
        userdata = lua_touserdata(L, -1);
        glm_vec4_normalize((float *) userdata);
    }

    lua_pop(L, 1);
    return 0;
}

int ir_vector_one_lua(lua_State *L) {
    vec4 *userdata;

    if(ir_vector_isvector(L, -1)) {
        userdata = lua_touserdata(L, -1);
        glm_vec4_one((float *) userdata);
    }

    lua_pop(L, 1);
    return 0;
}

int ir_vector_peek_lua(lua_State *L);

int ir_vector_poke_lua(lua_State *L);

int ir_vector_reflect_lua(lua_State *L);

int ir_vector_subtract_lua(lua_State *L); // glm_vec4_sub, glm_vec4_subs

int ir_vector_zero_lua(lua_State *L) {
    vec4 *userdata;

    if(ir_vector_isvector(L, -1)) {
        userdata = lua_touserdata(L, -1);
        glm_vec4_zero((float *) userdata);
    }

    lua_pop(L, 1);
    return 0;
}