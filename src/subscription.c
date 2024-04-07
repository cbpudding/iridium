// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <allegro5/events.h>
#include <allegro5/joystick.h>
#include <allegro5/keyboard.h>
#include <allegro5/mouse.h>

#include "log.h"
#include "main.h"
#include "subscription.h"

void ir_subscription_drop(ir_subscription *subs) {
    // ...

    if(al_is_joystick_installed()) {
        al_unregister_event_source(subs->queue, al_get_joystick_event_source());
        al_uninstall_joystick();
    }

    if(al_is_keyboard_installed()) {
        al_unregister_event_source(subs->queue, al_get_keyboard_event_source());
        al_uninstall_keyboard();
    }

    if(al_is_mouse_installed()) {
        al_unregister_event_source(subs->queue, al_get_mouse_event_source());
        al_uninstall_mouse();
    }

    al_destroy_event_queue(subs->queue);
}

int ir_subscription_new(ir_subscription *subs) {
    ALLEGRO_EVENT_SOURCE *source;
    subs->queue = al_create_event_queue();

    if(al_install_joystick()) {
        if((source = al_get_joystick_event_source())) {
            al_register_event_source(subs->queue, source);
        }
    }

    if(al_install_keyboard()) {
        if((source = al_get_keyboard_event_source())) {
            al_register_event_source(subs->queue, source);
        }
    }

    if(al_install_mouse()) {
        if((source = al_get_mouse_event_source())) {
            al_register_event_source(subs->queue, source);
        }
    }

    // ...

    return 0;
}

int ir_subscription_poll(lua_State *L) {
    ALLEGRO_EVENT event;
    if(al_get_next_event(ENGINE.subs.queue, &event)) {
        lua_createtable(L, 0, 2);

        lua_pushstring(L, "type");
        lua_pushinteger(L, event.type);
        lua_settable(L, -3);

        // TODO: Do we care about the source? ~ahill

        lua_pushstring(L, "timestamp");
        lua_pushnumber(L, event.any.timestamp);
        lua_settable(L, -3);

        switch(event.type) {
            default:
                ir_warn("ir_subscription_poll: Unhandled event: %u", event.type);
                break;
        }
        return 1;
    } else {
        return 0;
    }
}