#include <easy2d/platform/input.h>
#include <easy2d/event/input_codes.h>
#include <easy2d/utils/logger.h>

namespace easy2d {

Input::Input()
    : controller_(nullptr)
    , leftStickX_(0.0f)
    , leftStickY_(0.0f)
    , rightStickX_(0.0f)
    , rightStickY_(0.0f)
    , touching_(false)
    , prevTouching_(false)
    , touchCount_(0) {
    buttonsDown_.fill(false);
    prevButtonsDown_.fill(false);
}

Input::~Input() {
    shutdown();
}

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
        E2D_LOG_WARN("No game controller found, input may be limited");
    }
}

void Input::shutdown() {
    if (controller_) {
        SDL_GameControllerClose(controller_);
        controller_ = nullptr;
    }
}

void Input::update() {
    // 保存上一帧状态
    prevButtonsDown_ = buttonsDown_;
    prevTouching_ = touching_;
    prevTouchPosition_ = touchPosition_;

    if (controller_) {
        // 更新按钮状态
        for (int i = 0; i < MAX_BUTTONS; ++i) {
            buttonsDown_[i] = SDL_GameControllerGetButton(
                controller_,
                static_cast<SDL_GameControllerButton>(i)) != 0;
        }

        // 读取摇杆（归一化到 -1.0 ~ 1.0）
        leftStickX_ = static_cast<float>(SDL_GameControllerGetAxis(
                          controller_, SDL_CONTROLLER_AXIS_LEFTX)) /
                      32767.0f;
        leftStickY_ = static_cast<float>(SDL_GameControllerGetAxis(
                          controller_, SDL_CONTROLLER_AXIS_LEFTY)) /
                      32767.0f;
        rightStickX_ = static_cast<float>(SDL_GameControllerGetAxis(
                           controller_, SDL_CONTROLLER_AXIS_RIGHTX)) /
                       32767.0f;
        rightStickY_ = static_cast<float>(SDL_GameControllerGetAxis(
                           controller_, SDL_CONTROLLER_AXIS_RIGHTY)) /
                       32767.0f;
    } else {
        buttonsDown_.fill(false);
        leftStickX_ = leftStickY_ = rightStickX_ = rightStickY_ = 0.0f;
    }

    // 更新触摸屏（SDL2 Touch API）
    SDL_TouchID touchId = SDL_GetTouchDevice(0);
    if (touchId != 0) {
        touchCount_ = SDL_GetNumTouchFingers(touchId);
        if (touchCount_ > 0) {
            SDL_Finger* finger = SDL_GetTouchFinger(touchId, 0);
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
}

// ============================================================================
// 键盘输入映射到 SDL GameController 按钮
// ============================================================================

SDL_GameControllerButton Input::mapKeyToButton(int keyCode) const {
    switch (keyCode) {
        // 方向键 → DPad
        case Key::Up:    return SDL_CONTROLLER_BUTTON_DPAD_UP;
        case Key::Down:  return SDL_CONTROLLER_BUTTON_DPAD_DOWN;
        case Key::Left:  return SDL_CONTROLLER_BUTTON_DPAD_LEFT;
        case Key::Right: return SDL_CONTROLLER_BUTTON_DPAD_RIGHT;

        // WASD → 也映射到 DPad
        case Key::W: return SDL_CONTROLLER_BUTTON_DPAD_UP;
        case Key::S: return SDL_CONTROLLER_BUTTON_DPAD_DOWN;
        case Key::A: return SDL_CONTROLLER_BUTTON_DPAD_LEFT;
        case Key::D: return SDL_CONTROLLER_BUTTON_DPAD_RIGHT;

        // 常用键 → 手柄按钮
        case Key::Z:      return SDL_CONTROLLER_BUTTON_B;       // 确认
        case Key::X:      return SDL_CONTROLLER_BUTTON_A;       // 取消
        case Key::C:      return SDL_CONTROLLER_BUTTON_Y;
        case Key::V:      return SDL_CONTROLLER_BUTTON_X;
        case Key::Space:  return SDL_CONTROLLER_BUTTON_A;       // 空格 = A
        case Key::Enter:  return SDL_CONTROLLER_BUTTON_A;       // 回车 = A
        case Key::Escape: return SDL_CONTROLLER_BUTTON_START;   // ESC = Start

        // 肩键
        case Key::Q: return SDL_CONTROLLER_BUTTON_LEFTSHOULDER;
        case Key::E: return SDL_CONTROLLER_BUTTON_RIGHTSHOULDER;

        // Start/Select
        case Key::Tab:       return SDL_CONTROLLER_BUTTON_BACK;
        case Key::Backspace: return SDL_CONTROLLER_BUTTON_START;

        default: return SDL_CONTROLLER_BUTTON_INVALID;
    }
}

bool Input::isKeyDown(int keyCode) const {
    SDL_GameControllerButton button = mapKeyToButton(keyCode);
    if (button == SDL_CONTROLLER_BUTTON_INVALID) return false;
    return buttonsDown_[button];
}

bool Input::isKeyPressed(int keyCode) const {
    SDL_GameControllerButton button = mapKeyToButton(keyCode);
    if (button == SDL_CONTROLLER_BUTTON_INVALID) return false;
    return buttonsDown_[button] && !prevButtonsDown_[button];
}

bool Input::isKeyReleased(int keyCode) const {
    SDL_GameControllerButton button = mapKeyToButton(keyCode);
    if (button == SDL_CONTROLLER_BUTTON_INVALID) return false;
    return !buttonsDown_[button] && prevButtonsDown_[button];
}

// ============================================================================
// 手柄按钮直接访问
// ============================================================================

bool Input::isButtonDown(int button) const {
    if (button < 0 || button >= MAX_BUTTONS) return false;
    return buttonsDown_[button];
}

bool Input::isButtonPressed(int button) const {
    if (button < 0 || button >= MAX_BUTTONS) return false;
    return buttonsDown_[button] && !prevButtonsDown_[button];
}

bool Input::isButtonReleased(int button) const {
    if (button < 0 || button >= MAX_BUTTONS) return false;
    return !buttonsDown_[button] && prevButtonsDown_[button];
}

Vec2 Input::getLeftStick() const {
    return Vec2(leftStickX_, leftStickY_);
}

Vec2 Input::getRightStick() const {
    return Vec2(rightStickX_, rightStickY_);
}

// ============================================================================
// 鼠标输入映射到触摸屏
// ============================================================================

bool Input::isMouseDown(MouseButton button) const {
    if (button == MouseButton::Left) {
        return touching_;
    }
    // A 键映射为右键
    if (button == MouseButton::Right) {
        return buttonsDown_[SDL_CONTROLLER_BUTTON_A];
    }
    return false;
}

bool Input::isMousePressed(MouseButton button) const {
    if (button == MouseButton::Left) {
        return touching_ && !prevTouching_;
    }
    if (button == MouseButton::Right) {
        return buttonsDown_[SDL_CONTROLLER_BUTTON_A] &&
               !prevButtonsDown_[SDL_CONTROLLER_BUTTON_A];
    }
    return false;
}

bool Input::isMouseReleased(MouseButton button) const {
    if (button == MouseButton::Left) {
        return !touching_ && prevTouching_;
    }
    if (button == MouseButton::Right) {
        return !buttonsDown_[SDL_CONTROLLER_BUTTON_A] &&
               prevButtonsDown_[SDL_CONTROLLER_BUTTON_A];
    }
    return false;
}

Vec2 Input::getMousePosition() const {
    return touchPosition_;
}

Vec2 Input::getMouseDelta() const {
    if (touching_ && prevTouching_) {
        return touchPosition_ - prevTouchPosition_;
    }
    return Vec2::Zero();
}

void Input::setMousePosition(const Vec2& /*position*/) {
    // 不支持在 Switch 上设置触摸位置
}

void Input::setMouseVisible(bool /*visible*/) {
    // Switch 无鼠标光标
}

void Input::setMouseLocked(bool /*locked*/) {
    // Switch 无鼠标光标
}

bool Input::isAnyKeyDown() const {
    for (int i = 0; i < MAX_BUTTONS; ++i) {
        if (buttonsDown_[i]) return true;
    }
    return false;
}

bool Input::isAnyMouseDown() const {
    return touching_;
}

} // namespace easy2d
