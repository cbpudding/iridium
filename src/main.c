// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <allegro5/allegro.h>

int main(int argc, char *argv[]) {
    ALLEGRO_DISPLAY *display;
    // Allegro registers an atexit function to clean itself up later. ~ahill
    if(al_init()) {
        al_set_new_display_flags(ALLEGRO_FULLSCREEN | ALLEGRO_OPENGL_3_0);
        al_set_new_display_option(ALLEGRO_OPENGL_MAJOR_VERSION, 3, ALLEGRO_REQUIRE);
        al_set_new_display_option(ALLEGRO_OPENGL_MINOR_VERSION, 2, ALLEGRO_REQUIRE);
        al_set_new_display_option(ALLEGRO_DEFAULT_SHADER_PLATFORM, ALLEGRO_SHADER_GLSL, ALLEGRO_REQUIRE);
        al_set_new_display_option(ALLEGRO_VSYNC, 1, ALLEGRO_SUGGEST);
        al_set_new_window_title("Iridium");
        if((display = al_create_display(1920, 1080))) {
            // ...
            al_destroy_display(display);
        }
    }
    return 1;
}