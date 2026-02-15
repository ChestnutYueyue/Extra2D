// ============================================
// Extra2D Combined Shader File
// Name: grayscale_outline
// Category: effects
// Version: 1.0
// ============================================

#meta
{
    "name": "grayscale_outline",
    "category": "effects",
    "author": "Extra2D Team",
    "description": "灰度+描边组合效果Shader"
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
uniform float u_grayIntensity;
uniform vec4 u_outlineColor;
uniform float u_thickness;
uniform vec2 u_textureSize;
uniform float u_opacity;

out vec4 fragColor;

void main() {
    vec4 color = texture(u_texture, v_texCoord) * v_color;

    float gray = dot(color.rgb, vec3(0.299, 0.587, 0.114));
    color.rgb = mix(color.rgb, vec3(gray), u_grayIntensity);

    float alpha = 0.0;
    vec2 offset = u_thickness / u_textureSize;

    alpha += texture(u_texture, v_texCoord + vec2(-offset.x, 0.0)).a;
    alpha += texture(u_texture, v_texCoord + vec2(offset.x, 0.0)).a;
    alpha += texture(u_texture, v_texCoord + vec2(0.0, -offset.y)).a;
    alpha += texture(u_texture, v_texCoord + vec2(0.0, offset.y)).a;

    if (color.a < 0.1 && alpha > 0.0) {
        fragColor = u_outlineColor;
    } else {
        fragColor = color;
    }

    fragColor.a *= u_opacity;

    if (fragColor.a < 0.01) {
        discard;
    }
}
