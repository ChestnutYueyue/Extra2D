## Node 节点优化计划

### 阶段一：内存布局与数据结构优化

#### 1.1 成员变量重排（减少内存占用）
- **目标**: [node.h](file:///c:/Users/soulcoco/Desktop/Easy2D/Extra2D/Extra2D/include/extra2d/scene/node.h) 和 [node.cpp](file:///c:/Users/soulcoco/Desktop/Easy2D/Extra2D/Extra2D/src/scene/node.cpp)
- **内容**: 按类型大小降序排列成员变量，减少内存对齐填充
- **预期收益**: 减少约 16-32 字节内存占用

#### 1.2 子节点查找优化（哈希索引）
- **目标**: [node.h](file:///c:/Users/soulcoco/Desktop/Easy2D/Extra2D/Extra2D/include/extra2d/scene/node.h) 和 [node.cpp](file:///c:/Users/soulcoco/Desktop/Easy2D/Extra2D/Extra2D/src/scene/node.cpp)
- **内容**: 
  - 添加 `std::unordered_map<std::string, WeakPtr<Node>> nameIndex_`
  - 添加 `std::unordered_map<int, WeakPtr<Node>> tagIndex_`
  - 修改 `addChild`/`removeChild` 维护索引
- **预期收益**: `getChildByName`/`getChildByTag` 从 O(n) 优化到 O(1)

### 阶段二：Action 系统优化

#### 2.1 Action 存储优化
- **目标**: [node.h](file:///c:/Users/soulcoco/Desktop/Easy2D/Extra2D/Extra2D/include/extra2d/scene/node.h) 和 [node.cpp](file:///c:/Users/soulcoco/Desktop/Easy2D/Extra2D/Extra2D/src/scene/node.cpp)
- **内容**: 
  - 使用 `std::unordered_map<int, Ptr<Action>>` 替代 `std::vector` 存储带 tag 的 Action
  - 使用侵入式链表管理 Action 更新顺序
- **预期收益**: Action 查找和删除从 O(n) 优化到 O(1)

### 阶段三：变换计算优化

#### 3.1 世界变换迭代计算
- **目标**: [node.cpp](file:///c:/Users/soulcoco/Desktop/Easy2D/Extra2D/Extra2D/src/scene/node.cpp)
- **内容**: 
  - 将 `getWorldTransform()` 的递归改为迭代实现
  - 使用栈结构收集父节点链
- **预期收益**: 避免深层级节点的栈溢出风险，减少函数调用开销

#### 3.2 矩阵计算优化
- **目标**: [node.cpp](file:///c:/Users/soulcoco/Desktop/Easy2D/Extra2D/Extra2D/src/scene/node.cpp)
- **内容**: 
  - 优化 `getLocalTransform()` 中的矩阵构造顺序
  - 延迟计算 skew 矩阵（仅在需要时）
- **预期收益**: 减少不必要的矩阵乘法

### 阶段四：渲染与批量操作优化

#### 4.1 渲染命令收集完善
- **目标**: [node.cpp](file:///c:/Users/soulcoco/Desktop/Easy2D/Extra2D/Extra2D/src/scene/node.cpp)
- **内容**: 
  - 完善 `collectRenderCommands` 实现
  - 添加 Z 序累积和递归收集
- **预期收益**: 支持多线程渲染命令收集

#### 4.2 批量操作接口
- **目标**: [node.h](file:///c:/Users/soulcoco/Desktop/Easy2D/Extra2D/Extra2D/include/extra2d/scene/node.h) 和 [node.cpp](file:///c:/Users/soulcoco/Desktop/Easy2D/Extra2D/Extra2D/src/scene/node.cpp)
- **内容**: 
  - 添加 `addChildren(std::vector<Ptr<Node>>&& children)` 批量添加
  - 优化 `removeAllChildren` 使用批量处理
- **预期收益**: 减少多次操作的开销

---

**预计总工作量**: 4-6 小时
**优先级**: 高（1.1, 1.2）> 中（2.1, 3.1）> 低（3.2, 4.1, 4.2）