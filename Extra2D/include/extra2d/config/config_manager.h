#pragma once

#include <extra2d/config/app_config.h>
#include <extra2d/config/config_loader.h>
#include <extra2d/config/platform_config.h>
#include <extra2d/core/types.h>
#include <mutex>
#include <string>
#include <unordered_map>

namespace extra2d {

// ============================================================================
// 配置变更事件
// ============================================================================
struct ConfigChangeEvent {
  std::string section;
  std::string field;
  std::string oldValue;
  std::string newValue;
};

// ============================================================================
// 配置变更回调类型
// ============================================================================
using ConfigChangeCallback = Function<void(const ConfigChangeEvent &)>;

// ============================================================================
// 配置管理器（单例）
// ============================================================================
class ConfigManager {
public:
  using ModuleConfigPtr = Ptr<void>;

  /**
   * @brief 获取单例实例
   * @return 配置管理器实例引用
   */
  static ConfigManager &instance();

  /**
   * @brief 初始化配置管理器
   * @param configPath 配置文件路径
   * @return 如果初始化成功返回 true
   */
  bool initialize(const std::string &configPath = "config.json");

  /**
   * @brief 关闭配置管理器
   */
  void shutdown();

  /**
   * @brief 检查是否已初始化
   * @return 如果已初始化返回 true
   */
  bool isInitialized() const;

  /**
   * @brief 加载配置文件
   * @param filepath 配置文件路径（可选，默认使用初始化时的路径）
   * @return 加载结果
   */
  ConfigLoadResult loadConfig(const std::string &filepath = "");

  /**
   * @brief 保存配置到文件
   * @param filepath 配置文件路径（可选，默认使用初始化时的路径）
   * @return 保存结果
   */
  ConfigSaveResult saveConfig(const std::string &filepath = "");

  /**
   * @brief 重新加载配置
   * @return 加载结果
   */
  ConfigLoadResult reload();

  /**
   * @brief 获取应用配置
   * @return 应用配置的常量引用
   */
  const AppConfig &appConfig() const;

  /**
   * @brief 获取可修改的应用配置
   * @return 应用配置的引用
   */
  AppConfig &appConfig();

  /**
   * @brief 设置应用配置
   * @param config 新的配置
   */
  void setAppConfig(const AppConfig &config);

  /**
   * @brief 获取平台配置
   * @return 平台配置接口指针
   */
  PlatformConfig *platformConfig();

  /**
   * @brief 获取平台配置（常量版本）
   * @return 平台配置接口常量指针
   */
  const PlatformConfig *platformConfig() const;

  /**
   * @brief 注册配置变更回调
   * @param callback 回调函数
   * @return 回调ID，用于取消注册
   */
  int registerChangeCallback(ConfigChangeCallback callback);

  /**
   * @brief 取消注册配置变更回调
   * @param callbackId 回调ID
   */
  void unregisterChangeCallback(int callbackId);

  /**
   * @brief 清除所有变更回调
   */
  void clearChangeCallbacks();

  /**
   * @brief 注册模块配置
   * @param moduleName 模块名称
   * @param config 模块配置指针
   */
  void registerModuleConfig(const std::string &moduleName, Ptr<void> config);

  /**
   * @brief 获取模块配置
   * @param moduleName 模块名称
   * @return 模块配置指针
   */
  template <typename T>
  Ptr<T> getModuleConfig(const std::string &moduleName) const {
    auto it = m_moduleConfigs.find(moduleName);
    if (it != m_moduleConfigs.end()) {
      return std::static_pointer_cast<T>(it->second);
    }
    return nullptr;
  }

  /**
   * @brief 移除模块配置
   * @param moduleName 模块名称
   */
  void removeModuleConfig(const std::string &moduleName);

  /**
   * @brief 检查模块配置是否存在
   * @param moduleName 模块名称
   * @return 如果存在返回 true
   */
  bool hasModuleConfig(const std::string &moduleName) const;

  /**
   * @brief 设置配置值（字符串）
   * @param section 配置节
   * @param key 配置键
   * @param value 配置值
   */
  void setValue(const std::string &section, const std::string &key,
                const std::string &value);

  /**
   * @brief 设置配置值（整数）
   * @param section 配置节
   * @param key 配置键
   * @param value 配置值
   */
  void setValue(const std::string &section, const std::string &key, int value);

  /**
   * @brief 设置配置值（浮点数）
   * @param section 配置节
   * @param key 配置键
   * @param value 配置值
   */
  void setValue(const std::string &section, const std::string &key,
                float value);

  /**
   * @brief 设置配置值（布尔值）
   * @param section 配置节
   * @param key 配置键
   * @param value 配置值
   */
  void setValue(const std::string &section, const std::string &key, bool value);

  /**
   * @brief 获取配置值（字符串）
   * @param section 配置节
   * @param key 配置键
   * @param defaultValue 默认值
   * @return 配置值
   */
  std::string getValue(const std::string &section, const std::string &key,
                       const std::string &defaultValue = "") const;

  /**
   * @brief 获取配置值（整数）
   * @param section 配置节
   * @param key 配置键
   * @param defaultValue 默认值
   * @return 配置值
   */
  int getIntValue(const std::string &section, const std::string &key,
                  int defaultValue = 0) const;

  /**
   * @brief 获取配置值（浮点数）
   * @param section 配置节
   * @param key 配置键
   * @param defaultValue 默认值
   * @return 配置值
   */
  float getFloatValue(const std::string &section, const std::string &key,
                      float defaultValue = 0.0f) const;

  /**
   * @brief 获取配置值（布尔值）
   * @param section 配置节
   * @param key 配置键
   * @param defaultValue 默认值
   * @return 配置值
   */
  bool getBoolValue(const std::string &section, const std::string &key,
                    bool defaultValue = false) const;

  /**
   * @brief 重置配置到默认值
   */
  void resetToDefaults();

  /**
   * @brief 检查配置是否有未保存的更改
   * @return 如果有未保存的更改返回 true
   */
  bool hasUnsavedChanges() const;

  /**
   * @brief 标记配置为已修改
   */
  void markModified();

  /**
   * @brief 清除修改标记
   */
  void clearModified();

  /**
   * @brief 获取配置文件路径
   * @return 配置文件路径
   */
  const std::string &configPath() const { return m_configPath; }

  /**
   * @brief 设置自动保存
   * @param enabled 是否启用自动保存
   * @param interval 自动保存间隔（秒）
   */
  void setAutoSave(bool enabled, float interval = 30.0f);

  /**
   * @brief 检查是否启用自动保存
   * @return 如果启用自动保存返回 true
   */
  bool isAutoSaveEnabled() const { return m_autoSaveEnabled; }

  /**
   * @brief 更新配置管理器（用于自动保存）
   * @param deltaTime 帧时间（秒）
   */
  void update(float deltaTime);

private:
  ConfigManager();
  ~ConfigManager();
  ConfigManager(const ConfigManager &) = delete;
  ConfigManager &operator=(const ConfigManager &) = delete;

  void notifyChangeCallbacks(const ConfigChangeEvent &event);
  void applyConfigToInternal(const AppConfig &config);
  void extractConfigFromInternal(AppConfig &config) const;

  AppConfig m_appConfig;
  UniquePtr<PlatformConfig> m_platformConfig;
  UniquePtr<ConfigLoader> m_loader;
  std::string m_configPath;
  bool m_initialized = false;
  bool m_modified = false;
  mutable std::mutex m_mutex;

  std::unordered_map<int, ConfigChangeCallback> m_changeCallbacks;
  int m_nextCallbackId = 1;

  std::unordered_map<std::string, ModuleConfigPtr> m_moduleConfigs;

  bool m_autoSaveEnabled = false;
  float m_autoSaveInterval = 30.0f;
  float m_autoSaveTimer = 0.0f;
};

// ============================================================================
// 便捷宏定义
// ============================================================================
#define CONFIG_MANAGER ConfigManager::instance()

} // namespace extra2d
