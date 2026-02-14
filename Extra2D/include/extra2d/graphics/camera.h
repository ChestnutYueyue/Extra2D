#pragma once

#include <extra2d/core/color.h>
#include <extra2d/core/math_types.h>
#include <extra2d/core/types.h>
#include <glm/mat4x4.hpp>

namespace extra2d {

class ViewportAdapter;

// ============================================================================
// 2D 正交相机
// ============================================================================
class Camera {
public:
  Camera();
  Camera(float left, float right, float bottom, float top);
  Camera(const Size &viewport);
  ~Camera() = default;

  // ------------------------------------------------------------------------
  // 位置和变换
  // ------------------------------------------------------------------------
  void setPos(const Vec2 &position);
  void setPos(float x, float y);
  Vec2 getPosition() const { return position_; }

  void setRotation(float degrees);
  float getRotation() const { return rotation_; }

  void setZoom(float zoom);
  float getZoom() const { return zoom_; }

  // ------------------------------------------------------------------------
  // 视口设置
  // ------------------------------------------------------------------------
  void setViewport(float left, float right, float bottom, float top);
  void setViewport(const Rect &rect);
  Rect getViewport() const;

  // ------------------------------------------------------------------------
  // 矩阵获取
  // ------------------------------------------------------------------------
  glm::mat4 getViewMatrix() const;
  glm::mat4 getProjectionMatrix() const;
  glm::mat4 getViewProjectionMatrix() const;

  // ------------------------------------------------------------------------
  // 坐标转换
  // ------------------------------------------------------------------------
  Vec2 screenToWorld(const Vec2 &screenPos) const;
  Vec2 worldToScreen(const Vec2 &worldPos) const;
  Vec2 screenToWorld(float x, float y) const;
  Vec2 worldToScreen(float x, float y) const;

  // ------------------------------------------------------------------------
  // 移动相机
  // ------------------------------------------------------------------------
  void move(const Vec2 &offset);
  void move(float x, float y);

  // ------------------------------------------------------------------------
  // 边界限制
  // ------------------------------------------------------------------------
  void setBounds(const Rect &bounds);
  void clearBounds();
  void clampToBounds();

  // ------------------------------------------------------------------------
  // 视口适配器
  // ------------------------------------------------------------------------
  /**
   * @brief 设置视口适配器
   * @param adapter 视口适配器指针
   */
  void setViewportAdapter(ViewportAdapter *adapter);

  /**
   * @brief 根据视口适配器自动设置视口
   */
  void applyViewportAdapter();

  // ------------------------------------------------------------------------
  // 快捷方法：看向某点
  // ------------------------------------------------------------------------
  void lookAt(const Vec2 &target);

private:
  Vec2 position_ = Vec2::Zero();
  float rotation_ = 0.0f;
  float zoom_ = 1.0f;

  float left_ = -1.0f;
  float right_ = 1.0f;
  float bottom_ = -1.0f;
  float top_ = 1.0f;

  Rect bounds_;
  bool hasBounds_ = false;

  ViewportAdapter *viewportAdapter_ = nullptr;

  mutable glm::mat4 viewMatrix_;
  mutable glm::mat4 projMatrix_;
  mutable glm::mat4 vpMatrix_;
  mutable bool viewDirty_ = true;
  mutable bool projDirty_ = true;
};

} // namespace extra2d
