// ============================================
// Extra2D Combined Shader File
// Name: shape
// Category: builtin
// Version: 1.0
// ============================================

#meta
{
    "name": "shape",
    "category": "builtin",
    "author": "Extra2D Team",
    "description": "形状渲染Shader，支持顶点颜色批处理"
}

#vertex
#version 300 es
precision highp float;

layout(location = 0) in vec2 a_position;
layout(location = 1) in vec4 a_color;

uniform mat4 u_viewProjection;

out vec4 v_color;

void main() {
    gl_Position = u_viewProjection * vec4(a_position, 0.0, 1.0);
    v_color = a_color;
}

#fragment
#version 300 es
precision highp float;

in vec4 v_color;

out vec4 fragColor;

void main() {
    fragColor = v_color;
}
