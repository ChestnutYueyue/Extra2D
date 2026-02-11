#include <extra2d/graphics/shader_preset.h>
#include <extra2d/utils/logger.h>

namespace extra2d {

// ============================================================================
// ShaderPreset实现
// ============================================================================

Ptr<GLShader> ShaderPreset::Water(const WaterParams &params) {
  auto shader = std::make_shared<GLShader>();

  if (!shader->compileFromSource(ShaderSource::StandardVert,
                                 ShaderSource::WaterFrag)) {
    E2D_ERROR("编译水波纹Shader失败");
    return nullptr;
  }

  // 设置默认参数
  shader->setFloat("u_waveSpeed", params.waveSpeed);
  shader->setFloat("u_waveAmplitude", params.waveAmplitude);
  shader->setFloat("u_waveFrequency", params.waveFrequency);

  E2D_INFO("创建水波纹Shader预设");
  return shader;
}

Ptr<GLShader> ShaderPreset::Outline(const OutlineParams &params) {
  auto shader = std::make_shared<GLShader>();

  if (!shader->compileFromSource(ShaderSource::StandardVert,
                                 ShaderSource::OutlineFrag)) {
    E2D_ERROR("编译描边Shader失败");
    return nullptr;
  }

  // 设置默认参数
  shader->setVec4("u_outlineColor", glm::vec4(params.color.r, params.color.g,
                                              params.color.b, params.color.a));
  shader->setFloat("u_thickness", params.thickness);

  E2D_INFO("创建描边Shader预设");
  return shader;
}

Ptr<GLShader> ShaderPreset::Distortion(const DistortionParams &params) {
  auto shader = std::make_shared<GLShader>();

  if (!shader->compileFromSource(ShaderSource::StandardVert,
                                 ShaderSource::DistortionFrag)) {
    E2D_ERROR("编译扭曲Shader失败");
    return nullptr;
  }

  // 设置默认参数
  shader->setFloat("u_distortionAmount", params.distortionAmount);
  shader->setFloat("u_timeScale", params.timeScale);

  E2D_INFO("创建扭曲Shader预设");
  return shader;
}

Ptr<GLShader> ShaderPreset::Pixelate(const PixelateParams &params) {
  auto shader = std::make_shared<GLShader>();

  if (!shader->compileFromSource(ShaderSource::StandardVert,
                                 ShaderSource::PixelateFrag)) {
    E2D_ERROR("编译像素化Shader失败");
    return nullptr;
  }

  // 设置默认参数
  shader->setFloat("u_pixelSize", params.pixelSize);

  E2D_INFO("创建像素化Shader预设");
  return shader;
}

Ptr<GLShader> ShaderPreset::Invert(const InvertParams &params) {
  auto shader = std::make_shared<GLShader>();

  if (!shader->compileFromSource(ShaderSource::StandardVert,
                                 ShaderSource::InvertFrag)) {
    E2D_ERROR("编译反相Shader失败");
    return nullptr;
  }

  // 设置默认参数
  shader->setFloat("u_strength", params.strength);

  E2D_INFO("创建反相Shader预设");
  return shader;
}

Ptr<GLShader> ShaderPreset::Grayscale(const GrayscaleParams &params) {
  auto shader = std::make_shared<GLShader>();

  if (!shader->compileFromSource(ShaderSource::StandardVert,
                                 ShaderSource::GrayscaleFrag)) {
    E2D_ERROR("编译灰度Shader失败");
    return nullptr;
  }

  // 设置默认参数
  shader->setFloat("u_intensity", params.intensity);

  E2D_INFO("创建灰度Shader预设");
  return shader;
}

Ptr<GLShader> ShaderPreset::Blur(const BlurParams &params) {
  auto shader = std::make_shared<GLShader>();

  if (!shader->compileFromSource(ShaderSource::StandardVert,
                                 ShaderSource::BlurFrag)) {
    E2D_ERROR("编译模糊Shader失败");
    return nullptr;
  }

  // 设置默认参数
  shader->setFloat("u_radius", params.radius);

  E2D_INFO("创建模糊Shader预设");
  return shader;
}

Ptr<GLShader>
ShaderPreset::GrayscaleOutline(const GrayscaleParams &grayParams,
                               const OutlineParams &outlineParams) {
  // 创建组合效果的片段着色器 (GLES 3.2)
  const char *combinedFrag = R"(
#version 300 es
precision highp float;
in vec2 v_texCoord;

uniform sampler2D u_texture;
uniform float u_grayIntensity;
uniform vec4 u_outlineColor;
uniform float u_thickness;
uniform vec2 u_textureSize;

out vec4 fragColor;

void main() {
    vec4 color = texture(u_texture, v_texCoord);
    
    // 灰度效果
    float gray = dot(color.rgb, vec3(0.299, 0.587, 0.114));
    color.rgb = mix(color.rgb, vec3(gray), u_grayIntensity);
    
    // 描边效果
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
}
)";

  auto shader = std::make_shared<GLShader>();

  if (!shader->compileFromSource(ShaderSource::StandardVert, combinedFrag)) {
    E2D_ERROR("编译灰度+描边Shader失败");
    return nullptr;
  }

  // 设置默认参数
  shader->setFloat("u_grayIntensity", grayParams.intensity);
  shader->setVec4("u_outlineColor",
                  glm::vec4(outlineParams.color.r, outlineParams.color.g,
                            outlineParams.color.b, outlineParams.color.a));
  shader->setFloat("u_thickness", outlineParams.thickness);

  E2D_INFO("创建灰度+描边组合Shader预设");
  return shader;
}

Ptr<GLShader> ShaderPreset::PixelateInvert(const PixelateParams &pixParams,
                                           const InvertParams &invParams) {
  // 创建组合效果的片段着色器 (GLES 3.2)
  const char *combinedFrag = R"(
#version 300 es
precision highp float;
in vec2 v_texCoord;

uniform sampler2D u_texture;
uniform float u_pixelSize;
uniform vec2 u_textureSize;
uniform float u_invertStrength;

out vec4 fragColor;

void main() {
    // 像素化
    vec2 pixel = u_pixelSize / u_textureSize;
    vec2 uv = floor(v_texCoord / pixel) * pixel + pixel * 0.5;
    
    vec4 color = texture(u_texture, uv);
    
    // 反相
    vec3 inverted = 1.0 - color.rgb;
    color.rgb = mix(color.rgb, inverted, u_invertStrength);
    
    fragColor = color;
}
)";

  auto shader = std::make_shared<GLShader>();

  if (!shader->compileFromSource(ShaderSource::StandardVert, combinedFrag)) {
    E2D_ERROR("编译像素化+反相Shader失败");
    return nullptr;
  }

  // 设置默认参数
  shader->setFloat("u_pixelSize", pixParams.pixelSize);
  shader->setFloat("u_invertStrength", invParams.strength);

  E2D_INFO("创建像素化+反相组合Shader预设");
  return shader;
}

} // namespace extra2d
