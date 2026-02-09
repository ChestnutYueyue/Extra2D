#pragma once

#include <easy2d/core/types.h>
#include <easy2d/core/math_types.h>
#include <easy2d/core/color.h>
#include <easy2d/animation/animation_frame.h>
#include <cmath>

namespace easy2d {

// ============================================================================
// 插值结果 - 两帧之间的插值后属性
// ============================================================================
struct InterpolatedProperties {
    Vec2  position;
    Vec2  scale = Vec2::One();
    float rotation = 0.0f;
    Color color = Colors::White;
};

// ============================================================================
// 插值曲线类型
// ============================================================================
enum class InterpolationCurve : uint8 {
    Linear,     // 线性（原始系统的 uniform velocity）
    EaseIn,     // 缓入
    EaseOut,    // 缓出
    EaseInOut,  // 缓入缓出
};

// ============================================================================
// InterpolationEngine - 帧间属性插值计算（静态方法，无状态）
// 独立于 AnimationController，可复用于其他系统
// ============================================================================
class InterpolationEngine {
public:
    /// 核心插值计算：根据 t 因子 (0~1) 计算两帧之间的插值属性
    static InterpolatedProperties interpolate(
        const AnimationFrame& from,
        const AnimationFrame& to,
        float t,
        InterpolationCurve curve = InterpolationCurve::Linear)
    {
        float curvedT = applyCurve(t, curve);

        InterpolatedProperties result;
        result.position = lerpPosition(from, to, curvedT);
        result.scale    = lerpScale(from, to, curvedT);
        result.rotation = lerpRotation(from, to, curvedT);
        result.color    = lerpColor(from, to, curvedT);
        return result;
    }

    /// 位置插值
    static Vec2 lerpPosition(const AnimationFrame& from, const AnimationFrame& to, float t) {
        return Vec2::lerp(from.offset, to.offset, t);
    }

    /// 缩放插值
    static Vec2 lerpScale(const AnimationFrame& from, const AnimationFrame& to, float t) {
        Vec2 fromScale = from.getEffectiveScale();
        Vec2 toScale   = to.getEffectiveScale();
        return Vec2::lerp(fromScale, toScale, t);
    }

    /// 旋转插值
    static float lerpRotation(const AnimationFrame& from, const AnimationFrame& to, float t) {
        float fromRot = from.getEffectiveRotation();
        float toRot   = to.getEffectiveRotation();
        return math::lerp(fromRot, toRot, t);
    }

    /// 颜色插值
    static Color lerpColor(const AnimationFrame& from, const AnimationFrame& to, float t) {
        Color fromColor = from.getEffectiveColor();
        Color toColor   = to.getEffectiveColor();
        return Color::lerp(fromColor, toColor, t);
    }

    /// 应用曲线函数
    static float applyCurve(float t, InterpolationCurve curve) {
        t = math::clamp(t, 0.0f, 1.0f);

        switch (curve) {
        case InterpolationCurve::Linear:
            return t;

        case InterpolationCurve::EaseIn:
            return t * t;

        case InterpolationCurve::EaseOut:
            return t * (2.0f - t);

        case InterpolationCurve::EaseInOut:
            if (t < 0.5f)
                return 2.0f * t * t;
            else
                return -1.0f + (4.0f - 2.0f * t) * t;
        }

        return t;
    }
};

} // namespace easy2d
