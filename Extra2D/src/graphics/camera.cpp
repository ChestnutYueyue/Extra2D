#include <algorithm>
#include <extra2d/graphics/camera.h>
#include <extra2d/graphics/viewport_adapter.h>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>


namespace extra2d {

/**
 * @brief 默认构造函数
 *
 * 创建一个默认的正交相机，视口范围为 (-1, -1) 到 (1, 1)
 */
Camera::Camera() : left_(-1.0f), right_(1.0f), bottom_(-1.0f), top_(1.0f) {}

/**
 * @brief 构造函数
 * @param left 视口左边界
 * @param right 视口右边界
 * @param bottom 视口底边界
 * @param top 视口顶边界
 *
 * 创建一个指定视口范围的正交相机
 */
Camera::Camera(float left, float right, float bottom, float top)
    : left_(left), right_(right), bottom_(bottom), top_(top) {}

/**
 * @brief 构造函数
 * @param viewport 视口尺寸
 *
 * 根据视口尺寸创建相机，视口原点在左上角
 */
Camera::Camera(const Size &viewport)
    : left_(0.0f), right_(viewport.width), bottom_(viewport.height),
      top_(0.0f) {}

/**
 * @brief 设置相机位置
 * @param position 新的位置坐标
 *
 * 设置相机在世界空间中的位置，会标记视图矩阵为脏
 */
void Camera::setPos(const Vec2 &position) {
  position_ = position;
  viewDirty_ = true;
}

/**
 * @brief 设置相机位置
 * @param x X坐标
 * @param y Y坐标
 *
 * 设置相机在世界空间中的位置，会标记视图矩阵为脏
 */
void Camera::setPos(float x, float y) {
  position_.x = x;
  position_.y = y;
  viewDirty_ = true;
}

/**
 * @brief 设置相机旋转角度
 * @param degrees 旋转角度（度数）
 *
 * 设置相机的旋转角度，会标记视图矩阵为脏
 */
void Camera::setRotation(float degrees) {
  rotation_ = degrees;
  viewDirty_ = true;
}

/**
 * @brief 设置相机缩放级别
 * @param zoom 缩放值（1.0为正常大小）
 *
 * 设置相机的缩放级别，会同时标记视图矩阵和投影矩阵为脏
 */
void Camera::setZoom(float zoom) {
  zoom_ = zoom;
  viewDirty_ = true;
  projDirty_ = true;
}

/**
 * @brief 设置视口范围
 * @param left 左边界
 * @param right 右边界
 * @param bottom 底边界
 * @param top 顶边界
 *
 * 设置相机的正交投影视口范围，会标记投影矩阵为脏
 */
void Camera::setViewport(float left, float right, float bottom, float top) {
  left_ = left;
  right_ = right;
  bottom_ = bottom;
  top_ = top;
  projDirty_ = true;
}

/**
 * @brief 设置视口范围
 * @param rect 视口矩形
 *
 * 使用矩形设置相机的正交投影视口范围，会标记投影矩阵为脏
 */
void Camera::setViewport(const Rect &rect) {
  left_ = rect.left();
  right_ = rect.right();
  bottom_ = rect.bottom();
  top_ = rect.top();
  projDirty_ = true;
}

/**
 * @brief 获取视口矩形
 * @return 当前视口的矩形表示
 *
 * 返回当前相机的视口范围
 */
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
      viewMatrix_ = glm::rotate(viewMatrix_, -rotation_ * DEG_TO_RAD,
                                glm::vec3(0.0f, 0.0f, 1.0f));
    }

    // 3. 缩放（最先应用）
    if (zoom_ != 1.0f) {
      viewMatrix_ =
          glm::scale(viewMatrix_, glm::vec3(1.0f / zoom_, 1.0f / zoom_, 1.0f));
    }

    viewDirty_ = false;
  }
  return viewMatrix_;
}

/**
 * @brief 获取投影矩阵
 * @return 正交投影矩阵
 *
 * 对于2D游戏，Y轴向下增长（屏幕坐标系）
 * OpenGL默认Y轴向上，所以需要反转Y轴
 */
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

/**
 * @brief 将屏幕坐标转换为世界坐标
 * @param x 屏幕X坐标
 * @param y 屏幕Y坐标
 * @return 世界坐标
 */
Vec2 Camera::screenToWorld(float x, float y) const {
  return screenToWorld(Vec2(x, y));
}

/**
 * @brief 将世界坐标转换为屏幕坐标
 * @param x 世界X坐标
 * @param y 世界Y坐标
 * @return 屏幕坐标
 */
Vec2 Camera::worldToScreen(float x, float y) const {
  return worldToScreen(Vec2(x, y));
}

/**
 * @brief 移动相机位置
 * @param offset 位置偏移量
 *
 * 按指定偏移量移动相机位置，会标记视图矩阵为脏
 */
void Camera::move(const Vec2 &offset) {
  position_ += offset;
  viewDirty_ = true;
}

/**
 * @brief 移动相机位置
 * @param x X方向偏移量
 * @param y Y方向偏移量
 *
 * 按指定偏移量移动相机位置，会标记视图矩阵为脏
 */
void Camera::move(float x, float y) {
  position_.x += x;
  position_.y += y;
  viewDirty_ = true;
}

/**
 * @brief 设置相机边界限制
 * @param bounds 边界矩形
 *
 * 设置相机的移动边界，相机位置将被限制在此边界内
 */
void Camera::setBounds(const Rect &bounds) {
  bounds_ = bounds;
  hasBounds_ = true;
}

/**
 * @brief 清除相机边界限制
 *
 * 移除相机的移动边界限制
 */
void Camera::clearBounds() { hasBounds_ = false; }

/**
 * @brief 将相机位置限制在边界内
 *
 * 如果设置了边界，将相机位置限制在边界矩形内
 */
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

/**
 * @brief 将相机移动到目标位置
 * @param target 目标位置
 *
 * 设置相机位置到指定的世界坐标
 */
void Camera::lookAt(const Vec2 &target) {
  position_ = target;
  viewDirty_ = true;
}

/**
 * @brief 设置视口适配器
 * @param adapter 视口适配器指针
 */
void Camera::setViewportAdapter(ViewportAdapter *adapter) {
  viewportAdapter_ = adapter;
}

/**
 * @brief 根据视口适配器自动设置视口
 *
 * 如果设置了视口适配器，根据其配置自动设置相机的视口范围
 */
void Camera::applyViewportAdapter() {
  if (viewportAdapter_) {
    const auto &config = viewportAdapter_->getConfig();
    setViewport(0.0f, config.logicWidth, config.logicHeight, 0.0f);
  }
}

} // namespace extra2d
