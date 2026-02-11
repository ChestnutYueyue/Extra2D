#include <extra2d/effects/post_process.h>
#include <extra2d/graphics/render_backend.h>
#include <extra2d/graphics/render_target.h>
#include <extra2d/utils/logger.h>

#include <glad/glad.h>

namespace extra2d {

// ============================================================================
// 静态成员初始化
// ============================================================================
GLuint PostProcessEffect::quadVao_ = 0;
GLuint PostProcessEffect::quadVbo_ = 0;
bool PostProcessEffect::quadInitialized_ = false;

// ============================================================================
// PostProcessEffect实现
// ============================================================================

PostProcessEffect::PostProcessEffect(const std::string &name) : name_(name) {}

bool PostProcessEffect::init() {
  initQuad();
  valid_ = true;
  return true;
}

void PostProcessEffect::shutdown() {
  shader_.reset();
  valid_ = false;
}

void PostProcessEffect::apply(const Texture &source, RenderTarget &target,
                              RenderBackend &renderer) {
  if (!enabled_ || !valid_)
    return;

  target.bind();

  if (shader_) {
    shader_->bind();
    shader_->setInt("u_texture", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D,
                  static_cast<GLuint>(
                      reinterpret_cast<uintptr_t>(source.getNativeHandle())));

    onShaderBind(*shader_);
  }

  renderFullscreenQuad();

  if (shader_) {
    shader_->unbind();
  }

  target.unbind();
}

bool PostProcessEffect::loadShader(const std::string &vertSource,
                                   const std::string &fragSource) {
  shader_ = std::make_shared<GLShader>();
  if (!shader_->compileFromSource(vertSource.c_str(), fragSource.c_str())) {
    E2D_ERROR("后处理效果 '{}' 加载Shader失败", name_);
    shader_.reset();
    return false;
  }
  return true;
}

bool PostProcessEffect::loadShaderFromFile(const std::string &vertPath,
                                           const std::string &fragPath) {
  shader_ = std::make_shared<GLShader>();
  if (!shader_->compileFromFile(vertPath, fragPath)) {
    E2D_ERROR("后处理效果 '{}' 从文件加载Shader失败", name_);
    shader_.reset();
    return false;
  }
  return true;
}

void PostProcessEffect::initQuad() {
  if (quadInitialized_)
    return;

  // 全屏四边形顶点数据（位置和纹理坐标）
  float quadVertices[] = {// 位置              // 纹理坐标
                          -1.0f, 1.0f,  0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f,
                          1.0f,  -1.0f, 1.0f, 0.0f, -1.0f, 1.0f,  0.0f, 1.0f,
                          1.0f,  -1.0f, 1.0f, 0.0f, 1.0f,  1.0f,  1.0f, 1.0f};

  glGenVertexArrays(1, &quadVao_);
  glGenBuffers(1, &quadVbo_);

  glBindVertexArray(quadVao_);
  glBindBuffer(GL_ARRAY_BUFFER, quadVbo_);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices,
               GL_STATIC_DRAW);

  // 位置属性
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);

  // 纹理坐标属性
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                        (void *)(2 * sizeof(float)));

  glBindVertexArray(0);

  quadInitialized_ = true;
}

void PostProcessEffect::destroyQuad() {
  if (quadVao_ != 0) {
    glDeleteVertexArrays(1, &quadVao_);
    quadVao_ = 0;
  }
  if (quadVbo_ != 0) {
    glDeleteBuffers(1, &quadVbo_);
    quadVbo_ = 0;
  }
  quadInitialized_ = false;
}

void PostProcessEffect::renderFullscreenQuad() {
  if (!quadInitialized_) {
    initQuad();
  }

  glBindVertexArray(quadVao_);
  glDrawArrays(GL_TRIANGLES, 0, 6);
  glBindVertexArray(0);
}

// ============================================================================
// PostProcessStack实现
// ============================================================================

PostProcessStack::PostProcessStack() = default;

PostProcessStack::~PostProcessStack() { shutdown(); }

bool PostProcessStack::init(int width, int height) {
  E2D_INFO("初始化后处理栈...");

  width_ = width;
  height_ = height;

  // 创建两个渲染目标用于乒乓渲染
  RenderTargetConfig config;
  config.width = width;
  config.height = height;
  config.hasDepthBuffer = false;
  config.autoResize = false;

  renderTargetA_ = std::make_shared<RenderTarget>();
  renderTargetB_ = std::make_shared<RenderTarget>();

  if (!renderTargetA_->init(config)) {
    E2D_ERROR("创建后处理渲染目标A失败");
    return false;
  }

  if (!renderTargetB_->init(config)) {
    E2D_ERROR("创建后处理渲染目标B失败");
    return false;
  }

  valid_ = true;
  E2D_INFO("后处理栈初始化成功");
  return true;
}

void PostProcessStack::shutdown() {
  E2D_INFO("关闭后处理栈...");

  clearEffects();

  if (renderTargetA_) {
    renderTargetA_->shutdown();
    renderTargetA_.reset();
  }

  if (renderTargetB_) {
    renderTargetB_->shutdown();
    renderTargetB_.reset();
  }

  valid_ = false;
}

void PostProcessStack::addEffect(Ptr<PostProcessEffect> effect) {
  if (effect && effect->init()) {
    effects_.push_back(effect);
    E2D_INFO("添加后处理效果: {}", effect->getName());
  }
}

void PostProcessStack::insertEffect(size_t index,
                                    Ptr<PostProcessEffect> effect) {
  if (effect && effect->init() && index <= effects_.size()) {
    effects_.insert(effects_.begin() + index, effect);
    E2D_INFO("插入后处理效果 '{}' 到位置 {}", effect->getName(), index);
  }
}

void PostProcessStack::removeEffect(const std::string &name) {
  for (auto it = effects_.begin(); it != effects_.end(); ++it) {
    if ((*it)->getName() == name) {
      (*it)->shutdown();
      effects_.erase(it);
      E2D_INFO("移除后处理效果: {}", name);
      return;
    }
  }
}

void PostProcessStack::removeEffect(size_t index) {
  if (index < effects_.size()) {
    effects_[index]->shutdown();
    effects_.erase(effects_.begin() + index);
  }
}

Ptr<PostProcessEffect> PostProcessStack::getEffect(const std::string &name) {
  for (auto &effect : effects_) {
    if (effect->getName() == name) {
      return effect;
    }
  }
  return nullptr;
}

Ptr<PostProcessEffect> PostProcessStack::getEffect(size_t index) {
  if (index < effects_.size()) {
    return effects_[index];
  }
  return nullptr;
}

void PostProcessStack::clearEffects() {
  for (auto &effect : effects_) {
    effect->shutdown();
  }
  effects_.clear();
}

void PostProcessStack::beginCapture() {
  if (!valid_)
    return;

  renderTargetA_->bind();
  renderTargetA_->clear(Colors::Black);
  capturing_ = true;
}

void PostProcessStack::endCapture(RenderBackend &renderer) {
  if (!valid_ || !capturing_)
    return;

  renderTargetA_->unbind();

  // 应用所有后处理效果
  if (effects_.empty()) {
    // 没有效果，直接渲染到屏幕
    // 这里需要渲染renderTargetA_的纹理到屏幕
    capturing_ = false;
    return;
  }

  // 乒乓渲染
  RenderTarget *readTarget = renderTargetA_.get();
  RenderTarget *writeTarget = renderTargetB_.get();

  for (size_t i = 0; i < effects_.size(); ++i) {
    auto &effect = effects_[i];

    if (effect->isEnabled()) {
      effect->apply(*readTarget->getColorTexture(), *writeTarget, renderer);
    }

    // 交换读写目标
    std::swap(readTarget, writeTarget);
  }

  // 最终结果在readTarget中（因为最后一次交换）
  // 这里应该将结果渲染到屏幕

  capturing_ = false;
}

void PostProcessStack::process(const Texture &source, RenderTarget &target,
                               RenderBackend &renderer) {
  if (!valid_)
    return;

  RenderTarget *readTarget = nullptr;
  RenderTarget *writeTarget = nullptr;

  // 确定读写目标
  if (target.getFBO() == renderTargetA_->getFBO()) {
    readTarget = renderTargetB_.get();
    writeTarget = renderTargetA_.get();
  } else {
    readTarget = renderTargetA_.get();
    writeTarget = renderTargetB_.get();
  }

  // 首先将源纹理复制到读目标
  readTarget->bind();
  // 这里需要渲染源纹理到readTarget
  readTarget->unbind();

  // 应用效果
  for (auto &effect : effects_) {
    if (effect->isEnabled()) {
      effect->apply(*readTarget->getColorTexture(), *writeTarget, renderer);
    }
    std::swap(readTarget, writeTarget);
  }

  // 将最终结果复制到目标
  readTarget->blitTo(target, true, false);
}

void PostProcessStack::resize(int width, int height) {
  if (width_ == width && height_ == height)
    return;

  width_ = width;
  height_ = height;

  if (renderTargetA_) {
    renderTargetA_->resize(width, height);
  }

  if (renderTargetB_) {
    renderTargetB_->resize(width, height);
  }
}

// ============================================================================
// PostProcessManager实现
// ============================================================================

PostProcessManager &PostProcessManager::getInstance() {
  static PostProcessManager instance;
  return instance;
}

void PostProcessManager::init(int width, int height) {
  if (initialized_)
    return;

  E2D_INFO("初始化后处理管理器...");
  mainStack_.init(width, height);
  initialized_ = true;
}

void PostProcessManager::shutdown() {
  if (!initialized_)
    return;

  E2D_INFO("关闭后处理管理器...");
  mainStack_.shutdown();
  initialized_ = false;
}

void PostProcessManager::resize(int width, int height) {
  if (initialized_) {
    mainStack_.resize(width, height);
  }
}

void PostProcessManager::beginFrame() {
  if (initialized_) {
    mainStack_.beginCapture();
  }
}

void PostProcessManager::endFrame(RenderBackend &renderer) {
  if (initialized_) {
    mainStack_.endCapture(renderer);
  }
}

} // namespace extra2d
