#include <extra2d/event/input_codes.h>
#include <extra2d/graphics/viewport_adapter.h>
#include <extra2d/platform/input.h>
#include <extra2d/utils/logger.h>

namespace extra2d {

/**
 * @brief 默认构造函数
 *
 * 初始化输入系统的所有成员变量为默认值，包括控制器指针、摇杆状态、
 * 鼠标状态、触摸状态等，并将所有按键和按钮状态数组初始化为false。
 */
Input::Input()
    : controller_(nullptr), 
      leftStickX_(0.0f), leftStickY_(0.0f),
      rightStickX_(0.0f), rightStickY_(0.0f),
      mouseScroll_(0.0f), prevMouseScroll_(0.0f),
      touching_(false), prevTouching_(false), touchCount_(0),
      viewportAdapter_(nullptr) {
  
  keysDown_.fill(false);
  prevKeysDown_.fill(false);
  buttonsDown_.fill(false);
  prevButtonsDown_.fill(false);
  mouseButtonsDown_.fill(false);
  prevMouseButtonsDown_.fill(false);
}

/**
 * @brief 析构函数
 *
 * 自动调用shutdown()方法释放所有资源。
 */
Input::~Input() { shutdown(); }

/**
 * @brief 初始化输入系统
 *
 * 打开第一个可用的游戏控制器，并在PC端获取初始鼠标位置。
 */
void Input::init() {
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

#ifndef PLATFORM_SWITCH
  int mouseX, mouseY;
  SDL_GetMouseState(&mouseX, &mouseY);
  mousePosition_ = Vec2(static_cast<float>(mouseX), static_cast<float>(mouseY));
  prevMousePosition_ = mousePosition_;
#endif
}

/**
 * @brief 关闭输入系统
 *
 * 关闭并释放游戏控制器资源。
 */
void Input::shutdown() {
  if (controller_) {
    SDL_GameControllerClose(controller_);
    controller_ = nullptr;
  }
}

/**
 * @brief 更新输入状态
 *
 * 保存上一帧的输入状态，并更新键盘、鼠标、手柄和触摸设备的当前状态。
 */
void Input::update() {
  prevKeysDown_ = keysDown_;
  prevButtonsDown_ = buttonsDown_;
  prevMouseButtonsDown_ = mouseButtonsDown_;
  prevMousePosition_ = mousePosition_;
  prevMouseScroll_ = mouseScroll_;
  prevTouching_ = touching_;
  prevTouchPosition_ = touchPosition_;

  updateKeyboard();
  updateMouse();
  updateGamepad();
  updateTouch();
}

/**
 * @brief 更新键盘状态
 *
 * 从SDL获取当前键盘状态并更新按键数组。
 */
void Input::updateKeyboard() {
  const Uint8* state = SDL_GetKeyboardState(nullptr);
  for (int i = 0; i < MAX_KEYS; ++i) {
    keysDown_[i] = state[i] != 0;
  }
}

/**
 * @brief 更新鼠标状态
 *
 * 获取当前鼠标位置和按钮状态。仅在非Switch平台执行。
 */
void Input::updateMouse() {
#ifndef PLATFORM_SWITCH
  int mouseX, mouseY;
  Uint32 buttonState = SDL_GetMouseState(&mouseX, &mouseY);
  mousePosition_ = Vec2(static_cast<float>(mouseX), static_cast<float>(mouseY));

  mouseButtonsDown_[0] = (buttonState & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;
  mouseButtonsDown_[1] = (buttonState & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0;
  mouseButtonsDown_[2] = (buttonState & SDL_BUTTON(SDL_BUTTON_MIDDLE)) != 0;
  mouseButtonsDown_[3] = (buttonState & SDL_BUTTON(SDL_BUTTON_X1)) != 0;
  mouseButtonsDown_[4] = (buttonState & SDL_BUTTON(SDL_BUTTON_X2)) != 0;
#endif
}

/**
 * @brief 更新手柄状态
 *
 * 读取手柄按钮状态和摇杆位置，摇杆值归一化到-1.0~1.0范围。
 */
void Input::updateGamepad() {
  if (controller_) {
    for (int i = 0; i < MAX_BUTTONS; ++i) {
      buttonsDown_[i] =
          SDL_GameControllerGetButton(
              controller_, static_cast<SDL_GameControllerButton>(i)) != 0;
    }

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

/**
 * @brief 更新触摸状态
 *
 * 获取触摸设备状态，将归一化坐标转换为像素坐标。
 * Switch平台使用原生触摸屏支持，PC端支持可选触摸设备。
 */
void Input::updateTouch() {
#ifdef PLATFORM_SWITCH
  SDL_TouchID touchId = SDL_GetTouchDevice(0);
  if (touchId != 0) {
    touchCount_ = SDL_GetNumTouchFingers(touchId);
    if (touchCount_ > 0) {
      SDL_Finger *finger = SDL_GetTouchFinger(touchId, 0);
      if (finger) {
        touching_ = true;
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
  SDL_TouchID touchId = SDL_GetTouchDevice(0);
  if (touchId != 0) {
    touchCount_ = SDL_GetNumTouchFingers(touchId);
    if (touchCount_ > 0) {
      SDL_Finger *finger = SDL_GetTouchFinger(touchId, 0);
      if (finger) {
        touching_ = true;
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

/**
 * @brief 将键盘按键映射到手柄按钮
 *
 * 提供键盘到手柄按钮的映射，用于在Switch平台模拟键盘输入。
 *
 * @param keyCode 键盘按键码
 * @return 对应的SDL手柄按钮枚举值
 */
SDL_GameControllerButton Input::mapKeyToButton(int keyCode) const {
  switch (keyCode) {
  case Key::Up:
    return SDL_CONTROLLER_BUTTON_DPAD_UP;
  case Key::Down:
    return SDL_CONTROLLER_BUTTON_DPAD_DOWN;
  case Key::Left:
    return SDL_CONTROLLER_BUTTON_DPAD_LEFT;
  case Key::Right:
    return SDL_CONTROLLER_BUTTON_DPAD_RIGHT;

  case Key::W:
    return SDL_CONTROLLER_BUTTON_DPAD_UP;
  case Key::S:
    return SDL_CONTROLLER_BUTTON_DPAD_DOWN;
  case Key::A:
    return SDL_CONTROLLER_BUTTON_DPAD_LEFT;
  case Key::D:
    return SDL_CONTROLLER_BUTTON_DPAD_RIGHT;

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

  case Key::Q:
    return SDL_CONTROLLER_BUTTON_LEFTSHOULDER;
  case Key::E:
    return SDL_CONTROLLER_BUTTON_RIGHTSHOULDER;

  case Key::Tab:
    return SDL_CONTROLLER_BUTTON_BACK;
  case Key::Backspace:
    return SDL_CONTROLLER_BUTTON_START;

  default:
    return SDL_CONTROLLER_BUTTON_INVALID;
  }
}

/**
 * @brief 检查按键是否被按下
 *
 * 检测指定按键当前是否处于按下状态。
 * Switch平台映射到手柄按钮，PC端直接读取键盘状态。
 *
 * @param keyCode 键盘按键码
 * @return 按键按下返回true，否则返回false
 */
bool Input::isKeyDown(int keyCode) const {
#ifdef PLATFORM_SWITCH
  SDL_GameControllerButton button = mapKeyToButton(keyCode);
  if (button == SDL_CONTROLLER_BUTTON_INVALID)
    return false;
  return buttonsDown_[button];
#else
  SDL_Scancode scancode = SDL_GetScancodeFromKey(keyCode);
  if (scancode >= 0 && scancode < MAX_KEYS) {
    return keysDown_[scancode];
  }
  return false;
#endif
}

/**
 * @brief 检查按键是否刚被按下
 *
 * 检测指定按键是否在本帧刚被按下（之前未按下，当前按下）。
 *
 * @param keyCode 键盘按键码
 * @return 按键刚按下返回true，否则返回false
 */
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

/**
 * @brief 检查按键是否刚被释放
 *
 * 检测指定按键是否在本帧刚被释放（之前按下，当前未按下）。
 *
 * @param keyCode 键盘按键码
 * @return 按键刚释放返回true，否则返回false
 */
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

/**
 * @brief 检查手柄按钮是否被按下
 *
 * @param button 手柄按钮索引
 * @return 按钮按下返回true，否则返回false
 */
bool Input::isButtonDown(int button) const {
  if (button < 0 || button >= MAX_BUTTONS)
    return false;
  return buttonsDown_[button];
}

/**
 * @brief 检查手柄按钮是否刚被按下
 *
 * @param button 手柄按钮索引
 * @return 按钮刚按下返回true，否则返回false
 */
bool Input::isButtonPressed(int button) const {
  if (button < 0 || button >= MAX_BUTTONS)
    return false;
  return buttonsDown_[button] && !prevButtonsDown_[button];
}

/**
 * @brief 检查手柄按钮是否刚被释放
 *
 * @param button 手柄按钮索引
 * @return 按钮刚释放返回true，否则返回false
 */
bool Input::isButtonReleased(int button) const {
  if (button < 0 || button >= MAX_BUTTONS)
    return false;
  return !buttonsDown_[button] && prevButtonsDown_[button];
}

/**
 * @brief 获取左摇杆位置
 *
 * @return 包含X和Y轴值的二维向量，范围-1.0~1.0
 */
Vec2 Input::getLeftStick() const { return Vec2(leftStickX_, leftStickY_); }

/**
 * @brief 获取右摇杆位置
 *
 * @return 包含X和Y轴值的二维向量，范围-1.0~1.0
 */
Vec2 Input::getRightStick() const { return Vec2(rightStickX_, rightStickY_); }

// ============================================================================
// 鼠标输入
// ============================================================================

/**
 * @brief 检查鼠标按钮是否被按下
 *
 * Switch平台左键映射到触摸，右键映射到A键。
 *
 * @param button 鼠标按钮枚举值
 * @return 按钮按下返回true，否则返回false
 */
bool Input::isMouseDown(MouseButton button) const {
  int index = static_cast<int>(button);
  if (index < 0 || index >= 8)
    return false;

#ifdef PLATFORM_SWITCH
  if (button == MouseButton::Left) {
    return touching_;
  }
  if (button == MouseButton::Right) {
    return buttonsDown_[SDL_CONTROLLER_BUTTON_A];
  }
  return false;
#else
  return mouseButtonsDown_[index];
#endif
}

/**
 * @brief 检查鼠标按钮是否刚被按下
 *
 * @param button 鼠标按钮枚举值
 * @return 按钮刚按下返回true，否则返回false
 */
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

/**
 * @brief 检查鼠标按钮是否刚被释放
 *
 * @param button 鼠标按钮枚举值
 * @return 按钮刚释放返回true，否则返回false
 */
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

/**
 * @brief 获取鼠标位置
 *
 * Switch平台返回触摸位置，PC端返回鼠标位置。
 *
 * @return 包含X和Y坐标的二维向量（屏幕像素坐标）
 */
Vec2 Input::getMousePosition() const {
#ifdef PLATFORM_SWITCH
  return touchPosition_;
#else
  return mousePosition_;
#endif
}

/**
 * @brief 获取鼠标移动增量
 *
 * 计算当前帧与上一帧之间的鼠标位置差值。
 *
 * @return 包含X和Y移动量的二维向量
 */
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

/**
 * @brief 设置鼠标位置
 *
 * 将鼠标移动到指定的屏幕坐标位置。仅在PC端有效。
 *
 * @param position 目标位置（屏幕像素坐标）
 */
void Input::setMousePosition(const Vec2 &position) {
#ifndef PLATFORM_SWITCH
  SDL_WarpMouseInWindow(SDL_GL_GetCurrentWindow(), 
                        static_cast<int>(position.x), 
                        static_cast<int>(position.y));
#else
  (void)position;
#endif
}

/**
 * @brief 设置鼠标光标可见性
 *
 * 显示或隐藏鼠标光标。仅在PC端有效。
 *
 * @param visible true显示光标，false隐藏光标
 */
void Input::setMouseVisible(bool visible) {
#ifndef PLATFORM_SWITCH
  SDL_ShowCursor(visible ? SDL_ENABLE : SDL_DISABLE);
#else
  (void)visible;
#endif
}

/**
 * @brief 设置鼠标锁定模式
 *
 * 启用或禁用相对鼠标模式，用于第一人称视角控制等场景。
 *
 * @param locked true锁定鼠标到窗口中心，false解锁
 */
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

/**
 * @brief 检查是否有任意按键被按下
 *
 * Switch平台检查手柄按钮，PC端检查键盘按键。
 *
 * @return 有按键按下返回true，否则返回false
 */
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

/**
 * @brief 检查是否有任意鼠标按钮被按下
 *
 * Switch平台检查触摸状态，PC端检查鼠标按钮。
 *
 * @return 有按钮按下返回true，否则返回false
 */
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

/**
 * @brief 设置视口适配器
 *
 * 用于将屏幕坐标转换为逻辑坐标。
 *
 * @param adapter 视口适配器指针
 */
void Input::setViewportAdapter(ViewportAdapter* adapter) {
  viewportAdapter_ = adapter;
}

/**
 * @brief 获取逻辑坐标系的鼠标位置
 *
 * 通过视口适配器将屏幕坐标转换为逻辑坐标。
 *
 * @return 逻辑坐标系的鼠标位置
 */
Vec2 Input::getMousePosLogic() const {
  Vec2 screenPos = getMousePosition();
  if (viewportAdapter_) {
    return viewportAdapter_->screenToLogic(screenPos);
  }
  return screenPos;
}

/**
 * @brief 获取逻辑坐标系的触摸位置
 *
 * 通过视口适配器将屏幕坐标转换为逻辑坐标。
 *
 * @return 逻辑坐标系的触摸位置
 */
Vec2 Input::getTouchPosLogic() const {
  Vec2 screenPos = getTouchPosition();
  if (viewportAdapter_) {
    return viewportAdapter_->screenToLogic(screenPos);
  }
  return screenPos;
}

/**
 * @brief 获取逻辑坐标系的鼠标移动增量
 *
 * 根据视口缩放比例调整鼠标移动量。
 *
 * @return 逻辑坐标系的鼠标移动增量
 */
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
