#pragma once

#include <extra2d/core/color.h>
#include <extra2d/core/types.h>
#include <extra2d/graphics/shader_interface.h>
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

class ShaderPreset {
public:
    /**
     * @brief 创建水波纹效果着色器
     * @param params 水波纹效果参数
     * @return 配置好的着色器
     */
    static Ptr<IShader> Water(const WaterParams& params = {});

    /**
     * @brief 创建描边效果着色器
     * @param params 描边效果参数
     * @return 配置好的着色器
     */
    static Ptr<IShader> Outline(const OutlineParams& params = {});

    /**
     * @brief 创建扭曲效果着色器
     * @param params 扭曲效果参数
     * @return 配置好的着色器
     */
    static Ptr<IShader> Distortion(const DistortionParams& params = {});

    /**
     * @brief 创建像素化效果着色器
     * @param params 像素化效果参数
     * @return 配置好的着色器
     */
    static Ptr<IShader> Pixelate(const PixelateParams& params = {});

    /**
     * @brief 创建反相效果着色器
     * @param params 反相效果参数
     * @return 配置好的着色器
     */
    static Ptr<IShader> Invert(const InvertParams& params = {});

    /**
     * @brief 创建灰度效果着色器
     * @param params 灰度效果参数
     * @return 配置好的着色器
     */
    static Ptr<IShader> Grayscale(const GrayscaleParams& params = {});

    /**
     * @brief 创建模糊效果着色器
     * @param params 模糊效果参数
     * @return 配置好的着色器
     */
    static Ptr<IShader> Blur(const BlurParams& params = {});

    /**
     * @brief 创建灰度+描边组合效果着色器
     * @param grayParams 灰度效果参数
     * @param outlineParams 描边效果参数
     * @return 配置好的着色器
     */
    static Ptr<IShader> GrayscaleOutline(const GrayscaleParams& grayParams,
                                         const OutlineParams& outlineParams);

    /**
     * @brief 创建像素化+反相组合效果着色器
     * @param pixParams 像素化效果参数
     * @param invParams 反相效果参数
     * @return 配置好的着色器
     */
    static Ptr<IShader> PixelateInvert(const PixelateParams& pixParams,
                                       const InvertParams& invParams);
};

} // namespace extra2d
