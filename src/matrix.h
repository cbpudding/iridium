// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef MATRIX_H
#define MATRIX_H

#include <cglm/cglm.h>
#include <luajit-2.1/lua.h>

// Matrix Interface

void ir_matrix_init_lua(lua_State *L);
int ir_matrix_ismatrix(lua_State *L, int index);
int ir_matrix_metatable_index(lua_State *L);
void ir_matrix_pushmatrix(lua_State *L, mat4 *victim);
void ir_matrix_tomatrix(lua_State *L, int index, mat4 *dest);

// Matrix Operations

int ir_matrix_from_lua(lua_State *L);
int ir_matrix_identity_lua(lua_State *L);
int ir_matrix_index_lua(lua_State *L);
int ir_matrix_inverse_lua(lua_State *L);
int ir_matrix_multiply_lua(lua_State *L);
int ir_matrix_transpose_lua(lua_State *L);
int ir_matrix_zero_lua(lua_State *L);

// Vector Interface

void ir_vector_init_lua(lua_State *L);
int ir_vector_isvector(lua_State *L, int index);
void ir_vector_metatable(lua_State *L, int index);
void ir_vector_pushvector(lua_State *L, vec4 *victim);
void ir_vector_tovector(lua_State *L, int index, vec4 *dest);

// Vector Operations

int ir_vector_add_lua(lua_State *L);
int ir_vector_clamp_lua(lua_State *L);
int ir_vector_distance_lua(lua_State *L);
int ir_vector_divide_lua(lua_State *L);
int ir_vector_dot_lua(lua_State *L);
int ir_vector_from_lua(lua_State *L);
int ir_vector_lerp_lua(lua_State *L);
int ir_vector_magnitude_lua(lua_State *L);
int ir_vector_max_lua(lua_State *L);
int ir_vector_min_lua(lua_State *L);
int ir_vector_multiply_lua(lua_State *L);
int ir_vector_negate_lua(lua_State *L);
int ir_vector_normalize_lua(lua_State *L);
int ir_vector_one_lua(lua_State *L);
int ir_vector_reflect_lua(lua_State *L);
int ir_vector_subtract_lua(lua_State *L);
int ir_vector_zero_lua(lua_State *L);

#endif