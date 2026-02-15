#pragma once

#include <extra2d/core/types.h>
#include <string>

namespace extra2d {

// ============================================================================
// 平台类型枚举
// ============================================================================
enum class PlatformType {
    Auto,       
    Windows,    
    Switch,     
    Linux,      
    macOS       
};

// ============================================================================
// 平台能力结构
// ============================================================================
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

// ============================================================================
// 平台配置抽象接口
// ============================================================================
class PlatformConfig {
public:
    virtual ~PlatformConfig() = default;

    virtual PlatformType platformType() const = 0;
    virtual const char* platformName() const = 0;
    virtual const PlatformCapabilities& capabilities() const = 0;
    virtual void applyConstraints(struct AppConfig& config) const = 0;
    virtual void applyDefaults(struct AppConfig& config) const = 0;
    virtual bool validateConfig(struct AppConfig& config) const = 0;
    
    virtual int getRecommendedWidth() const = 0;
    virtual int getRecommendedHeight() const = 0;
    virtual bool isResolutionSupported(int width, int height) const = 0;
};

// ============================================================================
// 平台配置工厂函数声明
// ============================================================================
UniquePtr<PlatformConfig> createPlatformConfig(PlatformType type = PlatformType::Auto);

const char* getPlatformTypeName(PlatformType type);

} 
