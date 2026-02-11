#pragma once

#include <extra2d/core/color.h>
#include <extra2d/core/math_types.h>
#include <extra2d/core/types.h>
#include <extra2d/graphics/texture.h>
#include <glm/mat4x4.hpp>
#include <cstdint>
#include <variant>

namespace extra2d {

// 前向声明
class Texture;
class FontAtlas;

/**
 * @brief 渲染命令类型枚举
 */
enum class RenderCommandType : uint8_t {
  None = 0,
  Sprite,       // 精灵绘制
  Line,         // 线条绘制
  Rect,         // 矩形绘制
  FilledRect,   // 填充矩形
  Circle,       // 圆形绘制
  FilledCircle, // 填充圆形
  Triangle,     // 三角形绘制
  FilledTriangle, // 填充三角形
  Polygon,      // 多边形绘制
  FilledPolygon, // 填充多边形
  Text,         // 文本绘制
  Custom        // 自定义绘制
};

/**
 * @brief 精灵渲染命令数据
 */
struct SpriteCommandData {
  const Texture* texture;
  Rect destRect;
  Rect srcRect;
  Color tint;
  float rotation;
  Vec2 anchor;
  uint32_t sortKey;  // 用于自动排序的键值
  
  SpriteCommandData() 
    : texture(nullptr), destRect(), srcRect(), tint(Colors::White), 
      rotation(0.0f), anchor(0.0f, 0.0f), sortKey(0) {}
  SpriteCommandData(const Texture* tex, const Rect& dest, const Rect& src, 
                    const Color& t, float rot, const Vec2& anc, uint32_t key)
    : texture(tex), destRect(dest), srcRect(src), tint(t), 
      rotation(rot), anchor(anc), sortKey(key) {}
};

/**
 * @brief 线条渲染命令数据
 */
struct LineCommandData {
  Vec2 start;
  Vec2 end;
  Color color;
  float width;
  
  LineCommandData() : start(), end(), color(Colors::White), width(1.0f) {}
  LineCommandData(const Vec2& s, const Vec2& e, const Color& c, float w)
    : start(s), end(e), color(c), width(w) {}
};

/**
 * @brief 矩形渲染命令数据
 */
struct RectCommandData {
  Rect rect;
  Color color;
  float width;
  bool filled;
  
  RectCommandData() : rect(), color(Colors::White), width(1.0f), filled(false) {}
  RectCommandData(const Rect& r, const Color& c, float w, bool f)
    : rect(r), color(c), width(w), filled(f) {}
};

/**
 * @brief 圆形渲染命令数据
 */
struct CircleCommandData {
  Vec2 center;
  float radius;
  Color color;
  int segments;
  float width;
  bool filled;
  
  CircleCommandData() : center(), radius(0.0f), color(Colors::White), 
                        segments(32), width(1.0f), filled(false) {}
  CircleCommandData(const Vec2& c, float r, const Color& col, int seg, float w, bool f)
    : center(c), radius(r), color(col), segments(seg), width(w), filled(f) {}
};

/**
 * @brief 三角形渲染命令数据
 */
struct TriangleCommandData {
  Vec2 p1, p2, p3;
  Color color;
  float width;
  bool filled;
  
  TriangleCommandData() : p1(), p2(), p3(), color(Colors::White), 
                          width(1.0f), filled(false) {}
  TriangleCommandData(const Vec2& a, const Vec2& b, const Vec2& c, const Color& col, float w, bool f)
    : p1(a), p2(b), p3(c), color(col), width(w), filled(f) {}
};

/**
 * @brief 多边形渲染命令数据
 */
struct PolygonCommandData {
  std::vector<Vec2> points;
  Color color;
  float width;
  bool filled;
  
  PolygonCommandData() : color(Colors::White), width(1.0f), filled(false) {}
  PolygonCommandData(std::vector<Vec2> pts, const Color& col, float w, bool f)
    : points(std::move(pts)), color(col), width(w), filled(f) {}
};

/**
 * @brief 文本渲染命令数据
 */
struct TextCommandData {
  const FontAtlas* font;
  std::string text;
  Vec2 position;
  Color color;
  
  TextCommandData() : font(nullptr), text(), position(), color(Colors::White) {}
};

/**
 * @brief 统一渲染命令结构
 * 使用 variant 存储不同类型的命令数据，减少内存分配
 */
struct RenderCommand {
  RenderCommandType type;
  uint32_t layer;        // 渲染层级，用于排序
  uint32_t order;        // 提交顺序，保证同层级内稳定排序
  glm::mat4 transform;   // 变换矩阵
  
  // 使用 variant 存储具体数据
  std::variant<
    SpriteCommandData,
    LineCommandData,
    RectCommandData,
    CircleCommandData,
    TriangleCommandData,
    PolygonCommandData,
    TextCommandData
  > data;
  
  RenderCommand() : type(RenderCommandType::None), layer(0), order(0), 
                    transform(1.0f) {}
  
  // 便捷构造函数
  static RenderCommand makeSprite(const Texture* tex, const Rect& dest, 
                                   const Rect& src, const Color& tint,
                                   float rot = 0.0f, const Vec2& anc = Vec2(0, 0),
                                   uint32_t lyr = 0);
  static RenderCommand makeLine(const Vec2& s, const Vec2& e, const Color& c, 
                                 float w = 1.0f, uint32_t lyr = 0);
  static RenderCommand makeRect(const Rect& r, const Color& c, 
                                 float w = 1.0f, bool fill = false, uint32_t lyr = 0);
};

/**
 * @brief 渲染命令缓冲区
 * 用于收集和批量处理渲染命令
 */
class RenderCommandBuffer {
public:
  static constexpr size_t INITIAL_CAPACITY = 1024;
  static constexpr size_t MAX_CAPACITY = 65536;
  
  RenderCommandBuffer();
  ~RenderCommandBuffer();
  
  // 添加渲染命令
  void addCommand(const RenderCommand& cmd);
  void addCommand(RenderCommand&& cmd);
  
  // 批量添加（预留空间后使用）
  RenderCommand& emplaceCommand();
  
  // 排序命令（按纹理、层级等）
  void sortCommands();
  
  // 清空缓冲区
  void clear();
  
  // 获取命令列表
  const std::vector<RenderCommand>& getCommands() const { return commands_; }
  std::vector<RenderCommand>& getCommands() { return commands_; }
  
  // 统计
  size_t size() const { return commands_.size(); }
  bool empty() const { return commands_.empty(); }
  size_t capacity() const { return commands_.capacity(); }
  
  // 预分配空间
  void reserve(size_t capacity);
  
private:
  std::vector<RenderCommand> commands_;
  uint32_t nextOrder_;
  
  // 排序比较函数
  static bool compareCommands(const RenderCommand& a, const RenderCommand& b);
};

} // namespace extra2d
