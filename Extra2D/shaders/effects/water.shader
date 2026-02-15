// ============================================
// Extra2D Combined Shader File
// Name: water
// Category: effects
// Version: 1.0
// ============================================

#meta
{
    "name": "water",
    "category": "effects",
    "author": "Extra2D Team",
    "description": "水波纹特效Shader"
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
uniform float u_waveSpeed;
uniform float u_waveAmplitude;
uniform float u_waveFrequency;
uniform float u_time;

out vec4 fragColor;

void main() {
    vec2 uv = v_texCoord;
    
    float wave = sin(uv.y * u_waveFrequency + u_time * u_waveSpeed) * u_waveAmplitude;
    uv.x += wave;
    
    vec4 texColor = texture(u_texture, uv);
    fragColor = texColor * v_color;
    
    if (fragColor.a < 0.01) {
        discard;
    }
}
