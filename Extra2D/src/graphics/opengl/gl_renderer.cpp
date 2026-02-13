#include <SDL.h>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <extra2d/core/string.h>
#include <extra2d/graphics/gpu_context.h>
#include <extra2d/graphics/opengl/gl_font_atlas.h>
#include <extra2d/graphics/opengl/gl_renderer.h>
#include <extra2d/graphics/opengl/gl_texture.h>
#include <extra2d/graphics/vram_manager.h>
#include <extra2d/platform/window.h>
#include <extra2d/utils/logger.h>
#include <vector>

namespace extra2d {

// 形状渲染着色器 - 支持顶点颜色批处理 (GLES 3.2)
static const char *SHAPE_VERTEX_SHADER = R"(
#version 300 es
precision highp float;
layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec4 aColor;
uniform mat4 uViewProjection;
out vec4 vColor;
void main() {
    gl_Position = uViewProjection * vec4(aPosition, 0.0, 1.0);
    vColor = aColor;
}
)";

static const char *SHAPE_FRAGMENT_SHADER = R"(
#version 300 es
precision highp float;
in vec4 vColor;
out vec4 fragColor;
void main() {
    fragColor = vColor;
}
)";

// VBO 初始大小（用于 VRAM 跟踪）
static constexpr size_t SHAPE_VBO_SIZE = 1024 * sizeof(float);

// ============================================================================
// BlendMode 查找表 - 编译期构建，运行时 O(1) 查找
// ============================================================================
struct BlendState {
  bool enable;
  GLenum srcFactor;
  GLenum dstFactor;
};

static constexpr BlendState BLEND_STATES[] = {
    {false, 0, 0},                                // BlendMode::None
    {true, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA}, // BlendMode::Alpha
    {true, GL_SRC_ALPHA, GL_ONE},                 // BlendMode::Additive
    {true, GL_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA}  // BlendMode::Multiply
};

static constexpr size_t BLEND_STATE_COUNT =
    sizeof(BLEND_STATES) / sizeof(BLEND_STATES[0]);

GLRenderer::GLRenderer()
    : window_(nullptr), shapeVao_(0), shapeVbo_(0), lineVao_(0), lineVbo_(0),
      vsync_(true), shapeVertexCount_(0), currentShapeMode_(GL_TRIANGLES),
      lineVertexCount_(0), currentLineWidth_(1.0f) {
  resetStats();
  for (auto &v : shapeVertexCache_) {
    v = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
  }
  for (auto &v : lineVertexCache_) {
    v = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
  }
}

GLRenderer::~GLRenderer() { shutdown(); }

bool GLRenderer::init(Window *window) {
  window_ = window;

  // Switch: GL 上下文已通过 SDL2 + EGL 初始化，无需 glewInit()

  // 初始化精灵批渲染器
  if (!spriteBatch_.init()) {
    E2D_LOG_ERROR("Failed to initialize sprite batch");
    return false;
  }

  // 初始化形状渲染
  initShapeRendering();

  // 设置 OpenGL 状态
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // 标记 GPU 上下文为有效
  GPUContext::getInstance().markValid();

  E2D_LOG_INFO("OpenGL Renderer initialized");
  E2D_LOG_INFO("OpenGL Version: {}",
               reinterpret_cast<const char *>(glGetString(GL_VERSION)));

  return true;
}

void GLRenderer::shutdown() {
  // 标记 GPU 上下文为无效
  // 这会在销毁 OpenGL 上下文之前通知所有 GPU 资源
  GPUContext::getInstance().markInvalid();

  spriteBatch_.shutdown();

  if (lineVbo_ != 0) {
    glDeleteBuffers(1, &lineVbo_);
    VRAMManager::getInstance().freeBuffer(MAX_LINE_VERTICES *
                                          sizeof(ShapeVertex));
    lineVbo_ = 0;
  }
  if (lineVao_ != 0) {
    glDeleteVertexArrays(1, &lineVao_);
    lineVao_ = 0;
  }
  if (shapeVbo_ != 0) {
    glDeleteBuffers(1, &shapeVbo_);
    VRAMManager::getInstance().freeBuffer(MAX_SHAPE_VERTICES *
                                          sizeof(ShapeVertex));
    shapeVbo_ = 0;
  }
  if (shapeVao_ != 0) {
    glDeleteVertexArrays(1, &shapeVao_);
    shapeVao_ = 0;
  }
}

void GLRenderer::beginFrame(const Color &clearColor) {
  glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
  glClear(GL_COLOR_BUFFER_BIT);
  resetStats();
}

void GLRenderer::endFrame() {
  // 刷新所有待处理的形状批次
  flushShapeBatch();
  // 刷新所有待处理的线条批次
  flushLineBatch();
}

void GLRenderer::setViewport(int x, int y, int width, int height) {
  glViewport(x, y, width, height);
}

void GLRenderer::setVSync(bool enabled) {
  vsync_ = enabled;
  // 使用 SDL2 设置交换间隔
  SDL_GL_SetSwapInterval(enabled ? 1 : 0);
}

void GLRenderer::setBlendMode(BlendMode mode) {
  // 状态缓存检查，避免冗余 GL 调用
  if (cachedBlendMode_ == mode) {
    return;
  }
  cachedBlendMode_ = mode;

  // 使用查找表替代 switch
  size_t index = static_cast<size_t>(mode);
  if (index >= BLEND_STATE_COUNT) {
    index = 0;
  }

  const BlendState &state = BLEND_STATES[index];
  if (state.enable) {
    if (!blendEnabled_) {
      glEnable(GL_BLEND);
      blendEnabled_ = true;
    }
    glBlendFunc(state.srcFactor, state.dstFactor);
  } else {
    if (blendEnabled_) {
      glDisable(GL_BLEND);
      blendEnabled_ = false;
    }
  }
}

void GLRenderer::setViewProjection(const glm::mat4 &matrix) {
  viewProjection_ = matrix;
}

void GLRenderer::pushTransform(const glm::mat4 &transform) {
  if (transformStack_.empty()) {
    transformStack_.push_back(transform);
  } else {
    transformStack_.push_back(transformStack_.back() * transform);
  }
}

void GLRenderer::popTransform() {
  if (!transformStack_.empty()) {
    transformStack_.pop_back();
  }
}

glm::mat4 GLRenderer::getCurrentTransform() const {
  if (transformStack_.empty()) {
    return glm::mat4(1.0f);
  }
  return transformStack_.back();
}

Ptr<Texture> GLRenderer::createTexture(int width, int height,
                                       const uint8_t *pixels, int channels) {
  return makePtr<GLTexture>(width, height, pixels, channels);
}

Ptr<Texture> GLRenderer::loadTexture(const std::string &filepath) {
  return makePtr<GLTexture>(filepath);
}

void GLRenderer::beginSpriteBatch() { spriteBatch_.begin(viewProjection_); }

void GLRenderer::drawSprite(const Texture &texture, const Rect &destRect,
                            const Rect &srcRect, const Color &tint,
                            float rotation, const Vec2 &anchor) {
  GLSpriteBatch::SpriteData data;
  data.position = glm::vec2(destRect.origin.x, destRect.origin.y);
  data.size = glm::vec2(destRect.size.width, destRect.size.height);

  Texture *tex = const_cast<Texture *>(&texture);
  float texW = static_cast<float>(tex->getWidth());
  float texH = static_cast<float>(tex->getHeight());

  // 纹理坐标计算
  float u1 = srcRect.origin.x / texW;
  float u2 = (srcRect.origin.x + srcRect.size.width) / texW;
  float v1 = srcRect.origin.y / texH;
  float v2 = (srcRect.origin.y + srcRect.size.height) / texH;

  data.texCoordMin = glm::vec2(glm::min(u1, u2), glm::min(v1, v2));
  data.texCoordMax = glm::vec2(glm::max(u1, u2), glm::max(v1, v2));

  data.color = glm::vec4(tint.r, tint.g, tint.b, tint.a);
  data.rotation = rotation * 3.14159f / 180.0f;
  data.anchor = glm::vec2(anchor.x, anchor.y);
  data.isSDF = false;

  spriteBatch_.draw(texture, data);
}

void GLRenderer::drawSprite(const Texture &texture, const Vec2 &position,
                            const Color &tint) {
  Rect destRect(position.x, position.y, static_cast<float>(texture.getWidth()),
                static_cast<float>(texture.getHeight()));
  Rect srcRect(0, 0, static_cast<float>(texture.getWidth()),
               static_cast<float>(texture.getHeight()));
  drawSprite(texture, destRect, srcRect, tint, 0.0f, Vec2(0, 0));
}

void GLRenderer::endSpriteBatch() {
  spriteBatch_.end();
  stats_.drawCalls += spriteBatch_.getDrawCallCount();
}

void GLRenderer::drawLine(const Vec2 &start, const Vec2 &end,
                          const Color &color, float width) {
  // 如果线宽改变，需要先刷新线条批次
  if (width != currentLineWidth_) {
    flushLineBatch();
    currentLineWidth_ = width;
  }

  // 添加两个顶点到线条缓冲区
  addLineVertex(start.x, start.y, color);
  addLineVertex(end.x, end.y, color);
}

void GLRenderer::drawRect(const Rect &rect, const Color &color, float width) {
  // 如果线宽改变，需要先刷新线条批次
  if (width != currentLineWidth_) {
    flushLineBatch();
    currentLineWidth_ = width;
  }

  float x1 = rect.origin.x;
  float y1 = rect.origin.y;
  float x2 = rect.origin.x + rect.size.width;
  float y2 = rect.origin.y + rect.size.height;

  // 4条线段 = 8个顶点
  // 上边
  addLineVertex(x1, y1, color);
  addLineVertex(x2, y1, color);
  // 右边
  addLineVertex(x2, y1, color);
  addLineVertex(x2, y2, color);
  // 下边
  addLineVertex(x2, y2, color);
  addLineVertex(x1, y2, color);
  // 左边
  addLineVertex(x1, y2, color);
  addLineVertex(x1, y1, color);
}

void GLRenderer::fillRect(const Rect &rect, const Color &color) {
  // 提交当前批次（如果模式不同）
  submitShapeBatch(GL_TRIANGLES);

  // 添加两个三角形组成矩形（6个顶点）
  float x1 = rect.origin.x;
  float y1 = rect.origin.y;
  float x2 = rect.origin.x + rect.size.width;
  float y2 = rect.origin.y + rect.size.height;

  // 三角形1: (x1,y1), (x2,y1), (x2,y2)
  addShapeVertex(x1, y1, color);
  addShapeVertex(x2, y1, color);
  addShapeVertex(x2, y2, color);

  // 三角形2: (x1,y1), (x2,y2), (x1,y2)
  addShapeVertex(x1, y1, color);
  addShapeVertex(x2, y2, color);
  addShapeVertex(x1, y2, color);
}

void GLRenderer::drawCircle(const Vec2 &center, float radius,
                            const Color &color, int segments, float width) {
  // 限制段数不超过缓存大小
  if (segments > static_cast<int>(MAX_CIRCLE_SEGMENTS)) {
    segments = static_cast<int>(MAX_CIRCLE_SEGMENTS);
  }

  // 如果线宽改变，需要先刷新线条批次
  if (width != currentLineWidth_) {
    flushLineBatch();
    currentLineWidth_ = width;
  }

  // 使用线条批处理绘制圆形
  for (int i = 0; i < segments; ++i) {
    float angle1 =
        2.0f * 3.14159f * static_cast<float>(i) / static_cast<float>(segments);
    float angle2 = 2.0f * 3.14159f * static_cast<float>(i + 1) /
                   static_cast<float>(segments);

    addLineVertex(center.x + radius * cosf(angle1),
                  center.y + radius * sinf(angle1), color);
    addLineVertex(center.x + radius * cosf(angle2),
                  center.y + radius * sinf(angle2), color);
  }
}

void GLRenderer::fillCircle(const Vec2 &center, float radius,
                            const Color &color, int segments) {
  // 限制段数不超过缓存大小
  if (segments > static_cast<int>(MAX_CIRCLE_SEGMENTS)) {
    segments = static_cast<int>(MAX_CIRCLE_SEGMENTS);
  }

  // 提交当前批次（如果模式不同）
  submitShapeBatch(GL_TRIANGLES);

  // 使用三角形扇形填充圆
  // 中心点 + 边缘点
  for (int i = 0; i < segments; ++i) {
    float angle1 =
        2.0f * 3.14159f * static_cast<float>(i) / static_cast<float>(segments);
    float angle2 = 2.0f * 3.14159f * static_cast<float>(i + 1) /
                   static_cast<float>(segments);

    // 每个三角形：中心 -> 边缘点1 -> 边缘点2
    addShapeVertex(center.x, center.y, color);
    addShapeVertex(center.x + radius * cosf(angle1),
                   center.y + radius * sinf(angle1), color);
    addShapeVertex(center.x + radius * cosf(angle2),
                   center.y + radius * sinf(angle2), color);
  }
}

void GLRenderer::drawTriangle(const Vec2 &p1, const Vec2 &p2, const Vec2 &p3,
                              const Color &color, float width) {
  drawLine(p1, p2, color, width);
  drawLine(p2, p3, color, width);
  drawLine(p3, p1, color, width);
}

void GLRenderer::fillTriangle(const Vec2 &p1, const Vec2 &p2, const Vec2 &p3,
                              const Color &color) {
  submitShapeBatch(GL_TRIANGLES);

  addShapeVertex(p1.x, p1.y, color);
  addShapeVertex(p2.x, p2.y, color);
  addShapeVertex(p3.x, p3.y, color);
}

void GLRenderer::drawPolygon(const std::vector<Vec2> &points,
                             const Color &color, float width) {
  if (points.size() < 2)
    return;

  // 如果线宽改变，需要先刷新线条批次
  if (width != currentLineWidth_) {
    flushLineBatch();
    currentLineWidth_ = width;
  }

  // 绘制所有边
  for (size_t i = 0; i < points.size(); ++i) {
    const Vec2 &p1 = points[i];
    const Vec2 &p2 = points[(i + 1) % points.size()];
    addLineVertex(p1.x, p1.y, color);
    addLineVertex(p2.x, p2.y, color);
  }
}

void GLRenderer::fillPolygon(const std::vector<Vec2> &points,
                             const Color &color) {
  if (points.size() < 3)
    return;

  submitShapeBatch(GL_TRIANGLES);

  // 使用三角形扇形填充
  // 从第一个点开始，每两个相邻点组成一个三角形
  for (size_t i = 1; i < points.size() - 1; ++i) {
    addShapeVertex(points[0].x, points[0].y, color);
    addShapeVertex(points[i].x, points[i].y, color);
    addShapeVertex(points[i + 1].x, points[i + 1].y, color);
  }
}

Ptr<FontAtlas> GLRenderer::createFontAtlas(const std::string &filepath,
                                           int fontSize, bool useSDF) {
  return makePtr<GLFontAtlas>(filepath, fontSize, useSDF);
}

void GLRenderer::drawText(const FontAtlas &font, const std::string &text,
                          const Vec2 &position, const Color &color) {
  drawText(font, text, position.x, position.y, color);
}

void GLRenderer::drawText(const FontAtlas &font, const std::string &text,
                          float x, float y, const Color &color) {
  float cursorX = x;
  float cursorY = y;
  float baselineY = cursorY + font.getAscent();

  // 收集所有字符数据用于批处理
  std::vector<GLSpriteBatch::SpriteData> sprites;
  sprites.reserve(text.size());  // 预分配空间

  for (char32_t codepoint : utf8ToUtf32(text)) {
    if (codepoint == '\n') {
      cursorX = x;
      cursorY += font.getLineHeight();
      baselineY = cursorY + font.getAscent();
      continue;
    }

    const Glyph *glyph = font.getGlyph(codepoint);
    if (glyph) {
      float penX = cursorX;
      cursorX += glyph->advance;

      if (glyph->width <= 0.0f || glyph->height <= 0.0f) {
        continue;
      }

      float xPos = penX + glyph->bearingX;
      float yPos = baselineY + glyph->bearingY;

      GLSpriteBatch::SpriteData data;
      data.position = glm::vec2(xPos, yPos);
      data.size = glm::vec2(glyph->width, glyph->height);
      data.texCoordMin = glm::vec2(glyph->u0, glyph->v0);
      data.texCoordMax = glm::vec2(glyph->u1, glyph->v1);
      data.color = glm::vec4(color.r, color.g, color.b, color.a);
      data.rotation = 0.0f;
      data.anchor = glm::vec2(0.0f, 0.0f);
      data.isSDF = font.isSDF();
      
      sprites.push_back(data);
    }
  }

  // 使用批处理绘制所有字符
  if (!sprites.empty()) {
    spriteBatch_.drawBatch(*font.getTexture(), sprites);
  }
}

void GLRenderer::resetStats() { stats_ = Stats{}; }

void GLRenderer::initShapeRendering() {
  // 编译形状着色器
  shapeShader_.compileFromSource(SHAPE_VERTEX_SHADER, SHAPE_FRAGMENT_SHADER);

  // 创建形状 VAO 和 VBO
  glGenVertexArrays(1, &shapeVao_);
  glGenBuffers(1, &shapeVbo_);

  glBindVertexArray(shapeVao_);
  glBindBuffer(GL_ARRAY_BUFFER, shapeVbo_);
  glBufferData(GL_ARRAY_BUFFER, MAX_SHAPE_VERTICES * sizeof(ShapeVertex),
               nullptr, GL_DYNAMIC_DRAW);

  // 位置属性 (location = 0)
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(ShapeVertex),
                        reinterpret_cast<void *>(offsetof(ShapeVertex, x)));

  // 颜色属性 (location = 1)
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(ShapeVertex),
                        reinterpret_cast<void *>(offsetof(ShapeVertex, r)));

  glBindVertexArray(0);

  // 创建线条专用 VAO 和 VBO
  glGenVertexArrays(1, &lineVao_);
  glGenBuffers(1, &lineVbo_);

  glBindVertexArray(lineVao_);
  glBindBuffer(GL_ARRAY_BUFFER, lineVbo_);
  glBufferData(GL_ARRAY_BUFFER, MAX_LINE_VERTICES * sizeof(ShapeVertex),
               nullptr, GL_DYNAMIC_DRAW);

  // 位置属性 (location = 0)
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(ShapeVertex),
                        reinterpret_cast<void *>(offsetof(ShapeVertex, x)));

  // 颜色属性 (location = 1)
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(ShapeVertex),
                        reinterpret_cast<void *>(offsetof(ShapeVertex, r)));

  glBindVertexArray(0);

  // VRAM 跟踪
  VRAMManager::getInstance().allocBuffer(MAX_SHAPE_VERTICES *
                                         sizeof(ShapeVertex));
  VRAMManager::getInstance().allocBuffer(MAX_LINE_VERTICES *
                                         sizeof(ShapeVertex));
}

void GLRenderer::addShapeVertex(float x, float y, const Color &color) {
  if (shapeVertexCount_ >= MAX_SHAPE_VERTICES) {
    flushShapeBatch();
  }
  ShapeVertex &v = shapeVertexCache_[shapeVertexCount_++];
  v.x = x;
  v.y = y;
  v.r = color.r;
  v.g = color.g;
  v.b = color.b;
  v.a = color.a;
}

void GLRenderer::addLineVertex(float x, float y, const Color &color) {
  if (lineVertexCount_ >= MAX_LINE_VERTICES) {
    flushLineBatch();
  }
  ShapeVertex &v = lineVertexCache_[lineVertexCount_++];
  v.x = x;
  v.y = y;
  v.r = color.r;
  v.g = color.g;
  v.b = color.b;
  v.a = color.a;
}

void GLRenderer::submitShapeBatch(GLenum mode) {
  if (shapeVertexCount_ == 0)
    return;

  // 如果模式改变，先刷新
  if (currentShapeMode_ != mode && shapeVertexCount_ > 0) {
    flushShapeBatch();
  }
  currentShapeMode_ = mode;
}

void GLRenderer::flushShapeBatch() {
  if (shapeVertexCount_ == 0)
    return;

  shapeShader_.bind();
  shapeShader_.setMat4("uViewProjection", viewProjection_);

  glBindBuffer(GL_ARRAY_BUFFER, shapeVbo_);
  glBufferSubData(GL_ARRAY_BUFFER, 0, shapeVertexCount_ * sizeof(ShapeVertex),
                  shapeVertexCache_.data());

  glBindVertexArray(shapeVao_);
  glDrawArrays(currentShapeMode_, 0, static_cast<GLsizei>(shapeVertexCount_));

  stats_.drawCalls++;
  stats_.triangleCount += static_cast<uint32_t>(shapeVertexCount_ / 3);

  shapeVertexCount_ = 0;
}

void GLRenderer::flushLineBatch() {
  if (lineVertexCount_ == 0)
    return;

  // 先刷新形状批次
  flushShapeBatch();

  glLineWidth(currentLineWidth_);
  shapeShader_.bind();
  shapeShader_.setMat4("uViewProjection", viewProjection_);

  glBindBuffer(GL_ARRAY_BUFFER, lineVbo_);
  glBufferSubData(GL_ARRAY_BUFFER, 0, lineVertexCount_ * sizeof(ShapeVertex),
                  lineVertexCache_.data());

  glBindVertexArray(lineVao_);
  glDrawArrays(GL_LINES, 0, static_cast<GLsizei>(lineVertexCount_));

  stats_.drawCalls++;

  lineVertexCount_ = 0;
}

} // namespace extra2d
