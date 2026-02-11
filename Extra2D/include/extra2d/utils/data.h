#pragma once

#include <extra2d/core/types.h>
#include <functional>
#include <string>
#include <vector>

namespace extra2d {

// ============================================================================
// 存档类型枚举
// ============================================================================
enum class SaveDataType {
  Account,   // 用户存档（与特定用户关联）
  Common,    // 公共存档（所有用户共享）
  Cache,     // 缓存数据（可删除）
  Device,    // 设备存档
  Temporary, // 临时数据
};

// ============================================================================
// 用户ID结构（封装 Switch AccountUid）
// ============================================================================
struct UserId {
  uint64_t uid[2] = {0, 0};

  bool isValid() const { return uid[0] != 0 || uid[1] != 0; }
  bool operator==(const UserId &other) const {
    return uid[0] == other.uid[0] && uid[1] == other.uid[1];
  }
  bool operator!=(const UserId &other) const { return !(*this == other); }
};

// ============================================================================
// DataStore 类 - 数据持久化（支持 Switch 存档系统）
// ============================================================================
class DataStore {
public:
  DataStore();
  ~DataStore();

  // ------------------------------------------------------------------------
  // 文件操作
  // ------------------------------------------------------------------------

  /// 加载 INI 文件
  bool load(const std::string &filename);

  /// 保存到 INI 文件
  bool save(const std::string &filename);

  /// 获取当前文件名
  const std::string &getFilename() const { return filename_; }

  // ------------------------------------------------------------------------
  // Switch 存档系统支持
  // ------------------------------------------------------------------------

  /**
   * @brief 挂载 Switch 存档数据
   * @param type 存档类型
   * @param userId 用户ID（Account 类型需要）
   * @param mountName 挂载点名称（默认 "save"）
   * @return 是否成功
   */
  bool mountSaveData(SaveDataType type = SaveDataType::Account,
                     const UserId &userId = UserId(),
                     const std::string &mountName = "save");

  /**
   * @brief 卸载存档挂载
   * @param mountName 挂载点名称
   */
  void unmountSaveData(const std::string &mountName = "save");

  /**
   * @brief 提交存档更改（重要：修改后必须调用）
   * @param mountName 挂载点名称
   * @return 是否成功
   */
  bool commitSaveData(const std::string &mountName = "save");

  /**
   * @brief 检查存档是否已挂载
   */
  bool isSaveDataMounted() const { return saveDataMounted_; }

  /**
   * @brief 获取挂载点路径
   */
  std::string getSaveDataPath(const std::string &path = "") const;

  // ------------------------------------------------------------------------
  // 用户账户管理
  // ------------------------------------------------------------------------

  /**
   * @brief 获取当前预选用户ID
   * @return 用户ID（无效时返回空ID）
   */
  static UserId getCurrentUserId();

  /**
   * @brief 设置默认用户ID
   */
  void setDefaultUserId(const UserId &userId) { defaultUserId_ = userId; }

  /**
   * @brief 获取默认用户ID
   */
  UserId getDefaultUserId() const { return defaultUserId_; }

  // ------------------------------------------------------------------------
  // 数据读写
  // ------------------------------------------------------------------------

  /// 获取字符串值
  std::string getString(const std::string &section, const std::string &key,
                        const std::string &defaultValue = "");

  /// 获取整数值
  int getInt(const std::string &section, const std::string &key,
             int defaultValue = 0);

  /// 获取浮点数值
  float getFloat(const std::string &section, const std::string &key,
                 float defaultValue = 0.0f);

  /// 获取布尔值
  bool getBool(const std::string &section, const std::string &key,
               bool defaultValue = false);

  /// 设置字符串值
  void setString(const std::string &section, const std::string &key,
                 const std::string &value);

  /// 设置整数值
  void setInt(const std::string &section, const std::string &key, int value);

  /// 设置浮点数值
  void setFloat(const std::string &section, const std::string &key,
                float value);

  /// 设置布尔值
  void setBool(const std::string &section, const std::string &key, bool value);

  /// 删除键
  void removeKey(const std::string &section, const std::string &key);

  /// 删除整个 section
  void removeSection(const std::string &section);

  /// 检查键是否存在
  bool hasKey(const std::string &section, const std::string &key);

  /// 检查 section 是否存在
  bool hasSection(const std::string &section);

  /// 清除所有数据
  void clear();

  // ------------------------------------------------------------------------
  // 事务支持
  // ------------------------------------------------------------------------

  /**
   * @brief 开始事务（批量操作，延迟写入）
   */
  void beginTransaction();

  /**
   * @brief 提交事务（写入文件）
   * @return 是否成功
   */
  bool commit();

  /**
   * @brief 回滚事务（放弃更改）
   */
  void rollback();

  /**
   * @brief 检查是否在事务中
   */
  bool isInTransaction() const { return inTransaction_; }

  // ------------------------------------------------------------------------
  // 工具方法
  // ------------------------------------------------------------------------

  /// 获取所有 section 名称
  std::vector<std::string> getAllSections() const;

  /// 获取指定 section 的所有 key
  std::vector<std::string> getAllKeys(const std::string &section) const;

  /// 从存档加载（自动处理挂载路径）
  bool loadFromSave(const std::string &path);

  /// 保存到存档（自动处理挂载路径和提交）
  bool saveToSave(const std::string &path);

private:
  class Impl;
  UniquePtr<Impl> impl_;
  std::string filename_;
  std::string mountName_;
  UserId defaultUserId_;
  bool saveDataMounted_ = false;
  bool inTransaction_ = false;
  bool dirty_ = false;

  // 内部辅助方法
  bool internalSave(const std::string &filename);
};

} // namespace extra2d
