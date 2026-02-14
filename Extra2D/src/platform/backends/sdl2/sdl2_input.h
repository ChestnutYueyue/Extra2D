#pragma once

#include <extra2d/platform/iinput.h>
#include <SDL.h>
#include <array>

namespace extra2d {

/**
 * @brief SDL2 输入实现
 */
class SDL2Input : public IInput {
public:
    SDL2Input();
    ~SDL2Input() override;

    void init() override;
    void shutdown() override;
    void update() override;

    bool down(Key key) const override;
    bool pressed(Key key) const override;
    bool released(Key key) const override;

    bool down(Mouse btn) const override;
    bool pressed(Mouse btn) const override;
    bool released(Mouse btn) const override;
    Vec2 mouse() const override;
    Vec2 mouseDelta() const override;
    float scroll() const override;
    float scrollDelta() const override;
    void setMouse(const Vec2& pos) override;

    bool gamepad() const override;
    bool down(Gamepad btn) const override;
    bool pressed(Gamepad btn) const override;
    bool released(Gamepad btn) const override;
    Vec2 leftStick() const override;
    Vec2 rightStick() const override;
    float leftTrigger() const override;
    float rightTrigger() const override;
    void vibrate(float left, float right) override;

    bool touching() const override;
    int touchCount() const override;
    Vec2 touch(int index) const override;
    TouchPoint touchPoint(int index) const override;

private:
    void updateKeyboard();
    void updateMouse();
    void updateGamepad();
    void openGamepad();
    void closeGamepad();

    static int keyToSDL(Key key);
    static int mouseToSDL(Mouse btn);
    static int gamepadToSDL(Gamepad btn);

    std::array<bool, static_cast<size_t>(Key::Count)> keyCurrent_{};
    std::array<bool, static_cast<size_t>(Key::Count)> keyPrevious_{};

    std::array<bool, static_cast<size_t>(Mouse::Count)> mouseCurrent_{};
    std::array<bool, static_cast<size_t>(Mouse::Count)> mousePrevious_{};

    Vec2 mousePos_;
    Vec2 mouseDelta_;
    float scroll_ = 0.0f;
    float scrollDelta_ = 0.0f;

    SDL_GameController* gamepad_ = nullptr;
    int gamepadIndex_ = -1;
    std::array<bool, static_cast<size_t>(Gamepad::Count)> gamepadCurrent_{};
    std::array<bool, static_cast<size_t>(Gamepad::Count)> gamepadPrevious_{};
    Vec2 leftStick_;
    Vec2 rightStick_;
    float leftTrigger_ = 0.0f;
    float rightTrigger_ = 0.0f;
    float deadzone_ = 0.15f;
};

} // namespace extra2d
