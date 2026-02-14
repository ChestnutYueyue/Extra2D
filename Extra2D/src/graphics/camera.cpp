#include <algorithm>
#include <extra2d/graphics/camera.h>
#include <extra2d/graphics/viewport_adapter.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>

namespace extra2d {

Camera::Camera() : left_(-1.0f), right_(1.0f), bottom_(-1.0f), top_(1.0f) {}

Camera::Camera(float left, float right, float bottom, float top)
    : left_(left), right_(right), bottom_(bottom), top_(top) {}

Camera::Camera(const Size &viewport)
    : left_(0.0f), right_(viewport.width), bottom_(viewport.height),
      top_(0.0f) {}

void Camera::setPosition(const Vec2 &position) {
  position_ = position;
  viewDirty_ = true;
}

void Camera::setPosition(float x, float y) {
  position_.x = x;
  position_.y = y;
  viewDirty_ = true;
}

void Camera::setRotation(float degrees) {
  rotation_ = degrees;
  viewDirty_ = true;
}

void Camera::setZoom(float zoom) {
  zoom_ = zoom;
  viewDirty_ = true;
  projDirty_ = true;
}

void Camera::setViewport(float left, float right, float bottom, float top) {
  left_ = left;
  right_ = right;
  bottom_ = bottom;
  top_ = top;
  projDirty_ = true;
}

void Camera::setViewport(const Rect &rect) {
  left_ = rect.left();
  right_ = rect.right();
  bottom_ = rect.bottom();
  top_ = rect.top();
  projDirty_ = true;
}

Rect Camera::getViewport() const {
  return Rect(left_, top_, right_ - left_, bottom_ - top_);
}

/**
 * @brief 获取视图矩阵
 * @return 视图矩阵
 * 
 * 变换顺序：平移 -> 旋转 -> 缩放（逆序应用）
 * View = T(-position) × R(-rotation) × S(1/zoom)
 */
glm::mat4 Camera::getViewMatrix() const {
  if (viewDirty_) {
    viewMatrix_ = glm::mat4(1.0f);
    
    // 1. 平移（最后应用）
    viewMatrix_ = glm::translate(viewMatrix_, 
                                 glm::vec3(-position_.x, -position_.y, 0.0f));
    
    // 2. 旋转（中间应用）
    if (rotation_ != 0.0f) {
      viewMatrix_ = glm::rotate(viewMatrix_, 
                                -rotation_ * DEG_TO_RAD,
                                glm::vec3(0.0f, 0.0f, 1.0f));
    }
    
    // 3. 缩放（最先应用）
    if (zoom_ != 1.0f) {
      viewMatrix_ = glm::scale(viewMatrix_, 
                               glm::vec3(1.0f / zoom_, 1.0f / zoom_, 1.0f));
    }
    
    viewDirty_ = false;
  }
  return viewMatrix_;
}

glm::mat4 Camera::getProjectionMatrix() const {
  if (projDirty_) {
    // 对于2D游戏，Y轴向下增长（屏幕坐标系）
    // OpenGL默认Y轴向上，所以需要反转Y轴
    // glm::ortho(left, right, bottom, top)
    // 为了Y轴向下：传入 (bottom=height, top=0)，这样Y轴翻转
    projMatrix_ = glm::ortho(
        left_, right_, // X轴：从左到右
        bottom_, top_, // Y轴：从下到上（传入bottom>top，实现Y轴向下增长）
        -1.0f, 1.0f);
    projDirty_ = false;
  }
  return projMatrix_;
}

/**
 * @brief 获取视图-投影矩阵
 * @return 视图-投影矩阵
 */
glm::mat4 Camera::getViewProjectionMatrix() const {
  return getProjectionMatrix() * getViewMatrix();
}

/**
 * @brief 将屏幕坐标转换为世界坐标
 * @param screenPos 屏幕坐标
 * @return 世界坐标
 */
Vec2 Camera::screenToWorld(const Vec2 &screenPos) const {
  Vec2 logicPos = screenPos;
  
  // 如果有视口适配器，先转换到逻辑坐标
  if (viewportAdapter_) {
    logicPos = viewportAdapter_->screenToLogic(screenPos);
  }
  
  // 使用逆视图-投影矩阵转换
  glm::mat4 invVP = glm::inverse(getViewProjectionMatrix());
  glm::vec4 ndc(logicPos.x, logicPos.y, 0.0f, 1.0f);
  glm::vec4 world = invVP * ndc;
  return Vec2(world.x, world.y);
}

/**
 * @brief 将世界坐标转换为屏幕坐标
 * @param worldPos 世界坐标
 * @return 屏幕坐标
 */
Vec2 Camera::worldToScreen(const Vec2 &worldPos) const {
  glm::vec4 world(worldPos.x, worldPos.y, 0.0f, 1.0f);
  glm::vec4 screen = getViewProjectionMatrix() * world;
  Vec2 logicPos(screen.x, screen.y);
  
  // 如果有视口适配器，转换到屏幕坐标
  if (viewportAdapter_) {
    return viewportAdapter_->logicToScreen(logicPos);
  }
  return logicPos;
}

Vec2 Camera::screenToWorld(float x, float y) const {
  return screenToWorld(Vec2(x, y));
}

Vec2 Camera::worldToScreen(float x, float y) const {
  return worldToScreen(Vec2(x, y));
}

void Camera::move(const Vec2 &offset) {
  position_ += offset;
  viewDirty_ = true;
}

void Camera::move(float x, float y) {
  position_.x += x;
  position_.y += y;
  viewDirty_ = true;
}

void Camera::setBounds(const Rect &bounds) {
  bounds_ = bounds;
  hasBounds_ = true;
}

void Camera::clearBounds() { hasBounds_ = false; }

void Camera::clampToBounds() {
  if (!hasBounds_)
    return;

  float viewportWidth = (right_ - left_) / zoom_;
  float viewportHeight = (bottom_ - top_) / zoom_;

  float minX = bounds_.left() + viewportWidth * 0.5f;
  float maxX = bounds_.right() - viewportWidth * 0.5f;
  float minY = bounds_.top() + viewportHeight * 0.5f;
  float maxY = bounds_.bottom() - viewportHeight * 0.5f;

  if (minX > maxX) {
    position_.x = bounds_.center().x;
  } else {
    position_.x = std::clamp(position_.x, minX, maxX);
  }

  if (minY > maxY) {
    position_.y = bounds_.center().y;
  } else {
    position_.y = std::clamp(position_.y, minY, maxY);
  }

  viewDirty_ = true;
}

void Camera::lookAt(const Vec2 &target) {
  position_ = target;
  viewDirty_ = true;
}

/**
 * @brief 设置视口适配器
 * @param adapter 视口适配器指针
 */
void Camera::setViewportAdapter(ViewportAdapter* adapter) {
  viewportAdapter_ = adapter;
}

/**
 * @brief 根据视口适配器自动设置视口
 */
void Camera::applyViewportAdapter() {
  if (viewportAdapter_) {
    const auto& config = viewportAdapter_->getConfig();
    setViewport(0.0f, config.logicWidth, config.logicHeight, 0.0f);
  }
}

} // namespace extra2d
