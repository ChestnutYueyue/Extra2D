#pragma once

#include <easy2d/core/types.h>
#include <easy2d/core/string.h>
#include <easy2d/core/math_types.h>
#include <functional>

#include <SDL.h>

namespace easy2d {

// 前向声明
class EventQueue;
class Input;

// ============================================================================
// 窗口配置
// ============================================================================
struct WindowConfig {
    String title = "Easy2D Application";
    int width = 1280;
    int height = 720;
    bool fullscreen = true;   // Switch 始终全屏
    bool resizable = false;
    bool vsync = true;
    int msaaSamples = 0;
    bool centerWindow = false;
};

// ============================================================================
// 鼠标光标形状枚举 (保留接口兼容性，Switch 上无效)
// ============================================================================
enum class CursorShape {
    Arrow,
    IBeam,
    Crosshair,
    Hand,
    HResize,
    VResize,
    ResizeAll,
    ResizeNWSE,
    ResizeNESW
};

// ============================================================================
// Window 类 - SDL2 Window + GLES 3.2 封装
// ============================================================================
class Window {
public:
    Window();
    ~Window();

    // 创建窗口
    bool create(const WindowConfig& config);
    void destroy();

    // 窗口操作
    void pollEvents();
    void swapBuffers();
    bool shouldClose() const;
    void setShouldClose(bool close);

    // 窗口属性 (Switch 上大部分为空操作)
    void setTitle(const String& title);
    void setSize(int width, int height);
    void setPosition(int x, int y);
    void setFullscreen(bool fullscreen);
    void setVSync(bool enabled);
    void setResizable(bool resizable);

    // 获取窗口属性
    int getWidth() const { return width_; }
    int getHeight() const { return height_; }
    Size getSize() const { return Size(static_cast<float>(width_), static_cast<float>(height_)); }
    Vec2 getPosition() const { return Vec2::Zero(); }
    bool isFullscreen() const { return true; }
    bool isVSync() const { return vsync_; }

    // DPI 缩放 (Switch 固定 1.0)
    float getContentScaleX() const { return 1.0f; }
    float getContentScaleY() const { return 1.0f; }
    Vec2 getContentScale() const { return Vec2(1.0f, 1.0f); }

    // 窗口状态
    bool isFocused() const { return true; }
    bool isMinimized() const { return false; }
    bool isMaximized() const { return true; }

    // 获取 SDL2 窗口和 GL 上下文
    SDL_Window* getSDLWindow() const { return sdlWindow_; }
    SDL_GLContext getGLContext() const { return glContext_; }

    // 设置/获取用户数据
    void setUserData(void* data) { userData_ = data; }
    void* getUserData() const { return userData_; }

    // 事件队列
    void setEventQueue(EventQueue* queue) { eventQueue_ = queue; }
    EventQueue* getEventQueue() const { return eventQueue_; }

    // 获取输入管理器
    Input* getInput() const { return input_.get(); }

    // 光标操作 (Switch 上为空操作)
    void setCursor(CursorShape shape);
    void resetCursor();

    // 窗口回调
    using ResizeCallback = std::function<void(int width, int height)>;
    using FocusCallback = std::function<void(bool focused)>;
    using CloseCallback = std::function<void()>;

    void setResizeCallback(ResizeCallback callback) { resizeCallback_ = callback; }
    void setFocusCallback(FocusCallback callback) { focusCallback_ = callback; }
    void setCloseCallback(CloseCallback callback) { closeCallback_ = callback; }

private:
    // SDL2 状态
    SDL_Window* sdlWindow_;
    SDL_GLContext glContext_;

    int width_;
    int height_;
    bool vsync_;
    bool shouldClose_;
    void* userData_;
    EventQueue* eventQueue_;
    UniquePtr<Input> input_;

    ResizeCallback resizeCallback_;
    FocusCallback focusCallback_;
    CloseCallback closeCallback_;

    bool initSDL();
    void deinitSDL();
};

} // namespace easy2d
