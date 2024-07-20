// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#extension GL_EXT_texture_array : enable

uniform sampler2DArray textures;

varying vec2 frag_texcoord;
varying float frag_textureid;

void main() {
    gl_FragColor = texture2DArray(textures, vec3(frag_texcoord, float(frag_textureid)));
}
