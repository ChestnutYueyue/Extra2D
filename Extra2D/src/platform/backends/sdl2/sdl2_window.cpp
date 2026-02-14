#include "sdl2_window.h"
#include "sdl2_input.h"
#include <extra2d/utils/logger.h>

namespace extra2d {

SDL2Window::SDL2Window() {
    for (int i = 0; i < 7; ++i) {
        sdlCursors_[i] = nullptr;
    }
}

SDL2Window::~SDL2Window() {
    destroy();
}

bool SDL2Window::create(const WindowConfig& cfg) {
    if (!initSDL()) {
        return false;
    }

    Uint32 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;
    if (cfg.fullscreen) {
        flags |= cfg.fullscreenDesktop ? SDL_WINDOW_FULLSCREEN_DESKTOP : SDL_WINDOW_FULLSCREEN;
    }
    if (cfg.resizable) {
        flags |= SDL_WINDOW_RESIZABLE;
    }
    if (!cfg.decorated) {
        flags |= SDL_WINDOW_BORDERLESS;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    if (cfg.msaaSamples > 0) {
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
        SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, cfg.msaaSamples);
    }

    int x = SDL_WINDOWPOS_CENTERED;
    int y = SDL_WINDOWPOS_CENTERED;
    if (!cfg.centerWindow) {
        x = SDL_WINDOWPOS_UNDEFINED;
        y = SDL_WINDOWPOS_UNDEFINED;
    }

    sdlWindow_ = SDL_CreateWindow(
        cfg.title.c_str(),
        x, y,
        cfg.width, cfg.height,
        flags
    );

    if (!sdlWindow_) {
        E2D_LOG_ERROR("Failed to create SDL window: {}", SDL_GetError());
        deinitSDL();
        return false;
    }

    glContext_ = SDL_GL_CreateContext(sdlWindow_);
    if (!glContext_) {
        E2D_LOG_ERROR("Failed to create OpenGL context: {}", SDL_GetError());
        SDL_DestroyWindow(sdlWindow_);
        sdlWindow_ = nullptr;
        deinitSDL();
        return false;
    }

    SDL_GL_SetSwapInterval(cfg.vsync ? 1 : 0);

    SDL_GetWindowSize(sdlWindow_, &width_, &height_);
    fullscreen_ = cfg.fullscreen;
    vsync_ = cfg.vsync;

    initCursors();
    updateContentScale();

    input_ = makeUnique<SDL2Input>();
    input_->init();

    E2D_LOG_INFO("SDL2 window created: {}x{}", width_, height_);
    return true;
}

void SDL2Window::destroy() {
    if (input_) {
        input_->shutdown();
        input_.reset();
    }

    deinitCursors();

    if (glContext_) {
        SDL_GL_DeleteContext(glContext_);
        glContext_ = nullptr;
    }

    if (sdlWindow_) {
        SDL_DestroyWindow(sdlWindow_);
        sdlWindow_ = nullptr;
    }

    deinitSDL();
}

void SDL2Window::poll() {
    if (!sdlWindow_) return;

    if (input_) {
        input_->update();
    }

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        handleEvent(event);
    }
}

void SDL2Window::swap() {
    if (sdlWindow_ && glContext_) {
        SDL_GL_SwapWindow(sdlWindow_);
    }
}

bool SDL2Window::shouldClose() const {
    return shouldClose_;
}

void SDL2Window::close() {
    shouldClose_ = true;
}

void SDL2Window::setTitle(const std::string& title) {
    if (sdlWindow_) {
        SDL_SetWindowTitle(sdlWindow_, title.c_str());
    }
}

void SDL2Window::setSize(int w, int h) {
    if (sdlWindow_) {
        SDL_SetWindowSize(sdlWindow_, w, h);
        width_ = w;
        height_ = h;
    }
}

void SDL2Window::setPos(int x, int y) {
    if (sdlWindow_) {
        SDL_SetWindowPosition(sdlWindow_, x, y);
    }
}

void SDL2Window::setFullscreen(bool fs) {
    if (sdlWindow_) {
        SDL_SetWindowFullscreen(sdlWindow_, fs ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
        fullscreen_ = fs;
    }
}

void SDL2Window::setVSync(bool vsync) {
    if (glContext_) {
        SDL_GL_SetSwapInterval(vsync ? 1 : 0);
        vsync_ = vsync;
    }
}

void SDL2Window::setVisible(bool visible) {
    if (sdlWindow_) {
        if (visible) {
            SDL_ShowWindow(sdlWindow_);
        } else {
            SDL_HideWindow(sdlWindow_);
        }
    }
}

int SDL2Window::width() const {
    return width_;
}

int SDL2Window::height() const {
    return height_;
}

Size SDL2Window::size() const {
    return Size(static_cast<float>(width_), static_cast<float>(height_));
}

Vec2 SDL2Window::pos() const {
    int x, y;
    if (sdlWindow_) {
        SDL_GetWindowPosition(sdlWindow_, &x, &y);
    } else {
        x = y = 0;
    }
    return Vec2(static_cast<float>(x), static_cast<float>(y));
}

bool SDL2Window::fullscreen() const {
    return fullscreen_;
}

bool SDL2Window::vsync() const {
    return vsync_;
}

bool SDL2Window::focused() const {
    return focused_;
}

bool SDL2Window::minimized() const {
    return minimized_;
}

float SDL2Window::scaleX() const {
    return scaleX_;
}

float SDL2Window::scaleY() const {
    return scaleY_;
}

void SDL2Window::setCursor(Cursor cursor) {
    if (cursor == Cursor::Hidden) {
        SDL_ShowCursor(SDL_DISABLE);
        return;
    }

    SDL_ShowCursor(SDL_ENABLE);

    int idx = static_cast<int>(cursor);
    if (idx >= 0 && idx < 7 && sdlCursors_[idx]) {
        SDL_SetCursor(sdlCursors_[idx]);
        currentCursor_ = idx;
    }
}

void SDL2Window::showCursor(bool show) {
    SDL_ShowCursor(show ? SDL_ENABLE : SDL_DISABLE);
    cursorVisible_ = show;
}

void SDL2Window::lockCursor(bool lock) {
    if (sdlWindow_) {
        SDL_SetRelativeMouseMode(lock ? SDL_TRUE : SDL_FALSE);
        cursorLocked_ = lock;
    }
}

IInput* SDL2Window::input() const {
    return input_.get();
}

void SDL2Window::onResize(ResizeCb cb) {
    resizeCb_ = cb;
}

void SDL2Window::onClose(CloseCb cb) {
    closeCb_ = cb;
}

void SDL2Window::onFocus(FocusCb cb) {
    focusCb_ = cb;
}

void* SDL2Window::native() const {
    return sdlWindow_;
}

bool SDL2Window::initSDL() {
    static int sdlInitCount = 0;
    if (sdlInitCount == 0) {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) != 0) {
            E2D_LOG_ERROR("Failed to initialize SDL: {}", SDL_GetError());
            return false;
        }
        sdlInitCount++;
    }
    return true;
}

void SDL2Window::deinitSDL() {
    static int sdlInitCount = 1;
    sdlInitCount--;
    if (sdlInitCount == 0) {
        SDL_Quit();
    }
}

void SDL2Window::initCursors() {
    sdlCursors_[0] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
    sdlCursors_[1] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);
    sdlCursors_[2] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_CROSSHAIR);
    sdlCursors_[3] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
    sdlCursors_[4] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENS);
    sdlCursors_[5] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE);
    sdlCursors_[6] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEALL);
}

void SDL2Window::deinitCursors() {
    for (int i = 0; i < 7; ++i) {
        if (sdlCursors_[i]) {
            SDL_FreeCursor(sdlCursors_[i]);
            sdlCursors_[i] = nullptr;
        }
    }
}

void SDL2Window::updateContentScale() {
    if (sdlWindow_) {
        SDL_GetWindowSize(sdlWindow_, &width_, &height_);
        int dw, dh;
        SDL_GL_GetDrawableSize(sdlWindow_, &dw, &dh);
        scaleX_ = dw > 0 ? static_cast<float>(dw) / width_ : 1.0f;
        scaleY_ = dh > 0 ? static_cast<float>(dh) / height_ : 1.0f;
    }
}

void SDL2Window::handleEvent(const SDL_Event& event) {
    switch (event.type) {
        case SDL_QUIT:
            shouldClose_ = true;
            if (closeCb_) closeCb_();
            break;

        case SDL_WINDOWEVENT:
            switch (event.window.event) {
                case SDL_WINDOWEVENT_RESIZED:
                case SDL_WINDOWEVENT_SIZE_CHANGED:
                    width_ = event.window.data1;
                    height_ = event.window.data2;
                    updateContentScale();
                    if (resizeCb_) resizeCb_(width_, height_);
                    break;

                case SDL_WINDOWEVENT_FOCUS_GAINED:
                    focused_ = true;
                    if (focusCb_) focusCb_(true);
                    break;

                case SDL_WINDOWEVENT_FOCUS_LOST:
                    focused_ = false;
                    if (focusCb_) focusCb_(false);
                    break;

                case SDL_WINDOWEVENT_MINIMIZED:
                    minimized_ = true;
                    break;

                case SDL_WINDOWEVENT_RESTORED:
                    minimized_ = false;
                    break;

                case SDL_WINDOWEVENT_CLOSE:
                    shouldClose_ = true;
                    if (closeCb_) closeCb_();
                    break;
            }
            break;
    }
}

} // namespace extra2d
