#include <extra2d/graphics/viewport_adapter.h>
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>

namespace extra2d {

/**
 * @brief 默认构造函数
 *
 * 创建一个未配置的视口适配器
 */
ViewportAdapter::ViewportAdapter() = default;

/**
 * @brief 构造函数
 * @param logicWidth 逻辑宽度
 * @param logicHeight 逻辑高度
 *
 * 创建一个指定逻辑尺寸的视口适配器
 */
ViewportAdapter::ViewportAdapter(float logicWidth, float logicHeight) {
  config_.logicWidth = logicWidth;
  config_.logicHeight = logicHeight;
}

/**
 * @brief 设置视口配置
 * @param config 视口配置
 *
 * 设置完整的视口配置并标记矩阵为脏
 */
void ViewportAdapter::setConfig(const ViewportConfig &config) {
  config_ = config;
  matrixDirty_ = true;
}

/**
 * @brief 设置逻辑尺寸
 * @param width 逻辑宽度
 * @param height 逻辑高度
 *
 * 设置视口的逻辑尺寸并标记矩阵为脏
 */
void ViewportAdapter::setLogicSize(float width, float height) {
  config_.logicWidth = width;
  config_.logicHeight = height;
  matrixDirty_ = true;
}

/**
 * @brief 设置视口模式
 * @param mode 视口模式
 *
 * 设置视口适配模式（宽高比、拉伸、居中等）
 */
void ViewportAdapter::setMode(ViewportMode mode) {
  config_.mode = mode;
  matrixDirty_ = true;
}

/**
 * @brief 设置黑边位置
 * @param position 黑边位置
 *
 * 设置黑边相对于视口的位置
 */
void ViewportAdapter::setLetterboxPosition(LetterboxPosition position) {
  config_.letterboxPosition = position;
  matrixDirty_ = true;
}

/**
 * @brief 设置黑边颜色
 * @param color 黑边颜色
 *
 * 设置黑边区域的填充颜色
 */
void ViewportAdapter::setLetterboxColor(const Color &color) {
  config_.letterboxColor = color;
}

/**
 * @brief 更新视口适配
 * @param screenWidth 屏幕宽度
 * @param screenHeight 屏幕高度
 *
 * 根据屏幕尺寸和配置计算视口参数
 */
void ViewportAdapter::update(int screenWidth, int screenHeight) {
  if (screenWidth_ == screenWidth && screenHeight_ == screenHeight &&
      !matrixDirty_) {
    return;
  }

  screenWidth_ = screenWidth;
  screenHeight_ = screenHeight;
  matrixDirty_ = true;

  result_.hasLetterbox = false;
  result_.letterbox.top = Rect::Zero();
  result_.letterbox.bottom = Rect::Zero();
  result_.letterbox.left = Rect::Zero();
  result_.letterbox.right = Rect::Zero();

  switch (config_.mode) {
  case ViewportMode::AspectRatio:
    calculateAspectRatio();
    break;
  case ViewportMode::Stretch:
    calculateStretch();
    break;
  case ViewportMode::Center:
    calculateCenter();
    break;
  case ViewportMode::Custom:
    calculateCustom();
    break;
  }
}

/**
 * @brief 计算宽高比适配模式
 *
 * 保持逻辑宽高比，根据屏幕尺寸计算缩放和偏移
 */
void ViewportAdapter::calculateAspectRatio() {
  if (config_.logicHeight <= 0.0f || screenHeight_ <= 0) {
    result_ = ViewportResult();
    return;
  }

  float logicAspect = config_.logicWidth / config_.logicHeight;
  float screenAspect = static_cast<float>(screenWidth_) / screenHeight_;

  if (screenAspect > logicAspect) {
    result_.uniformScale = static_cast<float>(screenHeight_) / config_.logicHeight;
    result_.scaleX = result_.uniformScale;
    result_.scaleY = result_.uniformScale;
    result_.viewport.size.width = config_.logicWidth * result_.uniformScale;
    result_.viewport.size.height = static_cast<float>(screenHeight_);
    result_.offset.x = (screenWidth_ - result_.viewport.size.width) / 2.0f;
    result_.offset.y = 0.0f;
  } else {
    result_.uniformScale = static_cast<float>(screenWidth_) / config_.logicWidth;
    result_.scaleX = result_.uniformScale;
    result_.scaleY = result_.uniformScale;
    result_.viewport.size.width = static_cast<float>(screenWidth_);
    result_.viewport.size.height = config_.logicHeight * result_.uniformScale;
    result_.offset.x = 0.0f;
    result_.offset.y = (screenHeight_ - result_.viewport.size.height) / 2.0f;
  }

  result_.viewport.origin = result_.offset;

  applyLetterboxPosition(
      static_cast<float>(screenWidth_) - result_.viewport.size.width,
      static_cast<float>(screenHeight_) - result_.viewport.size.height);

  calculateLetterbox();
}

/**
 * @brief 计算拉伸模式
 *
 * 拉伸逻辑视口以填满整个屏幕
 */
void ViewportAdapter::calculateStretch() {
  result_.scaleX = static_cast<float>(screenWidth_) / config_.logicWidth;
  result_.scaleY = static_cast<float>(screenHeight_) / config_.logicHeight;
  result_.uniformScale = std::min(result_.scaleX, result_.scaleY);

  result_.viewport.origin = Vec2::Zero();
  result_.viewport.size.width = static_cast<float>(screenWidth_);
  result_.viewport.size.height = static_cast<float>(screenHeight_);

  result_.offset = Vec2::Zero();
  result_.hasLetterbox = false;
}

/**
 * @brief 计算居中模式
 *
 * 将逻辑视口居中显示，可选自动缩放
 */
void ViewportAdapter::calculateCenter() {
  float displayWidth = config_.logicWidth;
  float displayHeight = config_.logicHeight;

  if (config_.autoScaleInCenterMode) {
    float scaleX = static_cast<float>(screenWidth_) / config_.logicWidth;
    float scaleY = static_cast<float>(screenHeight_) / config_.logicHeight;
    result_.uniformScale = std::min(scaleX, scaleY);

    if (result_.uniformScale < 1.0f) {
      displayWidth = config_.logicWidth * result_.uniformScale;
      displayHeight = config_.logicHeight * result_.uniformScale;
    } else {
      result_.uniformScale = 1.0f;
    }

    result_.scaleX = result_.uniformScale;
    result_.scaleY = result_.uniformScale;
  } else {
    result_.scaleX = 1.0f;
    result_.scaleY = 1.0f;
    result_.uniformScale = 1.0f;
  }

  result_.offset.x = (screenWidth_ - displayWidth) / 2.0f;
  result_.offset.y = (screenHeight_ - displayHeight) / 2.0f;

  result_.viewport.origin = result_.offset;
  result_.viewport.size.width = displayWidth;
  result_.viewport.size.height = displayHeight;

  applyLetterboxPosition(
      static_cast<float>(screenWidth_) - displayWidth,
      static_cast<float>(screenHeight_) - displayHeight);

  calculateLetterbox();
}

/**
 * @brief 计算自定义模式
 *
 * 使用自定义缩放和偏移参数
 */
void ViewportAdapter::calculateCustom() {
  result_.scaleX = config_.customScale;
  result_.scaleY = config_.customScale;
  result_.uniformScale = config_.customScale;

  if (config_.customViewport.empty()) {
    float displayWidth = config_.logicWidth * config_.customScale;
    float displayHeight = config_.logicHeight * config_.customScale;

    result_.offset = config_.customOffset;
    result_.viewport.origin = result_.offset;
    result_.viewport.size.width = displayWidth;
    result_.viewport.size.height = displayHeight;
  } else {
    result_.viewport = config_.customViewport;
    result_.offset = config_.customViewport.origin;
  }

  calculateLetterbox();
}

/**
 * @brief 计算黑边区域
 *
 * 根据视口偏移计算上下左右黑边矩形
 */
void ViewportAdapter::calculateLetterbox() {
  result_.hasLetterbox = false;

  float screenW = static_cast<float>(screenWidth_);
  float screenH = static_cast<float>(screenHeight_);

  if (result_.offset.y > 0.0f) {
    result_.hasLetterbox = true;
    result_.letterbox.top =
        Rect(0.0f, 0.0f, screenW, result_.offset.y);
    result_.letterbox.bottom =
        Rect(0.0f, result_.offset.y + result_.viewport.size.height, screenW,
             result_.offset.y);
  }

  if (result_.offset.x > 0.0f) {
    result_.hasLetterbox = true;
    result_.letterbox.left =
        Rect(0.0f, 0.0f, result_.offset.x, screenH);
    result_.letterbox.right =
        Rect(result_.offset.x + result_.viewport.size.width, 0.0f,
             result_.offset.x, screenH);
  }
}

/**
 * @brief 应用黑边位置
 * @param extraWidth 额外宽度
 * @param extraHeight 额外高度
 *
 * 根据配置调整视口偏移以实现不同的黑边位置
 */
void ViewportAdapter::applyLetterboxPosition(float extraWidth,
                                             float extraHeight) {
  if (extraWidth <= 0.0f && extraHeight <= 0.0f) {
    return;
  }

  switch (config_.letterboxPosition) {
  case LetterboxPosition::Center:
    break;

  case LetterboxPosition::LeftTop:
    if (extraWidth > 0.0f) {
      result_.offset.x = 0.0f;
    }
    if (extraHeight > 0.0f) {
      result_.offset.y = 0.0f;
    }
    break;

  case LetterboxPosition::RightTop:
    if (extraWidth > 0.0f) {
      result_.offset.x = extraWidth;
    }
    if (extraHeight > 0.0f) {
      result_.offset.y = 0.0f;
    }
    break;

  case LetterboxPosition::LeftBottom:
    if (extraWidth > 0.0f) {
      result_.offset.x = 0.0f;
    }
    if (extraHeight > 0.0f) {
      result_.offset.y = extraHeight;
    }
    break;

  case LetterboxPosition::RightBottom:
    if (extraWidth > 0.0f) {
      result_.offset.x = extraWidth;
    }
    if (extraHeight > 0.0f) {
      result_.offset.y = extraHeight;
    }
    break;
  }

  result_.viewport.origin = result_.offset;
}

/**
 * @brief 将屏幕坐标转换为逻辑坐标
 * @param screenPos 屏幕坐标
 * @return 逻辑坐标
 *
 * 根据当前缩放和偏移计算对应的逻辑坐标
 */
Vec2 ViewportAdapter::screenToLogic(const Vec2 &screenPos) const {
  return Vec2((screenPos.x - result_.offset.x) / result_.scaleX,
              (screenPos.y - result_.offset.y) / result_.scaleY);
}

/**
 * @brief 将逻辑坐标转换为屏幕坐标
 * @param logicPos 逻辑坐标
 * @return 屏幕坐标
 *
 * 根据当前缩放和偏移计算对应的屏幕坐标
 */
Vec2 ViewportAdapter::logicToScreen(const Vec2 &logicPos) const {
  return Vec2(logicPos.x * result_.scaleX + result_.offset.x,
              logicPos.y * result_.scaleY + result_.offset.y);
}

/**
 * @brief 将屏幕坐标转换为逻辑坐标
 * @param x 屏幕X坐标
 * @param y 屏幕Y坐标
 * @return 逻辑坐标
 */
Vec2 ViewportAdapter::screenToLogic(float x, float y) const {
  return screenToLogic(Vec2(x, y));
}

/**
 * @brief 将逻辑坐标转换为屏幕坐标
 * @param x 逻辑X坐标
 * @param y 逻辑Y坐标
 * @return 屏幕坐标
 */
Vec2 ViewportAdapter::logicToScreen(float x, float y) const {
  return logicToScreen(Vec2(x, y));
}

/**
 * @brief 获取视口变换矩阵
 * @return 4x4变换矩阵
 *
 * 返回用于将逻辑坐标转换为屏幕坐标的变换矩阵
 */
glm::mat4 ViewportAdapter::getMatrix() const {
  if (matrixDirty_) {
    viewportMatrix_ = glm::mat4(1.0f);
    viewportMatrix_ = glm::translate(viewportMatrix_,
                                     glm::vec3(result_.offset.x, result_.offset.y, 0.0f));
    viewportMatrix_ = glm::scale(viewportMatrix_,
                                 glm::vec3(result_.scaleX, result_.scaleY, 1.0f));
    matrixDirty_ = false;
  }
  return viewportMatrix_;
}

/**
 * @brief 获取视口逆变换矩阵
 * @return 4x4逆变换矩阵
 *
 * 返回用于将屏幕坐标转换为逻辑坐标的逆变换矩阵
 */
glm::mat4 ViewportAdapter::getInvMatrix() const {
  if (matrixDirty_) {
    getMatrix();
  }
  inverseViewportMatrix_ = glm::inverse(viewportMatrix_);
  return inverseViewportMatrix_;
}

/**
 * @brief 检查屏幕坐标是否在视口内
 * @param screenPos 屏幕坐标
 * @return 在视口内返回true，否则返回false
 */
bool ViewportAdapter::isInViewport(const Vec2 &screenPos) const {
  return result_.viewport.containsPoint(screenPos);
}

/**
 * @brief 检查屏幕坐标是否在黑边区域内
 * @param screenPos 屏幕坐标
 * @return 在黑边区域内返回true，否则返回false
 */
bool ViewportAdapter::isInLetterbox(const Vec2 &screenPos) const {
  if (!result_.hasLetterbox) {
    return false;
  }

  if (!result_.letterbox.top.empty() &&
      result_.letterbox.top.containsPoint(screenPos)) {
    return true;
  }
  if (!result_.letterbox.bottom.empty() &&
      result_.letterbox.bottom.containsPoint(screenPos)) {
    return true;
  }
  if (!result_.letterbox.left.empty() &&
      result_.letterbox.left.containsPoint(screenPos)) {
    return true;
  }
  if (!result_.letterbox.right.empty() &&
      result_.letterbox.right.containsPoint(screenPos)) {
    return true;
  }

  return false;
}

} // namespace extra2d
