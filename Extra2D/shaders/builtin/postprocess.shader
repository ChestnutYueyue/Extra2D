// ============================================
// Extra2D Combined Shader File
// Name: postprocess
// Category: builtin
// Version: 1.0
// ============================================

#meta
{
    "name": "postprocess",
    "category": "builtin",
    "author": "Extra2D Team",
    "description": "后处理基础Shader"
}

#vertex
#version 300 es
precision highp float;

layout(location = 0) in vec2 a_position;
layout(location = 1) in vec2 a_texCoord;

out vec2 v_texCoord;

void main() {
    gl_Position = vec4(a_position, 0.0, 1.0);
    v_texCoord = a_texCoord;
}

#fragment
#version 300 es
precision highp float;

in vec2 v_texCoord;

uniform sampler2D u_texture;

out vec4 fragColor;

void main() {
    fragColor = texture(u_texture, v_texCoord);
}
