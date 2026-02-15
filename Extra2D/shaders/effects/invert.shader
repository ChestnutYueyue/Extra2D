// ============================================
// Extra2D Combined Shader File
// Name: invert
// Category: effects
// Version: 1.0
// ============================================

#meta
{
    "name": "invert",
    "category": "effects",
    "author": "Extra2D Team",
    "description": "反相特效Shader"
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
uniform float u_strength;
uniform float u_opacity;

out vec4 fragColor;

void main() {
    vec4 texColor = texture(u_texture, v_texCoord) * v_color;
    vec3 inverted = vec3(1.0) - texColor.rgb;
    texColor.rgb = mix(texColor.rgb, inverted, u_strength);

    fragColor = texColor;
    fragColor.a *= u_opacity;

    if (fragColor.a < 0.01) {
        discard;
    }
}
