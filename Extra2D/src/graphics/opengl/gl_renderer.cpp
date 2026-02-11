#include <SDL.h>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <extra2d/core/string.h>
#include <extra2d/graphics/opengl/gl_font_atlas.h>
#include <extra2d/graphics/opengl/gl_renderer.h>
#include <extra2d/graphics/opengl/gl_texture.h>
#include <extra2d/graphics/vram_manager.h>
#include <extra2d/platform/window.h>
#include <extra2d/utils/logger.h>
#include <vector>

namespace extra2d {

// 形状渲染着色器 (GLES 3.2)
static const char *SHAPE_VERTEX_SHADER = R"(
#version 300 es
precision highp float;
layout(location = 0) in vec2 aPosition;
uniform mat4 uViewProjection;
void main() {
    gl_Position = uViewProjection * vec4(aPosition, 0.0, 1.0);
}
)";

static const char *SHAPE_FRAGMENT_SHADER = R"(
#version 300 es
precision highp float;
uniform vec4 uColor;
out vec4 fragColor;
void main() {
    fragColor = uColor;
}
)";

// VBO 初始大小（用于 VRAM 跟踪）
static constexpr size_t SHAPE_VBO_SIZE = 1024 * sizeof(float);

GLRenderer::GLRenderer()
    : window_(nullptr), shapeVao_(0), shapeVbo_(0), vsync_(true) {
  resetStats();
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

  E2D_LOG_INFO("OpenGL Renderer initialized");
  E2D_LOG_INFO("OpenGL Version: {}",
               reinterpret_cast<const char *>(glGetString(GL_VERSION)));

  return true;
}

void GLRenderer::shutdown() {
  spriteBatch_.shutdown();

  if (shapeVbo_ != 0) {
    glDeleteBuffers(1, &shapeVbo_);
    VRAMManager::getInstance().freeBuffer(SHAPE_VBO_SIZE);
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
  // 交换缓冲区在 Window 类中处理
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
  switch (mode) {
  case BlendMode::None:
    glDisable(GL_BLEND);
    break;
  case BlendMode::Alpha:
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    break;
  case BlendMode::Additive:
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    break;
  case BlendMode::Multiply:
    glEnable(GL_BLEND);
    glBlendFunc(GL_DST_COLOR, GL_ONE_MINUS_SRC_ALPHA);
    break;
  }
}

void GLRenderer::setViewProjection(const glm::mat4 &matrix) {
  viewProjection_ = matrix;
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
  float v1 = 1.0f - (srcRect.origin.y / texH);
  float v2 = 1.0f - ((srcRect.origin.y + srcRect.size.height) / texH);

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
  glLineWidth(width);

  shapeShader_.bind();
  shapeShader_.setMat4("uViewProjection", viewProjection_);
  shapeShader_.setVec4("uColor", glm::vec4(color.r, color.g, color.b, color.a));

  float vertices[] = {start.x, start.y, end.x, end.y};

  glBindBuffer(GL_ARRAY_BUFFER, shapeVbo_);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

  glBindVertexArray(shapeVao_);
  glDrawArrays(GL_LINES, 0, 2);

  stats_.drawCalls++;
  stats_.triangleCount += 1;
}

void GLRenderer::drawRect(const Rect &rect, const Color &color, float width) {
  float x1 = rect.origin.x;
  float y1 = rect.origin.y;
  float x2 = rect.origin.x + rect.size.width;
  float y2 = rect.origin.y + rect.size.height;

  drawLine(Vec2(x1, y1), Vec2(x2, y1), color, width);
  drawLine(Vec2(x2, y1), Vec2(x2, y2), color, width);
  drawLine(Vec2(x2, y2), Vec2(x1, y2), color, width);
  drawLine(Vec2(x1, y2), Vec2(x1, y1), color, width);
}

void GLRenderer::fillRect(const Rect &rect, const Color &color) {
  shapeShader_.bind();
  shapeShader_.setMat4("uViewProjection", viewProjection_);
  shapeShader_.setVec4("uColor", glm::vec4(color.r, color.g, color.b, color.a));

  float vertices[] = {rect.origin.x,
                      rect.origin.y,
                      rect.origin.x + rect.size.width,
                      rect.origin.y,
                      rect.origin.x + rect.size.width,
                      rect.origin.y + rect.size.height,
                      rect.origin.x,
                      rect.origin.y + rect.size.height};

  glBindBuffer(GL_ARRAY_BUFFER, shapeVbo_);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

  glBindVertexArray(shapeVao_);
  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

  stats_.drawCalls++;
  stats_.triangleCount += 2;
}

void GLRenderer::drawCircle(const Vec2 &center, float radius,
                            const Color &color, int segments, float width) {
  std::vector<float> vertices;
  vertices.reserve((segments + 1) * 2);

  for (int i = 0; i <= segments; ++i) {
    float angle = 2.0f * 3.14159f * i / segments;
    vertices.push_back(center.x + radius * cosf(angle));
    vertices.push_back(center.y + radius * sinf(angle));
  }

  glLineWidth(width);

  shapeShader_.bind();
  shapeShader_.setMat4("uViewProjection", viewProjection_);
  shapeShader_.setVec4("uColor", glm::vec4(color.r, color.g, color.b, color.a));

  glBindBuffer(GL_ARRAY_BUFFER, shapeVbo_);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
               vertices.data(), GL_DYNAMIC_DRAW);

  glBindVertexArray(shapeVao_);
  glDrawArrays(GL_LINE_STRIP, 0, segments + 1);

  stats_.drawCalls++;
}

void GLRenderer::fillCircle(const Vec2 &center, float radius,
                            const Color &color, int segments) {
  std::vector<float> vertices;
  vertices.reserve((segments + 2) * 2);

  vertices.push_back(center.x);
  vertices.push_back(center.y);

  for (int i = 0; i <= segments; ++i) {
    float angle = 2.0f * 3.14159f * i / segments;
    vertices.push_back(center.x + radius * cosf(angle));
    vertices.push_back(center.y + radius * sinf(angle));
  }

  shapeShader_.bind();
  shapeShader_.setMat4("uViewProjection", viewProjection_);
  shapeShader_.setVec4("uColor", glm::vec4(color.r, color.g, color.b, color.a));

  glBindBuffer(GL_ARRAY_BUFFER, shapeVbo_);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
               vertices.data(), GL_DYNAMIC_DRAW);

  glBindVertexArray(shapeVao_);
  glDrawArrays(GL_TRIANGLE_FAN, 0, segments + 2);

  stats_.drawCalls++;
  stats_.triangleCount += segments;
}

void GLRenderer::drawTriangle(const Vec2 &p1, const Vec2 &p2, const Vec2 &p3,
                              const Color &color, float width) {
  drawLine(p1, p2, color, width);
  drawLine(p2, p3, color, width);
  drawLine(p3, p1, color, width);
}

void GLRenderer::fillTriangle(const Vec2 &p1, const Vec2 &p2, const Vec2 &p3,
                              const Color &color) {
  shapeShader_.bind();
  shapeShader_.setMat4("uViewProjection", viewProjection_);
  shapeShader_.setVec4("uColor", glm::vec4(color.r, color.g, color.b, color.a));

  float vertices[] = {p1.x, p1.y, p2.x, p2.y, p3.x, p3.y};

  glBindBuffer(GL_ARRAY_BUFFER, shapeVbo_);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

  glBindVertexArray(shapeVao_);
  glDrawArrays(GL_TRIANGLES, 0, 3);

  stats_.drawCalls++;
  stats_.triangleCount += 1;
}

void GLRenderer::drawPolygon(const std::vector<Vec2> &points,
                             const Color &color, float width) {
  for (size_t i = 0; i < points.size(); ++i) {
    drawLine(points[i], points[(i + 1) % points.size()], color, width);
  }
}

void GLRenderer::fillPolygon(const std::vector<Vec2> &points,
                             const Color &color) {
  // 简化的三角形扇形填充
  if (points.size() < 3)
    return;

  shapeShader_.bind();
  shapeShader_.setMat4("uViewProjection", viewProjection_);
  shapeShader_.setVec4("uColor", glm::vec4(color.r, color.g, color.b, color.a));

  std::vector<float> vertices;
  for (const auto &p : points) {
    vertices.push_back(p.x);
    vertices.push_back(p.y);
  }

  glBindBuffer(GL_ARRAY_BUFFER, shapeVbo_);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float),
               vertices.data(), GL_DYNAMIC_DRAW);

  glBindVertexArray(shapeVao_);
  glDrawArrays(GL_TRIANGLE_FAN, 0, static_cast<GLsizei>(points.size()));

  stats_.drawCalls++;
  stats_.triangleCount += static_cast<uint32_t>(points.size() - 2);
}

Ptr<FontAtlas> GLRenderer::createFontAtlas(const std::string &filepath,
                                           int fontSize, bool useSDF) {
  return makePtr<GLFontAtlas>(filepath, fontSize, useSDF);
}

void GLRenderer::drawText(const FontAtlas &font, const std::string &text,
                          const Vec2 &position, const Color &color) {
  drawText(font, text, position.x, position.y, color);
}

void GLRenderer::drawText(const FontAtlas &font, const std::string &text, float x,
                          float y, const Color &color) {
  float cursorX = x;
  float cursorY = y;
  float baselineY = cursorY + font.getAscent();

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

      Rect destRect(xPos, yPos, glyph->width, glyph->height);

      GLSpriteBatch::SpriteData data;
      data.position = glm::vec2(destRect.origin.x, destRect.origin.y);
      data.size = glm::vec2(destRect.size.width, destRect.size.height);
      data.texCoordMin = glm::vec2(glyph->u0, glyph->v0);
      data.texCoordMax = glm::vec2(glyph->u1, glyph->v1);
      data.color = glm::vec4(color.r, color.g, color.b, color.a);
      data.rotation = 0.0f;
      data.anchor = glm::vec2(0.0f, 0.0f);
      data.isSDF = font.isSDF();
      spriteBatch_.draw(*font.getTexture(), data);
    }
  }
}

void GLRenderer::resetStats() { stats_ = Stats{}; }

void GLRenderer::initShapeRendering() {
  // 编译形状着色器
  shapeShader_.compileFromSource(SHAPE_VERTEX_SHADER, SHAPE_FRAGMENT_SHADER);

  // 创建 VAO 和 VBO
  glGenVertexArrays(1, &shapeVao_);
  glGenBuffers(1, &shapeVbo_);

  glBindVertexArray(shapeVao_);
  glBindBuffer(GL_ARRAY_BUFFER, shapeVbo_);
  glBufferData(GL_ARRAY_BUFFER, SHAPE_VBO_SIZE, nullptr, GL_DYNAMIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);

  glBindVertexArray(0);

  // VRAM 跟踪
  VRAMManager::getInstance().allocBuffer(SHAPE_VBO_SIZE);
}

} // namespace extra2d
