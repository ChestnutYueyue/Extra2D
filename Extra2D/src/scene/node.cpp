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

  if (running_) {
    child->onEnter();
    if (scene_) {
      child->onAttachToScene(scene_);
    }
  }
}

void Node::removeChild(Ptr<Node> child) {
  if (!child)
    return;

  auto it = std::find(children_.begin(), children_.end(), child);
  if (it != children_.end()) {
    if (running_) {
      (*it)->onDetachFromScene();
      (*it)->onExit();
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
}

Ptr<Node> Node::getChildByName(const std::string &name) const {
  for (const auto &child : children_) {
    if (child->getName() == name) {
      return child;
    }
  }
  return nullptr;
}

Ptr<Node> Node::getChildByTag(int tag) const {
  for (const auto &child : children_) {
    if (child->getTag() == tag) {
      return child;
    }
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

    // Apply anchor point offset
    localTransform_ = glm::translate(localTransform_,
                                     glm::vec3(-anchor_.x, -anchor_.y, 0.0f));

    transformDirty_ = false;
  }
  return localTransform_;
}

glm::mat4 Node::getWorldTransform() const {
  if (worldTransformDirty_) {
    worldTransform_ = getLocalTransform();

    auto p = parent_.lock();
    if (p) {
      worldTransform_ = p->getWorldTransform() * worldTransform_;
    }
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
  if (action) {
    action->start(this);
    actions_.push_back(action);
  }
}

void Node::stopAllActions() { actions_.clear(); }

void Node::stopAction(Ptr<Action> action) {
  auto it = std::find(actions_.begin(), actions_.end(), action);
  if (it != actions_.end()) {
    actions_.erase(it);
  }
}

void Node::stopActionByTag(int tag) {
  auto it = std::remove_if(
      actions_.begin(), actions_.end(),
      [tag](const Ptr<Action> &action) { return action->getTag() == tag; });
  actions_.erase(it, actions_.end());
}

Ptr<Action> Node::getActionByTag(int tag) const {
  for (const auto &action : actions_) {
    if (action->getTag() == tag) {
      return action;
    }
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
  // 暂时最小化实现以测试
  if (!visible_)
    return;

  // 不排序，不递归，只生成当前节点的命令
  int accumulatedZOrder = parentZOrder + zOrder_;
  generateRenderCommand(commands, accumulatedZOrder);
}

} // namespace extra2d
