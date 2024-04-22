// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <physfs.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "log.h"
#include "resources.h"

int ir_resources_fetch_lua(lua_State *L) {
	uint8_t *buffer;
	PHYSFS_File *file;
	PHYSFS_sint64 length;
	const char *name;
	if (lua_isstring(L, -1)) {
		name = lua_tostring(L, -1);
		if (PHYSFS_exists(name)) {
			if ((file = PHYSFS_openRead(name))) {
				length = PHYSFS_fileLength(file);
				if ((buffer = malloc(length + 1))) {
					if (PHYSFS_readBytes(file, buffer, length) == length) {
						lua_pop(L, -1);
						// Null terminator because PhysicsFS doesn't care about
						// text data! ~ahill
						*(buffer + length) = 0;
						lua_pushstring(L, (const char *)buffer);
						// Will this cause issues? ~ahill
						free(buffer);
						PHYSFS_close(file);
						return 1;
					} else {
						// Even though, PHYSFS_getLastError is deprecated, the
						// *correct* method doesn't seem much better! ~ahill
						ir_error(
							"ir_resources_fetch_lua: Failed to read file "
							"\"%s\": %s",
							name,
							PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode())
						);
					}
					free(buffer);
				} else {
					ir_error(
						"ir_resources_fetch_lua: Failed to allocate "
						"buffer for \"%s\"",
						name
					);
				}
				PHYSFS_close(file);
			} else {
				ir_error(
					"ir_resources_fetch_lua: Failed to open \"%s\": %s",
					name,
					PHYSFS_getErrorByCode(PHYSFS_getLastErrorCode())
				);
			}
		} else {
			ir_error("ir_resources_fetch_lua: \"%s\" does not exist", name);
		}
	}
	lua_pop(L, -1);
	lua_pushnil(L);
	return 1;
}

int ir_resources_mount_lua(lua_State *L) {
	bool status = false;
	if (lua_isstring(L, -1)) {
		if (PHYSFS_mount(lua_tostring(L, -1), NULL, true)) {
			status = true;
		}
	}
	lua_pop(L, -1);
	lua_pushboolean(L, status);
	return 1;
}

int ir_resources_umount_lua(lua_State *L) {
	if (lua_isstring(L, -1)) {
		PHYSFS_unmount(lua_tostring(L, -1));
	}
	lua_pop(L, -1);
	return 0;
}