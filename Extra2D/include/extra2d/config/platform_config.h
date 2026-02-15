#pragma once

#include <extra2d/core/types.h>
#include <string>

namespace extra2d {

/**
 * @file platform_config.h
 * @brief 平台配置接口
 * 
 * 平台配置只提供平台能力信息，不再直接修改应用配置。
 * 各模块通过 IModuleConfig::applyPlatformConstraints() 处理平台约束。
 */

/**
 * @brief 平台类型枚举
 */
enum class PlatformType {
    Auto,       
    Windows,    
    Switch,     
    Linux,      
    macOS       
};

/**
 * @brief 平台能力结构
 */
struct PlatformCapabilities {
    bool supportsWindowed = true;       
    bool supportsFullscreen = true;     
    bool supportsBorderless = true;     
    bool supportsCursor = true;         
    bool supportsCursorHide = true;     
    bool supportsDPIAwareness = true;   
    bool supportsVSync = true;          
    bool supportsMultiMonitor = true;   
    bool supportsClipboard = true;      
    bool supportsGamepad = true;        
    bool supportsTouch = false;         
    bool supportsKeyboard = true;       
    bool supportsMouse = true;          
    bool supportsResize = true;         
    bool supportsHighDPI = true;        
    int maxTextureSize = 16384;         
    int preferredScreenWidth = 1920;    
    int preferredScreenHeight = 1080;   
    float defaultDPI = 96.0f;           

    bool hasWindowSupport() const { return supportsWindowed || supportsFullscreen || supportsBorderless; }
    bool hasInputSupport() const { return supportsKeyboard || supportsMouse || supportsGamepad || supportsTouch; }
    bool isDesktop() const { return supportsKeyboard && supportsMouse && supportsWindowed; }
    bool isConsole() const { return !supportsWindowed && supportsGamepad; }
};

/**
 * @brief 平台配置抽象接口
 */
class PlatformConfig {
public:
    virtual ~PlatformConfig() = default;

    virtual PlatformType platformType() const = 0;
    virtual const char* platformName() const = 0;
    virtual const PlatformCapabilities& capabilities() const = 0;
    
    virtual int getRecommendedWidth() const = 0;
    virtual int getRecommendedHeight() const = 0;
    virtual bool isResolutionSupported(int width, int height) const = 0;
};

/**
 * @brief 创建平台配置实例
 * @param type 平台类型，默认为 Auto（自动检测）
 * @return 平台配置的智能指针
 */
UniquePtr<PlatformConfig> createPlatformConfig(PlatformType type = PlatformType::Auto);

/**
 * @brief 获取平台类型名称
 * @param type 平台类型枚举值
 * @return 平台名称字符串
 */
const char* getPlatformTypeName(PlatformType type);

} 
