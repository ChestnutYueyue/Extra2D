#pragma once

#include <extra2d/core/types.h>
#include <extra2d/core/math_types.h>

namespace extra2d {

// ============================================================================
// 像素格式枚举
// ============================================================================
enum class PixelFormat {
    R8,         // 单通道灰度
    RG8,        // 双通道
    RGB8,       // RGB 24位
    RGBA8,      // RGBA 32位（默认）
    RGB16F,     // RGB 半精度浮点
    RGBA16F,    // RGBA 半精度浮点
    RGB32F,     // RGB 全精度浮点
    RGBA32F,    // RGBA 全精度浮点
    Depth16,    // 16位深度
    Depth24,    // 24位深度
    Depth32F,   // 32位浮点深度
    Depth24Stencil8,  // 24位深度 + 8位模板

    // 压缩纹理格式
    ETC2_RGB8,   // ETC2 RGB 压缩
    ETC2_RGBA8,  // ETC2 RGBA 压缩
    ASTC_4x4,    // ASTC 4x4 压缩
    ASTC_6x6,    // ASTC 6x6 压缩
    ASTC_8x8     // ASTC 8x8 压缩
};

// ============================================================================
// 纹理接口
// ============================================================================
class Texture {
public:
    virtual ~Texture() = default;

    // 获取尺寸
    virtual int getWidth() const = 0;
    virtual int getHeight() const = 0;
    virtual Size getSize() const = 0;
    
    // 获取通道数
    virtual int getChannels() const = 0;
    
    // 获取像素格式
    virtual PixelFormat getFormat() const = 0;
    
    // 获取原始句柄（用于底层渲染）
    virtual void* getNativeHandle() const = 0;
    
    // 是否有效
    virtual bool isValid() const = 0;
    
    // 设置过滤模式
    virtual void setFilter(bool linear) = 0;
    
    // 设置环绕模式
    virtual void setWrap(bool repeat) = 0;
};

} // namespace extra2d
