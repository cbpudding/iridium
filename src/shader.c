// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "log.h"
#include "shader.h"

// TODO: Should I switch over to using Allegro's shader functions at some point? ~ahill

void ir_shader_drop(ir_shader *shader) {
    glDetachShader(shader->program, shader->fragment);
    glDetachShader(shader->program, shader->vertex);
    glDeleteProgram(shader->program);
    glDeleteShader(shader->fragment);
    glDeleteShader(shader->vertex);
}

int ir_shader_new(ir_shader *shader, size_t vert_len, char *vert_src, size_t frag_len, char *frag_src) {
    char buffer[BUFSIZ];
    GLint status;

    // TODO: Is this actually safe enough or should we zero the entire buffer out? ~ahill
    buffer[0] = 0;

    shader->vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(shader->vertex, 1, (const GLchar **)&vert_src, (const int *)&vert_len);
    glCompileShader(shader->vertex);
    glGetShaderiv(shader->vertex, GL_COMPILE_STATUS, &status);
    if(status != GL_TRUE) {
        glGetShaderInfoLog(shader->vertex, BUFSIZ, NULL, buffer);
        ir_error("ir_shader_new: vertex: %s", buffer);
        glDeleteShader(shader->vertex);
        return 1;
    }

    shader->fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(shader->fragment, 1, (const GLchar **)&frag_src, (const int *)&frag_len);
    glCompileShader(shader->fragment);
    glGetShaderiv(shader->fragment, GL_COMPILE_STATUS, &status);
    if(status != GL_TRUE) {
        glGetShaderInfoLog(shader->fragment, BUFSIZ, NULL, buffer);
        ir_error("ir_shader_new: fragment: %s", buffer);
        glDeleteShader(shader->fragment);
        glDeleteShader(shader->vertex);
        return 1;
    }

    shader->program = glCreateProgram();
    glAttachShader(shader->program, shader->fragment);
    glAttachShader(shader->program, shader->vertex);
    glLinkProgram(shader->program);
    glGetProgramiv(shader->program, GL_LINK_STATUS, &status);
    if(status != GL_TRUE) {
        glGetProgramInfoLog(shader->program, BUFSIZ, NULL, buffer);
        ir_error("ir_shader_new: program: %s", buffer);
        // The entire structure has been initialized anyways so why not? ~ahill
        ir_shader_drop(shader);
        return 1;
    }

    return 0;
}