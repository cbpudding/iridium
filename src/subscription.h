// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef SUBSCRIPTION_H
#define SUBSCRIPTION_H

#include <allegro5/events.h>
#include <allegro5/timer.h>
#include <luajit-2.1/lua.h>

typedef struct {
	ALLEGRO_TIMER *frame_timer;
	ALLEGRO_EVENT_QUEUE *queue;
} ir_subscription;

void ir_subscription_drop(ir_subscription *subs);
int ir_subscription_new(ir_subscription *subs);

int ir_subscription_frametimer_lua(lua_State *L);
int ir_subscription_keycode_lua(lua_State *L);
int ir_subscription_poll_lua(lua_State *L);

#endif