#include "sdl2_input.h"
#include <extra2d/utils/logger.h>
#include <cmath>

namespace extra2d {

SDL2Input::SDL2Input() {
    keyCurrent_.fill(false);
    keyPrevious_.fill(false);
    mouseCurrent_.fill(false);
    mousePrevious_.fill(false);
    gamepadCurrent_.fill(false);
    gamepadPrevious_.fill(false);
}

SDL2Input::~SDL2Input() {
    shutdown();
}

void SDL2Input::init() {
    SDL_GameControllerEventState(SDL_ENABLE);
    openGamepad();
    E2D_LOG_DEBUG("SDL2 input initialized");
}

void SDL2Input::shutdown() {
    closeGamepad();
}

void SDL2Input::update() {
    keyPrevious_ = keyCurrent_;
    mousePrevious_ = mouseCurrent_;
    gamepadPrevious_ = gamepadCurrent_;
    scrollDelta_ = 0.0f;
    mouseDelta_ = Vec2::Zero();

    updateKeyboard();
    updateMouse();
    updateGamepad();
}

bool SDL2Input::down(Key key) const {
    size_t idx = static_cast<size_t>(key);
    return idx < keyCurrent_.size() ? keyCurrent_[idx] : false;
}

bool SDL2Input::pressed(Key key) const {
    size_t idx = static_cast<size_t>(key);
    return idx < keyCurrent_.size() ? (keyCurrent_[idx] && !keyPrevious_[idx]) : false;
}

bool SDL2Input::released(Key key) const {
    size_t idx = static_cast<size_t>(key);
    return idx < keyCurrent_.size() ? (!keyCurrent_[idx] && keyPrevious_[idx]) : false;
}

bool SDL2Input::down(Mouse btn) const {
    size_t idx = static_cast<size_t>(btn);
    return idx < mouseCurrent_.size() ? mouseCurrent_[idx] : false;
}

bool SDL2Input::pressed(Mouse btn) const {
    size_t idx = static_cast<size_t>(btn);
    return idx < mouseCurrent_.size() ? (mouseCurrent_[idx] && !mousePrevious_[idx]) : false;
}

bool SDL2Input::released(Mouse btn) const {
    size_t idx = static_cast<size_t>(btn);
    return idx < mouseCurrent_.size() ? (!mouseCurrent_[idx] && mousePrevious_[idx]) : false;
}

Vec2 SDL2Input::mouse() const {
    return mousePos_;
}

Vec2 SDL2Input::mouseDelta() const {
    return mouseDelta_;
}

float SDL2Input::scroll() const {
    return scroll_;
}

float SDL2Input::scrollDelta() const {
    return scrollDelta_;
}

void SDL2Input::setMouse(const Vec2& pos) {
    SDL_WarpMouseInWindow(nullptr, static_cast<int>(pos.x), static_cast<int>(pos.y));
}

bool SDL2Input::gamepad() const {
    return gamepad_ != nullptr;
}

bool SDL2Input::down(Gamepad btn) const {
    size_t idx = static_cast<size_t>(btn);
    return idx < gamepadCurrent_.size() ? gamepadCurrent_[idx] : false;
}

bool SDL2Input::pressed(Gamepad btn) const {
    size_t idx = static_cast<size_t>(btn);
    return idx < gamepadCurrent_.size() ? (gamepadCurrent_[idx] && !gamepadPrevious_[idx]) : false;
}

bool SDL2Input::released(Gamepad btn) const {
    size_t idx = static_cast<size_t>(btn);
    return idx < gamepadCurrent_.size() ? (!gamepadCurrent_[idx] && gamepadPrevious_[idx]) : false;
}

Vec2 SDL2Input::leftStick() const {
    return leftStick_;
}

Vec2 SDL2Input::rightStick() const {
    return rightStick_;
}

float SDL2Input::leftTrigger() const {
    return leftTrigger_;
}

float SDL2Input::rightTrigger() const {
    return rightTrigger_;
}

void SDL2Input::vibrate(float left, float right) {
    if (gamepad_) {
        Uint16 lowFreq = static_cast<Uint16>(std::clamp(left, 0.0f, 1.0f) * 65535);
        Uint16 highFreq = static_cast<Uint16>(std::clamp(right, 0.0f, 1.0f) * 65535);
        SDL_GameControllerRumble(gamepad_, lowFreq, highFreq, 0);
    }
}

bool SDL2Input::touching() const {
    return false;
}

int SDL2Input::touchCount() const {
    return 0;
}

Vec2 SDL2Input::touch(int index) const {
    (void)index;
    return Vec2::Zero();
}

TouchPoint SDL2Input::touchPoint(int index) const {
    (void)index;
    return TouchPoint{};
}

void SDL2Input::updateKeyboard() {
    int numKeys = 0;
    const Uint8* state = SDL_GetKeyboardState(&numKeys);

    auto updateKey = [&](Key key, int sdlScancode) {
        size_t idx = static_cast<size_t>(key);
        if (idx < keyCurrent_.size() && sdlScancode < numKeys) {
            keyCurrent_[idx] = state[sdlScancode] != 0;
        }
    };

    updateKey(Key::A, SDL_SCANCODE_A);
    updateKey(Key::B, SDL_SCANCODE_B);
    updateKey(Key::C, SDL_SCANCODE_C);
    updateKey(Key::D, SDL_SCANCODE_D);
    updateKey(Key::E, SDL_SCANCODE_E);
    updateKey(Key::F, SDL_SCANCODE_F);
    updateKey(Key::G, SDL_SCANCODE_G);
    updateKey(Key::H, SDL_SCANCODE_H);
    updateKey(Key::I, SDL_SCANCODE_I);
    updateKey(Key::J, SDL_SCANCODE_J);
    updateKey(Key::K, SDL_SCANCODE_K);
    updateKey(Key::L, SDL_SCANCODE_L);
    updateKey(Key::M, SDL_SCANCODE_M);
    updateKey(Key::N, SDL_SCANCODE_N);
    updateKey(Key::O, SDL_SCANCODE_O);
    updateKey(Key::P, SDL_SCANCODE_P);
    updateKey(Key::Q, SDL_SCANCODE_Q);
    updateKey(Key::R, SDL_SCANCODE_R);
    updateKey(Key::S, SDL_SCANCODE_S);
    updateKey(Key::T, SDL_SCANCODE_T);
    updateKey(Key::U, SDL_SCANCODE_U);
    updateKey(Key::V, SDL_SCANCODE_V);
    updateKey(Key::W, SDL_SCANCODE_W);
    updateKey(Key::X, SDL_SCANCODE_X);
    updateKey(Key::Y, SDL_SCANCODE_Y);
    updateKey(Key::Z, SDL_SCANCODE_Z);
    updateKey(Key::Num0, SDL_SCANCODE_0);
    updateKey(Key::Num1, SDL_SCANCODE_1);
    updateKey(Key::Num2, SDL_SCANCODE_2);
    updateKey(Key::Num3, SDL_SCANCODE_3);
    updateKey(Key::Num4, SDL_SCANCODE_4);
    updateKey(Key::Num5, SDL_SCANCODE_5);
    updateKey(Key::Num6, SDL_SCANCODE_6);
    updateKey(Key::Num7, SDL_SCANCODE_7);
    updateKey(Key::Num8, SDL_SCANCODE_8);
    updateKey(Key::Num9, SDL_SCANCODE_9);
    updateKey(Key::F1, SDL_SCANCODE_F1);
    updateKey(Key::F2, SDL_SCANCODE_F2);
    updateKey(Key::F3, SDL_SCANCODE_F3);
    updateKey(Key::F4, SDL_SCANCODE_F4);
    updateKey(Key::F5, SDL_SCANCODE_F5);
    updateKey(Key::F6, SDL_SCANCODE_F6);
    updateKey(Key::F7, SDL_SCANCODE_F7);
    updateKey(Key::F8, SDL_SCANCODE_F8);
    updateKey(Key::F9, SDL_SCANCODE_F9);
    updateKey(Key::F10, SDL_SCANCODE_F10);
    updateKey(Key::F11, SDL_SCANCODE_F11);
    updateKey(Key::F12, SDL_SCANCODE_F12);
    updateKey(Key::Space, SDL_SCANCODE_SPACE);
    updateKey(Key::Enter, SDL_SCANCODE_RETURN);
    updateKey(Key::Escape, SDL_SCANCODE_ESCAPE);
    updateKey(Key::Tab, SDL_SCANCODE_TAB);
    updateKey(Key::Backspace, SDL_SCANCODE_BACKSPACE);
    updateKey(Key::Insert, SDL_SCANCODE_INSERT);
    updateKey(Key::Delete, SDL_SCANCODE_DELETE);
    updateKey(Key::Home, SDL_SCANCODE_HOME);
    updateKey(Key::End, SDL_SCANCODE_END);
    updateKey(Key::PageUp, SDL_SCANCODE_PAGEUP);
    updateKey(Key::PageDown, SDL_SCANCODE_PAGEDOWN);
    updateKey(Key::Up, SDL_SCANCODE_UP);
    updateKey(Key::Down, SDL_SCANCODE_DOWN);
    updateKey(Key::Left, SDL_SCANCODE_LEFT);
    updateKey(Key::Right, SDL_SCANCODE_RIGHT);
    updateKey(Key::LShift, SDL_SCANCODE_LSHIFT);
    updateKey(Key::RShift, SDL_SCANCODE_RSHIFT);
    updateKey(Key::LCtrl, SDL_SCANCODE_LCTRL);
    updateKey(Key::RCtrl, SDL_SCANCODE_RCTRL);
    updateKey(Key::LAlt, SDL_SCANCODE_LALT);
    updateKey(Key::RAlt, SDL_SCANCODE_RALT);
    updateKey(Key::CapsLock, SDL_SCANCODE_CAPSLOCK);
    updateKey(Key::NumLock, SDL_SCANCODE_NUMLOCKCLEAR);
    updateKey(Key::ScrollLock, SDL_SCANCODE_SCROLLLOCK);
}

void SDL2Input::updateMouse() {
    int x, y;
    Uint32 state = SDL_GetMouseState(&x, &y);

    Vec2 newPos(static_cast<float>(x), static_cast<float>(y));
    mouseDelta_ = newPos - mousePos_;
    mousePos_ = newPos;

    mouseCurrent_[static_cast<size_t>(Mouse::Left)] = (state & SDL_BUTTON_LMASK) != 0;
    mouseCurrent_[static_cast<size_t>(Mouse::Right)] = (state & SDL_BUTTON_RMASK) != 0;
    mouseCurrent_[static_cast<size_t>(Mouse::Middle)] = (state & SDL_BUTTON_MMASK) != 0;
    mouseCurrent_[static_cast<size_t>(Mouse::X1)] = (state & SDL_BUTTON_X1MASK) != 0;
    mouseCurrent_[static_cast<size_t>(Mouse::X2)] = (state & SDL_BUTTON_X2MASK) != 0;
}

void SDL2Input::updateGamepad() {
    if (!gamepad_) {
        openGamepad();
        if (!gamepad_) return;
    }

    auto applyDeadzone = [this](float value) -> float {
        if (std::abs(value) < deadzone_) return 0.0f;
        float sign = value >= 0 ? 1.0f : -1.0f;
        return sign * (std::abs(value) - deadzone_) / (1.0f - deadzone_);
    };

    auto getAxis = [this](SDL_GameControllerAxis axis) -> float {
        if (!gamepad_) return 0.0f;
        Sint16 value = SDL_GameControllerGetAxis(gamepad_, axis);
        return static_cast<float>(value) / 32767.0f;
    };

    auto getButton = [this](SDL_GameControllerButton btn) -> bool {
        return gamepad_ ? SDL_GameControllerGetButton(gamepad_, btn) != 0 : false;
    };

    leftStick_.x = applyDeadzone(getAxis(SDL_CONTROLLER_AXIS_LEFTX));
    leftStick_.y = applyDeadzone(getAxis(SDL_CONTROLLER_AXIS_LEFTY));
    rightStick_.x = applyDeadzone(getAxis(SDL_CONTROLLER_AXIS_RIGHTX));
    rightStick_.y = applyDeadzone(getAxis(SDL_CONTROLLER_AXIS_RIGHTY));

    leftTrigger_ = getAxis(SDL_CONTROLLER_AXIS_TRIGGERLEFT);
    rightTrigger_ = getAxis(SDL_CONTROLLER_AXIS_TRIGGERRIGHT);

    gamepadCurrent_[static_cast<size_t>(Gamepad::A)] = getButton(SDL_CONTROLLER_BUTTON_A);
    gamepadCurrent_[static_cast<size_t>(Gamepad::B)] = getButton(SDL_CONTROLLER_BUTTON_B);
    gamepadCurrent_[static_cast<size_t>(Gamepad::X)] = getButton(SDL_CONTROLLER_BUTTON_X);
    gamepadCurrent_[static_cast<size_t>(Gamepad::Y)] = getButton(SDL_CONTROLLER_BUTTON_Y);
    gamepadCurrent_[static_cast<size_t>(Gamepad::LB)] = getButton(SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
    gamepadCurrent_[static_cast<size_t>(Gamepad::RB)] = getButton(SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
    gamepadCurrent_[static_cast<size_t>(Gamepad::Back)] = getButton(SDL_CONTROLLER_BUTTON_BACK);
    gamepadCurrent_[static_cast<size_t>(Gamepad::Start)] = getButton(SDL_CONTROLLER_BUTTON_START);
    gamepadCurrent_[static_cast<size_t>(Gamepad::Guide)] = getButton(SDL_CONTROLLER_BUTTON_GUIDE);
    gamepadCurrent_[static_cast<size_t>(Gamepad::LStick)] = getButton(SDL_CONTROLLER_BUTTON_LEFTSTICK);
    gamepadCurrent_[static_cast<size_t>(Gamepad::RStick)] = getButton(SDL_CONTROLLER_BUTTON_RIGHTSTICK);
    gamepadCurrent_[static_cast<size_t>(Gamepad::DUp)] = getButton(SDL_CONTROLLER_BUTTON_DPAD_UP);
    gamepadCurrent_[static_cast<size_t>(Gamepad::DDown)] = getButton(SDL_CONTROLLER_BUTTON_DPAD_DOWN);
    gamepadCurrent_[static_cast<size_t>(Gamepad::DLeft)] = getButton(SDL_CONTROLLER_BUTTON_DPAD_LEFT);
    gamepadCurrent_[static_cast<size_t>(Gamepad::DRight)] = getButton(SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
}

void SDL2Input::openGamepad() {
    for (int i = 0; i < SDL_NumJoysticks(); ++i) {
        if (SDL_IsGameController(i)) {
            gamepad_ = SDL_GameControllerOpen(i);
            if (gamepad_) {
                gamepadIndex_ = i;
                E2D_LOG_INFO("Gamepad connected: {}", SDL_GameControllerName(gamepad_));
                break;
            }
        }
    }
}

void SDL2Input::closeGamepad() {
    if (gamepad_) {
        SDL_GameControllerClose(gamepad_);
        gamepad_ = nullptr;
        gamepadIndex_ = -1;
        E2D_LOG_INFO("Gamepad disconnected");
    }
}

int SDL2Input::keyToSDL(Key key) {
    switch (key) {
        case Key::A: return SDL_SCANCODE_A;
        case Key::B: return SDL_SCANCODE_B;
        case Key::C: return SDL_SCANCODE_C;
        case Key::D: return SDL_SCANCODE_D;
        case Key::E: return SDL_SCANCODE_E;
        case Key::F: return SDL_SCANCODE_F;
        case Key::G: return SDL_SCANCODE_G;
        case Key::H: return SDL_SCANCODE_H;
        case Key::I: return SDL_SCANCODE_I;
        case Key::J: return SDL_SCANCODE_J;
        case Key::K: return SDL_SCANCODE_K;
        case Key::L: return SDL_SCANCODE_L;
        case Key::M: return SDL_SCANCODE_M;
        case Key::N: return SDL_SCANCODE_N;
        case Key::O: return SDL_SCANCODE_O;
        case Key::P: return SDL_SCANCODE_P;
        case Key::Q: return SDL_SCANCODE_Q;
        case Key::R: return SDL_SCANCODE_R;
        case Key::S: return SDL_SCANCODE_S;
        case Key::T: return SDL_SCANCODE_T;
        case Key::U: return SDL_SCANCODE_U;
        case Key::V: return SDL_SCANCODE_V;
        case Key::W: return SDL_SCANCODE_W;
        case Key::X: return SDL_SCANCODE_X;
        case Key::Y: return SDL_SCANCODE_Y;
        case Key::Z: return SDL_SCANCODE_Z;
        default: return SDL_SCANCODE_UNKNOWN;
    }
}

int SDL2Input::mouseToSDL(Mouse btn) {
    switch (btn) {
        case Mouse::Left: return SDL_BUTTON_LEFT;
        case Mouse::Right: return SDL_BUTTON_RIGHT;
        case Mouse::Middle: return SDL_BUTTON_MIDDLE;
        case Mouse::X1: return SDL_BUTTON_X1;
        case Mouse::X2: return SDL_BUTTON_X2;
        default: return 0;
    }
}

int SDL2Input::gamepadToSDL(Gamepad btn) {
    switch (btn) {
        case Gamepad::A: return SDL_CONTROLLER_BUTTON_A;
        case Gamepad::B: return SDL_CONTROLLER_BUTTON_B;
        case Gamepad::X: return SDL_CONTROLLER_BUTTON_X;
        case Gamepad::Y: return SDL_CONTROLLER_BUTTON_Y;
        case Gamepad::LB: return SDL_CONTROLLER_BUTTON_LEFTSHOULDER;
        case Gamepad::RB: return SDL_CONTROLLER_BUTTON_RIGHTSHOULDER;
        case Gamepad::Back: return SDL_CONTROLLER_BUTTON_BACK;
        case Gamepad::Start: return SDL_CONTROLLER_BUTTON_START;
        case Gamepad::Guide: return SDL_CONTROLLER_BUTTON_GUIDE;
        case Gamepad::LStick: return SDL_CONTROLLER_BUTTON_LEFTSTICK;
        case Gamepad::RStick: return SDL_CONTROLLER_BUTTON_RIGHTSTICK;
        case Gamepad::DUp: return SDL_CONTROLLER_BUTTON_DPAD_UP;
        case Gamepad::DDown: return SDL_CONTROLLER_BUTTON_DPAD_DOWN;
        case Gamepad::DLeft: return SDL_CONTROLLER_BUTTON_DPAD_LEFT;
        case Gamepad::DRight: return SDL_CONTROLLER_BUTTON_DPAD_RIGHT;
        default: return SDL_CONTROLLER_BUTTON_INVALID;
    }
}

} // namespace extra2d
