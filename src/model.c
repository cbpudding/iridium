// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "model.h"

// Workaround for not having #embed yet. ~ahill
#include "subscriptions.lua.h"
#include "update.lua.h"

void ir_model_drop(ir_model *model) {
    // ...
    lua_close(model->state);
}

int ir_model_new(ir_model *model) {
    model->state = luaL_newstate();
    if(!model->state) {
        return 1;
    }
    lua_createtable(model->state, 0, 1);
    lua_pushstring(model->state, "msg");
    lua_createtable(model->state, 0, 1);
    lua_pushstring(model->state, "NOTHING");
    lua_pushinteger(model->state, IRMSG_NOTHING);
    lua_settable(model->state, -3);
    lua_settable(model->state, -3);
    lua_setglobal(model->state, "ir");
    luaL_loadbuffer(model->state, (const char *)src_subscriptions_lua, src_subscriptions_lua_len, "subscriptions.lua");
    if(lua_pcall(model->state, 0, LUA_MULTRET, 0) != 0) {
        // TODO: Log some debug information for troubleshooting ~ahill
        lua_close(model->state);
        return 1;
    }
    luaL_loadbuffer(model->state, (const char *)src_update_lua, src_update_lua_len, "update.lua");
    if(lua_pcall(model->state, 0, LUA_MULTRET, 0) != 0) {
        // TODO: Log some debug information for troubleshooting ~ahill
        lua_close(model->state);
        return 1;
    }
    // ...
    return 0;
}