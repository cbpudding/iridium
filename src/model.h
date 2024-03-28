// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef MODEL_H
#define MODEL_H

#include <luajit-2.1/lua.h>
#include <luajit-2.1/lauxlib.h>
#include <luajit-2.1/lualib.h>

typedef enum {
    // No operation
    IRMSG_NOTHING,
    // Starting point for custom message codes. Ignored by the engine.
    IRMSG_CUSTOM
} ir_message;

typedef struct {
    lua_State *state;
} ir_model;

void ir_model_drop(ir_model *model);
int ir_model_new(ir_model *model);

int ir_model_update(ir_model *model, ir_message msg);

#endif