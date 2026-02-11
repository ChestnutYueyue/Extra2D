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

RenderTarget::RenderTarget() = default;

RenderTarget::~RenderTarget() { destroy(); }

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

void RenderTarget::destroy() {
  deleteFBO();

  colorTexture_.reset();
  depthTexture_.reset();

  width_ = 0;
  height_ = 0;
}

void RenderTarget::bind() {
  if (!isValid()) {
    return;
  }

  glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
  glViewport(0, 0, width_, height_);
}

void RenderTarget::unbind() { bindDefault(); }

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

void RenderTarget::setViewport(int x, int y, int width, int height) {
  if (!isValid()) {
    return;
  }

  bind();
  glViewport(x, y, width, height);
}

void RenderTarget::getFullViewport(int &x, int &y, int &width,
                                   int &height) const {
  x = 0;
  y = 0;
  width = width_;
  height = height_;
}

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

Ptr<RenderTarget>
RenderTarget::createFromConfig(const RenderTargetConfig &config) {
  auto rt = std::make_shared<RenderTarget>();
  if (rt->create(config)) {
    return rt;
  }
  return nullptr;
}

GLuint RenderTarget::getCurrentFBO() {
  GLint fbo = 0;
  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &fbo);
  return static_cast<GLuint>(fbo);
}

void RenderTarget::bindDefault() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

// ============================================================================
// 内部方法
// ============================================================================

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

void MultisampleRenderTarget::destroy() {
  // 删除颜色渲染缓冲
  if (colorRBO_ != 0) {
    glDeleteRenderbuffers(1, &colorRBO_);
    colorRBO_ = 0;
  }

  // 调用基类destroy
  RenderTarget::destroy();
}

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

RenderTargetStack &RenderTargetStack::getInstance() {
  static RenderTargetStack instance;
  return instance;
}

void RenderTargetStack::push(RenderTarget *target) {
  std::lock_guard<std::mutex> lock(mutex_);

  if (target) {
    stack_.push_back(target);
    target->bind();
  }
}

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

RenderTarget *RenderTargetStack::getCurrent() const {
  std::lock_guard<std::mutex> lock(mutex_);

  if (stack_.empty()) {
    return nullptr;
  }
  return stack_.back();
}

size_t RenderTargetStack::size() const {
  std::lock_guard<std::mutex> lock(mutex_);
  return stack_.size();
}

void RenderTargetStack::clear() {
  std::lock_guard<std::mutex> lock(mutex_);
  stack_.clear();
  RenderTarget::bindDefault();
}

// ============================================================================
// RenderTargetManager实现
// ============================================================================

RenderTargetManager &RenderTargetManager::getInstance() {
  static RenderTargetManager instance;
  return instance;
}

bool RenderTargetManager::init(int width, int height) {
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

void RenderTargetManager::shutdown() {
  if (!initialized_) {
    return;
  }

  renderTargets_.clear();
  defaultRenderTarget_.reset();
  initialized_ = false;

  E2D_INFO("渲染目标管理器已关闭");
}

Ptr<RenderTarget>
RenderTargetManager::createRenderTarget(const RenderTargetConfig &config) {
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

void RenderTargetManager::resize(int width, int height) {
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
