// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <allegro5/events.h>
#include <luajit-2.1/lua.h>
#include <physfs.h>

#include "log.h"
#include "model.h"
#include "resources.h"
#include "subscription.h"
#include "view.h"

// Workaround for not having #embed yet. ~ahill
#include "default.lua.h"
#include "kernel.lua.h"
#include "listeners.lua.h"

// TODO: Figure out how to prevent subscriptions from changing the game's state!
// ~ahill

void ir_model_drop(ir_model *model) {
	// ...
	lua_close(model->state);
}

void ir_model_new_internal(ir_model *model);
int ir_model_time_lua(lua_State *L);

int ir_model_new(ir_model *model) {
	model->state = luaL_newstate();
	if (!model->state) {
		ir_error("ir_model_new: Failed to initialize Lua");
		return 1;
	}

	luaopen_base(model->state);
	luaopen_string(model->state);
	luaopen_table(model->state);
	luaopen_math(model->state);
	luaopen_bit(model->state);

	lua_createtable(model->state, 0, 7);

	lua_pushstring(model->state, "debug");
	lua_pushcfunction(model->state, ir_debug_lua);
	lua_settable(model->state, -3);

	lua_pushstring(model->state, "error");
	lua_pushcfunction(model->state, ir_error_lua);
	lua_settable(model->state, -3);

	lua_pushstring(model->state, "info");
	lua_pushcfunction(model->state, ir_info_lua);
	lua_settable(model->state, -3);

	lua_pushstring(model->state, "time");
	lua_pushcfunction(model->state, ir_model_time_lua);
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

	ir_model_new_internal(model);

	lua_setglobal(model->state, "ir");

	luaL_loadbuffer(
		model->state,
		(const char *)src_kernel_lua,
		src_kernel_lua_len,
		"kernel.lua"
	);
	if (lua_pcall(model->state, 0, LUA_MULTRET, 0)) {
		ir_error(
			"ir_model_new: Failed to initialize Lua: %s",
			lua_tostring(model->state, -1)
		);
		lua_close(model->state);
		return 1;
	}

	luaL_loadbuffer(
		model->state,
		(const char *)src_listeners_lua,
		src_listeners_lua_len,
		"listeners.lua"
	);
	if (lua_pcall(model->state, 0, LUA_MULTRET, 0)) {
		ir_error(
			"ir_model_new: Failed to initialize listeners: %s",
			lua_tostring(model->state, -1)
		);
		lua_close(model->state);
		return 1;
	}

	luaL_loadbuffer(
		model->state,
		(const char *)src_default_lua,
		src_default_lua_len,
		"default.lua"
	);
	if (lua_pcall(model->state, 0, LUA_MULTRET, 0)) {
		ir_error(
			"ir_model_new: Failed to initialize Lua: %s",
			lua_tostring(model->state, -1)
		);
		lua_close(model->state);
		return 1;
	}

	// ...

	return 0;
}

int ir_model_time_lua(lua_State *L) {
	lua_pushnumber(L, al_get_time());
	return 1;
}

// Abandon all hope ye who enter here.
void ir_model_new_internal(ir_model *model) {
	lua_pushstring(model->state, "internal");
	lua_createtable(model->state, 0, 38);

	// Internal Functions

	lua_pushstring(model->state, "clear");
	lua_pushcfunction(model->state, ir_view_clear_lua);
	lua_settable(model->state, -3);

	lua_pushstring(model->state, "fetch");
	lua_pushcfunction(model->state, ir_resources_fetch_lua);
	lua_settable(model->state, -3);

	lua_pushstring(model->state, "mount");
	lua_pushcfunction(model->state, ir_resources_mount_lua);
	lua_settable(model->state, -3);

	lua_pushstring(model->state, "poll");
	lua_pushcfunction(model->state, ir_subscription_poll_lua);
	lua_settable(model->state, -3);

	lua_pushstring(model->state, "present");
	lua_pushcfunction(model->state, ir_view_present_lua);
	lua_settable(model->state, -3);

	lua_pushstring(model->state, "render");
	lua_pushcfunction(model->state, ir_view_render_lua);
	lua_settable(model->state, -3);

	lua_pushstring(model->state, "umount");
	lua_pushcfunction(model->state, ir_resources_umount_lua);
	lua_settable(model->state, -3);

	// Internal Constants

	lua_pushstring(model->state, "EVENT_JOYSTICK_AXIS");
	lua_pushinteger(model->state, ALLEGRO_EVENT_JOYSTICK_AXIS);
	lua_settable(model->state, -3);

	lua_pushstring(model->state, "EVENT_JOYSTICK_BUTTON_DOWN");
	lua_pushinteger(model->state, ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN);
	lua_settable(model->state, -3);

	lua_pushstring(model->state, "EVENT_JOYSTICK_BUTTON_UP");
	lua_pushinteger(model->state, ALLEGRO_EVENT_JOYSTICK_BUTTON_UP);
	lua_settable(model->state, -3);

	lua_pushstring(model->state, "EVENT_JOYSTICK_CONFIGURATION");
	lua_pushinteger(model->state, ALLEGRO_EVENT_JOYSTICK_CONFIGURATION);
	lua_settable(model->state, -3);

	lua_pushstring(model->state, "EVENT_KEY_DOWN");
	lua_pushinteger(model->state, ALLEGRO_EVENT_KEY_DOWN);
	lua_settable(model->state, -3);

	lua_pushstring(model->state, "EVENT_KEY_CHAR");
	lua_pushinteger(model->state, ALLEGRO_EVENT_KEY_CHAR);
	lua_settable(model->state, -3);

	lua_pushstring(model->state, "EVENT_KEY_UP");
	lua_pushinteger(model->state, ALLEGRO_EVENT_KEY_UP);
	lua_settable(model->state, -3);

	lua_pushstring(model->state, "EVENT_MOUSE_AXES");
	lua_pushinteger(model->state, ALLEGRO_EVENT_MOUSE_AXES);
	lua_settable(model->state, -3);

	lua_pushstring(model->state, "EVENT_MOUSE_BUTTON_DOWN");
	lua_pushinteger(model->state, ALLEGRO_EVENT_MOUSE_BUTTON_DOWN);
	lua_settable(model->state, -3);

	lua_pushstring(model->state, "EVENT_MOUSE_BUTTON_UP");
	lua_pushinteger(model->state, ALLEGRO_EVENT_MOUSE_BUTTON_UP);
	lua_settable(model->state, -3);

	lua_pushstring(model->state, "EVENT_MOUSE_ENTER_DISPLAY");
	lua_pushinteger(model->state, ALLEGRO_EVENT_MOUSE_ENTER_DISPLAY);
	lua_settable(model->state, -3);

	lua_pushstring(model->state, "EVENT_MOUSE_LEAVE_DISPLAY");
	lua_pushinteger(model->state, ALLEGRO_EVENT_MOUSE_LEAVE_DISPLAY);
	lua_settable(model->state, -3);

	lua_pushstring(model->state, "EVENT_MOUSE_WARPED");
	lua_pushinteger(model->state, ALLEGRO_EVENT_MOUSE_WARPED);
	lua_settable(model->state, -3);

	lua_pushstring(model->state, "EVENT_TIMER");
	lua_pushinteger(model->state, ALLEGRO_EVENT_TIMER);
	lua_settable(model->state, -3);

	lua_pushstring(model->state, "EVENT_DISPLAY_EXPOSE");
	lua_pushinteger(model->state, ALLEGRO_EVENT_DISPLAY_EXPOSE);
	lua_settable(model->state, -3);

	lua_pushstring(model->state, "EVENT_DISPLAY_RESIZE");
	lua_pushinteger(model->state, ALLEGRO_EVENT_DISPLAY_RESIZE);
	lua_settable(model->state, -3);

	lua_pushstring(model->state, "EVENT_DISPLAY_CLOSE");
	lua_pushinteger(model->state, ALLEGRO_EVENT_DISPLAY_CLOSE);
	lua_settable(model->state, -3);

	lua_pushstring(model->state, "EVENT_DISPLAY_LOST");
	lua_pushinteger(model->state, ALLEGRO_EVENT_DISPLAY_LOST);
	lua_settable(model->state, -3);

	lua_pushstring(model->state, "EVENT_DISPLAY_FOUND");
	lua_pushinteger(model->state, ALLEGRO_EVENT_DISPLAY_FOUND);
	lua_settable(model->state, -3);

	lua_pushstring(model->state, "EVENT_DISPLAY_SWITCH_IN");
	lua_pushinteger(model->state, ALLEGRO_EVENT_DISPLAY_SWITCH_IN);
	lua_settable(model->state, -3);

	lua_pushstring(model->state, "EVENT_DISPLAY_SWITCH_OUT");
	lua_pushinteger(model->state, ALLEGRO_EVENT_DISPLAY_SWITCH_OUT);
	lua_settable(model->state, -3);

	lua_pushstring(model->state, "EVENT_DISPLAY_ORIENTATION");
	lua_pushinteger(model->state, ALLEGRO_EVENT_DISPLAY_ORIENTATION);
	lua_settable(model->state, -3);

	lua_pushstring(model->state, "EVENT_DISPLAY_HALT_DRAWING");
	lua_pushinteger(model->state, ALLEGRO_EVENT_DISPLAY_HALT_DRAWING);
	lua_settable(model->state, -3);

	lua_pushstring(model->state, "EVENT_DISPLAY_RESUME_DRAWING");
	lua_pushinteger(model->state, ALLEGRO_EVENT_DISPLAY_RESUME_DRAWING);
	lua_settable(model->state, -3);

	lua_pushstring(model->state, "EVENT_TOUCH_BEGIN");
	lua_pushinteger(model->state, ALLEGRO_EVENT_TOUCH_BEGIN);
	lua_settable(model->state, 3);

	lua_pushstring(model->state, "EVENT_TOUCH_END");
	lua_pushinteger(model->state, ALLEGRO_EVENT_TOUCH_END);
	lua_settable(model->state, -3);

	lua_pushstring(model->state, "EVENT_TOUCH_MOVE");
	lua_pushinteger(model->state, ALLEGRO_EVENT_TOUCH_MOVE);
	lua_settable(model->state, -3);

	lua_pushstring(model->state, "EVENT_TOUCH_CANCEL");
	lua_pushinteger(model->state, ALLEGRO_EVENT_TOUCH_CANCEL);
	lua_settable(model->state, -3);

	lua_pushstring(model->state, "EVENT_DISPLAY_CONNECTED");
	lua_pushinteger(model->state, ALLEGRO_EVENT_DISPLAY_CONNECTED);
	lua_settable(model->state, -3);

	lua_pushstring(model->state, "EVENT_DISPLAY_DISCONNECTED");
	lua_pushinteger(model->state, ALLEGRO_EVENT_DISPLAY_DISCONNECTED);
	lua_settable(model->state, -3);

	lua_pushstring(model->state, "EVENT_DROP");
	lua_pushinteger(model->state, ALLEGRO_EVENT_DROP);
	lua_settable(model->state, -3);

	lua_settable(model->state, -3);
}