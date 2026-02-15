// ============================================
// Extra2D Combined Shader File
// Name: blur
// Category: effects
// Version: 1.0
// ============================================

#meta
{
    "name": "blur",
    "category": "effects",
    "author": "Extra2D Team",
    "description": "模糊特效Shader"
}

#vertex
#version 300 es
precision highp float;

layout(location = 0) in vec2 a_position;
layout(location = 1) in vec2 a_texCoord;
layout(location = 2) in vec4 a_color;

uniform mat4 u_viewProjection;
uniform mat4 u_model;

out vec2 v_texCoord;
out vec4 v_color;

void main() {
    gl_Position = u_viewProjection * u_model * vec4(a_position, 0.0, 1.0);
    v_texCoord = a_texCoord;
    v_color = a_color;
}

#fragment
#version 300 es
precision highp float;

in vec2 v_texCoord;
in vec4 v_color;

uniform sampler2D u_texture;
uniform float u_radius;
uniform vec2 u_textureSize;
uniform float u_opacity;

out vec4 fragColor;

void main() {
    vec2 texel = u_radius / u_textureSize;

    vec4 sum = vec4(0.0);
    sum += texture(u_texture, v_texCoord + texel * vec2(-1.0, -1.0));
    sum += texture(u_texture, v_texCoord + texel * vec2( 0.0, -1.0));
    sum += texture(u_texture, v_texCoord + texel * vec2( 1.0, -1.0));
    sum += texture(u_texture, v_texCoord + texel * vec2(-1.0,  0.0));
    sum += texture(u_texture, v_texCoord + texel * vec2( 0.0,  0.0));
    sum += texture(u_texture, v_texCoord + texel * vec2( 1.0,  0.0));
    sum += texture(u_texture, v_texCoord + texel * vec2(-1.0,  1.0));
    sum += texture(u_texture, v_texCoord + texel * vec2( 0.0,  1.0));
    sum += texture(u_texture, v_texCoord + texel * vec2( 1.0,  1.0));

    vec4 texColor = sum / 9.0;
    fragColor = texColor * v_color;
    fragColor.a *= u_opacity;

    if (fragColor.a < 0.01) {
        discard;
    }
}
