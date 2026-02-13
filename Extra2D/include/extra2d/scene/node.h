#pragma once

#include <algorithm>
#include <extra2d/core/color.h>
#include <extra2d/core/math_types.h>
#include <extra2d/core/types.h>
#include <extra2d/event/event_dispatcher.h>
#include <extra2d/graphics/render_backend.h>
#include <functional>
#include <string>
#include <vector>

namespace extra2d {

// 前向声明
class Scene;
class Action;
class RenderBackend;
struct RenderCommand;

// ============================================================================
// 节点基类 - 场景图的基础
// ============================================================================
class Node : public std::enable_shared_from_this<Node> {
public:
  Node();
  virtual ~Node();

  // ------------------------------------------------------------------------
  // 层级管理
  // ------------------------------------------------------------------------
  void addChild(Ptr<Node> child);

  /**
   * @brief 批量添加子节点
   * @param children 子节点列表
   */
  void addChildren(std::vector<Ptr<Node>> &&children);

  void removeChild(Ptr<Node> child);
  void removeChildByName(const std::string &name);
  void removeFromParent();
  void removeAllChildren();

  Ptr<Node> getParent() const { return parent_.lock(); }
  const std::vector<Ptr<Node>> &getChildren() const { return children_; }
  Ptr<Node> getChildByName(const std::string &name) const;
  Ptr<Node> getChildByTag(int tag) const;

  // ------------------------------------------------------------------------
  // 变换属性
  // ------------------------------------------------------------------------
  void setPosition(const Vec2 &pos);
  void setPosition(float x, float y);
  Vec2 getPosition() const { return position_; }

  void setRotation(float degrees);
  float getRotation() const { return rotation_; }

  void setScale(const Vec2 &scale);
  void setScale(float scale);
  void setScale(float x, float y);
  Vec2 getScale() const { return scale_; }

  void setAnchor(const Vec2 &anchor);
  void setAnchor(float x, float y);
  Vec2 getAnchor() const { return anchor_; }

  void setSkew(const Vec2 &skew);
  void setSkew(float x, float y);
  Vec2 getSkew() const { return skew_; }

  void setOpacity(float opacity);
  float getOpacity() const { return opacity_; }

  void setVisible(bool visible);
  bool isVisible() const { return visible_; }

  /**
   * @brief 设置颜色
   * @param color RGB颜色
   */
  void setColor(const Color3B& color);
  Color3B getColor() const { return color_; }

  /**
   * @brief 设置X轴翻转
   */
  void setFlipX(bool flipX);
  bool isFlipX() const { return flipX_; }

  /**
   * @brief 设置Y轴翻转
   */
  void setFlipY(bool flipY);
  bool isFlipY() const { return flipY_; }

  void setZOrder(int zOrder);
  int getZOrder() const { return zOrder_; }

  // ------------------------------------------------------------------------
  // 世界变换
  // ------------------------------------------------------------------------
  Vec2 convertToWorldSpace(const Vec2 &localPos) const;
  Vec2 convertToNodeSpace(const Vec2 &worldPos) const;

  glm::mat4 getLocalTransform() const;
  glm::mat4 getWorldTransform() const;

  /**
   * @brief 标记变换矩阵为脏状态，并传播到所有子节点
   */
  void markTransformDirty();

  /**
   * @brief 批量更新变换矩阵
   * 在渲染前统一计算所有脏节点的变换矩阵，避免逐节点计算时的重复递归
   */
  void batchUpdateTransforms();

  /**
   * @brief 获取变换脏标记状态
   */
  bool isTransformDirty() const { return transformDirty_; }
  bool isWorldTransformDirty() const { return worldTransformDirty_; }

  // ------------------------------------------------------------------------
  // 名称和标签
  // ------------------------------------------------------------------------
  void setName(const std::string &name) { name_ = name; }
  const std::string &getName() const { return name_; }

  void setTag(int tag) { tag_ = tag; }
  int getTag() const { return tag_; }

  // ------------------------------------------------------------------------
  // 生命周期回调
  // ------------------------------------------------------------------------
  virtual void onEnter();
  virtual void onExit();
  virtual void onUpdate(float dt);
  virtual void onRender(RenderBackend &renderer);
  virtual void onAttachToScene(Scene *scene);
  virtual void onDetachFromScene();

  // ------------------------------------------------------------------------
  // 边界框（用于空间索引）
  // ------------------------------------------------------------------------
  virtual Rect getBoundingBox() const;

  // 是否需要参与空间索引（默认 true）
  void setSpatialIndexed(bool indexed) { spatialIndexed_ = indexed; }
  bool isSpatialIndexed() const { return spatialIndexed_; }

  // 更新空间索引（手动调用，通常在边界框变化后）
  void updateSpatialIndex();

  // ------------------------------------------------------------------------
  // 动作系统
  // ------------------------------------------------------------------------
  /**
   * @brief 运行动作
   * @param action 动作指针（所有权转移）
   * @return 动作指针
   */
  Action* runAction(Action* action);

  /**
   * @brief 停止所有动作
   */
  void stopAllActions();

  /**
   * @brief 停止指定动作
   * @param action 动作指针
   */
  void stopAction(Action* action);

  /**
   * @brief 根据标签停止动作
   * @param tag 标签值
   */
  void stopActionByTag(int tag);

  /**
   * @brief 根据标志位停止动作
   * @param flags 标志位
   */
  void stopActionsByFlags(unsigned int flags);

  /**
   * @brief 根据标签获取动作
   * @param tag 标签值
   * @return 动作指针，未找到返回 nullptr
   */
  Action* getActionByTag(int tag);

  /**
   * @brief 获取运行中的动作数量
   * @return 动作数量
   */
  size_t getActionCount() const;

  /**
   * @brief 检查是否有动作在运行
   * @return true 如果有动作在运行
   */
  bool isRunningActions() const;

  // ------------------------------------------------------------------------
  // 事件系统
  // ------------------------------------------------------------------------
  EventDispatcher &getEventDispatcher() { return eventDispatcher_; }

  // ------------------------------------------------------------------------
  // 内部方法
  // ------------------------------------------------------------------------
  void update(float dt);
  void render(RenderBackend &renderer);
  void sortChildren();

  bool isRunning() const { return running_; }
  Scene *getScene() const { return scene_; }

  // 多线程渲染命令收集
  virtual void collectRenderCommands(std::vector<RenderCommand> &commands,
                                     int parentZOrder = 0);

protected:
  // 子类重写
  virtual void onDraw(RenderBackend &renderer) {}
  virtual void onUpdateNode(float dt) {}
  virtual void generateRenderCommand(std::vector<RenderCommand> &commands,
                                     int zOrder) {};

  // 供子类访问的内部状态
  Vec2 &getPositionRef() { return position_; }
  Vec2 &getScaleRef() { return scale_; }
  Vec2 &getAnchorRef() { return anchor_; }
  float getRotationRef() { return rotation_; }
  float getOpacityRef() { return opacity_; }

private:
  // ==========================================================================
  // 成员变量按类型大小降序排列，减少内存对齐填充
  // 64位系统对齐：std::string(32) > glm::mat4(64) > std::vector(24) > 
  //              double(8) > float(4) > int(4) > bool(1)
  // ==========================================================================

  // 1. 大块内存（64字节）
  mutable glm::mat4 localTransform_;   // 64 bytes
  mutable glm::mat4 worldTransform_;   // 64 bytes

  // 2. 字符串和容器（24-32字节）
  std::string name_;                   // 32 bytes
  std::vector<Ptr<Node>> children_;    // 24 bytes

  // 3. 子节点索引（加速查找）
  std::unordered_map<std::string, WeakPtr<Node>> nameIndex_; // 56 bytes
  std::unordered_map<int, WeakPtr<Node>> tagIndex_;          // 56 bytes

  // 4. 事件分发器
  EventDispatcher eventDispatcher_;    // 大小取决于实现

  // 5. 父节点引用
  WeakPtr<Node> parent_;               // 16 bytes

  // 7. 变换属性（按访问频率分组）
  Vec2 position_ = Vec2::Zero();       // 8 bytes
  Vec2 scale_ = Vec2(1.0f, 1.0f);      // 8 bytes
  Vec2 anchor_ = Vec2(0.5f, 0.5f);     // 8 bytes
  Vec2 skew_ = Vec2::Zero();           // 8 bytes

  // 8. 边界框（用于空间索引）
  Rect lastSpatialBounds_;             // 16 bytes

  // 9. 浮点属性
  float rotation_ = 0.0f;              // 4 bytes
  float opacity_ = 1.0f;               // 4 bytes

  // 10. 颜色属性
  Color3B color_ = Color3B(255, 255, 255); // 3 bytes

  // 11. 整数属性
  int zOrder_ = 0;                     // 4 bytes
  int tag_ = -1;                       // 4 bytes

  // 12. 布尔属性
  bool flipX_ = false;                 // 1 byte
  bool flipY_ = false;                 // 1 byte

  // 11. 场景指针
  Scene *scene_ = nullptr;             // 8 bytes

  // 12. 布尔标志（打包在一起）
  mutable bool transformDirty_ = true;         // 1 byte
  mutable bool worldTransformDirty_ = true;    // 1 byte
  bool childrenOrderDirty_ = false;            // 1 byte
  bool visible_ = true;                        // 1 byte
  bool running_ = false;                       // 1 byte
  bool spatialIndexed_ = true;                 // 1 byte
  // 填充 2 bytes 到 8 字节对齐
};

} // namespace extra2d
