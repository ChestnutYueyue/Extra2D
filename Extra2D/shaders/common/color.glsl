// ============================================
// Common Color Functions
// ============================================

#ifndef E2D_COLOR_GLSL
#define E2D_COLOR_GLSL

/**
 * @brief RGB转灰度
 * @param color RGB颜色
 * @return 灰度值
 */
float rgbToGrayscale(vec3 color) {
    return dot(color, vec3(0.299, 0.587, 0.114));
}

/**
 * @brief RGB转HSV
 * @param c RGB颜色
 * @return HSV颜色
 */
vec3 rgbToHsv(vec3 c) {
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));
    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

/**
 * @brief HSV转RGB
 * @param c HSV颜色
 * @return RGB颜色
 */
vec3 hsvToRgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

/**
 * @brief 调整亮度
 * @param color 原始颜色
 * @param amount 亮度调整量
 * @return 调整后的颜色
 */
vec3 adjustBrightness(vec3 color, float amount) {
    return color + amount;
}

/**
 * @brief 调整对比度
 * @param color 原始颜色
 * @param amount 对比度调整量
 * @return 调整后的颜色
 */
vec3 adjustContrast(vec3 color, float amount) {
    return (color - 0.5) * amount + 0.5;
}

/**
 * @brief 调整饱和度
 * @param color 原始颜色
 * @param amount 饱和度调整量
 * @return 调整后的颜色
 */
vec3 adjustSaturation(vec3 color, float amount) {
    float gray = rgbToGrayscale(color);
    return mix(vec3(gray), color, amount);
}

/**
 * @brief 颜色混合（正片叠底）
 * @param a 底色
 * @param b 混合色
 * @return 混合结果
 */
vec3 blendMultiply(vec3 a, vec3 b) {
    return a * b;
}

/**
 * @brief 颜色混合（滤色）
 * @param a 底色
 * @param b 混合色
 * @return 混合结果
 */
vec3 blendScreen(vec3 a, vec3 b) {
    return 1.0 - (1.0 - a) * (1.0 - b);
}

/**
 * @brief 颜色混合（叠加）
 * @param a 底色
 * @param b 混合色
 * @return 混合结果
 */
vec3 blendOverlay(vec3 a, vec3 b) {
    return mix(
        2.0 * a * b,
        1.0 - 2.0 * (1.0 - a) * (1.0 - b),
        step(0.5, a)
    );
}

/**
 * @brief 颜色调色
 * @param color 原始颜色
 * @param tintColor 色调颜色
 * @param amount 色调强度
 * @return 调色结果
 */
vec3 tint(vec3 color, vec3 tintColor, float amount) {
    return mix(color, tintColor, amount);
}

/**
 * @brief 预乘Alpha
 * @param color RGBA颜色
 * @return 预乘后的RGB颜色
 */
vec3 premultiplyAlpha(vec4 color) {
    return color.rgb * color.a;
}

/**
 * @brief 取消预乘Alpha
 * @param color RGB颜色
 * @param alpha Alpha值
 * @return 未预乘的RGB颜色
 */
vec3 unpremultiplyAlpha(vec3 color, float alpha) {
    return alpha > 0.0 ? color / alpha : color;
}

#endif // E2D_COLOR_GLSL
