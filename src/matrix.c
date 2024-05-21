// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <string.h>

#include "log.h"
#include "matrix.h"

// Utility functions

void ir_matrix_frommatrix(lua_State *L, ir_matrix *victim) {
    float *raw;
    size_t size = victim->columns * victim->rows * sizeof(float);

    lua_createtable(L, 0, 4);

    lua_pushstring(L, "__type");
    lua_pushstring(L, "matrix");
    lua_settable(L, -3);

    lua_pushstring(L, "columns");
    lua_pushinteger(L, victim->columns);
    lua_settable(L, -3);

    lua_pushstring(L, "raw");
    raw = lua_newuserdata(L, size);
    lua_settable(L, -3);

    lua_pushstring(L, "rows");
    lua_pushinteger(L, victim->rows);
    lua_settable(L, -3);

    memcpy(raw, victim->raw, size);
}

int ir_matrix_ismatrix(lua_State *L, int idx) {
    const char *value;

    if(!lua_istable(L, idx)) {
        return 0;
    }

    lua_getfield(L, idx, "__type");
    if(!lua_isstring(L, -1)) {
        // TODO: What if __type doesn't exist? ~ahill
        lua_pop(L, 1);
        return 0;
    }

    value = lua_tostring(L, -1);
    if(strcmp(value, "matrix")) {
        return 0;
    }

    lua_pop(L, 1);
    return 1;
}

void ir_matrix_tomatrix(lua_State *L, int idx, ir_matrix *matrix) {
    // WARNING: This function assumes that we're working with a matrix already!
    // Check your matrices with ir_matrix_ismatrix first! ~ahill

    lua_getfield(L, idx, "columns");
    matrix->columns = lua_tonumber(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, idx, "raw");
    matrix->raw = lua_touserdata(L, -1);
    lua_pop(L, 1);

    lua_getfield(L, idx, "rows");
    matrix->rows = lua_tonumber(L, -1);
    lua_pop(L, 1);
}

// Constructors

void ir_matrix_new(int columns, int rows, ir_matrix *result) {
    result->columns = columns;
    result->rows = rows;

    // TODO: Should we even try to initialize data here? ~ahill

    ir_debug("ir_matrix_new: Am I the problem?");
}

int ir_matrix_new_lua(lua_State *L) {
    int columns;
    float *raw;
    int rows;
    if(lua_gettop(L) >= 2) {
        if(lua_isnumber(L, -1) && lua_isnumber(L, -2)) {
            columns = lua_tonumber(L, -2);
            rows = lua_tonumber(L, -1);
            lua_pop(L, 2);

            lua_createtable(L, 0, 4);

            lua_pushstring(L, "__type");
            lua_pushstring(L, "matrix");
            lua_settable(L, -3);

            lua_pushstring(L, "columns");
            lua_pushinteger(L, columns);
            lua_settable(L, -3);

            lua_pushstring(L, "raw");
            raw = lua_newuserdata(L, columns * rows * sizeof(float));
            lua_settable(L, -3);

            lua_pushstring(L, "rows");
            lua_pushinteger(L, rows);
            lua_settable(L, -3);

            for(int i = 0; i < columns * rows; i++) {
                raw[i] = 0.0;
            }

            return 1;
        }
    }
    return 0;
}

// Lua methods

int ir_matrix_index_lua(lua_State *L) {
    int column;
    int row;
    ir_matrix victim;

    if(lua_gettop(L) != 3) {
        return 0;
    }

    if(!ir_matrix_ismatrix(L, -3)) {
        return 0;
    }

    if(!lua_isnumber(L, -2) || !lua_isnumber(L, -1)) {
        return 0;
    }

    ir_matrix_tomatrix(L, -3, &victim);
    column = lua_tonumber(L, -2) - 1;
    row = lua_tonumber(L, -1) - 1;

    // TODO: Confirm that OpenGL matrices are column-major (at least on x86). ~ahill
    lua_pushnumber(L, victim.raw[(column * victim.rows) + row]);
    return 1;
}

int ir_matrix_mul_lua(lua_State *L) {
    ir_matrix a;
    ir_matrix b;
    ir_matrix c;
    float temp;

    if(lua_gettop(L) != 2) {
        return 0;
    }

    if(!ir_matrix_ismatrix(L, -1)) {
        return 0;
    }

    if(!ir_matrix_ismatrix(L, -2)) {
        return 0;
    }

    ir_matrix_tomatrix(L, -2, &a);
    ir_matrix_tomatrix(L, -1, &b);
    lua_pop(L, 2);

    // If we're multiplying two vectors, "transpose" the matrix so the following code works properly. ~ahill
    if(a.columns == 1 && b.columns == 1) {
        temp = b.columns;
        b.columns = b.rows;
        b.rows = temp;
    }

    // Of course, we need to make sure the operation is valid. ~ahill
    if(a.columns != b.rows) {
        return 0;
    }

    // We only support a minimum of 2 rows and 1 column and a maximum of 4 rows/columns ~ahill
    if(a.columns < 1 || a.columns > 4) {
        return 0;
    }

    if(a.rows < 2 || a.rows > 4) {
        return 0;
    }

    if(b.columns < 1 || b.columns > 4) {
        return 0;
    }

    if(b.rows < 2 || b.rows > 4) {
        return 0;
    }

    ir_matrix_new(b.columns, a.rows, &c);

    // Rather than having a bunch of nested if or switch statements, it might
    // make more sense to reduce the variables down to a single thing we can
    // jump around with.

    // Since all of our values range from 1 to 4, we can decrement and bitshift
    // to collapse all the possible values down to a simple "opcode" value. We
    // only need to keep track of the left row count and right column count
    // since we know the columns in the left matrix is equal to the number of
    // rows in the right matrix. With that in mind we can use a simple 6-bit
    // number in the following format:

    // ccaabb
    //     where
    //     c = left_columns - 1
    //     a = left_rows - 1
    //     b = right_columns - 1

    // For example, multiplying a 4x4 matrix by a four-dimensional vector will
    // create the "opcode" 0b111100 that we can use to call glm_mat4_mulv. ~ahill
    switch(((a.columns - 1) << 4) | ((a.rows - 1) << 2) | (b.columns - 1)) {
        // vec2 * vec2
        case 0b000101:
            glm_vec2_mul(a.raw, b.raw, c.raw);
            break;

        // vec3 * vec3
        case 0b001010:
            glm_vec3_mul(a.raw, b.raw, c.raw);
            break;

        // vec4 * vec4
        case 0b001111:
            glm_vec4_mul(a.raw, b.raw, c.raw);
            break;

        // mat2 * vec2
        case 0b010100:
            glm_mat2_mulv((float (*)[2]) a.raw, b.raw, c.raw);
            break;

        // mat2 * mat2
        case 0b010101:
            glm_mat2_mul((float (*)[2]) a.raw, (float (*)[2]) b.raw, (float (*)[2]) c.raw);
            break;

        // mat2x3 * vec2
        case 0b011000:
            glm_mat2x3_mulv((float (*)[3]) a.raw, b.raw, c.raw);
            break;
        
        // mat2x3 * mat3x2
        case 0b011010:
            glm_mat2x3_mul((float (*)[3]) a.raw, (float (*)[2]) b.raw, (float (*)[2]) c.raw);
            break;
        
        // mat2x4 * vec2
        case 0b011100:
            glm_mat2x4_mulv((float (*)[4]) a.raw, b.raw, c.raw);
            break;

        // mat2x4 * mat4x2
        case 0b011111:
            glm_mat2x4_mul((float (*)[4]) a.raw, (float (*)[2]) b.raw, (float (*)[2]) c.raw);
            break;

        // mat3x2 * vec3
        case 0b100100:
            glm_mat3x2_mulv((float (*)[2]) a.raw, b.raw, c.raw);
            break;

        // mat3x2 * mat2x3
        case 0b100101:
            glm_mat3x2_mul((float (*)[2]) a.raw, (float (*)[3]) b.raw, (float (*)[3]) c.raw);
            break;

        // mat3 * vec3
        case 0b101000:
            glm_mat3_mulv((float (*)[3]) a.raw, b.raw, c.raw);
            break;
        
        // mat3 * mat3
        case 0b101010:
            glm_mat3_mul((float (*)[3]) a.raw, (float (*)[3]) b.raw, (float (*)[3]) c.raw);
            break;

        // mat3x4 * vec3
        case 0b101100:
            glm_mat3x4_mulv((float (*)[4]) a.raw, b.raw, c.raw);
            break;

        // mat3x4 * mat4x3
        case 0b101111:
            glm_mat3x4_mul((float (*)[4]) a.raw, (float (*)[3]) b.raw, (float (*)[3]) c.raw);
            break;
        
        // mat4x2 * vec4
        case 0b110100:
            glm_mat4x2_mulv((float (*)[2]) a.raw, b.raw, c.raw);
            break;
        
        // mat4x2 * mat2x4
        case 0b110101:
            glm_mat4x2_mul((float (*)[2]) a.raw, (float (*)[4]) b.raw, (float (*)[4]) c.raw);
            break;
        
        // mat4x3 * vec4
        case 0b111000:
            glm_mat4x3_mulv((float (*)[3]) a.raw, b.raw, c.raw);
            break;

        // mat4x3 * mat3x4
        case 0b111010:
            glm_mat4x3_mul((float (*)[3]) a.raw, (float (*)[4]) b.raw, (float (*)[4]) c.raw);
            break;

        // mat4 * vec4
        case 0b111100:
            glm_mat4_mulv((float (*)[4]) a.raw, b.raw, c.raw);
            break;
        
        // mat4 * mat4
        case 0b111111:
            glm_mat4_mul((float (*)[4]) a.raw, (float (*)[4]) b.raw, (float (*)[4]) c.raw);
            break;

        // Of course, if it's not on the list, it's not happening. ~ahill
        default:
            return 0;
    }

    ir_matrix_frommatrix(L, &c);
    return 1;
}

int ir_matrix_newindex_lua(lua_State *L) {
    int column;
    int row;
    float value;
    ir_matrix victim;

    if(lua_gettop(L) != 4) {
        return 0;
    }

    if(!ir_matrix_ismatrix(L, -4)) {
        return 0;
    }

    if(!lua_isnumber(L, -3) || !lua_isnumber(L, -2) || !lua_isnumber(L, -1)) {
        return 0;
    }

    ir_matrix_tomatrix(L, -4, &victim);
    column = lua_tonumber(L, -3) - 1;
    row = lua_tonumber(L, -2) - 1;
    value = lua_tonumber(L, -1);

    // TODO: Confirm that OpenGL matrices are column-major (at least on x86). ~ahill
    victim.raw[(column * victim.rows) + row] = value;
    return 0;
}