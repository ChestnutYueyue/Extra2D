#include <easy2d/script/sq_binding_input.h>
#include <easy2d/script/sq_binding_types.h>
#include <easy2d/platform/input.h>
#include <easy2d/event/input_codes.h>
#include <easy2d/app/application.h>

namespace easy2d {
namespace sq {

// ============================================================================
// Input singleton bindings
// ============================================================================
static SQInteger inputIsKeyDown(HSQUIRRELVM vm) {
    int key = static_cast<int>(getInt(vm, 2));
    push(vm, Application::instance().input().isKeyDown(key));
    return 1;
}

static SQInteger inputIsKeyPressed(HSQUIRRELVM vm) {
    int key = static_cast<int>(getInt(vm, 2));
    push(vm, Application::instance().input().isKeyPressed(key));
    return 1;
}

static SQInteger inputIsKeyReleased(HSQUIRRELVM vm) {
    int key = static_cast<int>(getInt(vm, 2));
    push(vm, Application::instance().input().isKeyReleased(key));
    return 1;
}

static SQInteger inputIsMouseDown(HSQUIRRELVM vm) {
    int btn = static_cast<int>(getInt(vm, 2));
    push(vm, Application::instance().input().isMouseDown(static_cast<MouseButton>(btn)));
    return 1;
}

static SQInteger inputIsMousePressed(HSQUIRRELVM vm) {
    int btn = static_cast<int>(getInt(vm, 2));
    push(vm, Application::instance().input().isMousePressed(static_cast<MouseButton>(btn)));
    return 1;
}

static SQInteger inputIsMouseReleased(HSQUIRRELVM vm) {
    int btn = static_cast<int>(getInt(vm, 2));
    push(vm, Application::instance().input().isMouseReleased(static_cast<MouseButton>(btn)));
    return 1;
}

static SQInteger inputGetMousePosition(HSQUIRRELVM vm) {
    pushValueInstance(vm, Application::instance().input().getMousePosition());
    return 1;
}

static SQInteger inputGetMouseDelta(HSQUIRRELVM vm) {
    pushValueInstance(vm, Application::instance().input().getMouseDelta());
    return 1;
}

static SQInteger inputGetMouseScroll(HSQUIRRELVM vm) {
    push(vm, Application::instance().input().getMouseScroll());
    return 1;
}

void registerInput(HSQUIRRELVM vm) {
    ClassDef(vm, "InputClass")
        .method("isKeyDown", inputIsKeyDown)
        .method("isKeyPressed", inputIsKeyPressed)
        .method("isKeyReleased", inputIsKeyReleased)
        .method("isMouseDown", inputIsMouseDown)
        .method("isMousePressed", inputIsMousePressed)
        .method("isMouseReleased", inputIsMouseReleased)
        .method("getMousePosition", inputGetMousePosition)
        .method("getMouseDelta", inputGetMouseDelta)
        .method("getMouseScroll", inputGetMouseScroll)
        .commit();

    // Global "Input" instance
    pushSingleton(vm, &Application::instance().input(), "InputClass");
    sq_pushroottable(vm);
    sq_pushstring(vm, "Input", -1);
    sq_push(vm, -3);
    sq_newslot(vm, -3, SQFalse);
    sq_pop(vm, 2);
}

void registerKeyConstants(HSQUIRRELVM vm) {
    // Key constants table
    static const char* const keyNames[] = {
        "Space", "Apostrophe", "Comma", "Minus", "Period", "Slash",
        "Num0", "Num1", "Num2", "Num3", "Num4", "Num5", "Num6", "Num7", "Num8", "Num9",
        "Semicolon", "Equal",
        "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M",
        "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z",
        "LeftBracket", "Backslash", "RightBracket", "GraveAccent",
        "Escape", "Enter", "Tab", "Backspace", "Insert", "Delete",
        "Right", "Left", "Down", "Up", "PageUp", "PageDown", "Home", "End",
        "CapsLock", "ScrollLock", "NumLock", "PrintScreen", "Pause",
        "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12",
        "LeftShift", "LeftControl", "LeftAlt", "LeftSuper",
        "RightShift", "RightControl", "RightAlt", "RightSuper", "Menu"
    };
    static const SQInteger keyValues[] = {
        Key::Space, Key::Apostrophe, Key::Comma, Key::Minus, Key::Period, Key::Slash,
        Key::Num0, Key::Num1, Key::Num2, Key::Num3, Key::Num4, Key::Num5, Key::Num6, Key::Num7, Key::Num8, Key::Num9,
        Key::Semicolon, Key::Equal,
        Key::A, Key::B, Key::C, Key::D, Key::E, Key::F, Key::G, Key::H, Key::I, Key::J, Key::K, Key::L, Key::M,
        Key::N, Key::O, Key::P, Key::Q, Key::R, Key::S, Key::T, Key::U, Key::V, Key::W, Key::X, Key::Y, Key::Z,
        Key::LeftBracket, Key::Backslash, Key::RightBracket, Key::GraveAccent,
        Key::Escape, Key::Enter, Key::Tab, Key::Backspace, Key::Insert, Key::Delete,
        Key::Right, Key::Left, Key::Down, Key::Up, Key::PageUp, Key::PageDown, Key::Home, Key::End,
        Key::CapsLock, Key::ScrollLock, Key::NumLock, Key::PrintScreen, Key::Pause,
        Key::F1, Key::F2, Key::F3, Key::F4, Key::F5, Key::F6, Key::F7, Key::F8, Key::F9, Key::F10, Key::F11, Key::F12,
        Key::LeftShift, Key::LeftControl, Key::LeftAlt, Key::LeftSuper,
        Key::RightShift, Key::RightControl, Key::RightAlt, Key::RightSuper, Key::Menu
    };

    registerConstTable(vm, "Key", keyNames, keyValues,
                       static_cast<int>(sizeof(keyNames) / sizeof(keyNames[0])));

    // Mouse button constants
    static const char* const mouseNames[] = {
        "Left", "Right", "Middle", "Button4", "Button5"
    };
    static const SQInteger mouseValues[] = {
        Mouse::ButtonLeft, Mouse::ButtonRight, Mouse::ButtonMiddle,
        Mouse::Button4, Mouse::Button5
    };
    registerConstTable(vm, "Mouse", mouseNames, mouseValues,
                       static_cast<int>(sizeof(mouseNames) / sizeof(mouseNames[0])));
}

void registerInputBindings(HSQUIRRELVM vm) {
    registerInput(vm);
    registerKeyConstants(vm);
}

} // namespace sq
} // namespace easy2d
