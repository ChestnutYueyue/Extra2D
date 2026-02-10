#pragma once

#include <extra2d/core/types.h>
#include <extra2d/core/string.h>
#include <extra2d/core/math_types.h>
#include <functional>

#include <SDL.h>

namespace extra2d {

// 前向声明
class EventQueue;
class Input;

// ============================================================================
// 窗口配置
// ============================================================================
struct WindowConfig {
    String title = "Extra2D Application";
    int width = 1280;
    int height = 720;
    bool fullscreen = true;
    bool resizable = false;
    bool vsync = true;
    int msaaSamples = 0;
    bool centerWindow = true;
    bool enableCursors = true;
    bool enableDpiScale = true;
    bool fullscreenDesktop = true;  // true: SDL_WINDOW_FULLSCREEN_DESKTOP, false: SDL_WINDOW_FULLSCREEN
};

// ============================================================================
// 鼠标光标形状枚举
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
// 支持平台: Nintendo Switch, Windows, Linux, macOS
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

    // 窗口属性
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
    Vec2 getPosition() const;
    bool isFullscreen() const { return fullscreen_; }
    bool isVSync() const { return vsync_; }

    // DPI 缩放 (PC 端自动检测，Switch 固定 1.0)
    float getContentScaleX() const;
    float getContentScaleY() const;
    Vec2 getContentScale() const;

    // 窗口状态
    bool isFocused() const { return focused_; }
    bool isMinimized() const;
    bool isMaximized() const;

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

    // 光标操作 (PC 端有效，Switch 上为空操作)
    void setCursor(CursorShape shape);
    void resetCursor();
    void setMouseVisible(bool visible);

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
    SDL_Cursor* sdlCursors_[9];  // 光标缓存
    SDL_Cursor* currentCursor_;

    int width_;
    int height_;
    bool vsync_;
    bool shouldClose_;
    bool fullscreen_;
    bool focused_;
    float contentScaleX_;
    float contentScaleY_;
    bool enableDpiScale_;
    void* userData_;
    EventQueue* eventQueue_;
    UniquePtr<Input> input_;

    ResizeCallback resizeCallback_;
    FocusCallback focusCallback_;
    CloseCallback closeCallback_;

    bool initSDL(const WindowConfig& config);
    void deinitSDL();
    void initCursors();
    void deinitCursors();
    void updateContentScale();
};

} // namespace extra2d
