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
    // TODO: Remove dofile, load, and loadfile ~ahill
    luaopen_base(model->state);
    luaopen_string(model->state);
    luaopen_table(model->state);
    luaopen_math(model->state);
    luaopen_bit(model->state);
    // TODO: How would one implement os.time safely? ~ahill
    lua_createtable(model->state, 0, 1);
    lua_pushstring(model->state, "msg");
    lua_createtable(model->state, 0, 2);
    lua_pushstring(model->state, "NOTHING");
    lua_pushinteger(model->state, IRMSG_NOTHING);
    lua_settable(model->state, -3);
    lua_pushstring(model->state, "CUSTOM");
    lua_pushinteger(model->state, IRMSG_CUSTOM);
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

int ir_model_update(ir_model *model, ir_message msg) {
    lua_getglobal(model->state, "ir");
    if(!lua_istable(model->state, -1)) {
        ir_error("ir_model_update: Failed to get Lua \"ir\" table");
        lua_pop(model->state, 1);
        return 1;
    }
    lua_getfield(model->state, -1, "update");
    if(!lua_isfunction(model->state, -1)) {
        ir_error("ir_model_update: Failed to get Lua \"ir.update\" function");
        lua_pop(model->state, 2);
        return 1;
    }
    lua_pushinteger(model->state, msg);
    if(lua_pcall(model->state, 1, 0, 0)) {
        ir_error("ir_model_update: Failed to call Lua \"ir.update\" function: %s", lua_tostring(model->state, -1));
        lua_pop(model->state, 2);
        return 1;
    }
    lua_pop(model->state, 1);
    return 0;
}