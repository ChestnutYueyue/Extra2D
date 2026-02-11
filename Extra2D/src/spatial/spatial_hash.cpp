#include <algorithm>
#include <cstdint>
#include <extra2d/scene/node.h>
#include <extra2d/spatial/spatial_hash.h>

namespace extra2d {

// Cell 实现
void SpatialHash::Cell::insert(Node *node) {
  // 检查是否已存在
  if (!contains(node)) {
    objects.push_back(node);
  }
}

void SpatialHash::Cell::remove(Node *node) {
  auto it = std::find(objects.begin(), objects.end(), node);
  if (it != objects.end()) {
    // 用最后一个元素替换，然后pop_back，O(1)操作
    *it = objects.back();
    objects.pop_back();
  }
}

bool SpatialHash::Cell::contains(Node *node) const {
  return std::find(objects.begin(), objects.end(), node) != objects.end();
}

SpatialHash::SpatialHash(float cellSize) : cellSize_(cellSize) {
  // 预分配查询缓冲区，避免重复分配
  queryBuffer_.reserve(64);
  collisionBuffer_.reserve(128);
}

SpatialHash::CellKey SpatialHash::getCellKey(float x, float y) const {
  int64_t cellX = static_cast<int64_t>(std::floor(x / cellSize_));
  int64_t cellY = static_cast<int64_t>(std::floor(y / cellSize_));
  return {cellX, cellY};
}

void SpatialHash::getCellsForRect(const Rect &rect,
                                  std::vector<CellKey> &cells) const {
  cells.clear();

  CellKey minCell = getCellKey(rect.origin.x, rect.origin.y);
  CellKey maxCell = getCellKey(rect.origin.x + rect.size.width,
                               rect.origin.y + rect.size.height);

  // 预分配空间
  size_t cellCount = (maxCell.first - minCell.first + 1) *
                     (maxCell.second - minCell.second + 1);
  cells.reserve(cellCount);

  for (int64_t x = minCell.first; x <= maxCell.first; ++x) {
    for (int64_t y = minCell.second; y <= maxCell.second; ++y) {
      cells.emplace_back(x, y);
    }
  }
}

void SpatialHash::insertIntoCells(Node *node, const Rect &bounds) {
  std::vector<CellKey> cells;
  getCellsForRect(bounds, cells);

  for (const auto &cell : cells) {
    grid_[cell].insert(node);
  }
}

void SpatialHash::removeFromCells(Node *node, const Rect &bounds) {
  std::vector<CellKey> cells;
  getCellsForRect(bounds, cells);

  for (const auto &cell : cells) {
    auto it = grid_.find(cell);
    if (it != grid_.end()) {
      it->second.remove(node);
      if (it->second.empty()) {
        grid_.erase(it);
      }
    }
  }
}

void SpatialHash::insert(Node *node, const Rect &bounds) {
  if (!node)
    return;

  // 检查节点是否已存在，如果存在则先移除
  auto it = objectBounds_.find(node);
  if (it != objectBounds_.end()) {
    removeFromCells(node, it->second);
    it->second = bounds;
  } else {
    objectBounds_[node] = bounds;
    objectCount_++;
  }

  insertIntoCells(node, bounds);
}

void SpatialHash::remove(Node *node) {
  if (!node)
    return;

  auto it = objectBounds_.find(node);
  if (it != objectBounds_.end()) {
    removeFromCells(node, it->second);
    objectBounds_.erase(it);
    objectCount_--;
  } else {
    // 节点不在 objectBounds_ 中，但可能还在 grid_ 的某些单元格中
    // 需要遍历所有单元格来移除
    for (auto &[cellKey, cell] : grid_) {
      cell.remove(node);
    }
  }
}

void SpatialHash::update(Node *node, const Rect &newBounds) {
  auto it = objectBounds_.find(node);
  if (it != objectBounds_.end()) {
    removeFromCells(node, it->second);
    insertIntoCells(node, newBounds);
    it->second = newBounds;
  }
}

std::vector<Node *> SpatialHash::query(const Rect &area) const {
  queryBuffer_.clear();

  std::vector<CellKey> cells;
  getCellsForRect(area, cells);

  // 使用排序+去重代替unordered_set，减少内存分配
  for (const auto &cell : cells) {
    auto it = grid_.find(cell);
    if (it != grid_.end()) {
      for (Node *node : it->second.objects) {
        queryBuffer_.push_back(node);
      }
    }
  }

  // 排序并去重
  std::sort(queryBuffer_.begin(), queryBuffer_.end());
  queryBuffer_.erase(
      std::unique(queryBuffer_.begin(), queryBuffer_.end()),
      queryBuffer_.end());

  // 过滤实际相交的对象
  std::vector<Node *> results;
  results.reserve(queryBuffer_.size());

  for (Node *node : queryBuffer_) {
    auto boundsIt = objectBounds_.find(node);
    if (boundsIt != objectBounds_.end() &&
        boundsIt->second.intersects(area)) {
      results.push_back(node);
    }
  }

  return results;
}

std::vector<Node *> SpatialHash::query(const Vec2 &point) const {
  std::vector<Node *> results;

  CellKey cell = getCellKey(point.x, point.y);
  auto it = grid_.find(cell);

  if (it != grid_.end()) {
    for (Node *node : it->second.objects) {
      auto boundsIt = objectBounds_.find(node);
      if (boundsIt != objectBounds_.end() &&
          boundsIt->second.containsPoint(point)) {
        results.push_back(node);
      }
    }
  }

  return results;
}

std::vector<std::pair<Node *, Node *>> SpatialHash::queryCollisions() const {
  collisionBuffer_.clear();

  // 使用排序+唯一性检查代替unordered_set
  std::vector<std::pair<Node *, Node *>> tempCollisions;
  tempCollisions.reserve(objectCount_ * 2);

  for (const auto &[cell, cellData] : grid_) {
    const auto &objects = cellData.objects;
    size_t count = objects.size();

    // 使用扫描线算法优化单元格内碰撞检测
    for (size_t i = 0; i < count; ++i) {
      Node *nodeA = objects[i];
      auto boundsA = objectBounds_.find(nodeA);
      if (boundsA == objectBounds_.end())
        continue;

      for (size_t j = i + 1; j < count; ++j) {
        Node *nodeB = objects[j];
        auto boundsB = objectBounds_.find(nodeB);
        if (boundsB == objectBounds_.end())
          continue;

        if (boundsA->second.intersects(boundsB->second)) {
          // 确保有序对，便于去重
          if (nodeA < nodeB) {
            tempCollisions.emplace_back(nodeA, nodeB);
          } else {
            tempCollisions.emplace_back(nodeB, nodeA);
          }
        }
      }
    }
  }

  // 排序并去重
  std::sort(tempCollisions.begin(), tempCollisions.end());
  tempCollisions.erase(
      std::unique(tempCollisions.begin(), tempCollisions.end()),
      tempCollisions.end());

  return tempCollisions;
}

void SpatialHash::clear() {
  grid_.clear();
  objectBounds_.clear();
  objectCount_ = 0;
}

size_t SpatialHash::size() const { return objectCount_; }

bool SpatialHash::empty() const { return objectCount_ == 0; }

void SpatialHash::rebuild() {
  auto bounds = objectBounds_;
  clear();

  for (const auto &[node, bound] : bounds) {
    insert(node, bound);
  }
}

void SpatialHash::setCellSize(float cellSize) {
  if (cellSize != cellSize_ && cellSize > 0) {
    cellSize_ = cellSize;
    rebuild();
  }
}

} // namespace extra2d
