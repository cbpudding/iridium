// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "log.h"
#include "model.h"

// Workaround for not having #embed yet. ~ahill
#include "default.lua.h"

// TODO: Figure out how to prevent subscriptions from changing the game's state! ~ahill

void ir_model_drop(ir_model *model) {
    // ...
    lua_close(model->state);
}

int ir_model_new(ir_model *model) {
    model->state = luaL_newstate();
    if(!model->state) {
        ir_error("ir_model_new: Failed to initialize Lua");
        return 1;
    }

    luaopen_base(model->state);
    luaopen_string(model->state);
    luaopen_table(model->state);
    luaopen_math(model->state);
    luaopen_bit(model->state);
    // TODO: How would one implement os.time safely? ~ahill

    lua_createtable(model->state, 0, 6);

    lua_pushstring(model->state, "debug");
    lua_pushcfunction(model->state, ir_debug_lua);
    lua_settable(model->state, -3);

    lua_pushstring(model->state, "error");
    lua_pushcfunction(model->state, ir_error_lua);
    lua_settable(model->state, -3);

    lua_pushstring(model->state, "info");
    lua_pushcfunction(model->state, ir_info_lua);
    lua_settable(model->state, -3);

    lua_pushstring(model->state, "warn");
    lua_pushcfunction(model->state, ir_warn_lua);
    lua_settable(model->state, -3);

    lua_pushstring(model->state, "cmd");
    lua_createtable(model->state, 0, 2);

    lua_pushstring(model->state, "NONE");
    lua_pushinteger(model->state, IRCMD_NONE);
    lua_settable(model->state, -3);

    lua_pushstring(model->state, "HALT");
    lua_pushinteger(model->state, IRCMD_HALT);
    lua_settable(model->state, -3);

    lua_settable(model->state, -3);

    lua_pushstring(model->state, "msg");
    lua_createtable(model->state, 0, 1);

    lua_pushstring(model->state, "NOTHING");
    lua_pushinteger(model->state, 0);
    lua_settable(model->state, -3);

    lua_settable(model->state, -3);

    lua_setglobal(model->state, "ir");

    luaL_loadbuffer(model->state, (const char *)src_default_lua, src_default_lua_len, "default.lua");
    if(lua_pcall(model->state, 0, LUA_MULTRET, 0)) {
        ir_error("ir_model_new: Failed to initialize Lua: %s", lua_tostring(model->state, -1));
        lua_close(model->state);
        return 1;
    }
    // ...
    return 0;
}