// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "subscriptions.h"

void ir_subscriptions_drop(ir_subscriptions *subs) {
    subs->count = 0;
    // ...
    subs->total = 0;
}

int ir_subscriptions_new(ir_subscriptions *subs) {
    subs->count = 0;
    // ...
    subs->total = 8;
    return 0;
}