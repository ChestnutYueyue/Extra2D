#include <cstring>
#include <extra2d/graphics/opengl/gl_sprite_batch.h>
#include <extra2d/utils/logger.h>
#include <glm/gtc/matrix_transform.hpp>

namespace extra2d {

// 顶点着色器 (GLES 3.2)
static const char *SPRITE_VERTEX_SHADER = R"(
#version 300 es
precision highp float;
layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in vec4 aColor;

uniform mat4 uViewProjection;

out vec2 vTexCoord;
out vec4 vColor;

void main() {
    gl_Position = uViewProjection * vec4(aPosition, 0.0, 1.0);
    vTexCoord = aTexCoord;
    vColor = aColor;
}
)";

// 片段着色器 (GLES 3.2)
static const char *SPRITE_FRAGMENT_SHADER = R"(
#version 300 es
precision highp float;
in vec2 vTexCoord;
in vec4 vColor;

uniform sampler2D uTexture;
uniform int uUseSDF;
uniform float uSdfOnEdge;
uniform float uSdfScale;

out vec4 fragColor;

void main() {
    if (uUseSDF == 1) {
        float dist = texture(uTexture, vTexCoord).r;
        float sd = (dist - uSdfOnEdge) * uSdfScale;
        float w = fwidth(sd);
        float alpha = smoothstep(-w, w, sd);
        fragColor = vec4(vColor.rgb, vColor.a * alpha);
    } else {
        fragColor = texture(uTexture, vTexCoord) * vColor;
    }
}
)";

GLSpriteBatch::GLSpriteBatch()
    : vao_(0), vbo_(0), ibo_(0), currentTexture_(nullptr), currentIsSDF_(false),
      vertexCount_(0), drawCallCount_(0), spriteCount_(0), batchCount_(0) {
}

GLSpriteBatch::~GLSpriteBatch() { shutdown(); }

bool GLSpriteBatch::init() {
  // 创建并编译着色器
  if (!shader_.compileFromSource(SPRITE_VERTEX_SHADER,
                                 SPRITE_FRAGMENT_SHADER)) {
    E2D_LOG_ERROR("Failed to compile sprite batch shader");
    return false;
  }

  // 生成 VAO、VBO、IBO
  glGenVertexArrays(1, &vao_);
  glGenBuffers(1, &vbo_);
  glGenBuffers(1, &ibo_);

  glBindVertexArray(vao_);

  // 设置 VBO - 使用动态绘制模式
  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  glBufferData(GL_ARRAY_BUFFER, MAX_VERTICES * sizeof(Vertex), nullptr,
               GL_DYNAMIC_DRAW);

  // 设置顶点属性
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *)offsetof(Vertex, position));

  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *)offsetof(Vertex, texCoord));

  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *)offsetof(Vertex, color));

  // 生成索引缓冲区 - 静态，只需创建一次
  std::vector<GLuint> indices;
  indices.reserve(MAX_INDICES);
  for (size_t i = 0; i < MAX_SPRITES; ++i) {
    GLuint base = static_cast<GLuint>(i * VERTICES_PER_SPRITE);
    indices.push_back(base + 0);
    indices.push_back(base + 1);
    indices.push_back(base + 2);
    indices.push_back(base + 0);
    indices.push_back(base + 2);
    indices.push_back(base + 3);
  }

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint),
               indices.data(), GL_STATIC_DRAW);

  glBindVertexArray(0);

  E2D_LOG_INFO("GLSpriteBatch initialized with capacity for {} sprites", MAX_SPRITES);
  return true;
}

void GLSpriteBatch::shutdown() {
  if (vao_ != 0) {
    glDeleteVertexArrays(1, &vao_);
    vao_ = 0;
  }
  if (vbo_ != 0) {
    glDeleteBuffers(1, &vbo_);
    vbo_ = 0;
  }
  if (ibo_ != 0) {
    glDeleteBuffers(1, &ibo_);
    ibo_ = 0;
  }
}

void GLSpriteBatch::begin(const glm::mat4 &viewProjection) {
  viewProjection_ = viewProjection;
  vertexCount_ = 0;
  currentTexture_ = nullptr;
  currentIsSDF_ = false;
  drawCallCount_ = 0;
  spriteCount_ = 0;
  batchCount_ = 0;
}

bool GLSpriteBatch::needsFlush(const Texture& texture, bool isSDF) const {
  if (currentTexture_ == nullptr) {
    return false;
  }
  
  // 检查是否需要刷新：纹理改变、SDF 状态改变或缓冲区已满
  return (currentTexture_ != &texture) || 
         (currentIsSDF_ != isSDF) || 
         (vertexCount_ + VERTICES_PER_SPRITE > MAX_VERTICES);
}

void GLSpriteBatch::addVertices(const SpriteData& data) {
  // 计算变换后的顶点位置
  glm::vec2 anchorOffset(data.size.x * data.anchor.x,
                         data.size.y * data.anchor.y);

  float cosR = cosf(data.rotation);
  float sinR = sinf(data.rotation);

  auto transform = [&](float x, float y) -> glm::vec2 {
    float rx = x - anchorOffset.x;
    float ry = y - anchorOffset.y;
    return glm::vec2(data.position.x + rx * cosR - ry * sinR,
                     data.position.y + rx * sinR + ry * cosR);
  };

  glm::vec4 color(data.color.r, data.color.g, data.color.b, data.color.a);

  // 添加四个顶点（图片已在加载时翻转，纹理坐标直接使用）
  // v0(左上) -- v1(右上)
  //   |           |
  // v3(左下) -- v2(右下)
  Vertex v0{transform(0, 0), glm::vec2(data.texCoordMin.x, data.texCoordMin.y), color};
  Vertex v1{transform(data.size.x, 0), glm::vec2(data.texCoordMax.x, data.texCoordMin.y), color};
  Vertex v2{transform(data.size.x, data.size.y), glm::vec2(data.texCoordMax.x, data.texCoordMax.y), color};
  Vertex v3{transform(0, data.size.y), glm::vec2(data.texCoordMin.x, data.texCoordMax.y), color};

  vertexBuffer_[vertexCount_++] = v0;
  vertexBuffer_[vertexCount_++] = v1;
  vertexBuffer_[vertexCount_++] = v2;
  vertexBuffer_[vertexCount_++] = v3;
}

void GLSpriteBatch::draw(const Texture &texture, const SpriteData &data) {
  // 如果需要刷新，先提交当前批次
  if (needsFlush(texture, data.isSDF)) {
    flush();
  }

  currentTexture_ = &texture;
  currentIsSDF_ = data.isSDF;

  addVertices(data);
  spriteCount_++;
}

void GLSpriteBatch::drawBatch(const Texture& texture, const std::vector<SpriteData>& sprites) {
  if (sprites.empty()) {
    return;
  }
  
  // 如果当前有未提交的批次且纹理不同，先刷新
  if (currentTexture_ != nullptr && currentTexture_ != &texture) {
    flush();
  }
  
  currentTexture_ = &texture;
  currentIsSDF_ = sprites[0].isSDF; // 假设批量中的精灵 SDF 状态一致
  
  // 分批处理，避免超过缓冲区大小
  size_t index = 0;
  while (index < sprites.size()) {
    size_t remainingSpace = (MAX_VERTICES - vertexCount_) / VERTICES_PER_SPRITE;
    size_t batchSize = std::min(sprites.size() - index, remainingSpace);
    
    for (size_t i = 0; i < batchSize; ++i) {
      addVertices(sprites[index + i]);
      spriteCount_++;
    }
    
    index += batchSize;
    
    // 如果还有更多精灵，刷新当前批次
    if (index < sprites.size()) {
      flush();
    }
  }
  
  batchCount_++;
}

void GLSpriteBatch::drawImmediate(const Texture& texture, const SpriteData& data) {
  // 立即绘制，不缓存 - 用于需要立即显示的情况
  flush(); // 先提交当前批次
  
  currentTexture_ = &texture;
  currentIsSDF_ = data.isSDF;
  addVertices(data);
  spriteCount_++;
  
  flush(); // 立即提交
}

void GLSpriteBatch::end() {
  if (vertexCount_ > 0) {
    flush();
  }
}

void GLSpriteBatch::flush() {
  if (vertexCount_ == 0 || currentTexture_ == nullptr) {
    return;
  }

  // 绑定纹理
  GLuint texID = static_cast<GLuint>(
      reinterpret_cast<uintptr_t>(currentTexture_->getNativeHandle()));
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texID);

  // 使用着色器
  shader_.bind();
  shader_.setMat4("uViewProjection", viewProjection_);
  shader_.setInt("uTexture", 0);
  shader_.setInt("uUseSDF", currentIsSDF_ ? 1 : 0);
  shader_.setFloat("uSdfOnEdge", 128.0f / 255.0f);
  shader_.setFloat("uSdfScale", 255.0f / 64.0f);

  // 更新 VBO 数据 - 只更新实际使用的部分
  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  glBufferSubData(GL_ARRAY_BUFFER, 0, vertexCount_ * sizeof(Vertex), vertexBuffer_.data());

  // 绘制
  glBindVertexArray(vao_);
  GLsizei indexCount = static_cast<GLsizei>(
      (vertexCount_ / VERTICES_PER_SPRITE) * INDICES_PER_SPRITE);
  glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);

  drawCallCount_++;
  batchCount_++;
  
  // 重置状态
  vertexCount_ = 0;
  currentTexture_ = nullptr;
  currentIsSDF_ = false;
}

} // namespace extra2d
