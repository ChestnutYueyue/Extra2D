#include <extra2d/graphics/render_command.h>
#include <algorithm>

namespace extra2d {

// ============================================================================
// RenderCommand 便捷构造函数
// ============================================================================

RenderCommand RenderCommand::makeSprite(const Texture* tex, const Rect& dest, 
                                         const Rect& src, const Color& tint,
                                         float rot, const Vec2& anc,
                                         uint32_t lyr) {
  RenderCommand cmd;
  cmd.type = RenderCommandType::Sprite;
  cmd.layer = lyr;
  cmd.transform = glm::mat4(1.0f);
  
  SpriteCommandData data;
  data.texture = tex;
  data.destRect = dest;
  data.srcRect = src;
  data.tint = tint;
  data.rotation = rot;
  data.anchor = anc;
  // 生成排序键：纹理指针的高位 + 层级的低位
  data.sortKey = (reinterpret_cast<uintptr_t>(tex) >> 4) & 0xFFFFFFF0;
  data.sortKey |= (lyr & 0xF);
  
  cmd.data = data;
  return cmd;
}

RenderCommand RenderCommand::makeLine(const Vec2& s, const Vec2& e, const Color& c, 
                                       float w, uint32_t lyr) {
  RenderCommand cmd;
  cmd.type = RenderCommandType::Line;
  cmd.layer = lyr;
  cmd.transform = glm::mat4(1.0f);
  
  LineCommandData data;
  data.start = s;
  data.end = e;
  data.color = c;
  data.width = w;
  
  cmd.data = data;
  return cmd;
}

RenderCommand RenderCommand::makeRect(const Rect& r, const Color& c, 
                                       float w, bool fill, uint32_t lyr) {
  RenderCommand cmd;
  cmd.type = fill ? RenderCommandType::FilledRect : RenderCommandType::Rect;
  cmd.layer = lyr;
  cmd.transform = glm::mat4(1.0f);
  
  RectCommandData data;
  data.rect = r;
  data.color = c;
  data.width = w;
  data.filled = fill;
  
  cmd.data = data;
  return cmd;
}

// ============================================================================
// RenderCommandBuffer 实现
// ============================================================================

RenderCommandBuffer::RenderCommandBuffer() : nextOrder_(0) {
  commands_.reserve(INITIAL_CAPACITY);
}

RenderCommandBuffer::~RenderCommandBuffer() = default;

void RenderCommandBuffer::addCommand(const RenderCommand& cmd) {
  if (commands_.size() >= MAX_CAPACITY) {
    // 缓冲区已满，可能需要立即刷新
    return;
  }
  
  RenderCommand copy = cmd;
  copy.order = nextOrder_++;
  commands_.push_back(std::move(copy));
}

void RenderCommandBuffer::addCommand(RenderCommand&& cmd) {
  if (commands_.size() >= MAX_CAPACITY) {
    return;
  }
  
  cmd.order = nextOrder_++;
  commands_.push_back(std::move(cmd));
}

RenderCommand& RenderCommandBuffer::emplaceCommand() {
  if (commands_.size() >= MAX_CAPACITY) {
    // 如果已满，返回一个虚拟命令（不应该发生）
    static RenderCommand dummy;
    return dummy;
  }
  
  commands_.emplace_back();
  commands_.back().order = nextOrder_++;
  return commands_.back();
}

void RenderCommandBuffer::sortCommands() {
  // 按以下优先级排序：
  // 1. 层级 (layer) - 低层级先渲染
  // 2. 命令类型 - 精灵类命令优先批处理
  // 3. 纹理/材质 - 相同纹理的精灵连续渲染
  // 4. 提交顺序 - 保证稳定性
  
  std::sort(commands_.begin(), commands_.end(), compareCommands);
}

void RenderCommandBuffer::clear() {
  commands_.clear();
  nextOrder_ = 0;
}

void RenderCommandBuffer::reserve(size_t capacity) {
  if (capacity <= MAX_CAPACITY) {
    commands_.reserve(capacity);
  }
}

bool RenderCommandBuffer::compareCommands(const RenderCommand& a, const RenderCommand& b) {
  // 首先按层级排序
  if (a.layer != b.layer) {
    return a.layer < b.layer;
  }
  
  // 然后按类型排序（精灵类命令放在一起以便批处理）
  if (a.type != b.type) {
    // 精灵和文本命令优先（需要纹理）
    bool aIsSprite = (a.type == RenderCommandType::Sprite || 
                      a.type == RenderCommandType::Text);
    bool bIsSprite = (b.type == RenderCommandType::Sprite || 
                      b.type == RenderCommandType::Text);
    
    if (aIsSprite != bIsSprite) {
      return aIsSprite > bIsSprite; // 精灵类命令在前
    }
    
    return static_cast<uint8_t>(a.type) < static_cast<uint8_t>(b.type);
  }
  
  // 对于精灵命令，按纹理排序
  if (a.type == RenderCommandType::Sprite && b.type == RenderCommandType::Sprite) {
    const auto& dataA = std::get<SpriteCommandData>(a.data);
    const auto& dataB = std::get<SpriteCommandData>(b.data);
    if (dataA.texture != dataB.texture) {
      return dataA.texture < dataB.texture;
    }
    // 相同纹理时按 sortKey 排序
    if (dataA.sortKey != dataB.sortKey) {
      return dataA.sortKey < dataB.sortKey;
    }
  }
  
  // 最后按提交顺序排序（保证稳定性）
  return a.order < b.order;
}

} // namespace extra2d
