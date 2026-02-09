#include <easy2d/event/event_queue.h>
#include <easy2d/platform/input.h>
#include <easy2d/platform/window.h>
#include <easy2d/utils/logger.h>

#include <SDL.h>

// 使用标准 GLES3.2
#include <GLES3/gl32.h>

namespace easy2d {

Window::Window()
    : sdlWindow_(nullptr), glContext_(nullptr), width_(1280), height_(720),
      vsync_(true), shouldClose_(false), userData_(nullptr),
      eventQueue_(nullptr) {}

Window::~Window() { destroy(); }

bool Window::create(const WindowConfig &config) {
  if (sdlWindow_ != nullptr) {
    E2D_LOG_WARN("Window already created");
    return false;
  }

  width_ = 1280; // Switch 固定分辨率
  height_ = 720;
  vsync_ = config.vsync;

  // 初始化 SDL2 + 创建窗口 + GL 上下文
  if (!initSDL()) {
    E2D_LOG_ERROR("Failed to initialize SDL2");
    return false;
  }

  // 创建输入管理器
  input_ = makeUnique<Input>();
  input_->init();

  E2D_LOG_INFO("Window created: {}x{}", width_, height_);
  return true;
}

bool Window::initSDL() {
  // SDL2 全局初始化（视频 + 游戏控制器 + 音频）
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_AUDIO) !=
      0) {
    E2D_LOG_ERROR("SDL_Init failed: {}", SDL_GetError());
    return false;
  }

  // 设置 OpenGL ES 3.2 上下文属性
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                      SDL_GL_CONTEXT_PROFILE_ES);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

  // 颜色/深度/模板缓冲配置
  SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

  // 创建 SDL2 窗口（Switch 全屏）
  Uint32 windowFlags = SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN;
  sdlWindow_ = SDL_CreateWindow("Easy2D", SDL_WINDOWPOS_CENTERED,
                                SDL_WINDOWPOS_CENTERED, width_, height_,
                                windowFlags);
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

  // 设置 VSync
  SDL_GL_SetSwapInterval(vsync_ ? 1 : 0);

  E2D_LOG_INFO("SDL2 + GLES 3.2 initialized successfully");
  return true;
}

void Window::deinitSDL() {
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
        width_ = event.window.data1;
        height_ = event.window.data2;
        if (resizeCallback_) {
          resizeCallback_(width_, height_);
        }
        break;
      case SDL_WINDOWEVENT_FOCUS_GAINED:
        if (focusCallback_) {
          focusCallback_(true);
        }
        break;
      case SDL_WINDOWEVENT_FOCUS_LOST:
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

void Window::setTitle(const String & /*title*/) {
  // Switch 无窗口标题
}

void Window::setSize(int /*width*/, int /*height*/) {
  // Switch 固定 1280x720
}

void Window::setPosition(int /*x*/, int /*y*/) {
  // Switch 无窗口位置
}

void Window::setFullscreen(bool /*fullscreen*/) {
  // Switch 始终全屏
}

void Window::setVSync(bool enabled) {
  vsync_ = enabled;
  SDL_GL_SetSwapInterval(enabled ? 1 : 0);
}

void Window::setResizable(bool /*resizable*/) {
  // Switch 不支持
}

void Window::setCursor(CursorShape /*shape*/) {
  // Switch 无鼠标光标
}

void Window::resetCursor() {
  // Switch 无鼠标光标
}

} // namespace easy2d
