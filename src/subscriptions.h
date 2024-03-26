// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef SUBSCRIPTIONS_H
#define SUBSCRIPTIONS_H

typedef struct {
    int count;
    // ...
    int total;
} ir_subscriptions;

void ir_subscriptions_drop(ir_subscriptions *subs);
int ir_subscriptions_new(ir_subscriptions *subs);

#endif