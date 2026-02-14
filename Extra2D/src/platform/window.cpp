#include <extra2d/event/event_queue.h>
#include <extra2d/platform/input.h>
#include <extra2d/platform/window.h>
#include <extra2d/utils/logger.h>

#include <SDL.h>

#include <glad/glad.h>

namespace extra2d {

/**
 * @brief 默认构造函数
 *
 * 初始化窗口的所有成员变量为默认值，包括SDL窗口指针、OpenGL上下文、
 * 光标数组、窗口尺寸、VSync状态等。
 */
Window::Window()
    : sdlWindow_(nullptr), glContext_(nullptr), currentCursor_(nullptr),
      width_(1280), height_(720), vsync_(true), shouldClose_(false),
      fullscreen_(true), focused_(true), contentScaleX_(1.0f),
      contentScaleY_(1.0f), enableDpiScale_(true), userData_(nullptr),
      eventQueue_(nullptr) {
  for (int i = 0; i < 9; ++i) {
    sdlCursors_[i] = nullptr;
  }
}

/**
 * @brief 析构函数
 *
 * 自动调用destroy()方法释放所有资源。
 */
Window::~Window() { destroy(); }

/**
 * @brief 创建窗口
 *
 * 根据配置参数创建SDL窗口和OpenGL ES上下文，初始化输入管理器和光标。
 *
 * @param config 窗口配置参数，包含尺寸、标题、全屏模式等信息
 * @return 创建成功返回true，失败返回false
 */
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

  if (!initSDL(config)) {
    E2D_LOG_ERROR("Failed to initialize SDL2");
    return false;
  }

  input_ = makeUnique<Input>();
  input_->init();

  if (config.enableCursors) {
    initCursors();
  }

  E2D_LOG_INFO("Window created: {}x{}", width_, height_);
  return true;
}

/**
 * @brief 初始化SDL库和OpenGL上下文
 *
 * 执行SDL2全局初始化、设置OpenGL ES 3.2上下文属性、创建窗口、
 * 创建OpenGL上下文并加载GLES函数指针。
 *
 * @param config 窗口配置参数
 * @return 初始化成功返回true，失败返回false
 */
bool Window::initSDL(const WindowConfig &config) {
  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER | SDL_INIT_AUDIO) !=
      0) {
    E2D_LOG_ERROR("SDL_Init failed: {}", SDL_GetError());
    return false;
  }

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

  SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
  SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

  Uint32 windowFlags = SDL_WINDOW_OPENGL;

  if (config.fullscreen) {
    if (config.fullscreenDesktop) {
      windowFlags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    } else {
      windowFlags |= SDL_WINDOW_FULLSCREEN;
    }
  } else {
    if (config.resizable) {
      windowFlags |= SDL_WINDOW_RESIZABLE;
    }
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

  if (gladLoadGLES2Loader(
          reinterpret_cast<GLADloadproc>(SDL_GL_GetProcAddress)) == 0) {
    E2D_LOG_ERROR("gladLoadGLES2Loader failed");
    SDL_GL_DeleteContext(glContext_);
    glContext_ = nullptr;
    SDL_DestroyWindow(sdlWindow_);
    sdlWindow_ = nullptr;
    SDL_Quit();
    return false;
  }

  SDL_GL_SetSwapInterval(vsync_ ? 1 : 0);

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

/**
 * @brief 反初始化SDL资源
 *
 * 释放光标资源、删除OpenGL上下文、销毁SDL窗口并退出SDL库。
 */
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

/**
 * @brief 销毁窗口
 *
 * 释放输入管理器并调用deinitSDL()清理所有SDL相关资源。
 */
void Window::destroy() {
  if (sdlWindow_ != nullptr) {
    input_.reset();
    deinitSDL();
    E2D_LOG_INFO("Window destroyed");
  }
}

/**
 * @brief 轮询并处理窗口事件
 *
 * 处理SDL事件队列中的所有事件，包括窗口关闭、大小改变、焦点变化等，
 * 并更新输入管理器状态。
 */
void Window::pollEvents() {
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

  if (input_) {
    input_->update();
  }
}

/**
 * @brief 交换前后缓冲区
 *
 * 将后台缓冲区内容呈现到屏幕上，实现双缓冲渲染。
 */
void Window::swapBuffers() {
  if (sdlWindow_) {
    SDL_GL_SwapWindow(sdlWindow_);
  }
}

/**
 * @brief 检查窗口是否应该关闭
 *
 * @return 如果窗口应该关闭返回true，否则返回false
 */
bool Window::shouldClose() const { return shouldClose_; }

/**
 * @brief 设置窗口关闭标志
 *
 * @param close 是否应该关闭窗口
 */
void Window::setShouldClose(bool close) { shouldClose_ = close; }

/**
 * @brief 设置窗口标题
 *
 * @param title 新的窗口标题字符串
 */
void Window::setTitle(const std::string &title) {
  if (sdlWindow_) {
    SDL_SetWindowTitle(sdlWindow_, title.c_str());
  }
}

/**
 * @brief 设置窗口大小
 *
 * @param width 新的窗口宽度（像素）
 * @param height 新的窗口高度（像素）
 */
void Window::setSize(int width, int height) {
  if (sdlWindow_) {
    SDL_SetWindowSize(sdlWindow_, width, height);
    width_ = width;
    height_ = height;
  }
}

/**
 * @brief 设置窗口位置
 *
 * @param x 窗口左上角的X坐标（屏幕坐标）
 * @param y 窗口左上角的Y坐标（屏幕坐标）
 */
void Window::setPos(int x, int y) {
  if (sdlWindow_) {
    SDL_SetWindowPosition(sdlWindow_, x, y);
  }
}

/**
 * @brief 设置窗口全屏模式
 *
 * @param fullscreen true为全屏模式，false为窗口模式
 */
void Window::setFullscreen(bool fullscreen) {
  if (sdlWindow_) {
    Uint32 flags = fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0;
    SDL_SetWindowFullscreen(sdlWindow_, flags);
    fullscreen_ = fullscreen;
  }
}

/**
 * @brief 设置垂直同步
 *
 * @param enabled true启用VSync，false禁用VSync
 */
void Window::setVSync(bool enabled) {
  vsync_ = enabled;
  SDL_GL_SetSwapInterval(enabled ? 1 : 0);
}

/**
 * @brief 设置窗口是否可调整大小
 *
 * @param resizable true允许用户调整窗口大小，false禁止调整
 */
void Window::setResizable(bool resizable) {
  if (sdlWindow_) {
    SDL_SetWindowResizable(sdlWindow_, resizable ? SDL_TRUE : SDL_FALSE);
  }
}

/**
 * @brief 获取窗口位置
 *
 * @return 包含窗口左上角X和Y坐标的二维向量
 */
Vec2 Window::getPosition() const {
  if (sdlWindow_) {
    int x, y;
    SDL_GetWindowPosition(sdlWindow_, &x, &y);
    return Vec2(static_cast<float>(x), static_cast<float>(y));
  }
  return Vec2::Zero();
}

/**
 * @brief 获取X轴内容缩放比例
 *
 * 根据DPI设置返回X轴的内容缩放比例，用于高DPI显示适配。
 *
 * @return X轴缩放比例，如果DPI缩放被禁用则返回1.0
 */
float Window::getContentScaleX() const {
  return enableDpiScale_ ? contentScaleX_ : 1.0f;
}

/**
 * @brief 获取Y轴内容缩放比例
 *
 * 根据DPI设置返回Y轴的内容缩放比例，用于高DPI显示适配。
 *
 * @return Y轴缩放比例，如果DPI缩放被禁用则返回1.0
 */
float Window::getContentScaleY() const {
  return enableDpiScale_ ? contentScaleY_ : 1.0f;
}

/**
 * @brief 获取内容缩放比例
 *
 * @return 包含X和Y轴缩放比例的二维向量
 */
Vec2 Window::getContentScale() const {
  return Vec2(getContentScaleX(), getContentScaleY());
}

/**
 * @brief 检查窗口是否最小化
 *
 * @return 如果窗口处于最小化状态返回true，否则返回false
 */
bool Window::isMinimized() const {
  if (sdlWindow_) {
    Uint32 flags = SDL_GetWindowFlags(sdlWindow_);
    return (flags & SDL_WINDOW_MINIMIZED) != 0;
  }
  return false;
}

/**
 * @brief 检查窗口是否最大化
 *
 * @return 如果窗口处于最大化状态返回true，否则返回false
 */
bool Window::isMaximized() const {
  if (sdlWindow_) {
    Uint32 flags = SDL_GetWindowFlags(sdlWindow_);
    return (flags & SDL_WINDOW_MAXIMIZED) != 0;
  }
  return true;
}

/**
 * @brief 初始化系统光标
 *
 * 创建9种常用的系统光标，包括箭头、文本选择、十字、手形、
 * 水平调整、垂直调整、移动、对角线调整等。
 */
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

/**
 * @brief 释放光标资源
 *
 * 释放所有已创建的系统光标并重置当前光标指针。
 */
void Window::deinitCursors() {
  for (int i = 0; i < 9; ++i) {
    if (sdlCursors_[i]) {
      SDL_FreeCursor(sdlCursors_[i]);
      sdlCursors_[i] = nullptr;
    }
  }
  currentCursor_ = nullptr;
}

/**
 * @brief 设置鼠标光标样式
 *
 * 将鼠标光标更改为指定的系统光标样式。
 *
 * @param shape 光标形状枚举值
 */
void Window::setCursor(CursorShape shape) {
  int index = static_cast<int>(shape);
  if (index >= 0 && index < 9 && sdlCursors_[index]) {
    SDL_SetCursor(sdlCursors_[index]);
    currentCursor_ = sdlCursors_[index];
  }
}

/**
 * @brief 重置鼠标光标为默认样式
 *
 * 将鼠标光标恢复为系统默认光标。
 */
void Window::resetCursor() {
  SDL_SetCursor(SDL_GetDefaultCursor());
  currentCursor_ = nullptr;
}

/**
 * @brief 设置鼠标光标可见性
 *
 * @param visible true显示鼠标光标，false隐藏鼠标光标
 */
void Window::setMouseVisible(bool visible) {
  SDL_ShowCursor(visible ? SDL_ENABLE : SDL_DISABLE);
}

/**
 * @brief 更新内容缩放比例
 *
 * 根据当前窗口所在显示器的DPI值更新内容缩放比例，
 * 以标准96 DPI为基准计算缩放因子。
 */
void Window::updateContentScale() {
  if (sdlWindow_) {
    int displayIndex = SDL_GetWindowDisplayIndex(sdlWindow_);
    if (displayIndex >= 0) {
      float ddpi, hdpi, vdpi;
      if (SDL_GetDisplayDPI(displayIndex, &ddpi, &hdpi, &vdpi) == 0) {
        contentScaleX_ = hdpi / 96.0f;
        contentScaleY_ = vdpi / 96.0f;
      }
    }
  }
}

} // namespace extra2d
