#include <extra2d/graphics/shader_preset.h>
#include <extra2d/utils/logger.h>

namespace extra2d {

// ============================================================================
// ShaderPreset实现
// ============================================================================

/**
 * @brief 创建水波纹效果着色器
 *
 * 创建并配置一个水波纹效果的GLShader对象，包含波动速度、振幅和频率参数
 *
 * @param params 水波纹效果参数，包含waveSpeed、waveAmplitude和waveFrequency
 * @return 成功返回配置好的着色器智能指针，失败返回nullptr
 */
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

/**
 * @brief 创建描边效果着色器
 *
 * 创建并配置一个描边效果的GLShader对象，包含描边颜色和厚度参数
 *
 * @param params 描边效果参数，包含color和thickness
 * @return 成功返回配置好的着色器智能指针，失败返回nullptr
 */
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

/**
 * @brief 创建扭曲效果着色器
 *
 * 创建并配置一个扭曲效果的GLShader对象，包含扭曲强度和时间缩放参数
 *
 * @param params 扭曲效果参数，包含distortionAmount和timeScale
 * @return 成功返回配置好的着色器智能指针，失败返回nullptr
 */
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

/**
 * @brief 创建像素化效果着色器
 *
 * 创建并配置一个像素化效果的GLShader对象，包含像素大小参数
 *
 * @param params 像素化效果参数，包含pixelSize
 * @return 成功返回配置好的着色器智能指针，失败返回nullptr
 */
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

/**
 * @brief 创建反相效果着色器
 *
 * 创建并配置一个颜色反相效果的GLShader对象，包含反相强度参数
 *
 * @param params 反相效果参数，包含strength
 * @return 成功返回配置好的着色器智能指针，失败返回nullptr
 */
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

/**
 * @brief 创建灰度效果着色器
 *
 * 创建并配置一个灰度效果的GLShader对象，包含灰度强度参数
 *
 * @param params 灰度效果参数，包含intensity
 * @return 成功返回配置好的着色器智能指针，失败返回nullptr
 */
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

/**
 * @brief 创建模糊效果着色器
 *
 * 创建并配置一个模糊效果的GLShader对象，包含模糊半径参数
 *
 * @param params 模糊效果参数，包含radius
 * @return 成功返回配置好的着色器智能指针，失败返回nullptr
 */
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

/**
 * @brief 创建灰度+描边组合效果着色器
 *
 * 创建并配置一个同时应用灰度和描边效果的GLShader对象
 *
 * @param grayParams 灰度效果参数，包含intensity
 * @param outlineParams 描边效果参数，包含color和thickness
 * @return 成功返回配置好的着色器智能指针，失败返回nullptr
 */
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

/**
 * @brief 创建像素化+反相组合效果着色器
 *
 * 创建并配置一个同时应用像素化和反相效果的GLShader对象
 *
 * @param pixParams 像素化效果参数，包含pixelSize
 * @param invParams 反相效果参数，包含strength
 * @return 成功返回配置好的着色器智能指针，失败返回nullptr
 */
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
