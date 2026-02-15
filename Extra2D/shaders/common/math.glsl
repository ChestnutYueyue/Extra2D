// ============================================
// Common Math Functions
// ============================================

#ifndef E2D_MATH_GLSL
#define E2D_MATH_GLSL

const float PI = 3.14159265359;
const float E = 2.71828182846;

/**
 * @brief 角度转弧度
 * @param deg 角度值
 * @return 弧度值
 */
float degToRad(float deg) {
    return deg * PI / 180.0;
}

/**
 * @brief 弧度转角度
 * @param rad 弧度值
 * @return 角度值
 */
float radToDeg(float rad) {
    return rad * 180.0 / PI;
}

/**
 * @brief 线性插值
 * @param a 起始值
 * @param b 结束值
 * @param t 插值因子 [0, 1]
 * @return 插值结果
 */
float lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

/**
 * @brief 平滑插值
 * @param edge0 下边界
 * @param edge1 上边界
 * @param x 输入值
 * @return 平滑插值结果
 */
float smoothStep(float edge0, float edge1, float x) {
    float t = clamp((x - edge0) / (edge1 - edge0), 0.0, 1.0);
    return t * t * (3.0 - 2.0 * t);
}

/**
 * @brief 2D向量线性插值
 */
vec2 lerpVec2(vec2 a, vec2 b, float t) {
    return a + (b - a) * t;
}

/**
 * @brief 计算两点之间的距离
 */
float distance2D(vec2 a, vec2 b) {
    return length(b - a);
}

/**
 * @brief 计算两点之间的距离平方
 */
float distance2DSquared(vec2 a, vec2 b) {
    vec2 diff = b - a;
    return dot(diff, diff);
}

/**
 * @brief 将值限制在范围内
 */
float clamp01(float x) {
    return clamp(x, 0.0, 1.0);
}

/**
 * @brief 重复平铺
 */
float repeat(float x, float period) {
    return mod(x, period);
}

/**
 * @brief 镜像重复
 */
float mirrorRepeat(float x, float period) {
    float m = mod(x, period * 2.0);
    return m > period ? period * 2.0 - m : m;
}

#endif // E2D_MATH_GLSL
