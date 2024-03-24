// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <allegro5/allegro.h>

#include "view.h"

int main(int argc, char *argv[]) {
    ir_view view;
    // Allegro registers an atexit function to clean itself up later. ~ahill
    if(!al_init()) {
        return 1;
    }
    if(!ir_view_new(&view)) {
        // ...
        ir_view_drop(&view);
    }
    return 1;
}