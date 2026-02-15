#include <extra2d/graphics/shader_manager.h>
#include <extra2d/graphics/shader_preset.h>
#include <extra2d/utils/logger.h>

namespace extra2d {

/**
 * @brief 创建水波纹效果着色器
 * @param params 水波纹效果参数
 * @return 配置好的着色器
 */
Ptr<IShader> ShaderPreset::Water(const WaterParams &params) {
  Ptr<IShader> shader = ShaderManager::getInstance().get("water");
  if (!shader) {
    E2D_LOG_ERROR("Failed to get water shader");
    return nullptr;
  }

  shader->setFloat("u_waveSpeed", params.waveSpeed);
  shader->setFloat("u_waveAmplitude", params.waveAmplitude);
  shader->setFloat("u_waveFrequency", params.waveFrequency);

  return shader;
}

/**
 * @brief 创建描边效果着色器
 * @param params 描边效果参数
 * @return 配置好的着色器
 */
Ptr<IShader> ShaderPreset::Outline(const OutlineParams &params) {
  Ptr<IShader> shader = ShaderManager::getInstance().get("outline");
  if (!shader) {
    E2D_LOG_ERROR("Failed to get outline shader");
    return nullptr;
  }

  shader->setVec4("u_outlineColor", glm::vec4(params.color.r, params.color.g,
                                              params.color.b, params.color.a));
  shader->setFloat("u_thickness", params.thickness);

  return shader;
}

/**
 * @brief 创建扭曲效果着色器
 * @param params 扭曲效果参数
 * @return 配置好的着色器
 */
Ptr<IShader> ShaderPreset::Distortion(const DistortionParams &params) {
  Ptr<IShader> shader = ShaderManager::getInstance().get("distortion");
  if (!shader) {
    E2D_LOG_ERROR("Failed to get distortion shader");
    return nullptr;
  }

  shader->setFloat("u_distortionAmount", params.distortionAmount);
  shader->setFloat("u_timeScale", params.timeScale);

  return shader;
}

/**
 * @brief 创建像素化效果着色器
 * @param params 像素化效果参数
 * @return 配置好的着色器
 */
Ptr<IShader> ShaderPreset::Pixelate(const PixelateParams &params) {
  Ptr<IShader> shader = ShaderManager::getInstance().get("pixelate");
  if (!shader) {
    E2D_LOG_ERROR("Failed to get pixelate shader");
    return nullptr;
  }

  shader->setFloat("u_pixelSize", params.pixelSize);

  return shader;
}

/**
 * @brief 创建反相效果着色器
 * @param params 反相效果参数
 * @return 配置好的着色器
 */
Ptr<IShader> ShaderPreset::Invert(const InvertParams &params) {
  Ptr<IShader> shader = ShaderManager::getInstance().get("invert");
  if (!shader) {
    E2D_LOG_ERROR("Failed to get invert shader");
    return nullptr;
  }

  shader->setFloat("u_strength", params.strength);

  return shader;
}

/**
 * @brief 创建灰度效果着色器
 * @param params 灰度效果参数
 * @return 配置好的着色器
 */
Ptr<IShader> ShaderPreset::Grayscale(const GrayscaleParams &params) {
  Ptr<IShader> shader = ShaderManager::getInstance().get("grayscale");
  if (!shader) {
    E2D_LOG_ERROR("Failed to get grayscale shader");
    return nullptr;
  }

  shader->setFloat("u_intensity", params.intensity);

  return shader;
}

/**
 * @brief 创建模糊效果着色器
 * @param params 模糊效果参数
 * @return 配置好的着色器
 */
Ptr<IShader> ShaderPreset::Blur(const BlurParams &params) {
  Ptr<IShader> shader = ShaderManager::getInstance().get("blur");
  if (!shader) {
    E2D_LOG_ERROR("Failed to get blur shader");
    return nullptr;
  }

  shader->setFloat("u_radius", params.radius);

  return shader;
}

/**
 * @brief 创建灰度+描边组合效果着色器
 * @param grayParams 灰度效果参数
 * @param outlineParams 描边效果参数
 * @return 配置好的着色器
 */
Ptr<IShader>
ShaderPreset::GrayscaleOutline(const GrayscaleParams &grayParams,
                               const OutlineParams &outlineParams) {
  std::string shaderDir = ShaderManager::getInstance().getShaderDir();
  std::string shaderPath = shaderDir + "effects/grayscale_outline.shader";

  Ptr<IShader> shader =
      ShaderManager::getInstance().loadFromCombinedFile(shaderPath);
  if (!shader) {
    E2D_LOG_ERROR("Failed to load grayscale_outline shader from: {}",
                  shaderPath);
    return nullptr;
  }

  shader->setFloat("u_grayIntensity", grayParams.intensity);
  shader->setVec4("u_outlineColor",
                  glm::vec4(outlineParams.color.r, outlineParams.color.g,
                            outlineParams.color.b, outlineParams.color.a));
  shader->setFloat("u_thickness", outlineParams.thickness);

  return shader;
}

/**
 * @brief 创建像素化+反相组合效果着色器
 * @param pixParams 像素化效果参数
 * @param invParams 反相效果参数
 * @return 配置好的着色器
 */
Ptr<IShader> ShaderPreset::PixelateInvert(const PixelateParams &pixParams,
                                          const InvertParams &invParams) {
  std::string shaderDir = ShaderManager::getInstance().getShaderDir();
  std::string shaderPath = shaderDir + "effects/pixelate_invert.shader";

  Ptr<IShader> shader =
      ShaderManager::getInstance().loadFromCombinedFile(shaderPath);
  if (!shader) {
    E2D_LOG_ERROR("Failed to load pixelate_invert shader from: {}", shaderPath);
    return nullptr;
  }

  shader->setFloat("u_pixelSize", pixParams.pixelSize);
  shader->setFloat("u_invertStrength", invParams.strength);

  return shader;
}

} // namespace extra2d
