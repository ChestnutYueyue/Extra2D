#pragma once

#include <extra2d/spatial/spatial_index.h>
#include <unordered_map>
#include <vector>

namespace extra2d {

/**
 * @brief 空间哈希实现 - 优化内存布局版本
 * 使用连续内存存储单元格内容，减少内存碎片
 */
class SpatialHash : public ISpatialIndex {
public:
  using CellKey = std::pair<int64_t, int64_t>;

  struct CellKeyHash {
    size_t operator()(const CellKey &key) const {
      return std::hash<int64_t>()(key.first) ^
             (std::hash<int64_t>()(key.second) << 1);
    }
  };

  explicit SpatialHash(float cellSize = 64.0f);
  ~SpatialHash() override = default;

  void insert(Node *node, const Rect &bounds) override;
  void remove(Node *node) override;
  void update(Node *node, const Rect &newBounds) override;

  std::vector<Node *> query(const Rect &area) const override;
  std::vector<Node *> query(const Vec2 &point) const override;
  std::vector<std::pair<Node *, Node *>> queryCollisions() const override;

  void clear() override;
  size_t size() const override;
  bool empty() const override;

  void rebuild() override;

  void setCellSize(float cellSize);
  float getCellSize() const { return cellSize_; }

private:
  /**
   * @brief 单元格数据 - 使用vector代替unordered_set减少内存开销
   */
  struct Cell {
    std::vector<Node *> objects;

    void insert(Node *node);
    void remove(Node *node);
    bool contains(Node *node) const;
    void clear() { objects.clear(); }
    size_t size() const { return objects.size(); }
    bool empty() const { return objects.empty(); }
  };

  CellKey getCellKey(float x, float y) const;
  void getCellsForRect(const Rect &rect, std::vector<CellKey> &cells) const;
  void insertIntoCells(Node *node, const Rect &bounds);
  void removeFromCells(Node *node, const Rect &bounds);

  float cellSize_;
  // 使用vector存储对象列表代替unordered_set，内存更紧凑
  std::unordered_map<CellKey, Cell, CellKeyHash> grid_;
  std::unordered_map<Node *, Rect> objectBounds_;
  size_t objectCount_ = 0;

  // 查询用的临时缓冲区，避免重复分配
  mutable std::vector<Node *> queryBuffer_;
  mutable std::vector<std::pair<Node *, Node *>> collisionBuffer_;
};

} // namespace extra2d
