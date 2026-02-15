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
    E2D_LOG_INFO("SDL2Input initialized");
    
    if (SDL_Init(SDL_INIT_GAMECONTROLLER) != 0) {
        E2D_LOG_WARN("Failed to init gamecontroller subsystem: {}", SDL_GetError());
    }
    
    openGamepad();
}

void SDL2Input::shutdown() {
    closeGamepad();
    E2D_LOG_INFO("SDL2Input shutdown");
}

void SDL2Input::update() {
    keyPrevious_ = keyCurrent_;
    mousePrevious_ = mouseCurrent_;
    gamepadPrevious_ = gamepadCurrent_;
    
    scrollDelta_ = 0.0f;
    mouseDelta_ = Vec2{0.0f, 0.0f};
    
    updateGamepad();
}

void SDL2Input::setEventCallback(EventCallback callback) {
    eventCallback_ = std::move(callback);
}

void SDL2Input::handleSDLEvent(const SDL_Event& event) {
    switch (event.type) {
        case SDL_KEYDOWN: {
            int key = event.key.keysym.scancode;
            if (key >= 0 && key < static_cast<int>(Key::Count)) {
                if (!keyCurrent_[key]) {
                    keyCurrent_[key] = true;
                    
                    Event e = Event::createKeyPress(
                        event.key.keysym.sym,
                        event.key.keysym.scancode,
                        event.key.keysym.mod
                    );
                    dispatchEvent(e);
                }
            }
            break;
        }
        
        case SDL_KEYUP: {
            int key = event.key.keysym.scancode;
            if (key >= 0 && key < static_cast<int>(Key::Count)) {
                keyCurrent_[key] = false;
                
                Event e = Event::createKeyRelease(
                    event.key.keysym.sym,
                    event.key.keysym.scancode,
                    event.key.keysym.mod
                );
                dispatchEvent(e);
            }
            break;
        }
        
        case SDL_MOUSEBUTTONDOWN: {
            int btn = event.button.button - 1;
            if (btn >= 0 && btn < static_cast<int>(Mouse::Count)) {
                mouseCurrent_[btn] = true;
                
                Vec2 pos{static_cast<float>(event.button.x), 
                        static_cast<float>(event.button.y)};
                Event e = Event::createMouseButtonPress(btn, 0, pos);
                dispatchEvent(e);
            }
            break;
        }
        
        case SDL_MOUSEBUTTONUP: {
            int btn = event.button.button - 1;
            if (btn >= 0 && btn < static_cast<int>(Mouse::Count)) {
                mouseCurrent_[btn] = false;
                
                Vec2 pos{static_cast<float>(event.button.x), 
                        static_cast<float>(event.button.y)};
                Event e = Event::createMouseButtonRelease(btn, 0, pos);
                dispatchEvent(e);
            }
            break;
        }
        
        case SDL_MOUSEMOTION: {
            Vec2 newPos{static_cast<float>(event.motion.x), 
                       static_cast<float>(event.motion.y)};
            Vec2 delta{static_cast<float>(event.motion.xrel), 
                      static_cast<float>(event.motion.yrel)};
            
            mouseDelta_ = mouseDelta_ + delta;
            mousePos_ = newPos;
            
            Event e = Event::createMouseMove(newPos, delta);
            dispatchEvent(e);
            break;
        }
        
        case SDL_MOUSEWHEEL: {
            Vec2 offset{static_cast<float>(event.wheel.x), 
                       static_cast<float>(event.wheel.y)};
            Vec2 pos = mousePos_;
            
            scroll_ += event.wheel.y;
            scrollDelta_ += event.wheel.y;
            
            Event e = Event::createMouseScroll(offset, pos);
            dispatchEvent(e);
            break;
        }
        
        case SDL_CONTROLLERDEVICEADDED:
            E2D_LOG_INFO("Gamepad connected: index {}", event.cdevice.which);
            openGamepad();
            break;
            
        case SDL_CONTROLLERDEVICEREMOVED:
            if (gamepad_ && event.cdevice.which == SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(gamepad_))) {
                E2D_LOG_INFO("Gamepad disconnected");
                closeGamepad();
            }
            break;
            
        case SDL_CONTROLLERBUTTONDOWN:
            if (gamepad_) {
                int btn = event.cbutton.button;
                if (btn >= 0 && btn < static_cast<int>(Gamepad::Count)) {
                    gamepadCurrent_[btn] = true;
                    
                    GamepadButtonEvent btnEvent;
                    btnEvent.gamepadId = gamepadIndex_;
                    btnEvent.button = btn;
                    
                    Event e;
                    e.type = EventType::GamepadButtonPressed;
                    e.data = btnEvent;
                    dispatchEvent(e);
                }
            }
            break;
            
        case SDL_CONTROLLERBUTTONUP:
            if (gamepad_) {
                int btn = event.cbutton.button;
                if (btn >= 0 && btn < static_cast<int>(Gamepad::Count)) {
                    gamepadCurrent_[btn] = false;
                    
                    GamepadButtonEvent btnEvent;
                    btnEvent.gamepadId = gamepadIndex_;
                    btnEvent.button = btn;
                    
                    Event e;
                    e.type = EventType::GamepadButtonReleased;
                    e.data = btnEvent;
                    dispatchEvent(e);
                }
            }
            break;
            
        default:
            break;
    }
}

void SDL2Input::dispatchEvent(const Event& event) {
    if (eventCallback_) {
        eventCallback_(event);
    }
}

bool SDL2Input::down(Key key) const {
    size_t idx = static_cast<size_t>(key);
    if (idx < keyCurrent_.size()) {
        return keyCurrent_[idx];
    }
    return false;
}

bool SDL2Input::pressed(Key key) const {
    size_t idx = static_cast<size_t>(key);
    if (idx < keyCurrent_.size()) {
        return keyCurrent_[idx] && !keyPrevious_[idx];
    }
    return false;
}

bool SDL2Input::released(Key key) const {
    size_t idx = static_cast<size_t>(key);
    if (idx < keyCurrent_.size()) {
        return !keyCurrent_[idx] && keyPrevious_[idx];
    }
    return false;
}

bool SDL2Input::down(Mouse btn) const {
    size_t idx = static_cast<size_t>(btn);
    if (idx < mouseCurrent_.size()) {
        return mouseCurrent_[idx];
    }
    return false;
}

bool SDL2Input::pressed(Mouse btn) const {
    size_t idx = static_cast<size_t>(btn);
    if (idx < mouseCurrent_.size()) {
        return mouseCurrent_[idx] && !mousePrevious_[idx];
    }
    return false;
}

bool SDL2Input::released(Mouse btn) const {
    size_t idx = static_cast<size_t>(btn);
    if (idx < mouseCurrent_.size()) {
        return !mouseCurrent_[idx] && mousePrevious_[idx];
    }
    return false;
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
    if (idx < gamepadCurrent_.size()) {
        return gamepadCurrent_[idx];
    }
    return false;
}

bool SDL2Input::pressed(Gamepad btn) const {
    size_t idx = static_cast<size_t>(btn);
    if (idx < gamepadCurrent_.size()) {
        return gamepadCurrent_[idx] && !gamepadPrevious_[idx];
    }
    return false;
}

bool SDL2Input::released(Gamepad btn) const {
    size_t idx = static_cast<size_t>(btn);
    if (idx < gamepadCurrent_.size()) {
        return !gamepadCurrent_[idx] && gamepadPrevious_[idx];
    }
    return false;
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
        Uint16 lowFreq = static_cast<Uint16>(left * 65535.0f);
        Uint16 highFreq = static_cast<Uint16>(right * 65535.0f);
        SDL_GameControllerRumble(gamepad_, lowFreq, highFreq, 500);
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
    return Vec2{0.0f, 0.0f};
}

TouchPoint SDL2Input::touchPoint(int index) const {
    (void)index;
    return TouchPoint{};
}

void SDL2Input::updateKeyboard() {
}

void SDL2Input::updateMouse() {
    int x = 0, y = 0;
    SDL_GetMouseState(&x, &y);
    mousePos_ = Vec2{static_cast<float>(x), static_cast<float>(y)};
}

void SDL2Input::updateGamepad() {
    if (!gamepad_) {
        return;
    }
    
    auto applyDeadzone = [this](float value) -> float {
        if (std::abs(value) < deadzone_) {
            return 0.0f;
        }
        float sign = value >= 0.0f ? 1.0f : -1.0f;
        return sign * (std::abs(value) - deadzone_) / (1.0f - deadzone_);
    };
    
    int lx = SDL_GameControllerGetAxis(gamepad_, SDL_CONTROLLER_AXIS_LEFTX);
    int ly = SDL_GameControllerGetAxis(gamepad_, SDL_CONTROLLER_AXIS_LEFTY);
    int rx = SDL_GameControllerGetAxis(gamepad_, SDL_CONTROLLER_AXIS_RIGHTX);
    int ry = SDL_GameControllerGetAxis(gamepad_, SDL_CONTROLLER_AXIS_RIGHTY);
    
    leftStick_.x = applyDeadzone(lx / 32767.0f);
    leftStick_.y = applyDeadzone(ly / 32767.0f);
    rightStick_.x = applyDeadzone(rx / 32767.0f);
    rightStick_.y = applyDeadzone(ry / 32767.0f);
    
    int lt = SDL_GameControllerGetAxis(gamepad_, SDL_CONTROLLER_AXIS_TRIGGERLEFT);
    int rt = SDL_GameControllerGetAxis(gamepad_, SDL_CONTROLLER_AXIS_TRIGGERRIGHT);
    
    leftTrigger_ = lt / 32767.0f;
    rightTrigger_ = rt / 32767.0f;
}

void SDL2Input::openGamepad() {
    int numJoysticks = SDL_NumJoysticks();
    for (int i = 0; i < numJoysticks; ++i) {
        if (SDL_IsGameController(i)) {
            gamepad_ = SDL_GameControllerOpen(i);
            if (gamepad_) {
                gamepadIndex_ = i;
                E2D_LOG_INFO("Gamepad opened: {}", SDL_GameControllerName(gamepad_));
                return;
            }
        }
    }
}

void SDL2Input::closeGamepad() {
    if (gamepad_) {
        SDL_GameControllerClose(gamepad_);
        gamepad_ = nullptr;
        gamepadIndex_ = -1;
        gamepadCurrent_.fill(false);
        gamepadPrevious_.fill(false);
    }
}

int SDL2Input::keyToSDL(Key key) {
    return static_cast<int>(key);
}

int SDL2Input::mouseToSDL(Mouse btn) {
    switch (btn) {
        case Mouse::Left: return SDL_BUTTON_LEFT;
        case Mouse::Middle: return SDL_BUTTON_MIDDLE;
        case Mouse::Right: return SDL_BUTTON_RIGHT;
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
        case Gamepad::Back: return SDL_CONTROLLER_BUTTON_BACK;
        case Gamepad::Start: return SDL_CONTROLLER_BUTTON_START;
        case Gamepad::LStick: return SDL_CONTROLLER_BUTTON_LEFTSTICK;
        case Gamepad::RStick: return SDL_CONTROLLER_BUTTON_RIGHTSTICK;
        case Gamepad::LB: return SDL_CONTROLLER_BUTTON_LEFTSHOULDER;
        case Gamepad::RB: return SDL_CONTROLLER_BUTTON_RIGHTSHOULDER;
        case Gamepad::DUp: return SDL_CONTROLLER_BUTTON_DPAD_UP;
        case Gamepad::DDown: return SDL_CONTROLLER_BUTTON_DPAD_DOWN;
        case Gamepad::DLeft: return SDL_CONTROLLER_BUTTON_DPAD_LEFT;
        case Gamepad::DRight: return SDL_CONTROLLER_BUTTON_DPAD_RIGHT;
        case Gamepad::Guide: return SDL_CONTROLLER_BUTTON_GUIDE;
        default: return 0;
    }
}

Key SDL2Input::sdlToKey(int sdlKey) {
    if (sdlKey >= 0 && sdlKey < static_cast<int>(Key::Count)) {
        return static_cast<Key>(sdlKey);
    }
    return Key::None;
}

Mouse SDL2Input::sdlToMouse(int sdlButton) {
    switch (sdlButton) {
        case SDL_BUTTON_LEFT: return Mouse::Left;
        case SDL_BUTTON_MIDDLE: return Mouse::Middle;
        case SDL_BUTTON_RIGHT: return Mouse::Right;
        case SDL_BUTTON_X1: return Mouse::X1;
        case SDL_BUTTON_X2: return Mouse::X2;
        default: return Mouse::Count;
    }
}

} 
