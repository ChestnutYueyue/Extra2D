## DataStore 改进计划（基于 Switch FS Save 示例）

### 阶段一：Switch 平台存档支持

#### 1.1 添加存档挂载管理
- **目标**: [data.h](file:///c:/Users/soulcoco/Desktop/Easy2D/Extra2D/Extra2D/include/extra2d/utils/data.h) 和 [data.cpp](file:///c:/Users/soulcoco/Desktop/Easy2D/Extra2D/Extra2D/src/utils/data.cpp)
- **内容**:
  - 添加 `mountSaveData()` 方法挂载存档
  - 添加 `unmountSaveData()` 方法卸载存档
  - 添加 `commitSaveData()` 方法提交更改
  - 支持自动挂载/卸载 RAII 模式
- **预期收益**: 支持 Switch 官方存档系统

#### 1.2 用户账户集成
- **目标**: [data.h](file:///c:/Users/soulcoco/Desktop/Easy2D/Extra2D/Extra2D/include/extra2d/utils/data.h)
- **内容**:
  - 添加 `setUserId()` / `getUserId()` 方法
  - 自动获取当前用户ID（使用 `accountGetPreselectedUser`）
  - 支持用户特定的存档路径
- **预期收益**: 支持多用户存档隔离

### 阶段二：事务和缓存优化

#### 2.1 事务支持
- **目标**: [data.cpp](file:///c:/Users/soulcoco/Desktop/Easy2D/Extra2D/Extra2D/src/utils/data.cpp)
- **内容**:
  - 添加 `beginTransaction()` 开始事务
  - 添加 `commit()` 提交事务
  - 添加 `rollback()` 回滚事务
  - 批量写入优化，减少文件IO
- **预期收益**: 数据一致性和性能提升

#### 2.2 缓存机制
- **目标**: [data.cpp](file:///c:/Users/soulcoco/Desktop/Easy2D/Extra2D/Extra2D/src/utils/data.cpp)
- **内容**:
  - 添加内存缓存层
  - 延迟写入（Lazy Write）
  - 缓存失效策略
- **预期收益**: 减少文件IO，提升读写性能

### 阶段三：数据格式扩展

#### 3.1 JSON 支持
- **目标**: [data.h](file:///c:/Users/soulcoco/Desktop/Easy2D/Extra2D/Extra2D/include/extra2d/utils/data.h)
- **内容**:
  - 添加 JSON 格式支持（保留 INI 兼容）
  - 嵌套数据结构支持
  - 数组类型支持
- **预期收益**: 更灵活的数据结构

#### 3.2 二进制格式支持
- **目标**: [data.cpp](file:///c:/Users/soulcoco/Desktop/Easy2D/Extra2D/Extra2D/src/utils/data.cpp)
- **内容**:
  - 添加二进制序列化格式
  - 更小的文件体积
  - 更快的读写速度
- **预期收益**: 性能和存储优化

### 阶段四：安全和备份

#### 4.1 数据校验
- **目标**: [data.cpp](file:///c:/Users/soulcoco/Desktop/Easy2D/Extra2D/Extra2D/src/utils/data.cpp)
- **内容**:
  - 添加 CRC32/MD5 校验
  - 数据完整性检查
  - 损坏数据恢复机制
- **预期收益**: 数据可靠性

#### 4.2 自动备份
- **目标**: [data.h](file:///c:/Users/soulcoco/Desktop/Easy2D/Extra2D/Extra2D/include/extra2d/utils/data.h)
- **内容**:
  - 自动备份机制
  - 多版本存档
  - 备份恢复接口
- **预期收益**: 数据安全

---

**预计总工作量**: 6-8 小时
**优先级**: 高（1.1, 1.2）> 中（2.1, 3.1）> 低（2.2, 3.2, 4.1, 4.2）