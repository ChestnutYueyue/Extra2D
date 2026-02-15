#pragma once

#include <array>
#include <extra2d/core/color.h>
#include <extra2d/core/math_types.h>
#include <extra2d/core/types.h>
#include <extra2d/graphics/opengl/gl_shader.h>
#include <extra2d/graphics/texture.h>
#include <glm/mat4x4.hpp>
#include <vector>

#include <glad/glad.h>

namespace extra2d {

// ============================================================================
// OpenGL 精灵批渲染器 - 优化版本
// ============================================================================
class GLSpriteBatch {
public:
  static constexpr size_t MAX_SPRITES = 10000;
  static constexpr size_t VERTICES_PER_SPRITE = 4;
  static constexpr size_t INDICES_PER_SPRITE = 6;
  static constexpr size_t MAX_VERTICES = MAX_SPRITES * VERTICES_PER_SPRITE;
  static constexpr size_t MAX_INDICES = MAX_SPRITES * INDICES_PER_SPRITE;

  struct Vertex {
    glm::vec2 position;
    glm::vec2 texCoord;
    glm::vec4 color;
  };

  struct SpriteData {
    glm::vec2 position;
    glm::vec2 size;
    glm::vec2 texCoordMin;
    glm::vec2 texCoordMax;
    glm::vec4 color;
    float rotation;
    glm::vec2 anchor;
    bool isSDF = false;
  };

  GLSpriteBatch();
  ~GLSpriteBatch();

  bool init();
  void shutdown();

  void begin(const glm::mat4 &viewProjection);
  void draw(const Texture &texture, const SpriteData &data);
  void end();

  // 批量绘制接口 - 用于自动批处理
  void drawBatch(const Texture &texture,
                 const std::vector<SpriteData> &sprites);

  // 立即绘制（不缓存）
  void drawImmediate(const Texture &texture, const SpriteData &data);

  // 统计
  uint32_t getDrawCallCount() const { return drawCallCount_; }
  uint32_t getSpriteCount() const { return spriteCount_; }
  uint32_t getBatchCount() const { return batchCount_; }

  // 检查是否需要刷新
  bool needsFlush(const Texture &texture, bool isSDF) const;

private:
  GLuint vao_;
  GLuint vbo_;
  GLuint ibo_;
  GLShader shader_;

  // 使用固定大小数组减少内存分配
  std::array<Vertex, MAX_VERTICES> vertexBuffer_;
  size_t vertexCount_;

  const Texture *currentTexture_;
  bool currentIsSDF_;
  glm::mat4 viewProjection_;

  // 缓存上一帧的 viewProjection，避免重复设置
  glm::mat4 cachedViewProjection_;
  bool viewProjectionDirty_ = true;

  uint32_t drawCallCount_;
  uint32_t spriteCount_;
  uint32_t batchCount_;

  void flush();
  void setupShader();

  // 添加顶点到缓冲区
  void addVertices(const SpriteData &data);
};

} // namespace extra2d
