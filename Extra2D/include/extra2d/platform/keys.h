#pragma once

namespace extra2d {

/**
 * @brief 键盘按键码
 */
enum class Key : int {
    None = 0,
    A, B, C, D, E, F, G, H, I, J, K, L, M,
    N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
    Num0, Num1, Num2, Num3, Num4,
    Num5, Num6, Num7, Num8, Num9,
    F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
    Space, Enter, Escape, Tab, Backspace,
    Insert, Delete, Home, End, PageUp, PageDown,
    Up, Down, Left, Right,
    LShift, RShift, LCtrl, RCtrl, LAlt, RAlt,
    CapsLock, NumLock, ScrollLock,
    Count
};

/**
 * @brief 鼠标按钮
 */
enum class Mouse : int {
    Left = 0,
    Right,
    Middle,
    X1,
    X2,
    Count
};

/**
 * @brief 游戏手柄按钮
 */
enum class Gamepad : int {
    A = 0,
    B,
    X,
    Y,
    LB,
    RB,
    LT,
    RT,
    Back,
    Start,
    Guide,
    LStick,
    RStick,
    DUp,
    DDown,
    DLeft,
    DRight,
    Count
};

/**
 * @brief 手柄轴
 */
enum class GamepadAxis : int {
    LeftX = 0,
    LeftY,
    RightX,
    RightY,
    LeftTrigger,
    RightTrigger,
    Count
};

} // namespace extra2d
