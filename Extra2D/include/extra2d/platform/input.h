#pragma once

#include <array>
#include <extra2d/core/math_types.h>
#include <extra2d/core/types.h>
#include <extra2d/platform/platform_compat.h>
#include <extra2d/event/input_codes.h>

#include <SDL.h>

namespace extra2d {

// ============================================================================
// 鼠标按钮枚举
// ============================================================================
enum class MouseButton {
  Left = 0,
  Right = 1,
  Middle = 2,
  Button4 = 3,
  Button5 = 4,
  Button6 = 5,
  Button7 = 6,
  Button8 = 7,
  Count = 8
};

// ============================================================================
// Input 类 - 跨平台输入管理
// 支持: 键盘、鼠标、手柄、触摸屏
// ============================================================================
class Input {
public:
  Input();
  ~Input();

  // 初始化
  void init();
  void shutdown();

  // 每帧更新
  void update();

  // ------------------------------------------------------------------------
  // 键盘输入
  // ------------------------------------------------------------------------
  bool isKeyDown(int keyCode) const;
  bool isKeyPressed(int keyCode) const;
  bool isKeyReleased(int keyCode) const;

  // ------------------------------------------------------------------------
  // 手柄按钮
  // ------------------------------------------------------------------------
  bool isButtonDown(int button) const;
  bool isButtonPressed(int button) const;
  bool isButtonReleased(int button) const;

  // 摇杆
  Vec2 getLeftStick() const;
  Vec2 getRightStick() const;

  // ------------------------------------------------------------------------
  // 鼠标输入
  // ------------------------------------------------------------------------
  bool isMouseDown(MouseButton button) const;
  bool isMousePressed(MouseButton button) const;
  bool isMouseReleased(MouseButton button) const;

  Vec2 getMousePosition() const;
  Vec2 getMouseDelta() const;
  float getMouseScroll() const { return mouseScroll_; }
  float getMouseScrollDelta() const { return mouseScroll_ - prevMouseScroll_; }

  void setMousePosition(const Vec2 &position);
  void setMouseVisible(bool visible);
  void setMouseLocked(bool locked);

  // ------------------------------------------------------------------------
  // 触摸屏 (Switch 原生支持，PC 端模拟或禁用)
  // ------------------------------------------------------------------------
  bool isTouching() const { return touching_; }
  Vec2 getTouchPosition() const { return touchPosition_; }
  int getTouchCount() const { return touchCount_; }

  // ------------------------------------------------------------------------
  // 便捷方法
  // ------------------------------------------------------------------------
  bool isAnyKeyDown() const;
  bool isAnyMouseDown() const;

private:
  static constexpr int MAX_BUTTONS = SDL_CONTROLLER_BUTTON_MAX;
  static constexpr int MAX_KEYS = SDL_NUM_SCANCODES;

  SDL_GameController *controller_;

  // 键盘状态 (PC 端使用)
  std::array<bool, MAX_KEYS> keysDown_;
  std::array<bool, MAX_KEYS> prevKeysDown_;

  // 手柄按钮状态
  std::array<bool, MAX_BUTTONS> buttonsDown_;
  std::array<bool, MAX_BUTTONS> prevButtonsDown_;

  // 摇杆状态
  float leftStickX_;
  float leftStickY_;
  float rightStickX_;
  float rightStickY_;

  // 鼠标状态 (PC 端使用)
  Vec2 mousePosition_;
  Vec2 prevMousePosition_;
  float mouseScroll_;
  float prevMouseScroll_;
  std::array<bool, 8> mouseButtonsDown_;
  std::array<bool, 8> prevMouseButtonsDown_;

  // 触摸屏状态 (Switch 原生)
  bool touching_;
  bool prevTouching_;
  Vec2 touchPosition_;
  Vec2 prevTouchPosition_;
  int touchCount_;

  // 映射键盘 keyCode 到 SDL GameController 按钮 (Switch 兼容模式)
  SDL_GameControllerButton mapKeyToButton(int keyCode) const;

  // 更新键盘状态
  void updateKeyboard();

  // 更新鼠标状态
  void updateMouse();

  // 更新手柄状态
  void updateGamepad();

  // 更新触摸屏状态
  void updateTouch();
};

} // namespace extra2d
