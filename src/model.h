// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef MODEL_H
#define MODEL_H

#include <cglm/types.h>
#include <luajit-2.1/lua.h>
#include <luajit-2.1/lauxlib.h>
#include <luajit-2.1/lualib.h>

#define IR_LUA_UMATRIX 10
#define IR_LUA_USHADER 11

typedef enum {
	// No command
	IRCMD_NONE,
	// Halt engine
	IRCMD_HALT
} ir_command;

typedef struct {
	bool mouselock;
	lua_State *state;
} ir_model;

void *ir_new(lua_State *L, int typeid);

void ir_model_drop(ir_model *model);
int ir_model_new(ir_model *model);

int ir_model_mouselock_lua(lua_State *L);
int ir_model_time_lua(lua_State *L);
int ir_push_error_lua(lua_State *L, const char *restrict fmt, ...);

const char *ir_totypename(lua_State *L, int idx);
int ir_typename_lua(lua_State *L);

#endif
