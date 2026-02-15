#pragma once

#include <extra2d/platform/iwindow.h>
#include <SDL.h>

namespace extra2d {

class SDL2Input;

/**
 * @brief SDL2 窗口实现
 */
class SDL2Window : public IWindow {
public:
    SDL2Window();
    ~SDL2Window() override;

    bool create(const WindowConfigData& cfg) override;
    void destroy() override;

    void poll() override;
    void swap() override;
    bool shouldClose() const override;
    void close() override;

    void setTitle(const std::string& title) override;
    void setSize(int w, int h) override;
    void setPos(int x, int y) override;
    void setFullscreen(bool fs) override;
    void setVSync(bool vsync) override;
    void setVisible(bool visible) override;

    int width() const override;
    int height() const override;
    Size size() const override;
    Vec2 pos() const override;
    bool fullscreen() const override;
    bool vsync() const override;
    bool focused() const override;
    bool minimized() const override;

    float scaleX() const override;
    float scaleY() const override;

    void setCursor(Cursor cursor) override;
    void showCursor(bool show) override;
    void lockCursor(bool lock) override;

    IInput* input() const override;

    void onResize(ResizeCb cb) override;
    void onClose(CloseCb cb) override;
    void onFocus(FocusCb cb) override;

    void* native() const override;

    /**
     * @brief 获取 SDL 窗口句柄
     */
    SDL_Window* sdlWindow() const { return sdlWindow_; }

    /**
     * @brief 获取 OpenGL 上下文
     */
    SDL_GLContext glContext() const { return glContext_; }

private:
    bool initSDL();
    void deinitSDL();
    void initCursors();
    void deinitCursors();
    void updateContentScale();
    void handleEvent(const SDL_Event& event);

    SDL_Window* sdlWindow_ = nullptr;
    SDL_GLContext glContext_ = nullptr;
    SDL_Cursor* sdlCursors_[7] = {};
    int currentCursor_ = 0;

    UniquePtr<SDL2Input> input_;

    int width_ = 1280;
    int height_ = 720;
    bool fullscreen_ = false;
    bool vsync_ = true;
    bool focused_ = true;
    bool minimized_ = false;
    bool shouldClose_ = false;
    float scaleX_ = 1.0f;
    float scaleY_ = 1.0f;
    bool cursorVisible_ = true;
    bool cursorLocked_ = false;

    ResizeCb resizeCb_;
    CloseCb closeCb_;
    FocusCb focusCb_;
};

} // namespace extra2d
