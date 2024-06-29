// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef MODEL_H
#define MODEL_H

#include <luajit-2.1/lua.h>
#include <luajit-2.1/lauxlib.h>
#include <luajit-2.1/lualib.h>

typedef enum {
	// No command
	IRCMD_NONE,
	// Halt engine
	IRCMD_HALT
} ir_command;

typedef struct {
	lua_State *state;
} ir_model;

void ir_model_drop(ir_model *model);
int ir_model_new(ir_model *model);
int ir_push_error_lua(lua_State *L, const char *restrict fmt, ...);

const char *ir_totypename(lua_State *L, int idx);
int ir_typename_lua(lua_State *L);

#endif
