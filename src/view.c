// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "log.h"
#include "view.h"

// When will #embed be a thing? ~ahill
#include "default.frag.glsl.h"
#include "default.vert.glsl.h"

void ir_view_drop(ir_view *view) {
    // ...
    ir_shader_drop(&view->shader);
    glDeleteBuffers(1, &view->vbo);
    if(al_is_audio_installed()) {
        al_uninstall_audio();
    }
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
        ir_error("ir_view_new: Failed to create display");
        return 1;
    }
    if(!al_install_audio()) {
        ir_warn("ir_view_new: Failed to initialize the audio subsystem");
    }
    glGenBuffers(1, &view->vbo);
    glBindBuffer(GL_ARRAY_BUFFER, view->vbo);
    if(ir_shader_new(&view->shader, src_default_vert_glsl_len, (char *)src_default_vert_glsl, src_default_frag_glsl_len, (char *)src_default_frag_glsl)) {
        ir_error("ir_view_new: Failed to compile default shader");
        glDeleteBuffers(1, &view->vbo);
        if(al_is_audio_installed()) {
            al_uninstall_audio();
        }
        al_destroy_display(view->display);
        return 1;
    }
    // ...
    return 0;
}

int ir_view_clear_lua(lua_State *L) {
    // We don't actually use the Lua state in this case, it's just here to match
    // the function signature for Lua. ~ahill
    (void)L;

    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    return 0;
}

int ir_view_present_lua(lua_State *L) {
    // We don't actually use the Lua state in this case, it's just here to match
    // the function signature for Lua. ~ahill
    (void)L;

    al_flip_display();
    return 0;
}