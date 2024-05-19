// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef MATRIX_H
#define MATRIX_H

#include <cglm/cglm.h>
#include <luajit-2.1/lua.h>

typedef struct {
    int columns;
    float *raw;
    int rows;
} ir_matrix;

// Utility functions

void ir_matrix_frommatrix(lua_State *L, ir_matrix *victim);
int ir_matrix_ismatrix(lua_State *L, int idx);
void ir_matrix_tomatrix(lua_State *L, int idx, ir_matrix *matrix);

// Constructors

void ir_matrix_new(int columns, int rows, ir_matrix *result);
int ir_matrix_new_lua(lua_State *L);

// Lua methods

int ir_matrix_index_lua(lua_State *L);
int ir_matrix_mul_lua(lua_State *L);
int ir_matrix_newindex_lua(lua_State *L);

#endif