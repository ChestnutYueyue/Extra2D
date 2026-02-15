#pragma once

#include <extra2d/core/math_types.h>
#include <string>

namespace extra2d {

/**
 * @file window_config.h
 * @brief 窗口模块配置
 * 
 * 定义窗口相关的配置数据结构，由 WindowModule 管理。
 */

/**
 * @brief 窗口模式枚举
 */
enum class WindowMode {
    Windowed,       
    Fullscreen,     
    Borderless      
};

/**
 * @brief 窗口配置数据结构
 */
struct WindowConfigData {
    std::string title = "Extra2D Application";
    int width = 1280;
    int height = 720;
    int minWidth = 320;
    int minHeight = 240;
    int maxWidth = 0;  
    int maxHeight = 0;  
    WindowMode mode = WindowMode::Windowed;
    bool resizable = true;
    bool borderless = false;
    bool alwaysOnTop = false;
    bool centered = true;
    int posX = -1;     
    int posY = -1;     
    bool hideOnClose = false;
    bool minimizeOnClose = true;
    float opacity = 1.0f;
    bool transparentFramebuffer = false;
    bool highDPI = true;
    float contentScale = 1.0f;
    bool vsync = true;
    int multisamples = 0;
    bool visible = true;
    bool decorated = true;

    /**
     * @brief 检查窗口尺寸是否有效
     * @return 如果宽高都大于0返回 true
     */
    bool isSizeValid() const { return width > 0 && height > 0; }

    /**
     * @brief 检查是否设置了窗口位置
     * @return 如果设置了有效位置返回 true
     */
    bool hasPosition() const { return posX >= 0 && posY >= 0; }

    /**
     * @brief 获取窗口宽高比
     * @return 宽高比值
     */
    float aspectRatio() const { return static_cast<float>(width) / static_cast<float>(height); }

    /**
     * @brief 检查是否为全屏模式
     * @return 如果是全屏模式返回 true
     */
    bool isFullscreen() const { return mode == WindowMode::Fullscreen; }

    /**
     * @brief 检查是否为无边框模式
     * @return 如果是无边框模式返回 true
     */
    bool isBorderless() const { return mode == WindowMode::Borderless || borderless; }
};

} 
