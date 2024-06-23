// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

uniform mat4 camera;

attribute vec3 position;
attribute int textureid;
attribute vec2 texcoord;

varying vec2 frag_texcoord;
varying int frag_textureid;

void main() {
    frag_texcoord = texcoord;
    frag_textureid = textureid;
    gl_Position = camera * vec4(position, 1.0);
}