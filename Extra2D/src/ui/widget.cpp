#include <cmath>
#include <extra2d/graphics/camera.h>
#include <extra2d/scene/scene.h>
#include <extra2d/ui/widget.h>

namespace extra2d {

Widget::Widget() {
  setSpatialIndexed(false);
}

void Widget::setSize(const Size &size) {
  size_ = size;
  updateSpatialIndex();
}

void Widget::setSize(float width, float height) {
  setSize(Size(width, height));
}

Rect Widget::getBoundingBox() const {
  if (size_.empty()) {
    return Rect();
  }

  auto pos = getRenderPosition();
  auto anchor = getAnchor();
  auto scale = getScale();

  float w = size_.width * scale.x;
  float h = size_.height * scale.y;
  float x0 = pos.x - size_.width * anchor.x * scale.x;
  float y0 = pos.y - size_.height * anchor.y * scale.y;
  float x1 = x0 + w;
  float y1 = y0 + h;

  float l = std::min(x0, x1);
  float t = std::min(y0, y1);
  return Rect(l, t, std::abs(w), std::abs(h));
}

// ------------------------------------------------------------------------
// 坐标空间设置
// ------------------------------------------------------------------------
void Widget::setCoordinateSpace(CoordinateSpace space) {
  coordinateSpace_ = space;
}

void Widget::setScreenPosition(const Vec2 &pos) {
  screenPosition_ = pos;
  if (coordinateSpace_ == CoordinateSpace::Screen) {
    updateSpatialIndex();
  }
}

void Widget::setScreenPosition(float x, float y) {
  setScreenPosition(Vec2(x, y));
}

void Widget::setCameraOffset(const Vec2 &offset) {
  cameraOffset_ = offset;
  if (coordinateSpace_ == CoordinateSpace::Camera) {
    updateSpatialIndex();
  }
}

void Widget::setCameraOffset(float x, float y) {
  setCameraOffset(Vec2(x, y));
}

// ------------------------------------------------------------------------
// 获取实际渲染位置（根据坐标空间计算）
// ------------------------------------------------------------------------
Vec2 Widget::getRenderPosition() const {
  switch (coordinateSpace_) {
    case CoordinateSpace::Screen:
      // 屏幕空间：直接使用屏幕位置
      return screenPosition_;

    case CoordinateSpace::Camera: {
      // 相机空间：相机位置 + 偏移
      Scene *scene = getScene();
      if (scene) {
        Camera *camera = scene->getActiveCamera();
        if (camera) {
          return camera->getPosition() + cameraOffset_;
        }
      }
      // 如果没有场景或相机，使用偏移作为绝对位置
      return cameraOffset_;
    }

    case CoordinateSpace::World:
    default:
      // 世界空间：使用节点的世界位置
      return getPosition();
  }
}

// ------------------------------------------------------------------------
// 重写 onDraw 以处理坐标空间
// ------------------------------------------------------------------------
void Widget::onDraw(RenderBackend &renderer) {
  // 根据坐标空间调整渲染
  if (coordinateSpace_ == CoordinateSpace::Screen) {
    // 屏幕空间：临时修改位置为屏幕位置，保持锚点不变
    Vec2 worldPos = getPosition();
    const_cast<Widget*>(this)->setPosition(screenPosition_);

    // 调用子类的绘制
    onDrawWidget(renderer);

    // 恢复原始位置
    const_cast<Widget*>(this)->setPosition(worldPos);
  } else if (coordinateSpace_ == CoordinateSpace::Camera) {
    // 相机空间：计算相对于相机的位置
    Scene *scene = getScene();
    if (scene) {
      Camera *camera = scene->getActiveCamera();
      if (camera) {
        Vec2 worldPos = getPosition();
        Vec2 cameraRelativePos = camera->getPosition() + cameraOffset_;
        const_cast<Widget*>(this)->setPosition(cameraRelativePos);

        // 调用子类的绘制
        onDrawWidget(renderer);

        // 恢复原始位置
        const_cast<Widget*>(this)->setPosition(worldPos);
        return;
      }
    }
    // 如果没有场景或相机，按世界空间处理
    onDrawWidget(renderer);
  } else {
    // 世界空间：正常渲染
    onDrawWidget(renderer);
  }
}

} // namespace extra2d
