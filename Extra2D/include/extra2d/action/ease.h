#pragma once

namespace extra2d {

/**
 * @brief 缓动函数类型
 */
using EaseFunction = float (*)(float);

// ============================================================================
// 线性缓动
// ============================================================================

/**
 * @brief 线性缓动（无缓动）
 * @param t 归一化时间 [0, 1]
 * @return 缓动后的值
 */
float easeLinear(float t);

// ============================================================================
// 二次缓动 (Quad)
// ============================================================================

float easeInQuad(float t);
float easeOutQuad(float t);
float easeInOutQuad(float t);

// ============================================================================
// 三次缓动 (Cubic)
// ============================================================================

float easeInCubic(float t);
float easeOutCubic(float t);
float easeInOutCubic(float t);

// ============================================================================
// 四次缓动 (Quart)
// ============================================================================

float easeInQuart(float t);
float easeOutQuart(float t);
float easeInOutQuart(float t);

// ============================================================================
// 五次缓动 (Quint)
// ============================================================================

float easeInQuint(float t);
float easeOutQuint(float t);
float easeInOutQuint(float t);

// ============================================================================
// 正弦缓动 (Sine)
// ============================================================================

float easeInSine(float t);
float easeOutSine(float t);
float easeInOutSine(float t);

// ============================================================================
// 指数缓动 (Exponential)
// ============================================================================

float easeInExpo(float t);
float easeOutExpo(float t);
float easeInOutExpo(float t);

// ============================================================================
// 圆形缓动 (Circular)
// ============================================================================

float easeInCirc(float t);
float easeOutCirc(float t);
float easeInOutCirc(float t);

// ============================================================================
// 回震缓动 (Back)
// ============================================================================

float easeInBack(float t);
float easeOutBack(float t);
float easeInOutBack(float t);

// ============================================================================
// 弹性缓动 (Elastic)
// ============================================================================

float easeInElastic(float t);
float easeOutElastic(float t);
float easeInOutElastic(float t);

// ============================================================================
// 弹跳缓动 (Bounce)
// ============================================================================

float easeInBounce(float t);
float easeOutBounce(float t);
float easeInOutBounce(float t);

} // namespace extra2d
