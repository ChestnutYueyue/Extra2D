#include <extra2d/graphics/render_command.h>
#include <algorithm>

namespace extra2d {

// ============================================================================
// RenderCommand 便捷构造函数
// ============================================================================

/**
 * @brief 创建精灵渲染命令
 *
 * 创建一个用于渲染2D精灵的RenderCommand对象，包含纹理、目标区域、源区域、
 * 着色、旋转、锚点和层级等信息
 *
 * @param tex 指向纹理对象的指针
 * @param dest 目标渲染区域
 * @param src 源纹理区域
 * @param tint 着色颜色
 * @param rot 旋转角度（弧度）
 * @param anc 锚点位置（0.0-1.0范围）
 * @param lyr 渲染层级
 * @return 配置好的RenderCommand对象
 */
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

/**
 * @brief 创建线段渲染命令
 *
 * 创建一个用于渲染线段的RenderCommand对象
 *
 * @param s 线段起点坐标
 * @param e 线段终点坐标
 * @param c 线段颜色
 * @param w 线段宽度
 * @param lyr 渲染层级
 * @return 配置好的RenderCommand对象
 */
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

/**
 * @brief 创建矩形渲染命令
 *
 * 创建一个用于渲染矩形的RenderCommand对象，可选择填充或描边模式
 *
 * @param r 矩形区域
 * @param c 矩形颜色
 * @param w 线条宽度（仅描边模式有效）
 * @param fill 是否填充矩形
 * @param lyr 渲染层级
 * @return 配置好的RenderCommand对象
 */
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

/**
 * @brief 默认构造函数
 *
 * 初始化渲染命令缓冲区，预留初始容量
 */
RenderCommandBuffer::RenderCommandBuffer() : nextOrder_(0) {
  commands_.reserve(INITIAL_CAPACITY);
}

/**
 * @brief 析构函数
 *
 * 释放渲染命令缓冲区资源
 */
RenderCommandBuffer::~RenderCommandBuffer() = default;

/**
 * @brief 添加渲染命令（左值引用版本）
 *
 * 将渲染命令以拷贝方式添加到缓冲区，自动分配顺序号
 *
 * @param cmd 要添加的渲染命令
 */
void RenderCommandBuffer::addCommand(const RenderCommand& cmd) {
  if (commands_.size() >= MAX_CAPACITY) {
    // 缓冲区已满，可能需要立即刷新
    return;
  }
  
  RenderCommand copy = cmd;
  copy.order = nextOrder_++;
  commands_.push_back(std::move(copy));
}

/**
 * @brief 添加渲染命令（右值引用版本）
 *
 * 将渲染命令以移动方式添加到缓冲区，自动分配顺序号
 *
 * @param cmd 要添加的渲染命令（右值引用）
 */
void RenderCommandBuffer::addCommand(RenderCommand&& cmd) {
  if (commands_.size() >= MAX_CAPACITY) {
    return;
  }
  
  cmd.order = nextOrder_++;
  commands_.push_back(std::move(cmd));
}

/**
 * @brief 原地构造渲染命令
 *
 * 在缓冲区中直接构造一个渲染命令对象，避免额外的拷贝或移动操作
 *
 * @return 新构造的渲染命令对象的引用
 */
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

/**
 * @brief 对渲染命令进行排序
 *
 * 按层级、命令类型、纹理/材质和提交顺序对命令进行排序，
 * 以优化渲染性能和批处理效率
 */
void RenderCommandBuffer::sortCommands() {
  // 按以下优先级排序：
  // 1. 层级 (layer) - 低层级先渲染
  // 2. 命令类型 - 精灵类命令优先批处理
  // 3. 纹理/材质 - 相同纹理的精灵连续渲染
  // 4. 提交顺序 - 保证稳定性
  
  std::sort(commands_.begin(), commands_.end(), compareCommands);
}

/**
 * @brief 清空缓冲区
 *
 * 移除所有渲染命令并重置顺序计数器
 */
void RenderCommandBuffer::clear() {
  commands_.clear();
  nextOrder_ = 0;
}

/**
 * @brief 预留缓冲区容量
 *
 * 预先分配缓冲区内存以减少动态分配开销
 *
 * @param capacity 要预留的容量大小
 */
void RenderCommandBuffer::reserve(size_t capacity) {
  if (capacity <= MAX_CAPACITY) {
    commands_.reserve(capacity);
  }
}

/**
 * @brief 渲染命令比较函数
 *
 * 用于排序的比较函数，按层级、类型、纹理和顺序进行比较
 *
 * @param a 第一个渲染命令
 * @param b 第二个渲染命令
 * @return 如果a应排在b前面返回true，否则返回false
 */
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
