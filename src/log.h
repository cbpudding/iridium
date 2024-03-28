// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef LOG_H
#define LOG_H

#include <stdarg.h>

typedef enum {
    IRLOG_DEBUG = 0,
    IRLOG_ERROR,
    IRLOG_INFO,
    IRLOG_WARNING
} ir_loglevel;

void ir_log(ir_loglevel level, const char *format, va_list args);

void ir_debug(const char *format, ...);
void ir_error(const char *format, ...);
void ir_info(const char *format, ...);
void ir_warn(const char *format, ...);

#endif