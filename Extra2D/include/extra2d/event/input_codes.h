#pragma once

// SDL2 键码定义
#include <SDL2/SDL.h>

namespace extra2d {

// ============================================================================
// 键盘按键码 (基于 SDL2)
// ============================================================================
namespace Key {
enum : int {
  Unknown = SDLK_UNKNOWN,
  Space = SDLK_SPACE,
  Apostrophe = SDLK_QUOTE,
  Comma = SDLK_COMMA,
  Minus = SDLK_MINUS,
  Period = SDLK_PERIOD,
  Slash = SDLK_SLASH,
  Num0 = SDLK_0,
  Num1 = SDLK_1,
  Num2 = SDLK_2,
  Num3 = SDLK_3,
  Num4 = SDLK_4,
  Num5 = SDLK_5,
  Num6 = SDLK_6,
  Num7 = SDLK_7,
  Num8 = SDLK_8,
  Num9 = SDLK_9,
  Semicolon = SDLK_SEMICOLON,
  Equal = SDLK_EQUALS,
  A = SDLK_a,
  B = SDLK_b,
  C = SDLK_c,
  D = SDLK_d,
  E = SDLK_e,
  F = SDLK_f,
  G = SDLK_g,
  H = SDLK_h,
  I = SDLK_i,
  J = SDLK_j,
  K = SDLK_k,
  L = SDLK_l,
  M = SDLK_m,
  N = SDLK_n,
  O = SDLK_o,
  P = SDLK_p,
  Q = SDLK_q,
  R = SDLK_r,
  S = SDLK_s,
  T = SDLK_t,
  U = SDLK_u,
  V = SDLK_v,
  W = SDLK_w,
  X = SDLK_x,
  Y = SDLK_y,
  Z = SDLK_z,
  LeftBracket = SDLK_LEFTBRACKET,
  Backslash = SDLK_BACKSLASH,
  RightBracket = SDLK_RIGHTBRACKET,
  GraveAccent = SDLK_BACKQUOTE,
  Escape = SDLK_ESCAPE,
  Enter = SDLK_RETURN,
  Tab = SDLK_TAB,
  Backspace = SDLK_BACKSPACE,
  Insert = SDLK_INSERT,
  Delete = SDLK_DELETE,
  Right = SDLK_RIGHT,
  Left = SDLK_LEFT,
  Down = SDLK_DOWN,
  Up = SDLK_UP,
  PageUp = SDLK_PAGEUP,
  PageDown = SDLK_PAGEDOWN,
  Home = SDLK_HOME,
  End = SDLK_END,
  CapsLock = SDLK_CAPSLOCK,
  ScrollLock = SDLK_SCROLLLOCK,
  NumLock = SDLK_NUMLOCKCLEAR,
  PrintScreen = SDLK_PRINTSCREEN,
  Pause = SDLK_PAUSE,
  F1 = SDLK_F1,
  F2 = SDLK_F2,
  F3 = SDLK_F3,
  F4 = SDLK_F4,
  F5 = SDLK_F5,
  F6 = SDLK_F6,
  F7 = SDLK_F7,
  F8 = SDLK_F8,
  F9 = SDLK_F9,
  F10 = SDLK_F10,
  F11 = SDLK_F11,
  F12 = SDLK_F12,
  F13 = SDLK_F13,
  F14 = SDLK_F14,
  F15 = SDLK_F15,
  F16 = SDLK_F16,
  F17 = SDLK_F17,
  F18 = SDLK_F18,
  F19 = SDLK_F19,
  F20 = SDLK_F20,
  F21 = SDLK_F21,
  F22 = SDLK_F22,
  F23 = SDLK_F23,
  F24 = SDLK_F24,
  KP0 = SDLK_KP_0,
  KP1 = SDLK_KP_1,
  KP2 = SDLK_KP_2,
  KP3 = SDLK_KP_3,
  KP4 = SDLK_KP_4,
  KP5 = SDLK_KP_5,
  KP6 = SDLK_KP_6,
  KP7 = SDLK_KP_7,
  KP8 = SDLK_KP_8,
  KP9 = SDLK_KP_9,
  KPDecimal = SDLK_KP_PERIOD,
  KPDivide = SDLK_KP_DIVIDE,
  KPMultiply = SDLK_KP_MULTIPLY,
  KPSubtract = SDLK_KP_MINUS,
  KPAdd = SDLK_KP_PLUS,
  KPEnter = SDLK_KP_ENTER,
  KPEqual = SDLK_KP_EQUALS,
  LeftShift = SDLK_LSHIFT,
  LeftControl = SDLK_LCTRL,
  LeftAlt = SDLK_LALT,
  LeftSuper = SDLK_LGUI,
  RightShift = SDLK_RSHIFT,
  RightControl = SDLK_RCTRL,
  RightAlt = SDLK_RALT,
  RightSuper = SDLK_RGUI,
  Menu = SDLK_MENU,
  Last = SDLK_MENU
};
}

// ============================================================================
// 修饰键
// ============================================================================
namespace Mod {
enum : int {
  Shift = KMOD_SHIFT,
  Control = KMOD_CTRL,
  Alt = KMOD_ALT,
  Super = KMOD_GUI,
  CapsLock = KMOD_CAPS,
  NumLock = KMOD_NUM
};
}

// ============================================================================
// 鼠标按键码
// ============================================================================
namespace Mouse {
enum : int {
  Button1 = 0,
  Button2 = 1,
  Button3 = 2,
  Button4 = 3,
  Button5 = 4,
  Button6 = 5,
  Button7 = 6,
  Button8 = 7,
  ButtonLast = Button8,
  ButtonLeft = Button1,
  ButtonRight = Button2,
  ButtonMiddle = Button3
};
}

// ============================================================================
// 游戏手柄按键
// ============================================================================
namespace GamepadButton {
enum : int {
  A = SDL_CONTROLLER_BUTTON_A,
  B = SDL_CONTROLLER_BUTTON_B,
  X = SDL_CONTROLLER_BUTTON_X,
  Y = SDL_CONTROLLER_BUTTON_Y,
  LeftBumper = SDL_CONTROLLER_BUTTON_LEFTSHOULDER,
  RightBumper = SDL_CONTROLLER_BUTTON_RIGHTSHOULDER,
  Back = SDL_CONTROLLER_BUTTON_BACK,
  Start = SDL_CONTROLLER_BUTTON_START,
  Guide = SDL_CONTROLLER_BUTTON_GUIDE,
  LeftThumb = SDL_CONTROLLER_BUTTON_LEFTSTICK,
  RightThumb = SDL_CONTROLLER_BUTTON_RIGHTSTICK,
  DPadUp = SDL_CONTROLLER_BUTTON_DPAD_UP,
  DPadRight = SDL_CONTROLLER_BUTTON_DPAD_RIGHT,
  DPadDown = SDL_CONTROLLER_BUTTON_DPAD_DOWN,
  DPadLeft = SDL_CONTROLLER_BUTTON_DPAD_LEFT,
  Last = SDL_CONTROLLER_BUTTON_DPAD_LEFT,
  Cross = A,
  Circle = B,
  Square = X,
  Triangle = Y
};
}

// ============================================================================
// 游戏手柄轴
// ============================================================================
namespace GamepadAxis {
enum : int {
  LeftX = SDL_CONTROLLER_AXIS_LEFTX,
  LeftY = SDL_CONTROLLER_AXIS_LEFTY,
  RightX = SDL_CONTROLLER_AXIS_RIGHTX,
  RightY = SDL_CONTROLLER_AXIS_RIGHTY,
  LeftTrigger = SDL_CONTROLLER_AXIS_TRIGGERLEFT,
  RightTrigger = SDL_CONTROLLER_AXIS_TRIGGERRIGHT,
  Last = SDL_CONTROLLER_AXIS_TRIGGERRIGHT
};
}

} // namespace extra2d
