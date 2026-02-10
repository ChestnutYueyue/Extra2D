#include <extra2d/event/event_queue.h>
#include <extra2d/platform/input.h>
#include <extra2d/platform/window.h>
#include <extra2d/utils/logger.h>

#include <SDL.h>

#include <glad/glad.h>

namespace extra2d {

Window::Window()
    : sdlWindow_(nullptr), glContext_(nullptr), currentCursor_(nullptr),
      width_(1280), height_(720), vsync_(true), shouldClose_(false),
      fullscreen_(true), focused_(true), contentScaleX_(1.0f), contentScaleY_(1.0f),
      enableDpiScale_(true), userData_(nullptr), eventQueue_(nullptr) {
    // 初始化光标数组
    for (int i = 0; i < 9; ++i) {
        sdlCursors_[i] = nullptr;
    }
}

Window::~Window() { destroy(); }

bool Window::create(const WindowConfig &config) {
  if (sdlWindow_ != nullptr) {
    E2D_LOG_WARN("Window already created");
    return false;
  }

  width_ = config.width;
  height_ = config.height;
  vsync_ = config.vsync;
  fullscreen_ = config.fullscreen;
  enableDpiScale_ = config.enableDpiScale;

  // 初始化 SDL2 + 创建窗口 + GL 上下文
  if (!initSDL(config)) {
    E2D_LOG_ERROR("Failed to initialize SDL2");
    return false;
  }

  // 创建输入管理器
  input_ = makeUnique<Input>();
  input_->init();

  // 初始化光标
  if (config.enableCursors) {
    initCursors();
  }

  E2D_LOG_INFO("Window created: {}x{}", width_, height_);
  return true;
}

bool Window::initSDL(const WindowConfig &config) {
  // SDL2 全局初始化（视频 + 游戏控制器 + 音频）
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_AUDIO) != 0) {
    E2D_LOG_ERROR("SDL_Init failed: {}", SDL_GetError());
    return false;
  }

  // 设置 OpenGL ES 3.2 上下文属性
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

  // 颜色/深度/模板缓冲配置
  SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

  // 双缓冲
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  // 创建 SDL2 窗口
  Uint32 windowFlags = SDL_WINDOW_OPENGL;
  
  // 根据配置设置窗口模式
  if (config.fullscreen) {
    // Switch 平台使用 SDL_WINDOW_FULLSCREEN（固定分辨率）
    // PC 平台使用 SDL_WINDOW_FULLSCREEN_DESKTOP（桌面全屏）
    if (config.fullscreenDesktop) {
      windowFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    } else {
      windowFlags |= SDL_WINDOW_FULLSCREEN;
    }
  } else {
    if (config.resizable) {
      windowFlags |= SDL_WINDOW_RESIZABLE;
    }
    // 注意：SDL_WINDOWPOS_CENTERED 是位置参数，不是窗口标志
    // 窗口居中在 SDL_CreateWindow 的位置参数中处理
  }

  sdlWindow_ = SDL_CreateWindow(
      config.title.c_str(),
      config.centerWindow ? SDL_WINDOWPOS_CENTERED : SDL_WINDOWPOS_UNDEFINED,
      config.centerWindow ? SDL_WINDOWPOS_CENTERED : SDL_WINDOWPOS_UNDEFINED,
      width_, height_, windowFlags);
      
  if (!sdlWindow_) {
    E2D_LOG_ERROR("SDL_CreateWindow failed: {}", SDL_GetError());
    SDL_Quit();
    return false;
  }

  // 创建 OpenGL ES 上下文
  glContext_ = SDL_GL_CreateContext(sdlWindow_);
  if (!glContext_) {
    E2D_LOG_ERROR("SDL_GL_CreateContext failed: {}", SDL_GetError());
    SDL_DestroyWindow(sdlWindow_);
    sdlWindow_ = nullptr;
    SDL_Quit();
    return false;
  }

  if (SDL_GL_MakeCurrent(sdlWindow_, glContext_) != 0) {
    E2D_LOG_ERROR("SDL_GL_MakeCurrent failed: {}", SDL_GetError());
    SDL_GL_DeleteContext(glContext_);
    glContext_ = nullptr;
    SDL_DestroyWindow(sdlWindow_);
    sdlWindow_ = nullptr;
    SDL_Quit();
    return false;
  }

  // 加载 OpenGL ES 函数指针
  if (gladLoadGLES2Loader(reinterpret_cast<GLADloadproc>(SDL_GL_GetProcAddress)) == 0) {
    E2D_LOG_ERROR("gladLoadGLES2Loader failed");
    SDL_GL_DeleteContext(glContext_);
    glContext_ = nullptr;
    SDL_DestroyWindow(sdlWindow_);
    sdlWindow_ = nullptr;
    SDL_Quit();
    return false;
  }

  // 设置 VSync
  SDL_GL_SetSwapInterval(vsync_ ? 1 : 0);

  // 更新 DPI 缩放
  if (config.enableDpiScale) {
    updateContentScale();
  }

  E2D_LOG_INFO("SDL2 + GLES 3.2 initialized successfully");
  E2D_LOG_INFO("OpenGL Version: {}",
               reinterpret_cast<const char *>(glGetString(GL_VERSION)));
  E2D_LOG_INFO("OpenGL Renderer: {}",
               reinterpret_cast<const char *>(glGetString(GL_RENDERER)));

  return true;
}

void Window::deinitSDL() {
  deinitCursors();
  
  if (glContext_) {
    SDL_GL_DeleteContext(glContext_);
    glContext_ = nullptr;
  }

  if (sdlWindow_) {
    SDL_DestroyWindow(sdlWindow_);
    sdlWindow_ = nullptr;
  }

  SDL_Quit();
}

void Window::destroy() {
  if (sdlWindow_ != nullptr) {
    input_.reset();
    deinitSDL();
    E2D_LOG_INFO("Window destroyed");
  }
}

void Window::pollEvents() {
  // SDL2 事件循环
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
    case SDL_QUIT:
      shouldClose_ = true;
      if (closeCallback_) {
        closeCallback_();
      }
      break;

    case SDL_WINDOWEVENT:
      switch (event.window.event) {
      case SDL_WINDOWEVENT_RESIZED:
      case SDL_WINDOWEVENT_SIZE_CHANGED:
        width_ = event.window.data1;
        height_ = event.window.data2;
        updateContentScale();
        if (resizeCallback_) {
          resizeCallback_(width_, height_);
        }
        break;
      case SDL_WINDOWEVENT_FOCUS_GAINED:
        focused_ = true;
        if (focusCallback_) {
          focusCallback_(true);
        }
        break;
      case SDL_WINDOWEVENT_FOCUS_LOST:
        focused_ = false;
        if (focusCallback_) {
          focusCallback_(false);
        }
        break;
      }
      break;
    }
  }

  // 输入更新
  if (input_) {
    input_->update();
  }
}

void Window::swapBuffers() {
  if (sdlWindow_) {
    SDL_GL_SwapWindow(sdlWindow_);
  }
}

bool Window::shouldClose() const { return shouldClose_; }

void Window::setShouldClose(bool close) { shouldClose_ = close; }

void Window::setTitle(const String &title) {
  if (sdlWindow_) {
    SDL_SetWindowTitle(sdlWindow_, title.c_str());
  }
}

void Window::setSize(int width, int height) {
  if (sdlWindow_) {
    SDL_SetWindowSize(sdlWindow_, width, height);
    width_ = width;
    height_ = height;
  }
}

void Window::setPosition(int x, int y) {
  if (sdlWindow_) {
    SDL_SetWindowPosition(sdlWindow_, x, y);
  }
}

void Window::setFullscreen(bool fullscreen) {
  if (sdlWindow_) {
    // 默认使用桌面全屏模式（PC 平台）
    Uint32 flags = fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0;
    SDL_SetWindowFullscreen(sdlWindow_, flags);
    fullscreen_ = fullscreen;
  }
}

void Window::setVSync(bool enabled) {
  vsync_ = enabled;
  SDL_GL_SetSwapInterval(enabled ? 1 : 0);
}

void Window::setResizable(bool resizable) {
  if (sdlWindow_) {
    SDL_SetWindowResizable(sdlWindow_, resizable ? SDL_TRUE : SDL_FALSE);
  }
}

Vec2 Window::getPosition() const {
  if (sdlWindow_) {
    int x, y;
    SDL_GetWindowPosition(sdlWindow_, &x, &y);
    return Vec2(static_cast<float>(x), static_cast<float>(y));
  }
  return Vec2::Zero();
}

float Window::getContentScaleX() const {
  return enableDpiScale_ ? contentScaleX_ : 1.0f;
}

float Window::getContentScaleY() const {
  return enableDpiScale_ ? contentScaleY_ : 1.0f;
}

Vec2 Window::getContentScale() const {
  return Vec2(getContentScaleX(), getContentScaleY());
}

bool Window::isMinimized() const {
  if (sdlWindow_) {
    Uint32 flags = SDL_GetWindowFlags(sdlWindow_);
    return (flags & SDL_WINDOW_MINIMIZED) != 0;
  }
  return false;
}

bool Window::isMaximized() const {
  if (sdlWindow_) {
    Uint32 flags = SDL_GetWindowFlags(sdlWindow_);
    return (flags & SDL_WINDOW_MAXIMIZED) != 0;
  }
  return true;
}

void Window::initCursors() {
  sdlCursors_[0] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_ARROW);
  sdlCursors_[1] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_IBEAM);
  sdlCursors_[2] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_CROSSHAIR);
  sdlCursors_[3] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_HAND);
  sdlCursors_[4] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE);
  sdlCursors_[5] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENS);
  sdlCursors_[6] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEALL);
  sdlCursors_[7] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENWSE);
  sdlCursors_[8] = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENESW);
}

void Window::deinitCursors() {
  for (int i = 0; i < 9; ++i) {
    if (sdlCursors_[i]) {
      SDL_FreeCursor(sdlCursors_[i]);
      sdlCursors_[i] = nullptr;
    }
  }
  currentCursor_ = nullptr;
}

void Window::setCursor(CursorShape shape) {
  int index = static_cast<int>(shape);
  if (index >= 0 && index < 9 && sdlCursors_[index]) {
    SDL_SetCursor(sdlCursors_[index]);
    currentCursor_ = sdlCursors_[index];
  }
}

void Window::resetCursor() {
  SDL_SetCursor(SDL_GetDefaultCursor());
  currentCursor_ = nullptr;
}

void Window::setMouseVisible(bool visible) {
  SDL_ShowCursor(visible ? SDL_ENABLE : SDL_DISABLE);
}

void Window::updateContentScale() {
  if (sdlWindow_) {
    // 使用 DPI 计算内容缩放比例
    int displayIndex = SDL_GetWindowDisplayIndex(sdlWindow_);
    if (displayIndex >= 0) {
      float ddpi, hdpi, vdpi;
      if (SDL_GetDisplayDPI(displayIndex, &ddpi, &hdpi, &vdpi) == 0) {
        // 假设标准 DPI 为 96
        contentScaleX_ = hdpi / 96.0f;
        contentScaleY_ = vdpi / 96.0f;
      }
    }
  }
}

} // namespace extra2d
