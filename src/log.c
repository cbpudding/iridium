// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <stdio.h>

#include "log.h"

const char *LOGLEVEL_PREFIX[4] = {
    "DEBUG",
    "ERROR",
    "INFO ",
    "WARN "
};

// Internal logging function. Should not be used outside of the other logging
// functions for the developer's remaining sanity. ~ahill
void ir_log(ir_loglevel level, const char *format, va_list args) {
    printf("%5s ", LOGLEVEL_PREFIX[level]);
    vprintf(format, args);
// Differing newlines for DOS-based systems. ~ahill
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

void ir_error(const char *format, ...) {
    va_list args;
    va_start(args, format);
    ir_log(IRLOG_ERROR, format, args);
    va_end(args);
}

void ir_info(const char *format, ...) {
    va_list args;
    va_start(args, format);
    ir_log(IRLOG_INFO, format, args);
    va_end(args);
}

void ir_warn(const char *format, ...) {
    va_list args;
    va_start(args, format);
    ir_log(IRLOG_WARNING, format, args);
    va_end(args);
}