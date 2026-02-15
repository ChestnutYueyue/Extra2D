// ============================================
// Extra2D Combined Shader File
// Name: pixelate
// Category: effects
// Version: 1.0
// ============================================

#meta
{
    "name": "pixelate",
    "category": "effects",
    "author": "Extra2D Team",
    "description": "像素化特效Shader"
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
uniform float u_pixelSize;
uniform vec2 u_textureSize;
uniform float u_opacity;

out vec4 fragColor;

void main() {
    vec2 pixel = u_pixelSize / u_textureSize;
    vec2 uv = floor(v_texCoord / pixel) * pixel + pixel * 0.5;

    vec4 texColor = texture(u_texture, uv);
    fragColor = texColor * v_color;
    fragColor.a *= u_opacity;

    if (fragColor.a < 0.01) {
        discard;
    }
}
