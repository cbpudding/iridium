// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <allegro5/allegro.h>

#include "log.h"
#include "main.h"
#include "model.h"
#include "subscription.h"
#include "view.h"

ir_engine ENGINE;

int ir_run(ir_model *model, ir_subscription *subs, ir_view *view) {
    // These will get used eventually. This is just to prevent the compiler from
    // complaining about unused variables. ~ahill
    (void)subs;
    (void)view;

    lua_getglobal(model->state, "ir");
    if(!lua_istable(model->state, -1)) {
        ir_error("ir_run: Failed to get ir table");
        lua_pop(model->state, 1);
        return 1;
    }

    lua_getfield(model->state, -1, "kernel");
    if(!lua_isfunction(model->state, -1)) {
        ir_error("ir_run: Failed to get ir.kernel");
        lua_pop(model->state, 2);
        return 1;
    }
    
    if(lua_pcall(model->state, 0, 0, 0)) {
        ir_error("ir_run: Failed to run ir.kernel: %s", lua_tostring(model->state, -1));
        lua_pop(model->state, 2);
        return 1;
    }

    lua_pop(model->state, 1);

    return 0;
}

int main(int argc, char *argv[]) {
    int status = 1;

    // We'll ignore argc and argv for now, but we might need to parse command
    // line arguments later. The following is simply to get the compiler to
    // stop complaining about unused variables in main. ~ahill
    (void)argc;
    (void)argv;

    // al_init registers an atexit function to clean itself up later. ~ahill
    if(!al_init()) {
        ir_error("main: Failed to initialize Allegro");
        return 1;
    }

    if(!ir_model_new(&ENGINE.model)) {
        if(!ir_subscription_new(&ENGINE.subs)) {
            if(!ir_view_new(&ENGINE.view)) {
                if(!ir_run(&ENGINE.model, &ENGINE.subs, &ENGINE.view)) {
                    status = 0;
                }
                ir_view_drop(&ENGINE.view);
            }
            ir_subscription_drop(&ENGINE.subs);
        }
        ir_model_drop(&ENGINE.model);
    }

    return status;
}