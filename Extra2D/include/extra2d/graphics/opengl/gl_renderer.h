#pragma once

#include <extra2d/graphics/opengl/gl_shader.h>
#include <extra2d/graphics/opengl/gl_sprite_batch.h>
#include <extra2d/graphics/render_backend.h>

#include <array>
#include <glad/glad.h>
#include <vector>

namespace extra2d {

class Window;

// ============================================================================
// OpenGL 渲染器实现
// ============================================================================
class GLRenderer : public RenderBackend {
public:
  GLRenderer();
  ~GLRenderer() override;

  // RenderBackend 接口实现
  bool init(Window *window) override;
  void shutdown() override;

  void beginFrame(const Color &clearColor) override;
  void endFrame() override;
  void setViewport(int x, int y, int width, int height) override;
  void setVSync(bool enabled) override;

  void setBlendMode(BlendMode mode) override;
  void setViewProjection(const glm::mat4 &matrix) override;

  // 变换矩阵栈
  void pushTransform(const glm::mat4 &transform) override;
  void popTransform() override;
  glm::mat4 getCurrentTransform() const override;

  Ptr<Texture> createTexture(int width, int height, const uint8_t *pixels,
                             int channels) override;
  Ptr<Texture> loadTexture(const std::string &filepath) override;

  void beginSpriteBatch() override;
  void drawSprite(const Texture &texture, const Rect &destRect,
                  const Rect &srcRect, const Color &tint, float rotation,
                  const Vec2 &anchor) override;
  void drawSprite(const Texture &texture, const Vec2 &position,
                  const Color &tint) override;
  void endSpriteBatch() override;

  void drawLine(const Vec2 &start, const Vec2 &end, const Color &color,
                float width) override;
  void drawRect(const Rect &rect, const Color &color, float width) override;
  void fillRect(const Rect &rect, const Color &color) override;
  void drawCircle(const Vec2 &center, float radius, const Color &color,
                  int segments, float width) override;
  void fillCircle(const Vec2 &center, float radius, const Color &color,
                  int segments) override;
  void drawTriangle(const Vec2 &p1, const Vec2 &p2, const Vec2 &p3,
                    const Color &color, float width) override;
  void fillTriangle(const Vec2 &p1, const Vec2 &p2, const Vec2 &p3,
                    const Color &color) override;
  void drawPolygon(const std::vector<Vec2> &points, const Color &color,
                   float width) override;
  void fillPolygon(const std::vector<Vec2> &points,
                   const Color &color) override;

  Ptr<FontAtlas> createFontAtlas(const std::string &filepath, int fontSize,
                                 bool useSDF = false) override;
  void drawText(const FontAtlas &font, const std::string &text,
                const Vec2 &position, const Color &color) override;
  void drawText(const FontAtlas &font, const std::string &text, float x,
                float y, const Color &color) override;

  Stats getStats() const override { return stats_; }
  void resetStats() override;

private:
  // 形状批处理常量
  static constexpr size_t MAX_CIRCLE_SEGMENTS = 128;
  static constexpr size_t MAX_SHAPE_VERTICES = 8192; // 最大形状顶点数
  static constexpr size_t MAX_LINE_VERTICES = 16384; // 最大线条顶点数

  // 形状顶点结构（包含颜色）
  struct ShapeVertex {
    float x, y;
    float r, g, b, a;
  };

  Window *window_;
  GLSpriteBatch spriteBatch_;
  GLShader shapeShader_;

  GLuint shapeVao_;
  GLuint shapeVbo_;
  GLuint lineVao_; // 线条专用 VAO
  GLuint lineVbo_; // 线条专用 VBO

  glm::mat4 viewProjection_;
  std::vector<glm::mat4> transformStack_;
  Stats stats_;
  bool vsync_;

  // 形状批处理缓冲区（预分配，避免每帧内存分配）
  std::array<ShapeVertex, MAX_SHAPE_VERTICES> shapeVertexCache_;
  size_t shapeVertexCount_ = 0;
  GLenum currentShapeMode_ = GL_TRIANGLES;

  // 线条批处理缓冲区
  std::array<ShapeVertex, MAX_LINE_VERTICES> lineVertexCache_;
  size_t lineVertexCount_ = 0;
  float currentLineWidth_ = 1.0f;

  // OpenGL 状态缓存
  BlendMode cachedBlendMode_ = BlendMode::None;
  bool blendEnabled_ = false;
  int cachedViewportX_ = 0;
  int cachedViewportY_ = 0;
  int cachedViewportWidth_ = 0;
  int cachedViewportHeight_ = 0;

  void initShapeRendering();
  void flushShapeBatch();
  void flushLineBatch();
  void addShapeVertex(float x, float y, const Color &color);
  void addLineVertex(float x, float y, const Color &color);
  void submitShapeBatch(GLenum mode);
};

} // namespace extra2d
