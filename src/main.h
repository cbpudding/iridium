// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef MAIN_H
#define MAIN_H

#include "model.h"
#include "subscription.h"
#include "view.h"

typedef struct {
	ir_model model;
	ir_subscription subs;
	ir_view view;
} ir_engine;

extern ir_engine ENGINE;

#endif