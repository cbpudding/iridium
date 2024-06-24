// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <allegro5/events.h>
#include <luajit-2.1/lua.h>
#include <physfs.h>

#include "log.h"
#include "matrix.h"
#include "model.h"
#include "resources.h"
#include "subscription.h"
#include "view.h"

// Workaround for not having #embed yet. ~ahill
#include "kernel.lua.h"

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

	lua_createtable(model->state, 0, 9);

	lua_pushcfunction(model->state, ir_debug_lua);
	lua_setfield(model->state, -2, "debug");

	lua_pushcfunction(model->state, ir_error_lua);
	lua_setfield(model->state, -2, "error");

	lua_pushcfunction(model->state, ir_info_lua);
	lua_setfield(model->state, -2, "info");

	lua_pushcfunction(model->state, ir_shader_new_lua);
	lua_setfield(model->state, -2, "shader");

	lua_pushcfunction(model->state, ir_model_time_lua);
	lua_setfield(model->state, -2, "time");

	lua_pushcfunction(model->state, ir_warn_lua);
	lua_setfield(model->state, -2, "warn");

	ir_model_new_internal(model);

	ir_matrix_init_lua(model->state);

	ir_vector_init_lua(model->state);

	lua_setglobal(model->state, "ir");

	luaL_loadbuffer(
		model->state,
		(const char *)src_kernel_lua,
		src_kernel_lua_len,
		"kernel.lua"
	);
	if (lua_pcall(model->state, 0, LUA_MULTRET, 0)) {
		ir_error(
			"ir_model_new: Failed to initialize kernel: %s",
			lua_tostring(model->state, -1)
		);
		lua_close(model->state);
		return 1;
	}

	return 0;
}

int ir_model_time_lua(lua_State *L) {
	lua_pushnumber(L, al_get_time());
	return 1;
}

// Abandon all hope ye who enter here.
void ir_model_new_internal(ir_model *model) {
	lua_createtable(model->state, 0, 44);

	// Internal Functions

	lua_pushcfunction(model->state, ir_view_clear_lua);
	lua_setfield(model->state, -2, "clear");

	lua_pushcfunction(model->state, ir_resources_fetch_lua);
	lua_setfield(model->state, -2, "fetch");

	lua_pushcfunction(model->state, ir_view_fullscreen_lua);
	lua_setfield(model->state, -2, "fullscreen");

	lua_pushcfunction(model->state, ir_view_height_lua);
	lua_setfield(model->state, -2, "height");

	lua_pushcfunction(model->state, ir_resources_list_lua);
	lua_setfield(model->state, -2, "list");

	lua_pushcfunction(model->state, ir_resources_mount_lua);
	lua_setfield(model->state, -2, "mount");

	lua_pushcfunction(model->state, ir_subscription_poll_lua);
	lua_setfield(model->state, -2, "poll");

	lua_pushcfunction(model->state, ir_view_present_lua);
	lua_setfield(model->state, -2, "present");

	lua_pushcfunction(model->state, ir_view_render_lua);
	lua_setfield(model->state, -2, "render");

	lua_pushcfunction(model->state, ir_view_setcamera_lua);
	lua_setfield(model->state, -2, "setcamera");

	lua_pushcfunction(model->state, ir_view_texturemap_lua);
	lua_setfield(model->state, -2, "texturemap");

	lua_pushcfunction(model->state, ir_resources_umount_lua);
	lua_setfield(model->state, -2, "umount");

	lua_pushcfunction(model->state, ir_view_width_lua);
	lua_setfield(model->state, -2, "width");

	// Internal Constants

	lua_pushinteger(model->state, ALLEGRO_EVENT_JOYSTICK_AXIS);
	lua_setfield(model->state, -2, "EVENT_JOYSTICK_AXIS");

	lua_pushinteger(model->state, ALLEGRO_EVENT_JOYSTICK_BUTTON_DOWN);
	lua_setfield(model->state, -2, "EVENT_JOYSTICK_BUTTON_DOWN");

	lua_pushinteger(model->state, ALLEGRO_EVENT_JOYSTICK_BUTTON_UP);
	lua_setfield(model->state, -2, "EVENT_JOYSTICK_BUTTON_UP");

	lua_pushinteger(model->state, ALLEGRO_EVENT_JOYSTICK_CONFIGURATION);
	lua_setfield(model->state, -2, "EVENT_JOYSTICK_CONFIGURATION");

	lua_pushinteger(model->state, ALLEGRO_EVENT_KEY_DOWN);
	lua_setfield(model->state, -2, "EVENT_KEY_DOWN");

	lua_pushinteger(model->state, ALLEGRO_EVENT_KEY_CHAR);
	lua_setfield(model->state, -2, "EVENT_KEY_CHAR");

	lua_pushinteger(model->state, ALLEGRO_EVENT_KEY_UP);
	lua_setfield(model->state, -2, "EVENT_KEY_UP");

	lua_pushinteger(model->state, ALLEGRO_EVENT_MOUSE_AXES);
	lua_setfield(model->state, -2, "EVENT_MOUSE_AXES");

	lua_pushinteger(model->state, ALLEGRO_EVENT_MOUSE_BUTTON_DOWN);
	lua_setfield(model->state, -2, "EVENT_MOUSE_BUTTON_DOWN");

	lua_pushinteger(model->state, ALLEGRO_EVENT_MOUSE_BUTTON_UP);
	lua_setfield(model->state, -2, "EVENT_MOUSE_BUTTON_UP");

	lua_pushinteger(model->state, ALLEGRO_EVENT_MOUSE_ENTER_DISPLAY);
	lua_setfield(model->state, -2, "EVENT_MOUSE_ENTER_DISPLAY");

	lua_pushinteger(model->state, ALLEGRO_EVENT_MOUSE_LEAVE_DISPLAY);
	lua_setfield(model->state, -2, "EVENT_MOUSE_LEAVE_DISPLAY");

	lua_pushinteger(model->state, ALLEGRO_EVENT_MOUSE_WARPED);
	lua_setfield(model->state, -2, "EVENT_MOUSE_WARPED");

	lua_pushinteger(model->state, ALLEGRO_EVENT_TIMER);
	lua_setfield(model->state, -2, "EVENT_TIMER");

	lua_pushinteger(model->state, ALLEGRO_EVENT_DISPLAY_EXPOSE);
	lua_setfield(model->state, -2, "EVENT_DISPLAY_EXPOSE");

	lua_pushinteger(model->state, ALLEGRO_EVENT_DISPLAY_RESIZE);
	lua_setfield(model->state, -2, "EVENT_DISPLAY_RESIZE");

	lua_pushinteger(model->state, ALLEGRO_EVENT_DISPLAY_CLOSE);
	lua_setfield(model->state, -2, "EVENT_DISPLAY_CLOSE");

	lua_pushinteger(model->state, ALLEGRO_EVENT_DISPLAY_LOST);
	lua_setfield(model->state, -2, "EVENT_DISPLAY_LOST");

	lua_pushinteger(model->state, ALLEGRO_EVENT_DISPLAY_FOUND);
	lua_setfield(model->state, -2, "EVENT_DISPLAY_FOUND");

	lua_pushinteger(model->state, ALLEGRO_EVENT_DISPLAY_SWITCH_IN);
	lua_setfield(model->state, -2, "EVENT_DISPLAY_SWITCH_IN");

	lua_pushinteger(model->state, ALLEGRO_EVENT_DISPLAY_SWITCH_OUT);
	lua_setfield(model->state, -2, "EVENT_DISPLAY_SWITCH_OUT");

	lua_pushinteger(model->state, ALLEGRO_EVENT_DISPLAY_ORIENTATION);
	lua_setfield(model->state, -2, "EVENT_DISPLAY_ORIENTATION");

	lua_pushinteger(model->state, ALLEGRO_EVENT_DISPLAY_HALT_DRAWING);
	lua_setfield(model->state, -2, "EVENT_DISPLAY_HALT_DRAWING");

	lua_pushinteger(model->state, ALLEGRO_EVENT_DISPLAY_RESUME_DRAWING);
	lua_setfield(model->state, -2, "EVENT_DISPLAY_RESUME_DRAWING");

	lua_pushinteger(model->state, ALLEGRO_EVENT_TOUCH_BEGIN);
	lua_setfield(model->state, -2, "EVENT_TOUCH_BEGIN");

	lua_pushinteger(model->state, ALLEGRO_EVENT_TOUCH_END);
	lua_setfield(model->state, -2, "EVENT_TOUCH_END");

	lua_pushinteger(model->state, ALLEGRO_EVENT_TOUCH_MOVE);
	lua_setfield(model->state, -2, "EVENT_TOUCH_MOVE");

	lua_pushinteger(model->state, ALLEGRO_EVENT_TOUCH_CANCEL);
	lua_setfield(model->state, -2, "EVENT_TOUCH_CANCEL");

	lua_pushinteger(model->state, ALLEGRO_EVENT_DISPLAY_CONNECTED);
	lua_setfield(model->state, -2, "EVENT_DISPLAY_CONNECTED");

	lua_pushinteger(model->state, ALLEGRO_EVENT_DISPLAY_DISCONNECTED);
	lua_setfield(model->state, -2, "EVENT_DISPLAY_DISCONNECTED");

	lua_pushinteger(model->state, ALLEGRO_EVENT_DROP);
	lua_setfield(model->state, -2, "EVENT_DROP");

	lua_setfield(model->state, -2, "internal");
}

int ir_push_error_lua(lua_State *L, const char *restrict fmt, ...) {
    va_list args1, args2;
    va_start(args1, fmt);
    va_copy(args2, args1);

    size_t bufsize = vsnprintf(NULL, 0, fmt, args1);
    va_end(args1);

    char errstr[bufsize];
    vsnprintf(errstr, bufsize, fmt, args2);

    va_end(args2);

    lua_pushboolean(L, false);
    lua_pushstring(L, errstr);

    return 2;
}
