// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <allegro5/events.h>
#include <allegro5/joystick.h>
#include <allegro5/keyboard.h>
#include <allegro5/mouse.h>
#include <luajit-2.1/lua.h>

#include "log.h"
#include "main.h"
#include "subscription.h"

void ir_subscription_drop(ir_subscription *subs) {
	al_unregister_event_source(
		subs->queue, al_get_timer_event_source(subs->frame_timer)
	);
	al_destroy_timer(subs->frame_timer);

	if (al_is_joystick_installed()) {
		al_unregister_event_source(subs->queue, al_get_joystick_event_source());
		al_uninstall_joystick();
	}

	if (al_is_keyboard_installed()) {
		al_unregister_event_source(subs->queue, al_get_keyboard_event_source());
		al_uninstall_keyboard();
	}

	if (al_is_mouse_installed()) {
		al_unregister_event_source(subs->queue, al_get_mouse_event_source());
		al_uninstall_mouse();
	}

	al_destroy_event_queue(subs->queue);
}

int ir_subscription_new(ir_subscription *subs) {
	ALLEGRO_EVENT_SOURCE *source;
	subs->queue = al_create_event_queue();

	if (al_install_joystick()) {
		if ((source = al_get_joystick_event_source())) {
			al_register_event_source(subs->queue, source);
		}
	}

	if (al_install_keyboard()) {
		if ((source = al_get_keyboard_event_source())) {
			al_register_event_source(subs->queue, source);
		}
	}

	if (al_install_mouse()) {
		if ((source = al_get_mouse_event_source())) {
			al_register_event_source(subs->queue, source);
		}
	}

	// If ir.internal.frametimer isn't called, expect your game to have the
	// framerate of a slideshow. ~ahill
	subs->frame_timer = al_create_timer(1.0);
	source = al_get_timer_event_source(subs->frame_timer);
	al_register_event_source(subs->queue, source);

	return 0;
}

int ir_subscription_frametimer_lua(lua_State *L) {
	float framerate = lua_tonumber(L, -1);
	lua_pop(L, 1);
	al_set_timer_speed(ENGINE.subs.frame_timer, framerate);
	al_start_timer(ENGINE.subs.frame_timer);
	return 0;
}

// Allegro apparently doesn't have this functionality, so I'll need to hack it
// together. ~ahill
extern const char *_al_keyboard_common_names[];

int ir_subscription_keycode_lua(lua_State *L) {
	char *keycode = lua_tostring(L, -1);
	for(int i = 0; i < ALLEGRO_KEY_MAX; i++) {
		if(!strcmp(_al_keyboard_common_names[i], keycode)) {
			lua_pop(L, 1);
			lua_pushinteger(L, i);
			return 1;
		}
	}
	lua_pop(L, 1);
	return 0;
}

int ir_subscription_poll_lua(lua_State *L) {
	ALLEGRO_EVENT event;
	al_wait_for_event(ENGINE.subs.queue, &event);
	lua_createtable(L, 0, 2);

	lua_pushinteger(L, event.type);
	lua_setfield(L, -2, "type");

	lua_pushnumber(L, event.any.timestamp);
	lua_setfield(L, -2, "timestamp");

	switch (event.type) {
	case ALLEGRO_EVENT_DISPLAY_RESIZE:
		lua_pushinteger(L, event.display.height);
		lua_setfield(L, -2, "height");

		lua_pushinteger(L, event.display.width);
		lua_setfield(L, -2, "width");

		lua_pushinteger(L, event.display.x);
		lua_setfield(L, -2, "x");

		lua_pushinteger(L, event.display.y);
		lua_setfield(L, -2, "y");
		break;
	case ALLEGRO_EVENT_KEY_CHAR:
		lua_pushinteger(L, event.keyboard.unichar);
		lua_setfield(L, -2, "unichar");

		lua_pushinteger(L, event.keyboard.modifiers);
		lua_setfield(L, -2, "modifiers");

		lua_pushboolean(L, event.keyboard.repeat);
		lua_setfield(L, -2, "repeat");
		__attribute__((fallthrough));
	case ALLEGRO_EVENT_KEY_DOWN:
		__attribute__((fallthrough));
	case ALLEGRO_EVENT_KEY_UP:
		lua_pushinteger(L, event.keyboard.keycode);
		lua_setfield(L, -2, "keycode");
		break;
	case ALLEGRO_EVENT_MOUSE_AXES:
		lua_pushinteger(L, event.mouse.x);
		lua_setfield(L, -2, "x");

		lua_pushinteger(L, event.mouse.y);
		lua_setfield(L, -2, "y");

		lua_pushinteger(L, event.mouse.z);
		lua_setfield(L, -2, "z");

		lua_pushinteger(L, event.mouse.w);
		lua_setfield(L, -2, "w");

		lua_pushinteger(L, event.mouse.dx);
		lua_setfield(L, -2, "dx");

		lua_pushinteger(L, event.mouse.dy);
		lua_setfield(L, -2, "dy");

		lua_pushinteger(L, event.mouse.dz);
		lua_setfield(L, -2, "dz");

		lua_pushinteger(L, event.mouse.dw);
		lua_setfield(L, -2, "dw");

		lua_pushnumber(L, event.mouse.pressure);
		lua_setfield(L, -2, "pressure");
		break;
	case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
		__attribute__((fallthrough));
	case ALLEGRO_EVENT_MOUSE_BUTTON_UP:
		lua_pushinteger(L, event.mouse.button);
		lua_setfield(L, -2, "button");

		lua_pushnumber(L, event.mouse.pressure);
		lua_setfield(L, -2, "pressure");
		__attribute__((fallthrough));
	case ALLEGRO_EVENT_MOUSE_ENTER_DISPLAY:
		__attribute__((fallthrough));
	case ALLEGRO_EVENT_MOUSE_LEAVE_DISPLAY:
		lua_pushinteger(L, event.mouse.x);
		lua_setfield(L, -2, "x");

		lua_pushinteger(L, event.mouse.y);
		lua_setfield(L, -2, "y");

		lua_pushinteger(L, event.mouse.z);
		lua_setfield(L, -2, "z");

		lua_pushinteger(L, event.mouse.w);
		lua_setfield(L, -2, "w");
		break;
	case ALLEGRO_EVENT_TIMER:
		lua_pushinteger(L, event.timer.count);
		lua_setfield(L, -2, "count");

		// Pointers are just numbers... ~ahill
		lua_pushinteger(L, (intmax_t)event.timer.source);
		lua_setfield(L, -2, "source");
		break;
	default:
		ir_warn("ir_subscription_poll_lua: Unhandled event: %u", event.type);
		break;
	}
	return 1;
}