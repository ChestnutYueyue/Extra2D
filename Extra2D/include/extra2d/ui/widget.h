#pragma once

#include <extra2d/core/math_types.h>
#include <extra2d/core/types.h>
#include <extra2d/platform/input.h>
#include <extra2d/scene/node.h>

namespace extra2d {

// ============================================================================
// 鼠标事件结构
// ============================================================================
struct MouseEvent {
  MouseButton button;
  float x;
  float y;
  int mods; // 修饰键状态
};

// ============================================================================
// 坐标空间枚举 - 定义 UI 组件的渲染坐标空间
// ============================================================================
enum class CoordinateSpace {
  Screen,   // 屏幕空间 - 固定位置，不随相机移动
  World,    // 世界空间 - 随相机移动（默认行为）
  Camera,   // 相机空间 - 相对于相机位置的偏移
};

// ============================================================================
// Widget 基类 - UI 组件的基础
// ============================================================================
class Widget : public Node {
public:
  Widget();
  ~Widget() override = default;

  void setSize(const Size &size);
  void setSize(float width, float height);
  Size getSize() const { return size_; }

  Rect getBoundingBox() const override;

  // ------------------------------------------------------------------------
  // 坐标空间设置
  // ------------------------------------------------------------------------
  void setCoordinateSpace(CoordinateSpace space);
  CoordinateSpace getCoordinateSpace() const { return coordinateSpace_; }

  // ------------------------------------------------------------------------
  // 屏幕空间位置设置（仅在 Screen 空间下有效）
  // ------------------------------------------------------------------------
  void setScreenPosition(const Vec2 &pos);
  void setScreenPosition(float x, float y);
  Vec2 getScreenPosition() const { return screenPosition_; }

  // ------------------------------------------------------------------------
  // 相机空间偏移设置（仅在 Camera 空间下有效）
  // ------------------------------------------------------------------------
  void setCameraOffset(const Vec2 &offset);
  void setCameraOffset(float x, float y);
  Vec2 getCameraOffset() const { return cameraOffset_; }

  // ------------------------------------------------------------------------
  // 鼠标事件处理（子类可重写）
  // ------------------------------------------------------------------------
  virtual bool onMousePress(const MouseEvent &event) { return false; }
  virtual bool onMouseRelease(const MouseEvent &event) { return false; }
  virtual bool onMouseMove(const MouseEvent &event) { return false; }
  virtual void onMouseEnter() {}
  virtual void onMouseLeave() {}

  // ------------------------------------------------------------------------
  // 启用/禁用状态
  // ------------------------------------------------------------------------
  void setEnabled(bool enabled) { enabled_ = enabled; }
  bool isEnabled() const { return enabled_; }

  // ------------------------------------------------------------------------
  // 焦点状态
  // ------------------------------------------------------------------------
  void setFocused(bool focused) { focused_ = focused; }
  bool isFocused() const { return focused_; }

protected:
  // 供子类使用的辅助方法
  bool isPointInside(float x, float y) const {
    return getBoundingBox().containsPoint(Point(x, y));
  }

  // 获取实际渲染位置（根据坐标空间计算）
  Vec2 getRenderPosition() const;

  // 子类重写此方法以支持自定义渲染
  virtual void onDrawWidget(RenderBackend &renderer) {}

  // 重写 Node 的 onDraw 以处理坐标空间
  void onDraw(RenderBackend &renderer) override;

private:
  Size size_ = Size::Zero();
  bool enabled_ = true;
  bool focused_ = false;
  bool hovered_ = false;

  // 坐标空间相关
  CoordinateSpace coordinateSpace_ = CoordinateSpace::World;
  Vec2 screenPosition_ = Vec2::Zero();   // 屏幕空间位置
  Vec2 cameraOffset_ = Vec2::Zero();     // 相机空间偏移
};

} // namespace extra2d
