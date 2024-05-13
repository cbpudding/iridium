// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <allegro5/allegro.h>
#include <stdio.h>

#include "log.h"

const char *LOGLEVEL_PREFIX[4] = {"DBUG", "EROR", "INFO", "WARN"};

// Internal logging function. Should not be used outside of the other logging
// functions for the developer's remaining sanity. ~ahill
void ir_log(ir_loglevel level, const char *format, va_list args) {
	printf("%011.4f %4s ", al_get_time(), LOGLEVEL_PREFIX[level]);
	vprintf(format, args);
// Differing newlines for DOS-based systems and teletypewriters. ~ahill
#ifdef _WIN32
	printf("\r\n");
#else
	printf("\n");
#endif
}

void ir_log_lua(ir_loglevel level, lua_State *L) {
	int argc = lua_gettop(L);
	printf("%011.4f %4s ", al_get_time(), LOGLEVEL_PREFIX[level]);
	for (int i = argc; i >= 1; i--) {
		if (i < argc) {
			putc('\t', stdout);
		}
		printf("%s", lua_tostring(L, i * -1));
	}
	lua_pop(L, argc);
#ifdef _WIN32
	printf("\r\n");
#else
	printf("\n");
#endif
}

void ir_debug(const char *format, ...) {
	va_list args;
	va_start(args, format);
	ir_log(IRLOG_DEBUG, format, args);
	va_end(args);
}

int ir_debug_lua(lua_State *L) {
	ir_log_lua(IRLOG_DEBUG, L);
	return 0;
}

void ir_error(const char *format, ...) {
	va_list args;
	va_start(args, format);
	ir_log(IRLOG_ERROR, format, args);
	va_end(args);
}

int ir_error_lua(lua_State *L) {
	ir_log_lua(IRLOG_ERROR, L);
	return 0;
}

void ir_info(const char *format, ...) {
	va_list args;
	va_start(args, format);
	ir_log(IRLOG_INFO, format, args);
	va_end(args);
}

int ir_info_lua(lua_State *L) {
	ir_log_lua(IRLOG_INFO, L);
	return 0;
}

void ir_warn(const char *format, ...) {
	va_list args;
	va_start(args, format);
	ir_log(IRLOG_WARNING, format, args);
	va_end(args);
}

int ir_warn_lua(lua_State *L) {
	ir_log_lua(IRLOG_WARNING, L);
	return 0;
}