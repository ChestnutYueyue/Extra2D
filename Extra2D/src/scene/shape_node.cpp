#include <algorithm>
#include <cmath>
#include <extra2d/graphics/render_backend.h>
#include <extra2d/graphics/render_command.h>
#include <extra2d/scene/shape_node.h>
#include <limits>

namespace extra2d {

/**
 * @brief 默认构造函数
 *
 * 创建一个空的形状节点
 */
ShapeNode::ShapeNode() = default;

/**
 * @brief 创建空的形状节点
 * @return 新创建的形状节点智能指针
 */
Ptr<ShapeNode> ShapeNode::create() { return makePtr<ShapeNode>(); }

/**
 * @brief 创建点形状节点
 * @param pos 点的位置坐标
 * @param color 点的颜色
 * @return 新创建的点形状节点智能指针
 */
Ptr<ShapeNode> ShapeNode::createPoint(const Vec2 &pos, const Color &color) {
  auto node = makePtr<ShapeNode>();
  node->shapeType_ = ShapeType::Point;
  node->color_ = color;
  node->points_ = {pos};
  return node;
}

/**
 * @brief 创建线段形状节点
 * @param start 线段起点坐标
 * @param end 线段终点坐标
 * @param color 线段颜色
 * @param width 线段宽度
 * @return 新创建的线段形状节点智能指针
 */
Ptr<ShapeNode> ShapeNode::createLine(const Vec2 &start, const Vec2 &end,
                                     const Color &color, float width) {
  auto node = makePtr<ShapeNode>();
  node->shapeType_ = ShapeType::Line;
  node->color_ = color;
  node->lineWidth_ = width;
  node->points_ = {start, end};
  return node;
}

/**
 * @brief 创建矩形形状节点（空心）
 * @param rect 矩形区域
 * @param color 矩形边框颜色
 * @param width 边框线宽
 * @return 新创建的矩形形状节点智能指针
 */
Ptr<ShapeNode> ShapeNode::createRect(const Rect &rect, const Color &color,
                                     float width) {
  auto node = makePtr<ShapeNode>();
  node->shapeType_ = ShapeType::Rect;
  node->color_ = color;
  node->lineWidth_ = width;
  node->filled_ = false;
  node->points_ = {
      Vec2(rect.left(), rect.top()), Vec2(rect.right(), rect.top()),
      Vec2(rect.right(), rect.bottom()), Vec2(rect.left(), rect.bottom())};
  return node;
}

/**
 * @brief 创建填充矩形形状节点
 * @param rect 矩形区域
 * @param color 矩形填充颜色
 * @return 新创建的填充矩形形状节点智能指针
 */
Ptr<ShapeNode> ShapeNode::createFilledRect(const Rect &rect,
                                           const Color &color) {
  auto node = createRect(rect, color, 0);
  node->filled_ = true;
  return node;
}

/**
 * @brief 创建圆形形状节点（空心）
 * @param center 圆心坐标
 * @param radius 圆的半径
 * @param color 圆的边框颜色
 * @param segments 圆的分段数（边数）
 * @param width 边框线宽
 * @return 新创建的圆形形状节点智能指针
 */
Ptr<ShapeNode> ShapeNode::createCircle(const Vec2 &center, float radius,
                                       const Color &color, int segments,
                                       float width) {
  auto node = makePtr<ShapeNode>();
  node->shapeType_ = ShapeType::Circle;
  node->color_ = color;
  node->lineWidth_ = width;
  node->segments_ = segments;
  node->filled_ = false;
  node->points_ = {center};
  // Store radius in a point for simplicity
  node->addPoint(Vec2(radius, 0));
  return node;
}

/**
 * @brief 创建填充圆形形状节点
 * @param center 圆心坐标
 * @param radius 圆的半径
 * @param color 圆的填充颜色
 * @param segments 圆的分段数（边数）
 * @return 新创建的填充圆形形状节点智能指针
 */
Ptr<ShapeNode> ShapeNode::createFilledCircle(const Vec2 &center, float radius,
                                             const Color &color, int segments) {
  auto node = createCircle(center, radius, color, segments, 0);
  node->filled_ = true;
  return node;
}

/**
 * @brief 创建三角形形状节点（空心）
 * @param p1 三角形第一个顶点坐标
 * @param p2 三角形第二个顶点坐标
 * @param p3 三角形第三个顶点坐标
 * @param color 三角形边框颜色
 * @param width 边框线宽
 * @return 新创建的三角形形状节点智能指针
 */
Ptr<ShapeNode> ShapeNode::createTriangle(const Vec2 &p1, const Vec2 &p2,
                                         const Vec2 &p3, const Color &color,
                                         float width) {
  auto node = makePtr<ShapeNode>();
  node->shapeType_ = ShapeType::Triangle;
  node->color_ = color;
  node->lineWidth_ = width;
  node->filled_ = false;
  node->points_ = {p1, p2, p3};
  return node;
}

/**
 * @brief 创建填充三角形形状节点
 * @param p1 三角形第一个顶点坐标
 * @param p2 三角形第二个顶点坐标
 * @param p3 三角形第三个顶点坐标
 * @param color 三角形填充颜色
 * @return 新创建的填充三角形形状节点智能指针
 */
Ptr<ShapeNode> ShapeNode::createFilledTriangle(const Vec2 &p1, const Vec2 &p2,
                                               const Vec2 &p3,
                                               const Color &color) {
  auto node = createTriangle(p1, p2, p3, color, 0);
  node->filled_ = true;
  return node;
}

/**
 * @brief 创建多边形形状节点（空心）
 * @param points 多边形顶点坐标数组
 * @param color 多边形边框颜色
 * @param width 边框线宽
 * @return 新创建的多边形形状节点智能指针
 */
Ptr<ShapeNode> ShapeNode::createPolygon(const std::vector<Vec2> &points,
                                        const Color &color, float width) {
  auto node = makePtr<ShapeNode>();
  node->shapeType_ = ShapeType::Polygon;
  node->color_ = color;
  node->lineWidth_ = width;
  node->filled_ = false;
  node->points_ = points;
  return node;
}

/**
 * @brief 创建填充多边形形状节点
 * @param points 多边形顶点坐标数组
 * @param color 多边形填充颜色
 * @return 新创建的填充多边形形状节点智能指针
 */
Ptr<ShapeNode> ShapeNode::createFilledPolygon(const std::vector<Vec2> &points,
                                              const Color &color) {
  auto node = createPolygon(points, color, 0);
  node->filled_ = true;
  return node;
}

/**
 * @brief 设置形状的所有顶点
 * @param points 顶点坐标数组
 */
void ShapeNode::setPoints(const std::vector<Vec2> &points) {
  points_ = points;
}

/**
 * @brief 添加一个顶点到形状
 * @param point 要添加的顶点坐标
 */
void ShapeNode::addPoint(const Vec2 &point) {
  points_.push_back(point);
}

/**
 * @brief 清除所有顶点
 */
void ShapeNode::clearPoints() {
  points_.clear();
}

/**
 * @brief 获取形状的边界矩形
 * @return 包围形状的轴对齐边界矩形
 *
 * 计算形状在世界坐标系中的边界框，考虑位置偏移和线宽
 */
Rect ShapeNode::getBounds() const {
  if (points_.empty()) {
    return Rect();
  }

  Vec2 offset = getPosition();

  if (shapeType_ == ShapeType::Circle && points_.size() >= 2) {
    float radius = std::abs(points_[1].x);
    Vec2 center = points_[0] + offset;
    return Rect(center.x - radius, center.y - radius, radius * 2.0f,
                radius * 2.0f);
  }

  float minX = std::numeric_limits<float>::infinity();
  float minY = std::numeric_limits<float>::infinity();
  float maxX = -std::numeric_limits<float>::infinity();
  float maxY = -std::numeric_limits<float>::infinity();

  for (const auto &p : points_) {
    Vec2 world = p + offset;
    minX = std::min(minX, world.x);
    minY = std::min(minY, world.y);
    maxX = std::max(maxX, world.x);
    maxY = std::max(maxY, world.y);
  }

  float inflate = 0.0f;
  if (!filled_ &&
      (shapeType_ == ShapeType::Line || shapeType_ == ShapeType::Rect ||
       shapeType_ == ShapeType::Triangle || shapeType_ == ShapeType::Polygon ||
       shapeType_ == ShapeType::Point)) {
    inflate = std::max(0.0f, lineWidth_ * 0.5f);
  }
  if (shapeType_ == ShapeType::Point) {
    inflate = std::max(inflate, lineWidth_ * 0.5f);
  }

  return Rect(minX - inflate, minY - inflate, (maxX - minX) + inflate * 2.0f,
              (maxY - minY) + inflate * 2.0f);
}

/**
 * @brief 绘制形状节点
 * @param renderer 渲染后端引用
 *
 * 根据形状类型调用相应的渲染方法进行绘制
 * 注意：变换矩阵已由 Node::onRender 通过 pushTransform 应用，
 *       此处直接使用本地坐标即可。
 */
void ShapeNode::onDraw(RenderBackend &renderer) {
  if (points_.empty()) {
    return;
  }

  switch (shapeType_) {
  case ShapeType::Point:
    if (!points_.empty()) {
      renderer.fillCircle(points_[0], lineWidth_ * 0.5f, color_, 8);
    }
    break;

  case ShapeType::Line:
    if (points_.size() >= 2) {
      renderer.drawLine(points_[0], points_[1], color_, lineWidth_);
    }
    break;

  case ShapeType::Rect:
    if (points_.size() >= 4) {
      if (filled_) {
        Rect rect(points_[0].x, points_[0].y, points_[2].x - points_[0].x,
                  points_[2].y - points_[0].y);
        renderer.fillRect(rect, color_);
      } else {
        for (size_t i = 0; i < points_.size(); ++i) {
          Vec2 start = points_[i];
          Vec2 end = points_[(i + 1) % points_.size()];
          renderer.drawLine(start, end, color_, lineWidth_);
        }
      }
    }
    break;

  case ShapeType::Circle:
    if (points_.size() >= 2) {
      float radius = points_[1].x;
      if (filled_) {
        renderer.fillCircle(points_[0], radius, color_, segments_);
      } else {
        renderer.drawCircle(points_[0], radius, color_, segments_, lineWidth_);
      }
    }
    break;

  case ShapeType::Triangle:
    if (points_.size() >= 3) {
      if (filled_) {
        renderer.fillTriangle(points_[0], points_[1], points_[2], color_);
      } else {
        renderer.drawLine(points_[0], points_[1], color_, lineWidth_);
        renderer.drawLine(points_[1], points_[2], color_, lineWidth_);
        renderer.drawLine(points_[2], points_[0], color_, lineWidth_);
      }
    }
    break;

  case ShapeType::Polygon:
    if (!points_.empty()) {
      if (filled_) {
        renderer.fillPolygon(points_, color_);
      } else {
        renderer.drawPolygon(points_, color_, lineWidth_);
      }
    }
    break;
  }
}

/**
 * @brief 生成渲染命令
 * @param commands 渲染命令输出向量
 * @param zOrder 渲染层级
 *
 * 根据形状类型生成对应的渲染命令并添加到命令列表
 */
void ShapeNode::generateRenderCommand(std::vector<RenderCommand> &commands,
                                      int zOrder) {
  if (points_.empty()) {
    return;
  }

  Vec2 offset = getPosition();
  RenderCommand cmd;
  cmd.layer = zOrder;

  switch (shapeType_) {
  case ShapeType::Point:
    if (!points_.empty()) {
      cmd.type = RenderCommandType::FilledCircle;
      cmd.data =
          CircleCommandData{points_[0] + offset, lineWidth_ * 0.5f, color_, 8, 0.0f, true};
    }
    break;

  case ShapeType::Line:
    if (points_.size() >= 2) {
      cmd.type = RenderCommandType::Line;
      cmd.data = LineCommandData{points_[0] + offset, points_[1] + offset, color_,
                          lineWidth_};
    }
    break;

  case ShapeType::Rect:
    if (points_.size() >= 4) {
      if (filled_) {
        cmd.type = RenderCommandType::FilledRect;
        Rect rect(points_[0].x, points_[0].y, points_[2].x - points_[0].x,
                  points_[2].y - points_[0].y);
        cmd.data =
            RectCommandData{Rect(rect.origin + offset, rect.size), color_, 0.0f, true};
      } else {
        cmd.type = RenderCommandType::Rect;
        Rect rect(points_[0].x, points_[0].y, points_[2].x - points_[0].x,
                  points_[2].y - points_[0].y);
        cmd.data =
            RectCommandData{Rect(rect.origin + offset, rect.size), color_, lineWidth_, false};
      }
    }
    break;

  case ShapeType::Circle:
    if (points_.size() >= 2) {
      float radius = points_[1].x;
      if (filled_) {
        cmd.type = RenderCommandType::FilledCircle;
        cmd.data =
            CircleCommandData{points_[0] + offset, radius, color_, segments_, 0.0f, true};
      } else {
        cmd.type = RenderCommandType::Circle;
        cmd.data = CircleCommandData{points_[0] + offset, radius, color_, segments_,
                              lineWidth_, false};
      }
    }
    break;

  case ShapeType::Triangle:
    if (points_.size() >= 3) {
      Vec2 p1 = points_[0] + offset;
      Vec2 p2 = points_[1] + offset;
      Vec2 p3 = points_[2] + offset;
      if (filled_) {
        cmd.type = RenderCommandType::FilledTriangle;
        cmd.data = TriangleCommandData{p1, p2, p3, color_, 0.0f, true};
      } else {
        cmd.type = RenderCommandType::Triangle;
        cmd.data = TriangleCommandData{p1, p2, p3, color_, lineWidth_, false};
      }
    }
    break;

  case ShapeType::Polygon:
    if (!points_.empty()) {
      std::vector<Vec2> transformedPoints;
      transformedPoints.reserve(points_.size());
      for (const auto &p : points_) {
        transformedPoints.push_back(p + offset);
      }

      if (filled_) {
        cmd.type = RenderCommandType::FilledPolygon;
        cmd.data = PolygonCommandData{transformedPoints, color_, 0.0f, true};
      } else {
        cmd.type = RenderCommandType::Polygon;
        cmd.data = PolygonCommandData{transformedPoints, color_, lineWidth_, false};
      }
    }
    break;
  }

  commands.push_back(std::move(cmd));
}

} // namespace extra2d
