#include <algorithm>
#include <extra2d/scene/node.h>
#include <extra2d/spatial/quadtree.h>
#include <functional>

namespace extra2d {

QuadTree::QuadTreeNode::QuadTreeNode(const Rect &bounds, int level)
    : bounds(bounds), level(level) {}

bool QuadTree::QuadTreeNode::contains(const Rect &rect) const {
  return bounds.contains(rect);
}

bool QuadTree::QuadTreeNode::intersects(const Rect &rect) const {
  return bounds.intersects(rect);
}

QuadTree::QuadTree(const Rect &worldBounds) : worldBounds_(worldBounds) {
  root_ = std::make_unique<QuadTreeNode>(worldBounds, 0);
}

void QuadTree::insert(Node *node, const Rect &bounds) {
  if (!node || !root_->intersects(bounds))
    return;
  insertIntoNode(root_.get(), node, bounds);
  objectCount_++;
}

void QuadTree::insertIntoNode(QuadTreeNode *node, Node *object,
                              const Rect &bounds) {
  if (node->children[0]) {
    int index = -1;
    float midX = node->bounds.origin.x + node->bounds.size.width / 2.0f;
    float midY = node->bounds.origin.y + node->bounds.size.height / 2.0f;

    bool top = bounds.origin.y + bounds.size.height <= midY;
    bool bottom = bounds.origin.y >= midY;
    bool left = bounds.origin.x + bounds.size.width <= midX;
    bool right = bounds.origin.x >= midX;

    if (top && left)
      index = 0;
    else if (top && right)
      index = 1;
    else if (bottom && left)
      index = 2;
    else if (bottom && right)
      index = 3;

    if (index != -1) {
      insertIntoNode(node->children[index].get(), object, bounds);
      return;
    }
  }

  node->objects.emplace_back(object, bounds);

  if (node->objects.size() > MAX_OBJECTS && node->level < MAX_LEVELS) {
    if (!node->children[0]) {
      split(node);
    }
  }
}

void QuadTree::split(QuadTreeNode *node) {
  float midX = node->bounds.origin.x + node->bounds.size.width / 2.0f;
  float midY = node->bounds.origin.y + node->bounds.size.height / 2.0f;

  node->children[0] = std::make_unique<QuadTreeNode>(
      Rect(node->bounds.origin.x, node->bounds.origin.y,
           node->bounds.size.width / 2.0f, node->bounds.size.height / 2.0f),
      node->level + 1);
  node->children[1] = std::make_unique<QuadTreeNode>(
      Rect(midX, node->bounds.origin.y, node->bounds.size.width / 2.0f,
           node->bounds.size.height / 2.0f),
      node->level + 1);
  node->children[2] = std::make_unique<QuadTreeNode>(
      Rect(node->bounds.origin.x, midY, node->bounds.size.width / 2.0f,
           node->bounds.size.height / 2.0f),
      node->level + 1);
  node->children[3] = std::make_unique<QuadTreeNode>(
      Rect(midX, midY, node->bounds.size.width / 2.0f,
           node->bounds.size.height / 2.0f),
      node->level + 1);

  auto objects = std::move(node->objects);
  node->objects.clear();

  for (const auto &[obj, bounds] : objects) {
    insertIntoNode(node, obj, bounds);
  }
}

void QuadTree::remove(Node *node) {
  if (!node)
    return;
  if (removeFromNode(root_.get(), node)) {
    objectCount_--;
  }
}

bool QuadTree::removeFromNode(QuadTreeNode *node, Node *object) {
  auto it =
      std::find_if(node->objects.begin(), node->objects.end(),
                   [object](const auto &pair) { return pair.first == object; });

  if (it != node->objects.end()) {
    node->objects.erase(it);
    return true;
  }

  if (node->children[0]) {
    for (auto &child : node->children) {
      if (removeFromNode(child.get(), object)) {
        return true;
      }
    }
  }

  return false;
}

void QuadTree::update(Node *node, const Rect &newBounds) {
  remove(node);
  insert(node, newBounds);
}

std::vector<Node *> QuadTree::query(const Rect &area) const {
  std::vector<Node *> results;
  queryNode(root_.get(), area, results);
  return results;
}

void QuadTree::queryNode(const QuadTreeNode *node, const Rect &area,
                         std::vector<Node *> &results) const {
  if (!node || !node->intersects(area))
    return;

  for (const auto &[obj, bounds] : node->objects) {
    if (bounds.intersects(area)) {
      results.push_back(obj);
    }
  }

  if (node->children[0]) {
    for (const auto &child : node->children) {
      queryNode(child.get(), area, results);
    }
  }
}

std::vector<Node *> QuadTree::query(const Vec2 &point) const {
  std::vector<Node *> results;
  queryNode(root_.get(), point, results);
  return results;
}

void QuadTree::queryNode(const QuadTreeNode *node, const Vec2 &point,
                         std::vector<Node *> &results) const {
  if (!node || !node->bounds.containsPoint(point))
    return;

  for (const auto &[obj, bounds] : node->objects) {
    if (bounds.containsPoint(point)) {
      results.push_back(obj);
    }
  }

  if (node->children[0]) {
    for (const auto &child : node->children) {
      queryNode(child.get(), point, results);
    }
  }
}

std::vector<std::pair<Node *, Node *>> QuadTree::queryCollisions() const {
  std::vector<std::pair<Node *, Node *>> collisions;
  collectCollisions(root_.get(), collisions);
  return collisions;
}

void QuadTree::detectCollisionsInNode(
    const std::vector<std::pair<Node *, Rect>> &objects,
    std::vector<std::pair<Node *, Node *>> &collisions) const {
  size_t n = objects.size();
  if (n < 2)
    return;

  // 使用扫描线算法优化碰撞检测
  // 按 x 坐标排序，只检查可能重叠的对象
  collisionBuffer_.clear();
  collisionBuffer_.reserve(n);
  collisionBuffer_.assign(objects.begin(), objects.end());

  // 按左边界排序
  std::sort(collisionBuffer_.begin(), collisionBuffer_.end(),
            [](const auto &a, const auto &b) {
              return a.second.origin.x < b.second.origin.x;
            });

  // 扫描线检测
  for (size_t i = 0; i < n; ++i) {
    const auto &[objA, boundsA] = collisionBuffer_[i];
    float rightA = boundsA.origin.x + boundsA.size.width;

    // 只检查右边界在 objA 右侧的对象
    for (size_t j = i + 1; j < n; ++j) {
      const auto &[objB, boundsB] = collisionBuffer_[j];

      // 如果 objB 的左边界超过 objA 的右边界，后续对象都不会碰撞
      if (boundsB.origin.x > rightA)
        break;

      // 快速 AABB 检测
      if (boundsA.intersects(boundsB)) {
        collisions.emplace_back(objA, objB);
      }
    }
  }
}

void QuadTree::collectCollisions(
    const QuadTreeNode *node,
    std::vector<std::pair<Node *, Node *>> &collisions) const {
  if (!node)
    return;

  // 使用迭代而非递归，避免深层树栈溢出
  struct StackItem {
    const QuadTreeNode *node;
    size_t ancestorStart;
    size_t ancestorEnd;
  };

  std::vector<StackItem> stack;
  stack.reserve(32);
  stack.push_back({node, 0, 0});

  // 祖先对象列表，用于检测跨节点碰撞
  collisionBuffer_.clear();

  while (!stack.empty()) {
    StackItem item = stack.back();
    stack.pop_back();

    const QuadTreeNode *current = item.node;
    if (!current)
      continue;

    // 检测当前节点对象与祖先对象的碰撞
    for (const auto &[obj, bounds] : current->objects) {
      for (size_t i = item.ancestorStart; i < item.ancestorEnd; ++i) {
        const auto &[ancestorObj, ancestorBounds] = collisionBuffer_[i];
        if (bounds.intersects(ancestorBounds)) {
          collisions.emplace_back(ancestorObj, obj);
        }
      }
    }

    // 检测当前节点内对象之间的碰撞（使用扫描线算法）
    detectCollisionsInNode(current->objects, collisions);

    // 记录当前节点的对象作为祖先
    size_t oldSize = collisionBuffer_.size();
    collisionBuffer_.insert(collisionBuffer_.end(), current->objects.begin(),
                            current->objects.end());

    // 将子节点压入栈（逆序以保持遍历顺序）
    if (current->children[0]) {
      for (int i = 3; i >= 0; --i) {
        if (current->children[i]) {
          stack.push_back({current->children[i].get(), oldSize,
                           collisionBuffer_.size()});
        }
      }
    }

    // 恢复祖先列表（模拟递归返回）
    if (stack.empty() ||
        (stack.back().ancestorStart != oldSize &&
         stack.back().ancestorEnd != collisionBuffer_.size())) {
      collisionBuffer_.resize(oldSize);
    }
  }
}

void QuadTree::clear() {
  root_ = std::make_unique<QuadTreeNode>(worldBounds_, 0);
  objectCount_ = 0;
}

size_t QuadTree::size() const { return objectCount_; }

bool QuadTree::empty() const { return objectCount_ == 0; }

void QuadTree::rebuild() {
  std::vector<std::pair<Node *, Rect>> allObjects;

  std::function<void(QuadTreeNode *)> collect = [&](QuadTreeNode *node) {
    if (!node)
      return;
    for (const auto &obj : node->objects) {
      allObjects.push_back(obj);
    }
    if (node->children[0]) {
      for (auto &child : node->children) {
        collect(child.get());
      }
    }
  };

  collect(root_.get());
  clear();

  for (const auto &[obj, bounds] : allObjects) {
    insert(obj, bounds);
  }
}

} // namespace extra2d
