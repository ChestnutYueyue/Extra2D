#include <extra2d/event/input_codes.h>
#include <extra2d/graphics/viewport_adapter.h>
#include <extra2d/platform/input.h>
#include <extra2d/utils/logger.h>

namespace extra2d {

Input::Input()
    : controller_(nullptr), 
      leftStickX_(0.0f), leftStickY_(0.0f),
      rightStickX_(0.0f), rightStickY_(0.0f),
      mouseScroll_(0.0f), prevMouseScroll_(0.0f),
      touching_(false), prevTouching_(false), touchCount_(0),
      viewportAdapter_(nullptr) {
  
  // 初始化所有状态数组
  keysDown_.fill(false);
  prevKeysDown_.fill(false);
  buttonsDown_.fill(false);
  prevButtonsDown_.fill(false);
  mouseButtonsDown_.fill(false);
  prevMouseButtonsDown_.fill(false);
}

Input::~Input() { shutdown(); }

void Input::init() {
  // 打开第一个可用的游戏控制器
  for (int i = 0; i < SDL_NumJoysticks(); ++i) {
    if (SDL_IsGameController(i)) {
      controller_ = SDL_GameControllerOpen(i);
      if (controller_) {
        E2D_LOG_INFO("GameController opened: {}",
                     SDL_GameControllerName(controller_));
        break;
      }
    }
  }

  if (!controller_) {
    E2D_LOG_WARN("No game controller found");
  }

  // PC 端获取初始鼠标状态
#ifndef PLATFORM_SWITCH
  int mouseX, mouseY;
  SDL_GetMouseState(&mouseX, &mouseY);
  mousePosition_ = Vec2(static_cast<float>(mouseX), static_cast<float>(mouseY));
  prevMousePosition_ = mousePosition_;
#endif
}

void Input::shutdown() {
  if (controller_) {
    SDL_GameControllerClose(controller_);
    controller_ = nullptr;
  }
}

void Input::update() {
  // 保存上一帧状态
  prevKeysDown_ = keysDown_;
  prevButtonsDown_ = buttonsDown_;
  prevMouseButtonsDown_ = mouseButtonsDown_;
  prevMousePosition_ = mousePosition_;
  prevMouseScroll_ = mouseScroll_;
  prevTouching_ = touching_;
  prevTouchPosition_ = touchPosition_;

  // 更新各输入设备状态
  updateKeyboard();
  updateMouse();
  updateGamepad();
  updateTouch();
}

void Input::updateKeyboard() {
  // 获取当前键盘状态
  const Uint8* state = SDL_GetKeyboardState(nullptr);
  for (int i = 0; i < MAX_KEYS; ++i) {
    keysDown_[i] = state[i] != 0;
  }
}

void Input::updateMouse() {
#ifndef PLATFORM_SWITCH
  // 更新鼠标位置
  int mouseX, mouseY;
  Uint32 buttonState = SDL_GetMouseState(&mouseX, &mouseY);
  mousePosition_ = Vec2(static_cast<float>(mouseX), static_cast<float>(mouseY));

  // 更新鼠标按钮状态
  mouseButtonsDown_[0] = (buttonState & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;
  mouseButtonsDown_[1] = (buttonState & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0;
  mouseButtonsDown_[2] = (buttonState & SDL_BUTTON(SDL_BUTTON_MIDDLE)) != 0;
  mouseButtonsDown_[3] = (buttonState & SDL_BUTTON(SDL_BUTTON_X1)) != 0;
  mouseButtonsDown_[4] = (buttonState & SDL_BUTTON(SDL_BUTTON_X2)) != 0;

  // 处理鼠标滚轮事件（需要在事件循环中处理，这里简化处理）
  // 实际滚轮值通过 SDL_MOUSEWHEEL 事件更新
#endif
}

void Input::updateGamepad() {
  if (controller_) {
    // 更新按钮状态
    for (int i = 0; i < MAX_BUTTONS; ++i) {
      buttonsDown_[i] =
          SDL_GameControllerGetButton(
              controller_, static_cast<SDL_GameControllerButton>(i)) != 0;
    }

    // 读取摇杆（归一化到 -1.0 ~ 1.0）
    leftStickX_ = static_cast<float>(SDL_GameControllerGetAxis(
                      controller_, SDL_CONTROLLER_AXIS_LEFTX)) / 32767.0f;
    leftStickY_ = static_cast<float>(SDL_GameControllerGetAxis(
                      controller_, SDL_CONTROLLER_AXIS_LEFTY)) / 32767.0f;
    rightStickX_ = static_cast<float>(SDL_GameControllerGetAxis(
                       controller_, SDL_CONTROLLER_AXIS_RIGHTX)) / 32767.0f;
    rightStickY_ = static_cast<float>(SDL_GameControllerGetAxis(
                       controller_, SDL_CONTROLLER_AXIS_RIGHTY)) / 32767.0f;
  } else {
    buttonsDown_.fill(false);
    leftStickX_ = leftStickY_ = rightStickX_ = rightStickY_ = 0.0f;
  }
}

void Input::updateTouch() {
#ifdef PLATFORM_SWITCH
  // Switch 原生触摸屏支持
  SDL_TouchID touchId = SDL_GetTouchDevice(0);
  if (touchId != 0) {
    touchCount_ = SDL_GetNumTouchFingers(touchId);
    if (touchCount_ > 0) {
      SDL_Finger *finger = SDL_GetTouchFinger(touchId, 0);
      if (finger) {
        touching_ = true;
        // SDL 触摸坐标是归一化 0.0~1.0，转换为像素坐标
        touchPosition_ = Vec2(finger->x * 1280.0f, finger->y * 720.0f);
      } else {
        touching_ = false;
      }
    } else {
      touching_ = false;
    }
  } else {
    touchCount_ = 0;
    touching_ = false;
  }
#else
  // PC 端：触摸屏可选支持（如果有触摸设备）
  SDL_TouchID touchId = SDL_GetTouchDevice(0);
  if (touchId != 0) {
    touchCount_ = SDL_GetNumTouchFingers(touchId);
    if (touchCount_ > 0) {
      SDL_Finger *finger = SDL_GetTouchFinger(touchId, 0);
      if (finger) {
        touching_ = true;
        // PC 端需要根据窗口大小转换坐标
        int windowWidth, windowHeight;
        SDL_Window* window = SDL_GL_GetCurrentWindow();
        if (window) {
          SDL_GetWindowSize(window, &windowWidth, &windowHeight);
          touchPosition_ = Vec2(finger->x * windowWidth, finger->y * windowHeight);
        } else {
          touchPosition_ = Vec2(finger->x * 1280.0f, finger->y * 720.0f);
        }
      } else {
        touching_ = false;
      }
    } else {
      touching_ = false;
    }
  } else {
    touchCount_ = 0;
    touching_ = false;
  }
#endif
}

// ============================================================================
// 键盘输入
// ============================================================================

SDL_GameControllerButton Input::mapKeyToButton(int keyCode) const {
  switch (keyCode) {
  // 方向键 → DPad
  case Key::Up:
    return SDL_CONTROLLER_BUTTON_DPAD_UP;
  case Key::Down:
    return SDL_CONTROLLER_BUTTON_DPAD_DOWN;
  case Key::Left:
    return SDL_CONTROLLER_BUTTON_DPAD_LEFT;
  case Key::Right:
    return SDL_CONTROLLER_BUTTON_DPAD_RIGHT;

  // WASD → 也映射到 DPad
  case Key::W:
    return SDL_CONTROLLER_BUTTON_DPAD_UP;
  case Key::S:
    return SDL_CONTROLLER_BUTTON_DPAD_DOWN;
  case Key::A:
    return SDL_CONTROLLER_BUTTON_DPAD_LEFT;
  case Key::D:
    return SDL_CONTROLLER_BUTTON_DPAD_RIGHT;

  // 常用键 → 手柄按钮
  case Key::Z:
    return SDL_CONTROLLER_BUTTON_B;
  case Key::X:
    return SDL_CONTROLLER_BUTTON_A;
  case Key::C:
    return SDL_CONTROLLER_BUTTON_Y;
  case Key::V:
    return SDL_CONTROLLER_BUTTON_X;
  case Key::Space:
    return SDL_CONTROLLER_BUTTON_A;
  case Key::Enter:
    return SDL_CONTROLLER_BUTTON_A;
  case Key::Escape:
    return SDL_CONTROLLER_BUTTON_START;

  // 肩键
  case Key::Q:
    return SDL_CONTROLLER_BUTTON_LEFTSHOULDER;
  case Key::E:
    return SDL_CONTROLLER_BUTTON_RIGHTSHOULDER;

  // Start/Select
  case Key::Tab:
    return SDL_CONTROLLER_BUTTON_BACK;
  case Key::Backspace:
    return SDL_CONTROLLER_BUTTON_START;

  default:
    return SDL_CONTROLLER_BUTTON_INVALID;
  }
}

bool Input::isKeyDown(int keyCode) const {
#ifdef PLATFORM_SWITCH
  // Switch: 映射到手柄按钮
  SDL_GameControllerButton button = mapKeyToButton(keyCode);
  if (button == SDL_CONTROLLER_BUTTON_INVALID)
    return false;
  return buttonsDown_[button];
#else
  // PC: 直接使用键盘扫描码
  SDL_Scancode scancode = SDL_GetScancodeFromKey(keyCode);
  if (scancode >= 0 && scancode < MAX_KEYS) {
    return keysDown_[scancode];
  }
  return false;
#endif
}

bool Input::isKeyPressed(int keyCode) const {
#ifdef PLATFORM_SWITCH
  SDL_GameControllerButton button = mapKeyToButton(keyCode);
  if (button == SDL_CONTROLLER_BUTTON_INVALID)
    return false;
  return buttonsDown_[button] && !prevButtonsDown_[button];
#else
  SDL_Scancode scancode = SDL_GetScancodeFromKey(keyCode);
  if (scancode >= 0 && scancode < MAX_KEYS) {
    return keysDown_[scancode] && !prevKeysDown_[scancode];
  }
  return false;
#endif
}

bool Input::isKeyReleased(int keyCode) const {
#ifdef PLATFORM_SWITCH
  SDL_GameControllerButton button = mapKeyToButton(keyCode);
  if (button == SDL_CONTROLLER_BUTTON_INVALID)
    return false;
  return !buttonsDown_[button] && prevButtonsDown_[button];
#else
  SDL_Scancode scancode = SDL_GetScancodeFromKey(keyCode);
  if (scancode >= 0 && scancode < MAX_KEYS) {
    return !keysDown_[scancode] && prevKeysDown_[scancode];
  }
  return false;
#endif
}

// ============================================================================
// 手柄按钮
// ============================================================================

bool Input::isButtonDown(int button) const {
  if (button < 0 || button >= MAX_BUTTONS)
    return false;
  return buttonsDown_[button];
}

bool Input::isButtonPressed(int button) const {
  if (button < 0 || button >= MAX_BUTTONS)
    return false;
  return buttonsDown_[button] && !prevButtonsDown_[button];
}

bool Input::isButtonReleased(int button) const {
  if (button < 0 || button >= MAX_BUTTONS)
    return false;
  return !buttonsDown_[button] && prevButtonsDown_[button];
}

Vec2 Input::getLeftStick() const { return Vec2(leftStickX_, leftStickY_); }

Vec2 Input::getRightStick() const { return Vec2(rightStickX_, rightStickY_); }

// ============================================================================
// 鼠标输入
// ============================================================================

bool Input::isMouseDown(MouseButton button) const {
  int index = static_cast<int>(button);
  if (index < 0 || index >= 8)
    return false;

#ifdef PLATFORM_SWITCH
  // Switch: 左键映射到触摸，右键映射到 A 键
  if (button == MouseButton::Left) {
    return touching_;
  }
  if (button == MouseButton::Right) {
    return buttonsDown_[SDL_CONTROLLER_BUTTON_A];
  }
  return false;
#else
  // PC: 直接使用鼠标按钮
  return mouseButtonsDown_[index];
#endif
}

bool Input::isMousePressed(MouseButton button) const {
  int index = static_cast<int>(button);
  if (index < 0 || index >= 8)
    return false;

#ifdef PLATFORM_SWITCH
  if (button == MouseButton::Left) {
    return touching_ && !prevTouching_;
  }
  if (button == MouseButton::Right) {
    return buttonsDown_[SDL_CONTROLLER_BUTTON_A] &&
           !prevButtonsDown_[SDL_CONTROLLER_BUTTON_A];
  }
  return false;
#else
  return mouseButtonsDown_[index] && !prevMouseButtonsDown_[index];
#endif
}

bool Input::isMouseReleased(MouseButton button) const {
  int index = static_cast<int>(button);
  if (index < 0 || index >= 8)
    return false;

#ifdef PLATFORM_SWITCH
  if (button == MouseButton::Left) {
    return !touching_ && prevTouching_;
  }
  if (button == MouseButton::Right) {
    return !buttonsDown_[SDL_CONTROLLER_BUTTON_A] &&
           prevButtonsDown_[SDL_CONTROLLER_BUTTON_A];
  }
  return false;
#else
  return !mouseButtonsDown_[index] && prevMouseButtonsDown_[index];
#endif
}

Vec2 Input::getMousePosition() const {
#ifdef PLATFORM_SWITCH
  return touchPosition_;
#else
  return mousePosition_;
#endif
}

Vec2 Input::getMouseDelta() const {
#ifdef PLATFORM_SWITCH
  if (touching_ && prevTouching_) {
    return touchPosition_ - prevTouchPosition_;
  }
  return Vec2::Zero();
#else
  return mousePosition_ - prevMousePosition_;
#endif
}

void Input::setMousePosition(const Vec2 &position) {
#ifndef PLATFORM_SWITCH
  SDL_WarpMouseInWindow(SDL_GL_GetCurrentWindow(), 
                        static_cast<int>(position.x), 
                        static_cast<int>(position.y));
#else
  (void)position;
#endif
}

void Input::setMouseVisible(bool visible) {
#ifndef PLATFORM_SWITCH
  SDL_ShowCursor(visible ? SDL_ENABLE : SDL_DISABLE);
#else
  (void)visible;
#endif
}

void Input::setMouseLocked(bool locked) {
#ifndef PLATFORM_SWITCH
  SDL_SetRelativeMouseMode(locked ? SDL_TRUE : SDL_FALSE);
#else
  (void)locked;
#endif
}

// ============================================================================
// 便捷方法
// ============================================================================

bool Input::isAnyKeyDown() const {
#ifdef PLATFORM_SWITCH
  for (int i = 0; i < MAX_BUTTONS; ++i) {
    if (buttonsDown_[i])
      return true;
  }
#else
  for (int i = 0; i < MAX_KEYS; ++i) {
    if (keysDown_[i])
      return true;
  }
#endif
  return false;
}

bool Input::isAnyMouseDown() const {
#ifdef PLATFORM_SWITCH
  return touching_;
#else
  for (int i = 0; i < 8; ++i) {
    if (mouseButtonsDown_[i])
      return true;
  }
  return false;
#endif
}

// ============================================================================
// 视口适配器
// ============================================================================

void Input::setViewportAdapter(ViewportAdapter* adapter) {
  viewportAdapter_ = adapter;
}

Vec2 Input::getMousePositionLogic() const {
  Vec2 screenPos = getMousePosition();
  if (viewportAdapter_) {
    return viewportAdapter_->screenToLogic(screenPos);
  }
  return screenPos;
}

Vec2 Input::getTouchPositionLogic() const {
  Vec2 screenPos = getTouchPosition();
  if (viewportAdapter_) {
    return viewportAdapter_->screenToLogic(screenPos);
  }
  return screenPos;
}

Vec2 Input::getMouseDeltaLogic() const {
  Vec2 delta = getMouseDelta();
  if (viewportAdapter_) {
    float scale = viewportAdapter_->getUniformScale();
    if (scale > 0.0f) {
      return delta / scale;
    }
  }
  return delta;
}

} // namespace extra2d
