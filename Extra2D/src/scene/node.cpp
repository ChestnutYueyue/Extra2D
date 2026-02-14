#include <algorithm>
#include <cmath>
#include <extra2d/graphics/render_command.h>
#include <extra2d/scene/node.h>
#include <extra2d/scene/scene.h>
#include <extra2d/utils/logger.h>

namespace extra2d {

/**
 * @brief 默认构造函数
 *
 * 创建一个空的节点对象
 */
Node::Node() = default;

/**
 * @brief 析构函数
 *
 * 清除所有子节点
 */
Node::~Node() { clearChildren(); }

/**
 * @brief 添加子节点
 * @param child 要添加的子节点智能指针
 *
 * 将子节点添加到当前节点的子节点列表中，自动从原父节点分离
 */
void Node::addChild(Ptr<Node> child) {
  if (!child || child.get() == this) {
    return;
  }

  child->detach();
  child->parent_ = weak_from_this();
  children_.push_back(child);
  childrenOrderDirty_ = true;

  // 更新索引
  if (!child->getName().empty()) {
    nameIndex_[child->getName()] = child;
  }
  if (child->getTag() != -1) {
    tagIndex_[child->getTag()] = child;
  }

  if (running_) {
    child->onEnter();
    if (scene_) {
      child->onAttachToScene(scene_);
    }
  }
}

/**
 * @brief 批量添加子节点
 * @param children 要添加的子节点数组（右值引用）
 *
 * 高效地批量添加多个子节点，预分配内存以减少扩容次数
 */
void Node::addChildren(std::vector<Ptr<Node>> &&children) {
  // 预留空间，避免多次扩容
  size_t newSize = children_.size() + children.size();
  if (newSize > children_.capacity()) {
    children_.reserve(newSize);
  }

  for (auto &child : children) {
    if (!child || child.get() == this) {
      continue;
    }

    child->detach();
    child->parent_ = weak_from_this();
    children_.push_back(child);

    // 更新索引
    if (!child->getName().empty()) {
      nameIndex_[child->getName()] = child;
    }
    if (child->getTag() != -1) {
      tagIndex_[child->getTag()] = child;
    }

    if (running_) {
      child->onEnter();
      if (scene_) {
        child->onAttachToScene(scene_);
      }
    }
  }

  if (!children.empty()) {
    childrenOrderDirty_ = true;
  }
}

/**
 * @brief 移除子节点
 * @param child 要移除的子节点智能指针
 *
 * 从子节点列表中移除指定节点，并触发相应的退出回调
 */
void Node::removeChild(Ptr<Node> child) {
  if (!child)
    return;

  auto it = std::find(children_.begin(), children_.end(), child);
  if (it != children_.end()) {
    (*it)->onDetachFromScene();

    if (running_) {
      (*it)->onExit();
    }
    if (!(*it)->getName().empty()) {
      nameIndex_.erase((*it)->getName());
    }
    if ((*it)->getTag() != -1) {
      tagIndex_.erase((*it)->getTag());
    }
    (*it)->parent_.reset();
    children_.erase(it);
  }
}

/**
 * @brief 通过名称移除子节点
 * @param name 子节点的名称
 *
 * 查找并移除具有指定名称的子节点
 */
void Node::removeChildByName(const std::string &name) {
  auto child = findChild(name);
  if (child) {
    removeChild(child);
  }
}

/**
 * @brief 从父节点分离
 *
 * 将当前节点从其父节点的子节点列表中移除
 */
void Node::detach() {
  auto p = parent_.lock();
  if (p) {
    // 安全获取 shared_ptr，避免在对象未由 shared_ptr 管理时崩溃
    Ptr<Node> self;
    try {
      self = shared_from_this();
    } catch (const std::bad_weak_ptr &) {
      // 对象不是由 shared_ptr 管理的，直接重置父节点引用
      parent_.reset();
      return;
    }
    p->removeChild(self);
  }
}

/**
 * @brief 清除所有子节点
 *
 * 移除所有子节点并触发相应的退出回调
 */
void Node::clearChildren() {
  for (auto &child : children_) {
    if (running_) {
      child->onDetachFromScene();
      child->onExit();
    }
    child->parent_.reset();
  }
  children_.clear();
  nameIndex_.clear();
  tagIndex_.clear();
}

/**
 * @brief 通过名称查找子节点
 * @param name 子节点的名称
 * @return 找到的子节点智能指针，未找到返回nullptr
 *
 * 使用哈希索引进行O(1)时间复杂度查找
 */
Ptr<Node> Node::findChild(const std::string &name) const {
  // 使用哈希索引，O(1) 查找
  auto it = nameIndex_.find(name);
  if (it != nameIndex_.end()) {
    return it->second.lock();
  }
  return nullptr;
}

/**
 * @brief 通过标签查找子节点
 * @param tag 子节点的标签值
 * @return 找到的子节点智能指针，未找到返回nullptr
 *
 * 使用哈希索引进行O(1)时间复杂度查找
 */
Ptr<Node> Node::findChildByTag(int tag) const {
  // 使用哈希索引，O(1) 查找
  auto it = tagIndex_.find(tag);
  if (it != tagIndex_.end()) {
    return it->second.lock();
  }
  return nullptr;
}

/**
 * @brief 设置节点位置
 * @param pos 新的位置坐标
 */
void Node::setPos(const Vec2 &pos) {
  position_ = pos;
  markTransformDirty();
}

/**
 * @brief 设置节点位置
 * @param x X坐标
 * @param y Y坐标
 */
void Node::setPos(float x, float y) { setPos(Vec2(x, y)); }

/**
 * @brief 设置节点旋转角度
 * @param degrees 旋转角度（度数）
 */
void Node::setRotation(float degrees) {
  rotation_ = degrees;
  markTransformDirty();
}

/**
 * @brief 设置节点缩放
 * @param scale 缩放向量
 */
void Node::setScale(const Vec2 &scale) {
  scale_ = scale;
  markTransformDirty();
}

/**
 * @brief 设置节点统一缩放
 * @param scale 统一缩放值
 */
void Node::setScale(float scale) { setScale(Vec2(scale, scale)); }

/**
 * @brief 设置节点缩放
 * @param x X轴缩放值
 * @param y Y轴缩放值
 */
void Node::setScale(float x, float y) { setScale(Vec2(x, y)); }

/**
 * @brief 设置节点锚点
 * @param anchor 锚点位置（0-1范围）
 */
void Node::setAnchor(const Vec2 &anchor) {
  anchor_ = anchor;
  markTransformDirty();
}

/**
 * @brief 设置节点锚点
 * @param x 锚点X坐标（0-1范围）
 * @param y 锚点Y坐标（0-1范围）
 */
void Node::setAnchor(float x, float y) { setAnchor(Vec2(x, y)); }

/**
 * @brief 设置节点斜切
 * @param skew 斜切角度向量
 */
void Node::setSkew(const Vec2 &skew) {
  skew_ = skew;
  markTransformDirty();
}

/**
 * @brief 设置节点斜切
 * @param x X轴斜切角度
 * @param y Y轴斜切角度
 */
void Node::setSkew(float x, float y) { setSkew(Vec2(x, y)); }

/**
 * @brief 设置节点透明度
 * @param opacity 透明度值（0.0-1.0范围）
 */
void Node::setOpacity(float opacity) {
  opacity_ = std::clamp(opacity, 0.0f, 1.0f);
}

/**
 * @brief 设置节点可见性
 * @param visible 是否可见
 */
void Node::setVisible(bool visible) { visible_ = visible; }

/**
 * @brief 设置节点颜色
 * @param color RGB颜色值
 */
void Node::setColor(const Color3B &color) { color_ = color; }

/**
 * @brief 设置水平翻转
 * @param flipX 是否水平翻转
 */
void Node::setFlipX(bool flipX) { flipX_ = flipX; }

/**
 * @brief 设置垂直翻转
 * @param flipY 是否垂直翻转
 */
void Node::setFlipY(bool flipY) { flipY_ = flipY; }

/**
 * @brief 设置Z序
 * @param zOrder 渲染层级顺序
 *
 * 较大的Z序值会在上层渲染
 */
void Node::setZOrder(int zOrder) {
  if (zOrder_ != zOrder) {
    zOrder_ = zOrder;
    childrenOrderDirty_ = true;
  }
}

/**
 * @brief 将本地坐标转换为世界坐标
 * @param localPos 本地坐标位置
 * @return 世界坐标位置
 */
Vec2 Node::toWorld(const Vec2 &localPos) const {
  glm::vec4 worldPos =
      getWorldTransform() * glm::vec4(localPos.x, localPos.y, 0.0f, 1.0f);
  return Vec2(worldPos.x, worldPos.y);
}

/**
 * @brief 将世界坐标转换为本地坐标
 * @param worldPos 世界坐标位置
 * @return 本地坐标位置
 */
Vec2 Node::toLocal(const Vec2 &worldPos) const {
  glm::mat4 invWorld = glm::inverse(getWorldTransform());
  glm::vec4 localPos = invWorld * glm::vec4(worldPos.x, worldPos.y, 0.0f, 1.0f);
  return Vec2(localPos.x, localPos.y);
}

/**
 * @brief 获取本地变换矩阵
 * @return 本地变换矩阵
 *
 * 计算包含位置、旋转、斜切和缩放的本地变换矩阵
 */
glm::mat4 Node::getLocalTransform() const {
  if (transformDirty_) {
    localTransform_ = glm::mat4(1.0f);

    // T - R - S order
    localTransform_ = glm::translate(localTransform_,
                                     glm::vec3(position_.x, position_.y, 0.0f));

    if (rotation_ != 0.0f) {
      localTransform_ = glm::rotate(localTransform_, rotation_ * DEG_TO_RAD,
                                    glm::vec3(0.0f, 0.0f, 1.0f));
    }

    if (skew_.x != 0.0f || skew_.y != 0.0f) {
      glm::mat4 skewMatrix(1.0f);
      skewMatrix[1][0] = std::tan(skew_.x * DEG_TO_RAD);
      skewMatrix[0][1] = std::tan(skew_.y * DEG_TO_RAD);
      localTransform_ *= skewMatrix;
    }

    localTransform_ =
        glm::scale(localTransform_, glm::vec3(scale_.x, scale_.y, 1.0f));

    // 注意：锚点偏移在渲染时处理，不在本地变换中处理
    // 这样可以避免锚点偏移被父节点的缩放影响

    transformDirty_ = false;
  }
  return localTransform_;
}

/**
 * @brief 获取世界变换矩阵
 * @return 世界变换矩阵
 *
 * 计算从根节点到当前节点的累积变换矩阵
 */
glm::mat4 Node::getWorldTransform() const {
  if (worldTransformDirty_) {
    // 使用线程局部存储的固定数组，避免每帧内存分配
    // 限制最大深度为 256 层，足以覆盖绝大多数场景
    thread_local std::array<const Node *, 256> nodeChainCache;
    thread_local size_t chainCount = 0;

    chainCount = 0;
    const Node *current = this;
    while (current && chainCount < nodeChainCache.size()) {
      nodeChainCache[chainCount++] = current;
      auto p = current->parent_.lock();
      current = p.get();
    }

    // 从根节点开始计算
    glm::mat4 transform = glm::mat4(1.0f);
    for (size_t i = chainCount; i > 0; --i) {
      transform = transform * nodeChainCache[i - 1]->getLocalTransform();
    }
    worldTransform_ = transform;
    worldTransformDirty_ = false;
  }
  return worldTransform_;
}

/**
 * @brief 标记变换为脏
 *
 * 标记本地变换和世界变换需要重新计算，并递归标记所有子节点
 */
void Node::markTransformDirty() {
  // 避免重复标记，提高性能
  if (!transformDirty_ || !worldTransformDirty_) {
    transformDirty_ = true;
    worldTransformDirty_ = true;

    // 递归标记所有子节点
    for (auto &child : children_) {
      child->markTransformDirty();
    }
  }
}

/**
 * @brief 批量更新变换
 *
 * 从父节点到子节点依次更新世界变换矩阵
 */
void Node::batchTransforms() {
  // 如果本地变换脏了，先计算本地变换
  if (transformDirty_) {
    (void)getLocalTransform(); // 这会计算并缓存本地变换
  }

  // 如果世界变换脏了，需要重新计算
  if (worldTransformDirty_) {
    auto parent = parent_.lock();
    if (parent) {
      // 使用父节点的世界变换（确保父节点已经更新）
      worldTransform_ = parent->getWorldTransform() * localTransform_;
    } else {
      // 根节点
      worldTransform_ = localTransform_;
    }
    worldTransformDirty_ = false;
  }

  // 递归更新子节点
  for (auto &child : children_) {
    child->batchTransforms();
  }
}

/**
 * @brief 节点进入时的回调
 *
 * 标记节点为运行状态，并递归调用所有子节点的onEnter
 */
void Node::onEnter() {
  running_ = true;
  for (auto &child : children_) {
    child->onEnter();
  }
}

/**
 * @brief 节点退出时的回调
 *
 * 标记节点为非运行状态，并递归调用所有子节点的onExit
 */
void Node::onExit() {
  running_ = false;
  for (auto &child : children_) {
    child->onExit();
  }
}

/**
 * @brief 更新回调
 * @param dt 帧间隔时间（秒）
 *
 * 先调用节点自身的更新逻辑，再更新所有子节点
 */
void Node::onUpdate(float dt) {
  onUpdateNode(dt);

  // Update children
  for (auto &child : children_) {
    child->onUpdate(dt);
  }
}

/**
 * @brief 渲染回调
 * @param renderer 渲染后端引用
 *
 * 如果可见则绘制自身，然后递归渲染所有子节点
 */
void Node::onRender(RenderBackend &renderer) {
  if (!visible_)
    return;

  onDraw(renderer);

  for (auto &child : children_) {
    child->onRender(renderer);
  }
}

/**
 * @brief 附加到场景时的回调
 * @param scene 所属场景指针
 *
 * 设置场景引用并递归通知所有子节点
 */
void Node::onAttachToScene(Scene *scene) {
  scene_ = scene;

  for (auto &child : children_) {
    child->onAttachToScene(scene);
  }
}

/**
 * @brief 从场景分离时的回调
 *
 * 清除场景引用并递归通知所有子节点
 */
void Node::onDetachFromScene() {
  scene_ = nullptr;
  for (auto &child : children_) {
    child->onDetachFromScene();
  }
}

/**
 * @brief 获取节点边界矩形
 * @return 节点的边界矩形
 *
 * 默认返回以位置为中心的空矩形，子类应重写此方法
 */
Rect Node::getBounds() const { return Rect(position_.x, position_.y, 0, 0); }

/**
 * @brief 更新节点
 * @param dt 帧间隔时间（秒）
 *
 * 调用onUpdate进行更新
 */
void Node::update(float dt) { onUpdate(dt); }

/**
 * @brief 渲染节点
 * @param renderer 渲染后端引用
 *
 * 如果需要则对子节点排序，然后调用onRender进行渲染
 */
void Node::render(RenderBackend &renderer) {
  if (childrenOrderDirty_) {
    sortChildren();
  }
  onRender(renderer);
}

/**
 * @brief 对子节点按Z序排序
 *
 * 小数组使用插入排序，大数组使用标准排序以优化性能
 */
void Node::sortChildren() {
  // 使用插入排序优化小范围更新场景
  // 插入排序在大部分已有序的情况下性能接近O(n)
  size_t n = children_.size();
  if (n <= 1) {
    childrenOrderDirty_ = false;
    return;
  }

  // 小数组使用插入排序，大数组使用std::sort
  if (n < 32) {
    // 插入排序
    for (size_t i = 1; i < n; ++i) {
      auto key = children_[i];
      int keyZOrder = key->getZOrder();
      int j = static_cast<int>(i) - 1;

      while (j >= 0 && children_[j]->getZOrder() > keyZOrder) {
        children_[j + 1] = children_[j];
        --j;
      }
      children_[j + 1] = key;
    }
  } else {
    // 大数组使用标准排序
    std::sort(children_.begin(), children_.end(),
              [](const Ptr<Node> &a, const Ptr<Node> &b) {
                return a->getZOrder() < b->getZOrder();
              });
  }

  childrenOrderDirty_ = false;
}

/**
 * @brief 收集渲染命令
 * @param commands 渲染命令输出向量
 * @param parentZOrder 父节点的Z序
 *
 * 递归收集当前节点和所有子节点的渲染命令
 */
void Node::collectRenderCommands(std::vector<RenderCommand> &commands,
                                 int parentZOrder) {
  if (!visible_)
    return;

  // 计算累积 Z 序
  int accumulatedZOrder = parentZOrder + zOrder_;

  // 生成当前节点的渲染命令
  generateRenderCommand(commands, accumulatedZOrder);

  // 递归收集子节点的渲染命令
  // 注意：这里假设子节点已经按 Z 序排序
  for (auto &child : children_) {
    child->collectRenderCommands(commands, accumulatedZOrder);
  }
}

} // namespace extra2d
