#pragma once

#include <extra2d/graphics/opengl/gl_shader.h>
#include <extra2d/core/types.h>
#include <extra2d/core/color.h>
#include <glm/vec4.hpp>

namespace extra2d {

struct WaterParams {
    float waveSpeed = 1.0f;
    float waveAmplitude = 0.02f;
    float waveFrequency = 4.0f;
};

struct OutlineParams {
    Color color = Colors::Black;
    float thickness = 2.0f;
};

struct DistortionParams {
    float distortionAmount = 0.02f;
    float timeScale = 1.0f;
};

struct PixelateParams {
    float pixelSize = 8.0f;
};

struct InvertParams {
    float strength = 1.0f;
};

struct GrayscaleParams {
    float intensity = 1.0f;
};

struct BlurParams {
    float radius = 5.0f;
};

namespace ShaderSource {

static const char* StandardVert = R"(
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
)";

static const char* StandardFrag = R"(
#version 300 es
precision highp float;
in vec2 v_texCoord;
in vec4 v_color;

uniform sampler2D u_texture;
uniform float u_opacity;

out vec4 fragColor;

void main() {
    vec4 texColor = texture(u_texture, v_texCoord);
    fragColor = texColor * v_color;
    fragColor.a *= u_opacity;
    
    if (fragColor.a < 0.01) {
        discard;
    }
}
)";

static const char* WaterFrag = R"(
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
    
    // 水波纹效果
    float wave = sin(uv.y * u_waveFrequency + u_time * u_waveSpeed) * u_waveAmplitude;
    uv.x += wave;
    
    vec4 texColor = texture(u_texture, uv);
    fragColor = texColor * v_color;
    
    if (fragColor.a < 0.01) {
        discard;
    }
}
)";

static const char* OutlineFrag = R"(
#version 300 es
precision highp float;
in vec2 v_texCoord;
in vec4 v_color;

uniform sampler2D u_texture;
uniform vec4 u_outlineColor;
uniform float u_thickness;
uniform vec2 u_textureSize;

out vec4 fragColor;

void main() {
    vec4 color = texture(u_texture, v_texCoord);
    
    // 简单的描边检测
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
    
    if (fragColor.a < 0.01) {
        discard;
    }
}
)";

static const char* DistortionFrag = R"(
#version 300 es
precision highp float;
in vec2 v_texCoord;
in vec4 v_color;

uniform sampler2D u_texture;
uniform float u_distortionAmount;
uniform float u_time;
uniform float u_timeScale;

out vec4 fragColor;

void main() {
    vec2 uv = v_texCoord;
    
    // 扭曲效果
    float t = u_time * u_timeScale;
    float dx = sin(uv.y * 10.0 + t) * u_distortionAmount;
    float dy = cos(uv.x * 10.0 + t) * u_distortionAmount;
    uv += vec2(dx, dy);
    
    vec4 texColor = texture(u_texture, uv);
    fragColor = texColor * v_color;
    
    if (fragColor.a < 0.01) {
        discard;
    }
}
)";

static const char* PixelateFrag = R"(
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
)";

static const char* InvertFrag = R"(
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
)";

static const char* GrayscaleFrag = R"(
#version 300 es
precision highp float;
in vec2 v_texCoord;
in vec4 v_color;

uniform sampler2D u_texture;
uniform float u_intensity;
uniform float u_opacity;

out vec4 fragColor;

void main() {
    vec4 texColor = texture(u_texture, v_texCoord) * v_color;

    float gray = dot(texColor.rgb, vec3(0.299, 0.587, 0.114));
    texColor.rgb = mix(texColor.rgb, vec3(gray), u_intensity);

    fragColor = texColor;
    fragColor.a *= u_opacity;

    if (fragColor.a < 0.01) {
        discard;
    }
}
)";

static const char* BlurFrag = R"(
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
)";

} // namespace ShaderSource

class ShaderPreset {
public:
    static Ptr<GLShader> Water(const WaterParams& params);
    static Ptr<GLShader> Outline(const OutlineParams& params);
    static Ptr<GLShader> Distortion(const DistortionParams& params);
    static Ptr<GLShader> Pixelate(const PixelateParams& params);
    static Ptr<GLShader> Invert(const InvertParams& params);
    static Ptr<GLShader> Grayscale(const GrayscaleParams& params);
    static Ptr<GLShader> Blur(const BlurParams& params);
    
    static Ptr<GLShader> GrayscaleOutline(const GrayscaleParams& grayParams,
                                          const OutlineParams& outlineParams);
    static Ptr<GLShader> PixelateInvert(const PixelateParams& pixParams,
                                        const InvertParams& invParams);
};

} // namespace extra2d
