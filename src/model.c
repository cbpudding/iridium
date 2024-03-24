// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "model.h"

void ir_model_drop(ir_model *model) {
    // ...
    lua_close(model->state);
}

int ir_model_new(ir_model *model) {
    model->state = luaL_newstate();
    if(!model->state) {
        return 1;
    }
    // ...
    return 0;
}