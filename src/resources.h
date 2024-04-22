// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef RESOURCES_H
#define RESOURCES_H

#include <luajit-2.1/lua.h>

int ir_resources_fetch_lua(lua_State *L);
int ir_resources_mount_lua(lua_State *L);
int ir_resources_umount_lua(lua_State *L);

#endif