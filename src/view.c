// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "view.h"

void ir_view_drop(ir_view *view) {
    // ...
    al_destroy_display(view->display);
}

int ir_view_new(ir_view *view) {
    al_set_new_display_flags(ALLEGRO_FULLSCREEN | ALLEGRO_OPENGL);
    al_set_new_display_option(ALLEGRO_OPENGL_MAJOR_VERSION, 2, ALLEGRO_REQUIRE);
    al_set_new_display_option(ALLEGRO_OPENGL_MINOR_VERSION, 0, ALLEGRO_REQUIRE);
    al_set_new_display_option(ALLEGRO_DEFAULT_SHADER_PLATFORM, ALLEGRO_SHADER_GLSL, ALLEGRO_REQUIRE);
    al_set_new_display_option(ALLEGRO_VSYNC, 1, ALLEGRO_SUGGEST);
    al_set_new_window_title("Iridium");
    view->display = al_create_display(1920, 1080);
    if(!view->display) {
        return 1;
    }
    // ...
    return 0;
}