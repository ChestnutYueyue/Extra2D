#pragma once

#include <algorithm>
#include <cmath>
#include <extra2d/core/types.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>

namespace extra2d {

// ---------------------------------------------------------------------------
// 常量
// ---------------------------------------------------------------------------
constexpr float PI_F = 3.14159265358979323846f;
constexpr float DEG_TO_RAD = PI_F / 180.0f;
constexpr float RAD_TO_DEG = 180.0f / PI_F;

// ---------------------------------------------------------------------------
// 2D 向量
// ---------------------------------------------------------------------------
struct Vec2 {
  float x = 0.0f;
  float y = 0.0f;

  constexpr Vec2() = default;
  constexpr Vec2(float x, float y) : x(x), y(y) {}
  explicit Vec2(const glm::vec2 &v) : x(v.x), y(v.y) {}

  glm::vec2 toGlm() const { return {x, y}; }
  static Vec2 fromGlm(const glm::vec2 &v) { return {v.x, v.y}; }

  // 基础运算
  Vec2 operator+(const Vec2 &v) const { return {x + v.x, y + v.y}; }
  Vec2 operator-(const Vec2 &v) const { return {x - v.x, y - v.y}; }
  Vec2 operator*(float s) const { return {x * s, y * s}; }
  Vec2 operator/(float s) const { return {x / s, y / s}; }
  Vec2 operator-() const { return {-x, -y}; }

  Vec2 &operator+=(const Vec2 &v) {
    x += v.x;
    y += v.y;
    return *this;
  }
  Vec2 &operator-=(const Vec2 &v) {
    x -= v.x;
    y -= v.y;
    return *this;
  }
  Vec2 &operator*=(float s) {
    x *= s;
    y *= s;
    return *this;
  }
  Vec2 &operator/=(float s) {
    x /= s;
    y /= s;
    return *this;
  }

  bool operator==(const Vec2 &v) const { return x == v.x && y == v.y; }
  bool operator!=(const Vec2 &v) const { return !(*this == v); }

  // 向量运算
  float length() const { return std::sqrt(x * x + y * y); }
  float lengthSquared() const { return x * x + y * y; }

  Vec2 normalized() const {
    float len = length();
    if (len > 0.0f)
      return {x / len, y / len};
    return {0.0f, 0.0f};
  }

  float dot(const Vec2 &v) const { return x * v.x + y * v.y; }
  float cross(const Vec2 &v) const { return x * v.y - y * v.x; }

  float distance(const Vec2 &v) const { return (*this - v).length(); }
  float angle() const { return std::atan2(y, x) * RAD_TO_DEG; }

  static Vec2 lerp(const Vec2 &a, const Vec2 &b, float t) {
    return a + (b - a) * t;
  }

  static constexpr Vec2 Zero() { return {0.0f, 0.0f}; }
  static constexpr Vec2 One() { return {1.0f, 1.0f}; }
  static constexpr Vec2 UnitX() { return {1.0f, 0.0f}; }
  static constexpr Vec2 UnitY() { return {0.0f, 1.0f}; }
};

inline Vec2 operator*(float s, const Vec2 &v) { return v * s; }

using Point = Vec2;

// ---------------------------------------------------------------------------
// 3D 向量 (用于3D动作)
// ---------------------------------------------------------------------------
struct Vec3 {
  float x = 0.0f;
  float y = 0.0f;
  float z = 0.0f;

  constexpr Vec3() = default;
  constexpr Vec3(float x, float y, float z) : x(x), y(y), z(z) {}
  explicit Vec3(const glm::vec3 &v) : x(v.x), y(v.y), z(v.z) {}

  glm::vec3 toGlm() const { return {x, y, z}; }
  static Vec3 fromGlm(const glm::vec3 &v) { return {v.x, v.y, v.z}; }

  Vec3 operator+(const Vec3 &v) const { return {x + v.x, y + v.y, z + v.z}; }
  Vec3 operator-(const Vec3 &v) const { return {x - v.x, y - v.y, z - v.z}; }
  Vec3 operator*(float s) const { return {x * s, y * s, z * s}; }
  Vec3 operator/(float s) const { return {x / s, y / s, z / s}; }
  Vec3 operator-() const { return {-x, -y, -z}; }

  Vec3 &operator+=(const Vec3 &v) {
    x += v.x;
    y += v.y;
    z += v.z;
    return *this;
  }
  Vec3 &operator-=(const Vec3 &v) {
    x -= v.x;
    y -= v.y;
    z -= v.z;
    return *this;
  }
  Vec3 &operator*=(float s) {
    x *= s;
    y *= s;
    z *= s;
    return *this;
  }
  Vec3 &operator/=(float s) {
    x /= s;
    y /= s;
    z /= s;
    return *this;
  }

  bool operator==(const Vec3 &v) const {
    return x == v.x && y == v.y && z == v.z;
  }
  bool operator!=(const Vec3 &v) const { return !(*this == v); }

  float length() const { return std::sqrt(x * x + y * y + z * z); }
  float lengthSquared() const { return x * x + y * y + z * z; }

  Vec3 normalized() const {
    float len = length();
    if (len > 0.0f)
      return {x / len, y / len, z / len};
    return {0.0f, 0.0f, 0.0f};
  }

  float dot(const Vec3 &v) const { return x * v.x + y * v.y + z * v.z; }

  static Vec3 lerp(const Vec3 &a, const Vec3 &b, float t) {
    return a + (b - a) * t;
  }

  static constexpr Vec3 Zero() { return {0.0f, 0.0f, 0.0f}; }
  static constexpr Vec3 One() { return {1.0f, 1.0f, 1.0f}; }
};

inline Vec3 operator*(float s, const Vec3 &v) { return v * s; }

// ---------------------------------------------------------------------------
// 2D 尺寸
// ---------------------------------------------------------------------------
struct Size {
  float width = 0.0f;
  float height = 0.0f;

  constexpr Size() = default;
  constexpr Size(float w, float h) : width(w), height(h) {}

  bool operator==(const Size &s) const {
    return width == s.width && height == s.height;
  }
  bool operator!=(const Size &s) const { return !(*this == s); }

  float area() const { return width * height; }
  bool empty() const { return width <= 0.0f || height <= 0.0f; }

  static constexpr Size Zero() { return {0.0f, 0.0f}; }
};

// ---------------------------------------------------------------------------
// 2D 矩形
// ---------------------------------------------------------------------------
struct Rect {
  Point origin;
  Size size;

  constexpr Rect() = default;
  constexpr Rect(float x, float y, float w, float h)
      : origin(x, y), size(w, h) {}
  constexpr Rect(const Point &o, const Size &s) : origin(o), size(s) {}

  float left() const { return origin.x; }
  float top() const { return origin.y; }
  float right() const { return origin.x + size.width; }
  float bottom() const { return origin.y + size.height; }
  float width() const { return size.width; }
  float height() const { return size.height; }
  Point center() const {
    return {origin.x + size.width * 0.5f, origin.y + size.height * 0.5f};
  }

  bool empty() const { return size.empty(); }

  bool containsPoint(const Point &p) const {
    return p.x >= left() && p.x <= right() && p.y >= top() && p.y <= bottom();
  }

  bool contains(const Rect &r) const {
    return r.left() >= left() && r.right() <= right() && r.top() >= top() &&
           r.bottom() <= bottom();
  }

  bool intersects(const Rect &r) const {
    return !(left() > r.right() || right() < r.left() || top() > r.bottom() ||
             bottom() < r.top());
  }

  Rect intersection(const Rect &r) const {
    float l = std::max(left(), r.left());
    float t = std::max(top(), r.top());
    float ri = std::min(right(), r.right());
    float b = std::min(bottom(), r.bottom());
    if (l < ri && t < b)
      return {l, t, ri - l, b - t};
    return {};
  }

  Rect unionWith(const Rect &r) const {
    if (empty())
      return r;
    if (r.empty())
      return *this;
    float l = std::min(left(), r.left());
    float t = std::min(top(), r.top());
    float ri = std::max(right(), r.right());
    float b = std::max(bottom(), r.bottom());
    return {l, t, ri - l, b - t};
  }

  bool operator==(const Rect &r) const {
    return origin == r.origin && size == r.size;
  }
  bool operator!=(const Rect &r) const { return !(*this == r); }

  static constexpr Rect Zero() { return {0, 0, 0, 0}; }
};

// ---------------------------------------------------------------------------
// 2D 变换矩阵（基于 glm::mat4，兼容 OpenGL）
// ---------------------------------------------------------------------------
struct Transform2D {
  glm::mat4 matrix{1.0f}; // 单位矩阵

  Transform2D() = default;
  explicit Transform2D(const glm::mat4 &m) : matrix(m) {}

  static Transform2D identity() { return Transform2D{}; }

  static Transform2D translation(float x, float y) {
    Transform2D t;
    t.matrix = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, 0.0f));
    return t;
  }

  static Transform2D translation(const Vec2 &v) {
    return translation(v.x, v.y);
  }

  static Transform2D rotation(float degrees) {
    Transform2D t;
    t.matrix = glm::rotate(glm::mat4(1.0f), degrees * DEG_TO_RAD,
                           glm::vec3(0.0f, 0.0f, 1.0f));
    return t;
  }

  static Transform2D scaling(float sx, float sy) {
    Transform2D t;
    t.matrix = glm::scale(glm::mat4(1.0f), glm::vec3(sx, sy, 1.0f));
    return t;
  }

  static Transform2D scaling(float s) { return scaling(s, s); }

  static Transform2D skewing(float skewX, float skewY) {
    Transform2D t;
    t.matrix = glm::mat4(1.0f);
    t.matrix[1][0] = std::tan(skewX * DEG_TO_RAD);
    t.matrix[0][1] = std::tan(skewY * DEG_TO_RAD);
    return t;
  }

  Transform2D operator*(const Transform2D &other) const {
    return Transform2D(matrix * other.matrix);
  }

  Transform2D &operator*=(const Transform2D &other) {
    matrix *= other.matrix;
    return *this;
  }

  Vec2 transformPoint(const Vec2 &p) const {
    glm::vec4 result = matrix * glm::vec4(p.x, p.y, 0.0f, 1.0f);
    return {result.x, result.y};
  }

  Transform2D inverse() const { return Transform2D(glm::inverse(matrix)); }
};

// ---------------------------------------------------------------------------
// 数学工具函数
// ---------------------------------------------------------------------------
namespace math {

inline float clamp(float value, float minVal, float maxVal) {
  return std::clamp(value, minVal, maxVal);
}

inline float lerp(float a, float b, float t) { return a + (b - a) * t; }

inline float degrees(float radians) { return radians * RAD_TO_DEG; }

inline float radians(float degrees) { return degrees * DEG_TO_RAD; }

// ---------------------------------------------------------------------------
// 角度工具函数
// ---------------------------------------------------------------------------

/**
 * @brief 规范化角度到 [0, 360) 范围
 * @param degrees 输入角度（度数）
 * @return 规范化后的角度，范围 [0, 360)
 */
inline float normalizeAngle360(float degrees) {
  degrees = std::fmod(degrees, 360.0f);
  if (degrees < 0.0f) {
    degrees += 360.0f;
  }
  return degrees;
}

/**
 * @brief 规范化角度到 [-180, 180) 范围
 * @param degrees 输入角度（度数）
 * @return 规范化后的角度，范围 [-180, 180)
 */
inline float normalizeAngle180(float degrees) {
  degrees = std::fmod(degrees + 180.0f, 360.0f);
  if (degrees < 0.0f) {
    degrees += 360.0f;
  }
  return degrees - 180.0f;
}

/**
 * @brief 计算两个角度之间的最短差值
 * @param from 起始角度（度数）
 * @param to 目标角度（度数）
 * @return 从 from 到 to 的最短角度差，范围 [-180, 180]
 */
inline float angleDifference(float from, float to) {
  float diff = normalizeAngle360(to - from);
  if (diff > 180.0f) {
    diff -= 360.0f;
  }
  return diff;
}

/**
 * @brief 线性插值角度
 * @param from 起始角度（度数）
 * @param to 目标角度（度数）
 * @param t 插值因子 [0, 1]
 * @return 插值后的角度
 */
inline float lerpAngle(float from, float to, float t) {
  return from + angleDifference(from, to) * t;
}

// ---------------------------------------------------------------------------
// 向量工具函数
// ---------------------------------------------------------------------------

/**
 * @brief 计算方向向量（从 from 指向 to 的单位向量）
 * @param from 起始点
 * @param to 目标点
 * @return 归一化的方向向量
 */
inline Vec2 direction(const Vec2 &from, const Vec2 &to) {
  return (to - from).normalized();
}

/**
 * @brief 计算两点之间的角度
 * @param from 起始点
 * @param to 目标点
 * @return 角度（度数），范围 [-180, 180]
 */
inline float angleBetween(const Vec2 &from, const Vec2 &to) {
  Vec2 dir = to - from;
  return std::atan2(dir.y, dir.x) * RAD_TO_DEG;
}

/**
 * @brief 根据角度创建方向向量
 * @param degrees 角度（度数），0度指向右方，逆时针为正
 * @return 单位方向向量
 */
inline Vec2 angleToVector(float degrees) {
  float rad = degrees * DEG_TO_RAD;
  return {std::cos(rad), std::sin(rad)};
}

/**
 * @brief 将向量旋转指定角度
 * @param v 原始向量
 * @param degrees 旋转角度（度数），正值为逆时针旋转
 * @return 旋转后的向量
 */
inline Vec2 rotateVector(const Vec2 &v, float degrees) {
  float rad = degrees * DEG_TO_RAD;
  float cosA = std::cos(rad);
  float sinA = std::sin(rad);
  return {v.x * cosA - v.y * sinA, v.x * sinA + v.y * cosA};
}

// ---------------------------------------------------------------------------
// 坐标系转换工具
// ---------------------------------------------------------------------------

/**
 * @brief Y轴向上坐标转Y轴向下坐标
 * @param pos Y轴向上坐标系中的位置
 * @param height 画布/屏幕高度
 * @return Y轴向下坐标系中的位置
 */
inline Vec2 flipY(const Vec2 &pos, float height) {
  return {pos.x, height - pos.y};
}

/**
 * @brief Y轴向下坐标转Y轴向上坐标
 * @param pos Y轴向下坐标系中的位置
 * @param height 画布/屏幕高度
 * @return Y轴向上坐标系中的位置
 */
inline Vec2 unflipY(const Vec2 &pos, float height) {
  return {pos.x, height - pos.y};
}

// ---------------------------------------------------------------------------
// 矩阵工具函数
// ---------------------------------------------------------------------------

/**
 * @brief 从变换矩阵提取位置
 * @param matrix 4x4变换矩阵
 * @return 提取的位置向量
 */
inline Vec2 extractPosition(const glm::mat4 &matrix) {
  return {matrix[3][0], matrix[3][1]};
}

/**
 * @brief 从变换矩阵提取缩放
 * @param matrix 4x4变换矩阵
 * @return 提取的缩放向量
 */
inline Vec2 extractScale(const glm::mat4 &matrix) {
  float scaleX = std::sqrt(matrix[0][0] * matrix[0][0] + matrix[0][1] * matrix[0][1]);
  float scaleY = std::sqrt(matrix[1][0] * matrix[1][0] + matrix[1][1] * matrix[1][1]);
  return {scaleX, scaleY};
}

/**
 * @brief 从变换矩阵提取旋转角度
 * @param matrix 4x4变换矩阵
 * @return 提取的旋转角度（度数）
 */
inline float extractRotation(const glm::mat4 &matrix) {
  return std::atan2(matrix[0][1], matrix[0][0]) * RAD_TO_DEG;
}

// ---------------------------------------------------------------------------
// 碰撞检测工具
// ---------------------------------------------------------------------------

/**
 * @brief 判断点是否在矩形内
 * @param point 要检测的点
 * @param rect 矩形区域
 * @return 如果点在矩形内返回 true，否则返回 false
 */
inline bool pointInRect(const Vec2 &point, const Rect &rect) {
  return point.x >= rect.left() && point.x <= rect.right() &&
         point.y >= rect.top() && point.y <= rect.bottom();
}

/**
 * @brief 判断点是否在圆内
 * @param point 要检测的点
 * @param center 圆心
 * @param radius 圆的半径
 * @return 如果点在圆内返回 true，否则返回 false
 */
inline bool pointInCircle(const Vec2 &point, const Vec2 &center, float radius) {
  float dx = point.x - center.x;
  float dy = point.y - center.y;
  return (dx * dx + dy * dy) <= (radius * radius);
}

/**
 * @brief 判断两个矩形是否相交
 * @param a 第一个矩形
 * @param b 第二个矩形
 * @return 如果矩形相交返回 true，否则返回 false
 */
inline bool rectsIntersect(const Rect &a, const Rect &b) {
  return a.intersects(b);
}

/**
 * @brief 判断两个圆是否相交
 * @param center1 第一个圆的圆心
 * @param radius1 第一个圆的半径
 * @param center2 第二个圆的圆心
 * @param radius2 第二个圆的半径
 * @return 如果圆相交返回 true，否则返回 false
 */
inline bool circlesIntersect(const Vec2 &center1, float radius1,
                             const Vec2 &center2, float radius2) {
  float dx = center2.x - center1.x;
  float dy = center2.y - center1.y;
  float distSq = dx * dx + dy * dy;
  float radiusSum = radius1 + radius2;
  return distSq <= (radiusSum * radiusSum);
}

} // namespace math

} // namespace extra2d
