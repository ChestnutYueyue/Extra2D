#pragma once

#include <easy2d/animation/sprite_frame.h>
#include <easy2d/animation/frame_property.h>
#include <string>
#include <vector>
#include <array>
#include <cstdint>

namespace easy2d {

// ============================================================================
// AnimationFrame - 单帧数据
// 引用 SpriteFrame 而非直接持有纹理（借鉴 Cocos 模式）
// 通过 FramePropertySet 支持不固定数据（ANI Flag 系统增强版）
// ============================================================================
struct AnimationFrame {
    // ------ 核心数据（固定部分）------
    Ptr<SpriteFrame> spriteFrame;       // 精灵帧引用（Cocos 模式）
    std::string      texturePath;       // 原始图片路径（用于解析时定位资源）
    int              textureIndex = 0;  // 精灵图集索引
    Vec2             offset;            // 位置偏移
    float            delay = 100.0f;    // 帧延迟（毫秒）

    // ------ 碰撞盒数据（DNF ANI 格式）------
    std::vector<std::array<int32_t, 6>> damageBoxes;  // 伤害碰撞盒
    std::vector<std::array<int32_t, 6>> attackBoxes;   // 攻击碰撞盒

    // ------ 不固定数据（属性集合）------
    FramePropertySet properties;        // 类型安全的 Flag 系统

    // ------ 便捷方法 ------
    bool hasTexture() const {
        return spriteFrame != nullptr && spriteFrame->isValid();
    }

    bool hasInterpolation() const {
        return properties.getOr<bool>(FramePropertyKey::Interpolation, false);
    }

    bool hasKeyframeCallback() const {
        return properties.has(FramePropertyKey::SetFlag);
    }

    int getKeyframeIndex() const {
        return properties.getOr<int>(FramePropertyKey::SetFlag, -1);
    }

    Vec2 getEffectiveScale() const {
        return properties.getOr<Vec2>(FramePropertyKey::ImageRate, Vec2::One());
    }

    float getEffectiveRotation() const {
        return properties.getOr<float>(FramePropertyKey::ImageRotate, 0.0f);
    }

    Color getEffectiveColor() const {
        return properties.getOr<Color>(FramePropertyKey::ColorTint, Colors::White);
    }
};

} // namespace easy2d
