#include <glad/glad.h>
#include <extra2d/graphics/opengl/gl_texture.h>
#include <extra2d/graphics/render_target.h>
#include <extra2d/utils/logger.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

namespace extra2d {

// ============================================================================
// RenderTarget实现
// ============================================================================

/**
 * @brief 默认构造函数
 *
 * 创建一个空的渲染目标对象
 */
RenderTarget::RenderTarget() = default;

/**
 * @brief 析构函数
 *
 * 销毁渲染目标并释放相关资源
 */
RenderTarget::~RenderTarget() { destroy(); }

/**
 * @brief 移动构造函数
 * @param other 源渲染目标对象
 *
 * 将其他渲染目标的资源转移到新对象
 */
RenderTarget::RenderTarget(RenderTarget &&other) noexcept
    : fbo_(other.fbo_), rbo_(other.rbo_),
      colorTexture_(std::move(other.colorTexture_)),
      depthTexture_(std::move(other.depthTexture_)), width_(other.width_),
      height_(other.height_), colorFormat_(other.colorFormat_),
      hasDepth_(other.hasDepth_), hasStencil_(other.hasStencil_),
      samples_(other.samples_) {
  other.fbo_ = 0;
  other.rbo_ = 0;
  other.width_ = 0;
  other.height_ = 0;
}

/**
 * @brief 移动赋值运算符
 * @param other 源渲染目标对象
 * @return 当前对象引用
 *
 * 将其他渲染目标的资源转移到当前对象
 */
RenderTarget &RenderTarget::operator=(RenderTarget &&other) noexcept {
  if (this != &other) {
    destroy();

    fbo_ = other.fbo_;
    rbo_ = other.rbo_;
    colorTexture_ = std::move(other.colorTexture_);
    depthTexture_ = std::move(other.depthTexture_);
    width_ = other.width_;
    height_ = other.height_;
    colorFormat_ = other.colorFormat_;
    hasDepth_ = other.hasDepth_;
    hasStencil_ = other.hasStencil_;
    samples_ = other.samples_;

    other.fbo_ = 0;
    other.rbo_ = 0;
    other.width_ = 0;
    other.height_ = 0;
  }
  return *this;
}

/**
 * @brief 根据配置创建渲染目标
 * @param config 渲染目标配置
 * @return 创建成功返回true，失败返回false
 *
 * 根据指定的配置参数创建帧缓冲对象
 */
bool RenderTarget::create(const RenderTargetConfig &config) {
  destroy();

  width_ = config.width;
  height_ = config.height;
  colorFormat_ = config.colorFormat;
  hasDepth_ = config.hasDepth;
  hasStencil_ = config.hasStencil;
  samples_ = config.samples;

  if (!createFBO()) {
    E2D_ERROR("创建渲染目标失败: {}x{}", width_, height_);
    return false;
  }

  E2D_INFO("创建渲染目标: {}x{} (深度:{}, 模板:{}, 采样:{})", width_, height_,
           hasDepth_, hasStencil_, samples_);

  return true;
}

/**
 * @brief 从现有纹理创建渲染目标
 * @param texture 颜色纹理
 * @param hasDepth 是否创建深度缓冲
 * @return 创建成功返回true，失败返回false
 *
 * 使用现有纹理作为颜色附件创建帧缓冲对象
 */
bool RenderTarget::createFromTexture(Ptr<Texture> texture, bool hasDepth) {
  if (!texture || !texture->isValid()) {
    E2D_ERROR("无效的颜色纹理");
    return false;
  }

  destroy();

  width_ = texture->getWidth();
  height_ = texture->getHeight();
  colorFormat_ = texture->getFormat();
  hasDepth_ = hasDepth;
  hasStencil_ = false;
  samples_ = 1;

  colorTexture_ = texture;

  // 创建FBO
  glGenFramebuffers(1, &fbo_);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo_);

  // 附加颜色纹理
  GLuint texId = static_cast<GLuint>(
      reinterpret_cast<uintptr_t>(texture->getNativeHandle()));
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         texId, 0);

  // 创建深度缓冲（如果需要）
  if (hasDepth_) {
    glGenRenderbuffers(1, &rbo_);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo_);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width_,
                          height_);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                              GL_RENDERBUFFER, rbo_);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
  }

  // 检查完整性
  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  if (status != GL_FRAMEBUFFER_COMPLETE) {
    E2D_ERROR("FBO不完整: {:#x}", status);
    destroy();
    return false;
  }

  E2D_INFO("从纹理创建渲染目标: {}x{}", width_, height_);
  return true;
}

/**
 * @brief 销毁渲染目标
 *
 * 释放帧缓冲对象和相关资源
 */
void RenderTarget::destroy() {
  deleteFBO();

  colorTexture_.reset();
  depthTexture_.reset();

  width_ = 0;
  height_ = 0;
}

/**
 * @brief 绑定渲染目标
 *
 * 将此渲染目标绑定为当前渲染目标
 */
void RenderTarget::bind() {
  if (!isValid()) {
    return;
  }

  glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
  glViewport(0, 0, width_, height_);
}

/**
 * @brief 解绑渲染目标
 *
 * 恢复默认帧缓冲
 */
void RenderTarget::unbind() { bindDefault(); }

/**
 * @brief 清除渲染目标
 * @param color 清除颜色
 *
 * 使用指定颜色清除颜色缓冲，如有深度/模板缓冲也会清除
 */
void RenderTarget::clear(const Color &color) {
  if (!isValid()) {
    return;
  }

  bind();

  GLbitfield mask = GL_COLOR_BUFFER_BIT;
  if (hasDepth_) {
    mask |= GL_DEPTH_BUFFER_BIT;
    glClearDepthf(1.0f); // GLES 使用 glClearDepthf
  }
  if (hasStencil_) {
    mask |= GL_STENCIL_BUFFER_BIT;
    glClearStencil(0);
  }

  glClearColor(color.r, color.g, color.b, color.a);
  glClear(mask);
}

/**
 * @brief 设置视口区域
 * @param x 视口左下角X坐标
 * @param y 视口左下角Y坐标
 * @param width 视口宽度
 * @param height 视口高度
 *
 * 设置渲染目标的视口区域
 */
void RenderTarget::setViewport(int x, int y, int width, int height) {
  if (!isValid()) {
    return;
  }

  bind();
  glViewport(x, y, width, height);
}

/**
 * @brief 获取完整视口区域
 * @param[out] x 视口左下角X坐标
 * @param[out] y 视口左下角Y坐标
 * @param[out] width 视口宽度
 * @param[out] height 视口高度
 *
 * 获取渲染目标的完整视口区域
 */
void RenderTarget::getFullViewport(int &x, int &y, int &width,
                                   int &height) const {
  x = 0;
  y = 0;
  width = width_;
  height = height_;
}

/**
 * @brief 调整渲染目标大小
 * @param width 新宽度
 * @param height 新高度
 * @return 调整成功返回true，失败返回false
 *
 * 重新创建指定大小的渲染目标
 */
bool RenderTarget::resize(int width, int height) {
  if (!isValid()) {
    return false;
  }

  RenderTargetConfig config;
  config.width = width;
  config.height = height;
  config.colorFormat = colorFormat_;
  config.hasDepth = hasDepth_;
  config.hasStencil = hasStencil_;
  config.samples = samples_;

  return create(config);
}

/**
 * @brief 复制到另一个渲染目标
 * @param target 目标渲染目标
 *
 * 使用glBlitFramebuffer将内容复制到目标渲染目标
 */
void RenderTarget::copyTo(RenderTarget &target) {
  if (!isValid() || !target.isValid()) {
    return;
  }

  // 使用glBlitFramebuffer复制
  glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, target.fbo_);

  GLbitfield mask = GL_COLOR_BUFFER_BIT;
  if (hasDepth_ && target.hasDepth_) {
    mask |= GL_DEPTH_BUFFER_BIT;
  }

  glBlitFramebuffer(0, 0, width_, height_, 0, 0, target.width_, target.height_,
                    mask, GL_LINEAR);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

/**
 * @brief 将内容传输到另一个渲染目标
 * @param target 目标渲染目标
 * @param color 是否复制颜色缓冲
 * @param depth 是否复制深度缓冲
 *
 * 使用glBlitFramebuffer进行选择性复制
 */
void RenderTarget::blitTo(RenderTarget &target, bool color, bool depth) {
  if (!isValid() || !target.isValid()) {
    return;
  }

  // 使用glBlitFramebuffer复制
  glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, target.getFBO());

  GLbitfield mask = 0;
  if (color) {
    mask |= GL_COLOR_BUFFER_BIT;
  }
  if (depth && hasDepth_ && target.hasDepth_) {
    mask |= GL_DEPTH_BUFFER_BIT;
  }

  if (mask != 0) {
    glBlitFramebuffer(0, 0, width_, height_, 0, 0, target.getWidth(),
                      target.getHeight(), mask, GL_LINEAR);
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

/**
 * @brief 复制到屏幕
 * @param screenWidth 屏幕宽度
 * @param screenHeight 屏幕高度
 *
 * 将渲染目标内容复制到默认帧缓冲（屏幕）
 */
void RenderTarget::copyToScreen(int screenWidth, int screenHeight) {
  if (!isValid()) {
    return;
  }

  glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

  glBlitFramebuffer(0, 0, width_, height_, 0, 0, screenWidth, screenHeight,
                    GL_COLOR_BUFFER_BIT, GL_LINEAR);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

/**
 * @brief 保存渲染目标到文件
 * @param filepath 文件路径
 * @return 保存成功返回true，失败返回false
 *
 * 将渲染目标内容保存为PNG图片文件
 */
bool RenderTarget::saveToFile(const std::string &filepath) {
  if (!isValid() || !colorTexture_) {
    return false;
  }

  // 读取像素数据
  std::vector<uint8_t> pixels(width_ * height_ * 4);

  bind();
  glReadPixels(0, 0, width_, height_, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());
  unbind();

  // 翻转Y轴（OpenGL坐标系原点在左下角，PNG需要左上角原点）
  std::vector<uint8_t> flipped(width_ * height_ * 4);
  for (int y = 0; y < height_; ++y) {
    for (int x = 0; x < width_; ++x) {
      int srcIdx = ((height_ - 1 - y) * width_ + x) * 4;
      int dstIdx = (y * width_ + x) * 4;
      for (int c = 0; c < 4; ++c) {
        flipped[dstIdx + c] = pixels[srcIdx + c];
      }
    }
  }

  // 使用stb_image_write保存为PNG
  int result = stbi_write_png(filepath.c_str(), width_, height_, 4,
                              flipped.data(), width_ * 4);

  if (result == 0) {
    E2D_ERROR("保存渲染目标到PNG失败: {}", filepath);
    return false;
  }

  E2D_INFO("保存渲染目标到: {}", filepath);
  return true;
}

/**
 * @brief 根据配置创建渲染目标
 * @param config 渲染目标配置
 * @return 创建成功返回渲染目标指针，失败返回nullptr
 *
 * 静态工厂方法，创建并初始化渲染目标
 */
Ptr<RenderTarget>
RenderTarget::createFromConfig(const RenderTargetConfig &config) {
  auto rt = std::make_shared<RenderTarget>();
  if (rt->create(config)) {
    return rt;
  }
  return nullptr;
}

/**
 * @brief 获取当前绑定的帧缓冲对象ID
 * @return 当前绑定的FBO ID
 *
 * 查询OpenGL当前绑定的帧缓冲对象
 */
GLuint RenderTarget::getCurrentFBO() {
  GLint fbo = 0;
  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &fbo);
  return static_cast<GLuint>(fbo);
}

/**
 * @brief 绑定默认帧缓冲
 *
 * 将默认帧缓冲（屏幕）绑定为当前渲染目标
 */
void RenderTarget::bindDefault() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

// ============================================================================
// 内部方法
// ============================================================================

/**
 * @brief 创建帧缓冲对象
 * @return 创建成功返回true，失败返回false
 *
 * 内部方法，创建FBO及相关附件
 */
bool RenderTarget::createFBO() {
  // 创建颜色纹理
  colorTexture_ = GLTexture::create(width_, height_, colorFormat_);
  if (!colorTexture_ || !colorTexture_->isValid()) {
    E2D_ERROR("创建颜色纹理失败");
    return false;
  }

  // 创建FBO
  glGenFramebuffers(1, &fbo_);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo_);

  // 附加颜色纹理
  GLuint colorTexId =
      static_cast<GLTexture *>(colorTexture_.get())->getTextureID();
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         colorTexId, 0);

  // 创建深度/模板缓冲
  if (hasDepth_ || hasStencil_) {
    glGenRenderbuffers(1, &rbo_);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo_);

    if (hasDepth_ && hasStencil_) {
      glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width_,
                            height_);
      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                GL_RENDERBUFFER, rbo_);
      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT,
                                GL_RENDERBUFFER, rbo_);
    } else if (hasDepth_) {
      glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width_,
                            height_);
      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                GL_RENDERBUFFER, rbo_);
    }

    glBindRenderbuffer(GL_RENDERBUFFER, 0);
  }

  // 检查完整性
  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  if (status != GL_FRAMEBUFFER_COMPLETE) {
    E2D_ERROR("FBO创建失败，状态: {:#x}", status);
    deleteFBO();
    return false;
  }

  return true;
}

/**
 * @brief 删除帧缓冲对象
 *
 * 内部方法，删除FBO和渲染缓冲对象
 */
void RenderTarget::deleteFBO() {
  if (rbo_ != 0) {
    glDeleteRenderbuffers(1, &rbo_);
    rbo_ = 0;
  }

  if (fbo_ != 0) {
    glDeleteFramebuffers(1, &fbo_);
    fbo_ = 0;
  }
}

// ============================================================================
// MultisampleRenderTarget实现
// ============================================================================

/**
 * @brief 创建多重采样渲染目标
 * @param width 宽度
 * @param height 高度
 * @param samples 采样数
 * @return 创建成功返回true，失败返回false
 *
 * 创建支持多重采样抗锯齿的渲染目标
 */
bool MultisampleRenderTarget::create(int width, int height, int samples) {
  // 先销毁现有的
  destroy();

  width_ = width;
  height_ = height;
  samples_ = samples;
  hasDepth_ = true;
  hasStencil_ = false;
  colorFormat_ = PixelFormat::RGBA8;

  // 创建FBO
  glGenFramebuffers(1, &fbo_);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo_);

  // 创建多重采样颜色渲染缓冲
  glGenRenderbuffers(1, &colorRBO_);
  glBindRenderbuffer(GL_RENDERBUFFER, colorRBO_);
  glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_RGBA8, width,
                                   height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                            GL_RENDERBUFFER, colorRBO_);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);

  // 创建多重采样深度渲染缓冲
  glGenRenderbuffers(1, &rbo_);
  glBindRenderbuffer(GL_RENDERBUFFER, rbo_);
  glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples,
                                   GL_DEPTH_COMPONENT24, width, height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                            GL_RENDERBUFFER, rbo_);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);

  // 检查完整性
  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  if (status != GL_FRAMEBUFFER_COMPLETE) {
    E2D_ERROR("多重采样FBO创建失败，状态: {:#x}", status);
    destroy();
    return false;
  }

  E2D_INFO("创建多重采样渲染目标: {}x{} (采样数: {})", width, height, samples);
  return true;
}

/**
 * @brief 销毁多重采样渲染目标
 *
 * 释放多重采样渲染缓冲和帧缓冲对象
 */
void MultisampleRenderTarget::destroy() {
  // 删除颜色渲染缓冲
  if (colorRBO_ != 0) {
    glDeleteRenderbuffers(1, &colorRBO_);
    colorRBO_ = 0;
  }

  // 调用基类destroy
  RenderTarget::destroy();
}

/**
 * @brief 解析多重采样到目标渲染目标
 * @param target 目标渲染目标
 *
 * 将多重采样渲染目标解析到普通渲染目标
 */
void MultisampleRenderTarget::resolveTo(RenderTarget &target) {
  if (!isValid() || !target.isValid()) {
    return;
  }

  // 使用glBlitFramebuffer解析多重采样
  glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, target.getFBO());

  glBlitFramebuffer(0, 0, width_, height_, 0, 0, target.getWidth(),
                    target.getHeight(), GL_COLOR_BUFFER_BIT, GL_LINEAR);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// ============================================================================
// RenderTargetStack实现
// ============================================================================

/**
 * @brief 获取RenderTargetStack单例实例
 * @return RenderTargetStack单例的引用
 *
 * 使用静态局部变量实现线程安全的单例模式
 */
RenderTargetStack &RenderTargetStack::get() {
  static RenderTargetStack instance;
  return instance;
}

/**
 * @brief 压入渲染目标到栈
 * @param target 渲染目标指针
 *
 * 将渲染目标压入栈并绑定为当前渲染目标
 */
void RenderTargetStack::push(RenderTarget *target) {
  std::lock_guard<std::mutex> lock(mutex_);

  if (target) {
    stack_.push_back(target);
    target->bind();
  }
}

/**
 * @brief 弹出栈顶渲染目标
 *
 * 弹出当前渲染目标并恢复前一个渲染目标
 */
void RenderTargetStack::pop() {
  std::lock_guard<std::mutex> lock(mutex_);

  if (!stack_.empty()) {
    stack_.pop_back();
  }

  // 绑定新的当前渲染目标
  if (!stack_.empty()) {
    stack_.back()->bind();
  } else {
    RenderTarget::bindDefault();
  }
}

/**
 * @brief 获取当前渲染目标
 * @return 当前渲染目标指针，栈为空返回nullptr
 */
RenderTarget *RenderTargetStack::getCurrent() const {
  std::lock_guard<std::mutex> lock(mutex_);

  if (stack_.empty()) {
    return nullptr;
  }
  return stack_.back();
}

/**
 * @brief 获取栈大小
 * @return 栈中渲染目标的数量
 */
size_t RenderTargetStack::size() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return stack_.size();
}

/**
 * @brief 清空渲染目标栈
 *
 * 清空栈并恢复默认帧缓冲
 */
void RenderTargetStack::clear() {
  std::lock_guard<std::mutex> lock(mutex_);
  stack_.clear();
  RenderTarget::bindDefault();
}

// ============================================================================
// RenderTargetMgr实现
// ============================================================================

/**
 * @brief 获取RenderTargetMgr单例实例
 * @return RenderTargetMgr单例的引用
 *
 * 使用静态局部变量实现线程安全的单例模式
 */
RenderTargetMgr &RenderTargetMgr::get() {
  static RenderTargetMgr instance;
  return instance;
}

/**
 * @brief 初始化渲染目标管理器
 * @param width 默认渲染目标宽度
 * @param height 默认渲染目标高度
 * @return 初始化成功返回true，失败返回false
 *
 * 创建默认渲染目标
 */
bool RenderTargetMgr::init(int width, int height) {
  if (initialized_) {
    return true;
  }

  // 创建默认渲染目标
  RenderTargetConfig config;
  config.width = width;
  config.height = height;
  config.hasDepth = true;
  config.hasStencil = false;

  defaultRenderTarget_ = RenderTarget::createFromConfig(config);
  if (!defaultRenderTarget_) {
    E2D_ERROR("创建默认渲染目标失败");
    return false;
  }

  initialized_ = true;
  E2D_INFO("渲染目标管理器初始化完成: {}x{}", width, height);
  return true;
}

/**
 * @brief 关闭渲染目标管理器
 *
 * 清理所有渲染目标资源
 */
void RenderTargetMgr::shutdown() {
  if (!initialized_) {
    return;
  }

  renderTargets_.clear();
  defaultRenderTarget_.reset();
  initialized_ = false;

  E2D_INFO("渲染目标管理器已关闭");
}

/**
 * @brief 创建渲染目标
 * @param config 渲染目标配置
 * @return 创建成功返回渲染目标指针，失败返回nullptr
 *
 * 创建新的渲染目标并由管理器管理
 */
Ptr<RenderTarget>
RenderTargetMgr::createRenderTarget(const RenderTargetConfig &config) {
  if (!initialized_) {
    E2D_ERROR("渲染目标管理器未初始化");
    return nullptr;
  }

  auto rt = RenderTarget::createFromConfig(config);
  if (rt) {
    renderTargets_.push_back(rt);
  }
  return rt;
}

/**
 * @brief 调整所有渲染目标大小
 * @param width 新宽度
 * @param height 新高度
 *
 * 调整默认渲染目标的大小
 */
void RenderTargetMgr::resize(int width, int height) {
  if (!initialized_) {
    return;
  }

  // 调整默认渲染目标大小
  if (defaultRenderTarget_) {
    defaultRenderTarget_->resize(width, height);
  }

  E2D_INFO("渲染目标管理器调整大小: {}x{}", width, height);
}

} // namespace extra2d
