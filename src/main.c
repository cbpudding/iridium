// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <allegro5/allegro.h>
#include <luajit-2.1/lua.h>

#include "log.h"
#include "main.h"
#include "model.h"
#include "subscription.h"
#include "view.h"

ir_engine ENGINE;

int ir_run_opts(int argc, char *argv[], lua_State *L) {
    char args[BUFSIZ];
    int key = 0;
    int stage = 0;
    int len = 0;
    int value = 0;

    lua_createtable(L, 0, 0);

    if(argc > 1) {
        for(int i = 1; i < argc; i++) {
            strncpy(args + len, argv[i], BUFSIZ - len);
            len += strlen(argv[i]);
            if(len > BUFSIZ) {
                ir_error("ir_run_opts: Arguments list too long");
                return 1;
            }
            if(i == argc - 1) {
                args[len] = 0;
            } else {
                args[len++] = ' ';
            }
        }

        for(int i = 0; i < len; i++) {
            ir_debug("ir_run_opts: %c %d", args[i], stage);
            switch(stage) {
                case 0:
                    if(args[i] == '+') {
                        key = i + 1;
                        stage = 1;
                    }
                    break;
                case 1:
                    if(args[i] == '=') {
                        if(key == i) {
                            stage = 0;
                        } else {
                            args[i] = 0;
                            value = i + 1;
                            stage = 2;
                        }
                    }
                    break;
                case 2:
                    if(value == i && args[i] == '"') {
                        value++;
                        stage = 3;
                    } else if(args[i] == ' ') {
                        args[i] = 0;
                        lua_pushstring(L, args + key);
                        lua_pushstring(L, args + value);
                        lua_settable(L, -3);
                        stage = 0;
                    } else if(i + 1 == len) {
                        lua_pushstring(L, args + key);
                        lua_pushstring(L, args + value);
                        lua_settable(L, -3);
                        // This probably doesn't matter but my gut tells me that
                        // something will go terribly wrong if I don't do this. ~ahill
                        stage = 0;
                    }
                    break;
                case 3:
                    if(args[i] == '"') {
                        args[i] = 0;
                        lua_pushstring(L, args + key);
                        lua_pushstring(L, args + value);
                        lua_settable(L, -3);
                        stage = 0;
                    }
                    break;
            }
        }
    }

    return 0;
}

int ir_run(int argc, char *argv[], ir_engine *engine) {
    lua_getglobal(engine->model.state, "ir");
    if(!lua_istable(engine->model.state, -1)) {
        ir_error("ir_run: Failed to get ir table");
        lua_pop(engine->model.state, 1);
        return 1;
    }

    lua_getfield(engine->model.state, -1, "kernel");
    if(!lua_isfunction(engine->model.state, -1)) {
        ir_error("ir_run: Failed to get ir.kernel");
        lua_pop(engine->model.state, 2);
        return 1;
    }

    if(ir_run_opts(argc, argv, engine->model.state)) {
        lua_pop(engine->model.state, 2);
        return 1;
    }

    if(lua_pcall(engine->model.state, 1, 0, 0)) {
        ir_error("ir_run: Failed to run ir.kernel: %s", lua_tostring(engine->model.state, -1));
        lua_pop(engine->model.state, 2);
        return 1;
    }

    lua_pop(engine->model.state, 1);

    return 0;
}

int main(int argc, char *argv[]) {
    int status = 1;

    // al_init registers an atexit function to clean itself up later. ~ahill
    if(!al_init()) {
        ir_error("main: Failed to initialize Allegro");
        return 1;
    }

    if(!ir_model_new(&ENGINE.model)) {
        if(!ir_subscription_new(&ENGINE.subs)) {
            if(!ir_view_new(&ENGINE.view)) {
                if(!ir_run(argc, argv, &ENGINE)) {
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