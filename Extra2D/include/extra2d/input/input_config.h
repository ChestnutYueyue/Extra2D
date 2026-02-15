#pragma once

#include <string>

namespace extra2d {

/**
 * @file input_config.h
 * @brief 输入模块配置
 * 
 * 定义输入相关的配置数据结构，由 InputModule 管理。
 */

/**
 * @brief 输入配置数据结构
 */
struct InputConfigData {
    bool enabled = true;
    bool rawMouseInput = false;
    float mouseSensitivity = 1.0f;
    bool invertMouseY = false;
    bool invertMouseX = false;
    float deadzone = 0.15f;  
    float triggerThreshold = 0.5f;
    bool enableVibration = true;
    int maxGamepads = 4;
    bool autoConnectGamepads = true;
    std::string gamepadMappingFile;

    /**
     * @brief 验证死区值是否有效
     * @return 如果死区值在0-1范围内返回 true
     */
    bool isDeadzoneValid() const { return deadzone >= 0.0f && deadzone <= 1.0f; }
};

} 
