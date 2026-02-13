#include <algorithm>
#include <cmath>
#include <extra2d/action/action.h>
#include <extra2d/graphics/render_command.h>
#include <extra2d/scene/node.h>
#include <extra2d/scene/scene.h>
#include <extra2d/utils/logger.h>

namespace extra2d {

Node::Node() = default;

Node::~Node() {
  removeAllChildren();
  stopAllActions();
}

void Node::addChild(Ptr<Node> child) {
  if (!child || child.get() == this) {
    return;
  }

  child->removeFromParent();
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

    child->removeFromParent();
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

void Node::removeChild(Ptr<Node> child) {
  if (!child)
    return;

  auto it = std::find(children_.begin(), children_.end(), child);
  if (it != children_.end()) {
    // 始终从空间索引中移除（无论 running_ 状态）
    // 这确保节点被正确清理
    (*it)->onDetachFromScene();

    if (running_) {
      (*it)->onExit();
    }
    // 从索引中移除
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

void Node::removeChildByName(const std::string &name) {
  auto child = getChildByName(name);
  if (child) {
    removeChild(child);
  }
}

void Node::removeFromParent() {
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

void Node::removeAllChildren() {
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

Ptr<Node> Node::getChildByName(const std::string &name) const {
  // 使用哈希索引，O(1) 查找
  auto it = nameIndex_.find(name);
  if (it != nameIndex_.end()) {
    return it->second.lock();
  }
  return nullptr;
}

Ptr<Node> Node::getChildByTag(int tag) const {
  // 使用哈希索引，O(1) 查找
  auto it = tagIndex_.find(tag);
  if (it != tagIndex_.end()) {
    return it->second.lock();
  }
  return nullptr;
}

void Node::setPosition(const Vec2 &pos) {
  position_ = pos;
  markTransformDirty();
  updateSpatialIndex();
}

void Node::setPosition(float x, float y) { setPosition(Vec2(x, y)); }

void Node::setRotation(float degrees) {
  rotation_ = degrees;
  markTransformDirty();
  updateSpatialIndex();
}

void Node::setScale(const Vec2 &scale) {
  scale_ = scale;
  markTransformDirty();
  updateSpatialIndex();
}

void Node::setScale(float scale) { setScale(Vec2(scale, scale)); }

void Node::setScale(float x, float y) { setScale(Vec2(x, y)); }

void Node::setAnchor(const Vec2 &anchor) {
  anchor_ = anchor;
  markTransformDirty();
}

void Node::setAnchor(float x, float y) { setAnchor(Vec2(x, y)); }

void Node::setSkew(const Vec2 &skew) {
  skew_ = skew;
  markTransformDirty();
}

void Node::setSkew(float x, float y) { setSkew(Vec2(x, y)); }

void Node::setOpacity(float opacity) {
  opacity_ = std::clamp(opacity, 0.0f, 1.0f);
}

void Node::setVisible(bool visible) { visible_ = visible; }

void Node::setZOrder(int zOrder) {
  if (zOrder_ != zOrder) {
    zOrder_ = zOrder;
    childrenOrderDirty_ = true;
  }
}

Vec2 Node::convertToWorldSpace(const Vec2 &localPos) const {
  glm::vec4 worldPos =
      getWorldTransform() * glm::vec4(localPos.x, localPos.y, 0.0f, 1.0f);
  return Vec2(worldPos.x, worldPos.y);
}

Vec2 Node::convertToNodeSpace(const Vec2 &worldPos) const {
  glm::mat4 invWorld = glm::inverse(getWorldTransform());
  glm::vec4 localPos = invWorld * glm::vec4(worldPos.x, worldPos.y, 0.0f, 1.0f);
  return Vec2(localPos.x, localPos.y);
}

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

glm::mat4 Node::getWorldTransform() const {
  if (worldTransformDirty_) {
    // 使用线程局部存储的固定数组，避免每帧内存分配
    // 限制最大深度为 256 层，足以覆盖绝大多数场景
    thread_local std::array<const Node*, 256> nodeChainCache;
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

void Node::batchUpdateTransforms() {
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
    child->batchUpdateTransforms();
  }
}

void Node::onEnter() {
  running_ = true;
  for (auto &child : children_) {
    child->onEnter();
  }
}

void Node::onExit() {
  running_ = false;
  for (auto &child : children_) {
    child->onExit();
  }
}

void Node::onUpdate(float dt) {
  onUpdateNode(dt);

  // Update actions
  for (auto it = actions_.begin(); it != actions_.end();) {
    auto &action = *it;
    action->update(dt);
    if (action->isDone()) {
      it = actions_.erase(it);
    } else {
      ++it;
    }
  }

  // Update children
  for (auto &child : children_) {
    child->onUpdate(dt);
  }
}

void Node::onRender(RenderBackend &renderer) {
  if (!visible_)
    return;

  onDraw(renderer);

  for (auto &child : children_) {
    child->onRender(renderer);
  }
}

void Node::onAttachToScene(Scene *scene) {
  scene_ = scene;

  // 添加到场景的空间索引
  if (spatialIndexed_ && scene_) {
    lastSpatialBounds_ = Rect();
    updateSpatialIndex();
  }

  for (auto &child : children_) {
    child->onAttachToScene(scene);
  }
}

void Node::onDetachFromScene() {
  // 从场景的空间索引移除
  // 注意：即使 lastSpatialBounds_ 为空也要尝试移除，
  // 因为节点可能通过其他方式被插入到空间索引中
  if (spatialIndexed_ && scene_) {
    scene_->removeNodeFromSpatialIndex(this);
    lastSpatialBounds_ = Rect();
  }

  scene_ = nullptr;
  for (auto &child : children_) {
    child->onDetachFromScene();
  }
}

Rect Node::getBoundingBox() const {
  // 默认返回一个以位置为中心的点矩形
  return Rect(position_.x, position_.y, 0, 0);
}

void Node::updateSpatialIndex() {
  if (!spatialIndexed_ || !scene_) {
    return;
  }

  Rect newBounds = getBoundingBox();
  if (newBounds != lastSpatialBounds_) {
    scene_->updateNodeInSpatialIndex(this, lastSpatialBounds_, newBounds);
    lastSpatialBounds_ = newBounds;
  }
}

void Node::runAction(Ptr<Action> action) {
  if (!action) {
    return;
  }

  action->start(this);

  int tag = action->getTag();
  if (tag != -1) {
    // 有 tag 的 Action 存入哈希表，O(1) 查找
    // 如果已存在相同 tag 的 Action，先停止它
    auto it = actionByTag_.find(tag);
    if (it != actionByTag_.end()) {
      // 从 vector 中移除旧的 Action
      auto oldAction = it->second;
      auto vecIt = std::find(actions_.begin(), actions_.end(), oldAction);
      if (vecIt != actions_.end()) {
        actions_.erase(vecIt);
      }
    }
    actionByTag_[tag] = action;
  }

  actions_.push_back(action);
}

void Node::stopAllActions() {
  actions_.clear();
  actionByTag_.clear();
}

void Node::stopAction(Ptr<Action> action) {
  if (!action) {
    return;
  }

  // 从 vector 中移除
  auto it = std::find(actions_.begin(), actions_.end(), action);
  if (it != actions_.end()) {
    // 如果有 tag，从哈希表中也移除
    int tag = action->getTag();
    if (tag != -1) {
      actionByTag_.erase(tag);
    }
    actions_.erase(it);
  }
}

void Node::stopActionByTag(int tag) {
  auto it = actionByTag_.find(tag);
  if (it != actionByTag_.end()) {
    auto action = it->second;
    // 从 vector 中移除
    auto vecIt = std::find(actions_.begin(), actions_.end(), action);
    if (vecIt != actions_.end()) {
      actions_.erase(vecIt);
    }
    actionByTag_.erase(it);
  }
}

Ptr<Action> Node::getActionByTag(int tag) const {
  // O(1) 哈希查找
  auto it = actionByTag_.find(tag);
  if (it != actionByTag_.end()) {
    return it->second;
  }
  return nullptr;
}

void Node::update(float dt) { onUpdate(dt); }

void Node::render(RenderBackend &renderer) {
  if (childrenOrderDirty_) {
    sortChildren();
  }
  onRender(renderer);
}

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
