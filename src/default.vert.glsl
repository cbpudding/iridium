// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#extension GL_EXT_texture_array : enable

uniform mat4 camera;
uniform sampler2DArray textures;

attribute vec3 position;
attribute int textureId;
attribute vec2 texcoord;

void main() {
    gl_Position = camera * vec4(position, 1.0);
}