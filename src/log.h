// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef LOG_H
#define LOG_H

#include <luajit-2.1/lua.h>
#include <luajit-2.1/lauxlib.h>
#include <luajit-2.1/lualib.h>
#include <stdarg.h>

typedef enum {
    IRLOG_DEBUG = 0,
    IRLOG_ERROR,
    IRLOG_INFO,
    IRLOG_WARNING
} ir_loglevel;

void ir_debug(const char *format, ...);
int ir_debug_lua(lua_State *L);
void ir_error(const char *format, ...);
int ir_error_lua(lua_State *L);
void ir_info(const char *format, ...);
int ir_info_lua(lua_State *L);
void ir_warn(const char *format, ...);
int ir_warn_lua(lua_State *L);

#endif