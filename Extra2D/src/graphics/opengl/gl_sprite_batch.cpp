#include <cstring>
#include <extra2d/graphics/opengl/gl_sprite_batch.h>
#include <extra2d/utils/logger.h>
#include <glm/gtc/matrix_transform.hpp>

namespace extra2d {

// ============================================================================
// 三角函数查表 - 避免每帧重复计算 sin/cos
// ============================================================================
class TrigLookup {
public:
  static constexpr size_t TABLE_SIZE = 360 * 4;  // 0.25度精度
  static constexpr float INDEX_SCALE = 4.0f;      // 每度4个采样点
  static constexpr float RAD_TO_INDEX = INDEX_SCALE * 180.0f / 3.14159265359f;
  
  static float sinRad(float radians) {
    return table_.sinTable[normalizeIndexRad(radians)];
  }
  
  static float cosRad(float radians) {
    return table_.cosTable[normalizeIndexRad(radians)];
  }
  
private:
  struct Tables {
    std::array<float, TABLE_SIZE> sinTable;
    std::array<float, TABLE_SIZE> cosTable;
    
    Tables() {
      for (size_t i = 0; i < TABLE_SIZE; ++i) {
        float angle = static_cast<float>(i) / INDEX_SCALE * 3.14159265359f / 180.0f;
        sinTable[i] = std::sin(angle);
        cosTable[i] = std::cos(angle);
      }
    }
  };
  
  static size_t normalizeIndexRad(float radians) {
    int idx = static_cast<int>(radians * RAD_TO_INDEX) % static_cast<int>(TABLE_SIZE);
    if (idx < 0) {
      idx += static_cast<int>(TABLE_SIZE);
    }
    return static_cast<size_t>(idx);
  }
  
  static const Tables table_;
};

const TrigLookup::Tables TrigLookup::table_;

// 静态索引生成函数
static const std::array<GLuint, GLSpriteBatch::MAX_INDICES>& getIndices() {
  static std::array<GLuint, GLSpriteBatch::MAX_INDICES> indices = []() {
    std::array<GLuint, GLSpriteBatch::MAX_INDICES> arr{};
    for (size_t i = 0; i < GLSpriteBatch::MAX_SPRITES; ++i) {
      GLuint base = static_cast<GLuint>(i * GLSpriteBatch::VERTICES_PER_SPRITE);
      arr[i * GLSpriteBatch::INDICES_PER_SPRITE + 0] = base + 0;
      arr[i * GLSpriteBatch::INDICES_PER_SPRITE + 1] = base + 1;
      arr[i * GLSpriteBatch::INDICES_PER_SPRITE + 2] = base + 2;
      arr[i * GLSpriteBatch::INDICES_PER_SPRITE + 3] = base + 0;
      arr[i * GLSpriteBatch::INDICES_PER_SPRITE + 4] = base + 2;
      arr[i * GLSpriteBatch::INDICES_PER_SPRITE + 5] = base + 3;
    }
    return arr;
  }();
  return indices;
}

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
// SDF 常量硬编码：ONEDGE_VALUE=128/255=0.502, PIXEL_DIST_SCALE=255/64=3.98
// SDF 值存储在 Alpha 通道
static const char *SPRITE_FRAGMENT_SHADER = R"(
#version 300 es
precision highp float;
in vec2 vTexCoord;
in vec4 vColor;

uniform sampler2D uTexture;
uniform int uUseSDF;

out vec4 fragColor;

void main() {
    if (uUseSDF == 1) {
        float dist = texture(uTexture, vTexCoord).a;
        float sd = (dist - 0.502) * 3.98;
        float w = fwidth(sd);
        float alpha = smoothstep(-w, w, sd);
        fragColor = vec4(vColor.rgb, vColor.a * alpha);
    } else {
        fragColor = texture(uTexture, vTexCoord) * vColor;
    }
}
)";

GLSpriteBatch::GLSpriteBatch()
    : vao_(0), vbo_(0), ibo_(0), vertexCount_(0), currentTexture_(nullptr),
      currentIsSDF_(false), drawCallCount_(0), spriteCount_(0), batchCount_(0) {
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

  // 使用编译期生成的静态索引缓冲区
  const auto& indices = getIndices();
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint),
               indices.data(), GL_STATIC_DRAW);

  glBindVertexArray(0);

  E2D_LOG_INFO("GLSpriteBatch initialized with capacity for {} sprites",
               MAX_SPRITES);
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

bool GLSpriteBatch::needsFlush(const Texture &texture, bool isSDF) const {
  if (currentTexture_ == nullptr) {
    return false;
  }

  // 检查是否需要刷新：纹理改变、SDF 状态改变或缓冲区已满
  return (currentTexture_ != &texture) || (currentIsSDF_ != isSDF) ||
         (vertexCount_ + VERTICES_PER_SPRITE > MAX_VERTICES);
}

void GLSpriteBatch::addVertices(const SpriteData &data) {
  // 计算锚点偏移
  float anchorOffsetX = data.size.x * data.anchor.x;
  float anchorOffsetY = data.size.y * data.anchor.y;

  // 使用三角函数查表替代 cosf/sinf
  float cosR = TrigLookup::cosRad(data.rotation);
  float sinR = TrigLookup::sinRad(data.rotation);

  glm::vec4 color(data.color.r, data.color.g, data.color.b, data.color.a);

  // 直接计算变换后的位置
  float rx0 = -anchorOffsetX;
  float ry0 = -anchorOffsetY;
  float rx1 = data.size.x - anchorOffsetX;
  float ry1 = data.size.y - anchorOffsetY;
  
  // 预计算旋转后的偏移
  float cosRx0 = rx0 * cosR, sinRx0 = rx0 * sinR;
  float cosRx1 = rx1 * cosR, sinRx1 = rx1 * sinR;
  float cosRy0 = ry0 * cosR, sinRy0 = ry0 * sinR;
  float cosRy1 = ry1 * cosR, sinRy1 = ry1 * sinR;
  
  // v0: (0, 0) -> (rx0, ry0)
  vertexBuffer_[vertexCount_++] = {
    glm::vec2(data.position.x + cosRx0 - sinRy0, data.position.y + sinRx0 + cosRy0),
    glm::vec2(data.texCoordMin.x, data.texCoordMin.y),
    color
  };
  
  // v1: (size.x, 0) -> (rx1, ry0)
  vertexBuffer_[vertexCount_++] = {
    glm::vec2(data.position.x + cosRx1 - sinRy0, data.position.y + sinRx1 + cosRy0),
    glm::vec2(data.texCoordMax.x, data.texCoordMin.y),
    color
  };
  
  // v2: (size.x, size.y) -> (rx1, ry1)
  vertexBuffer_[vertexCount_++] = {
    glm::vec2(data.position.x + cosRx1 - sinRy1, data.position.y + sinRx1 + cosRy1),
    glm::vec2(data.texCoordMax.x, data.texCoordMax.y),
    color
  };
  
  // v3: (0, size.y) -> (rx0, ry1)
  vertexBuffer_[vertexCount_++] = {
    glm::vec2(data.position.x + cosRx0 - sinRy1, data.position.y + sinRx0 + cosRy1),
    glm::vec2(data.texCoordMin.x, data.texCoordMax.y),
    color
  };
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

void GLSpriteBatch::drawBatch(const Texture &texture,
                              const std::vector<SpriteData> &sprites) {
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

void GLSpriteBatch::drawImmediate(const Texture &texture,
                                  const SpriteData &data) {
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
  // SDF 常量已硬编码到着色器中

  // 更新 VBO 数据 - 只更新实际使用的部分
  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  glBufferSubData(GL_ARRAY_BUFFER, 0, vertexCount_ * sizeof(Vertex),
                  vertexBuffer_.data());

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
