#pragma once

#include <easy2d/core/types.h>
#include <easy2d/core/math_types.h>
#include <array>

#include <SDL.h>

namespace easy2d {

// ============================================================================
// 鼠标按钮枚举 (保留接口兼容性)
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
// Input 类 - SDL2 GameController + Touch 输入管理
// ============================================================================
class Input {
public:
    Input();
    ~Input();

    // 初始化 (使用 SDL2 GameController API)
    void init();
    void shutdown();

    // 每帧更新
    void update();

    // ------------------------------------------------------------------------
    // 键盘输入 (映射到手柄按钮)
    // ------------------------------------------------------------------------
    bool isKeyDown(int keyCode) const;
    bool isKeyPressed(int keyCode) const;
    bool isKeyReleased(int keyCode) const;

    // ------------------------------------------------------------------------
    // 手柄按钮 (通过 SDL_GameController)
    // ------------------------------------------------------------------------
    bool isButtonDown(int button) const;
    bool isButtonPressed(int button) const;
    bool isButtonReleased(int button) const;

    // 摇杆
    Vec2 getLeftStick() const;
    Vec2 getRightStick() const;

    // ------------------------------------------------------------------------
    // 鼠标输入 (映射到触摸屏)
    // ------------------------------------------------------------------------
    bool isMouseDown(MouseButton button) const;
    bool isMousePressed(MouseButton button) const;
    bool isMouseReleased(MouseButton button) const;

    Vec2 getMousePosition() const;
    Vec2 getMouseDelta() const;
    float getMouseScroll() const { return 0.0f; }
    float getMouseScrollDelta() const { return 0.0f; }

    void setMousePosition(const Vec2& position);
    void setMouseVisible(bool visible);
    void setMouseLocked(bool locked);

    // ------------------------------------------------------------------------
    // 触摸屏
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

    SDL_GameController* controller_;

    // 手柄按钮状态
    std::array<bool, MAX_BUTTONS> buttonsDown_;
    std::array<bool, MAX_BUTTONS> prevButtonsDown_;

    // 摇杆状态
    float leftStickX_;
    float leftStickY_;
    float rightStickX_;
    float rightStickY_;

    // 触摸屏状态
    bool touching_;
    bool prevTouching_;
    Vec2 touchPosition_;
    Vec2 prevTouchPosition_;
    int touchCount_;

    // 映射键盘 keyCode 到 SDL GameController 按钮
    SDL_GameControllerButton mapKeyToButton(int keyCode) const;
};

} // namespace easy2d
