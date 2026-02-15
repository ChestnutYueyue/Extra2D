#pragma once

#include <extra2d/core/math_types.h>
#include <extra2d/graphics/render_backend.h>
#include <string>

namespace extra2d {

/**
 * @file render_config.h
 * @brief 渲染模块配置
 * 
 * 定义渲染相关的配置数据结构，由 RenderModule 管理。
 */

/**
 * @brief 渲染配置数据结构
 */
struct RenderConfigData {
    BackendType backend = BackendType::OpenGL;
    int targetFPS = 60;
    bool vsync = true;
    bool tripleBuffering = false;
    int multisamples = 0;  
    bool sRGBFramebuffer = false;
    Color clearColor{0.0f, 0.0f, 0.0f, 1.0f};
    int maxTextureSize = 0;  
    int textureAnisotropy = 1;
    bool wireframeMode = false;
    bool depthTest = false;
    bool blending = true;
    bool dithering = false;
    int spriteBatchSize = 1000;
    int maxRenderTargets = 1;
    bool allowShaderHotReload = false;
    std::string shaderCachePath;

    /**
     * @brief 检查是否启用多重采样
     * @return 如果多重采样数大于0返回 true
     */
    bool isMultisampleEnabled() const { return multisamples > 0; }

    /**
     * @brief 检查是否限制帧率
     * @return 如果设置了目标帧率返回 true
     */
    bool isFPSCapped() const { return targetFPS > 0; }
};

} 
