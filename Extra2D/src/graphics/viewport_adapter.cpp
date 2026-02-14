#include <extra2d/graphics/viewport_adapter.h>
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>

namespace extra2d {

ViewportAdapter::ViewportAdapter() = default;

ViewportAdapter::ViewportAdapter(float logicWidth, float logicHeight) {
  config_.logicWidth = logicWidth;
  config_.logicHeight = logicHeight;
}

void ViewportAdapter::setConfig(const ViewportConfig &config) {
  config_ = config;
  matrixDirty_ = true;
}

void ViewportAdapter::setLogicSize(float width, float height) {
  config_.logicWidth = width;
  config_.logicHeight = height;
  matrixDirty_ = true;
}

void ViewportAdapter::setMode(ViewportMode mode) {
  config_.mode = mode;
  matrixDirty_ = true;
}

void ViewportAdapter::setLetterboxPosition(LetterboxPosition position) {
  config_.letterboxPosition = position;
  matrixDirty_ = true;
}

void ViewportAdapter::setLetterboxColor(const Color &color) {
  config_.letterboxColor = color;
}

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

Vec2 ViewportAdapter::screenToLogic(const Vec2 &screenPos) const {
  return Vec2((screenPos.x - result_.offset.x) / result_.scaleX,
              (screenPos.y - result_.offset.y) / result_.scaleY);
}

Vec2 ViewportAdapter::logicToScreen(const Vec2 &logicPos) const {
  return Vec2(logicPos.x * result_.scaleX + result_.offset.x,
              logicPos.y * result_.scaleY + result_.offset.y);
}

Vec2 ViewportAdapter::screenToLogic(float x, float y) const {
  return screenToLogic(Vec2(x, y));
}

Vec2 ViewportAdapter::logicToScreen(float x, float y) const {
  return logicToScreen(Vec2(x, y));
}

glm::mat4 ViewportAdapter::getViewportMatrix() const {
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

glm::mat4 ViewportAdapter::getInverseViewportMatrix() const {
  if (matrixDirty_) {
    getViewportMatrix();
  }
  inverseViewportMatrix_ = glm::inverse(viewportMatrix_);
  return inverseViewportMatrix_;
}

bool ViewportAdapter::isInViewport(const Vec2 &screenPos) const {
  return result_.viewport.containsPoint(screenPos);
}

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
