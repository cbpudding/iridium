// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "matrix.h"

int ir_matrix_new(lua_State *L) {
    int columns;
    float *raw;
    int rows;
    if(lua_gettop(L) >= 2) {
        if(lua_isnumber(L, -1) && lua_isnumber(L, -2)) {
            columns = lua_tonumber(L, -2);
            rows = lua_tonumber(L, -1);
            lua_pop(L, 2);

            lua_createtable(L, 0, 3);

            lua_pushstring(L, "columns");
            lua_pushinteger(L, columns);
            lua_settable(L, -3);

            lua_pushstring(L, "raw");
            raw = lua_newuserdata(L, columns * rows * sizeof(float));
            lua_settable(L, -3);

            lua_pushstring(L, "rows");
            lua_pushinteger(L, rows);
            lua_settable(L, -3);

            for(int i = 0; i < columns * rows; i++) {
                raw[i] = 0.0;
            }

            return 1;
        }
    }
    return 0;
}