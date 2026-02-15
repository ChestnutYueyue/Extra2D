#include <extra2d/config/config_manager.h>
#include <extra2d/config/platform_config.h>
#include <extra2d/config/platform_detector.h>
#include <extra2d/utils/logger.h>
#include <sstream>

namespace extra2d {

/**
 * @brief 构造函数
 * 初始化配置管理器的成员变量
 */
ConfigManager::ConfigManager()
    : m_nextCallbackId(1)
    , m_autoSaveEnabled(false)
    , m_autoSaveInterval(30.0f)
    , m_autoSaveTimer(0.0f)
{
    m_appConfig = AppConfig::createDefault();
}

/**
 * @brief 析构函数
 * 确保在销毁时关闭配置管理器
 */
ConfigManager::~ConfigManager() {
    if (m_initialized) {
        shutdown();
    }
}

/**
 * @brief 获取单例实例
 * 使用静态局部变量实现线程安全的单例模式
 * @return 配置管理器实例引用
 */
ConfigManager& ConfigManager::instance() {
    static ConfigManager instance;
    return instance;
}

/**
 * @brief 初始化配置管理器
 * 创建平台配置和配置加载器
 * @param configPath 配置文件路径
 * @return 如果初始化成功返回 true
 */
bool ConfigManager::initialize(const std::string& configPath) {
    if (m_initialized) {
        E2D_LOG_WARN("ConfigManager already initialized");
        return true;
    }

    std::lock_guard<std::mutex> lock(m_mutex);

    m_configPath = configPath;

    m_platformConfig = createPlatformConfig(PlatformType::Auto);
    if (!m_platformConfig) {
        E2D_LOG_ERROR("Failed to create platform config");
        return false;
    }

    m_loader = makeUnique<JsonConfigLoader>();
    if (!m_loader) {
        E2D_LOG_ERROR("Failed to create config loader");
        return false;
    }

    m_appConfig = AppConfig::createDefault();
    m_appConfig.targetPlatform = PlatformDetector::detect();

    m_platformConfig->applyDefaults(m_appConfig);

    m_initialized = true;
    m_modified = false;

    E2D_LOG_INFO("ConfigManager initialized for platform: {}", 
                 m_platformConfig->platformName());
    return true;
}

/**
 * @brief 关闭配置管理器
 * 清理所有资源并重置状态
 */
void ConfigManager::shutdown() {
    if (!m_initialized) {
        return;
    }

    std::lock_guard<std::mutex> lock(m_mutex);

    if (m_autoSaveEnabled && m_modified) {
        saveConfig();
    }

    m_changeCallbacks.clear();
    m_moduleConfigs.clear();
    m_loader.reset();
    m_platformConfig.reset();

    m_initialized = false;
    m_modified = false;

    E2D_LOG_INFO("ConfigManager shutdown complete");
}

/**
 * @brief 检查是否已初始化
 * @return 如果已初始化返回 true
 */
bool ConfigManager::isInitialized() const {
    return m_initialized;
}

/**
 * @brief 加载配置文件
 * @param filepath 配置文件路径（可选，默认使用初始化时的路径）
 * @return 加载结果
 */
ConfigLoadResult ConfigManager::loadConfig(const std::string& filepath) {
    std::lock_guard<std::mutex> lock(m_mutex);

    std::string path = filepath.empty() ? m_configPath : filepath;
    if (path.empty()) {
        return ConfigLoadResult::error("No config file path specified");
    }

    if (!m_loader) {
        return ConfigLoadResult::error("Config loader not initialized");
    }

    AppConfig loadedConfig;
    ConfigLoadResult result = m_loader->load(path, loadedConfig);

    if (result.success) {
        m_appConfig.merge(loadedConfig);
        
        if (m_platformConfig) {
            m_appConfig.applyPlatformConstraints(*m_platformConfig);
        }

        if (!m_appConfig.validate()) {
            E2D_LOG_WARN("Loaded config validation failed, using defaults");
            m_appConfig = AppConfig::createDefault();
        }

        m_configPath = path;
        m_modified = false;

        E2D_LOG_INFO("Config loaded from: {}", path);
    } else {
        E2D_LOG_ERROR("Failed to load config from {}: {}", path, result.errorMessage);
    }

    return result;
}

/**
 * @brief 保存配置到文件
 * @param filepath 配置文件路径（可选，默认使用初始化时的路径）
 * @return 保存结果
 */
ConfigSaveResult ConfigManager::saveConfig(const std::string& filepath) {
    std::lock_guard<std::mutex> lock(m_mutex);

    std::string path = filepath.empty() ? m_configPath : filepath;
    if (path.empty()) {
        return ConfigSaveResult::error("No config file path specified");
    }

    if (!m_loader) {
        return ConfigSaveResult::error("Config loader not initialized");
    }

    ConfigSaveResult result = m_loader->save(path, m_appConfig);

    if (result.success) {
        m_configPath = path;
        m_modified = false;

        E2D_LOG_INFO("Config saved to: {}", path);
    } else {
        E2D_LOG_ERROR("Failed to save config to {}: {}", path, result.errorMessage);
    }

    return result;
}

/**
 * @brief 重新加载配置
 * @return 加载结果
 */
ConfigLoadResult ConfigManager::reload() {
    return loadConfig(m_configPath);
}

/**
 * @brief 获取应用配置
 * @return 应用配置的常量引用
 */
const AppConfig& ConfigManager::appConfig() const {
    return m_appConfig;
}

/**
 * @brief 获取可修改的应用配置
 * @return 应用配置的引用
 */
AppConfig& ConfigManager::appConfig() {
    m_modified = true;
    return m_appConfig;
}

/**
 * @brief 设置应用配置
 * @param config 新的配置
 */
void ConfigManager::setAppConfig(const AppConfig& config) {
    std::lock_guard<std::mutex> lock(m_mutex);

    m_appConfig = config;
    m_modified = true;

    E2D_LOG_INFO("App config updated");
}

/**
 * @brief 获取平台配置
 * @return 平台配置接口指针
 */
PlatformConfig* ConfigManager::platformConfig() {
    return m_platformConfig.get();
}

/**
 * @brief 获取平台配置（常量版本）
 * @return 平台配置接口常量指针
 */
const PlatformConfig* ConfigManager::platformConfig() const {
    return m_platformConfig.get();
}

/**
 * @brief 注册配置变更回调
 * @param callback 回调函数
 * @return 回调ID，用于取消注册
 */
int ConfigManager::registerChangeCallback(ConfigChangeCallback callback) {
    std::lock_guard<std::mutex> lock(m_mutex);

    int id = m_nextCallbackId++;
    m_changeCallbacks[id] = std::move(callback);

    E2D_LOG_DEBUG("Registered config change callback with id {}", id);
    return id;
}

/**
 * @brief 取消注册配置变更回调
 * @param callbackId 回调ID
 */
void ConfigManager::unregisterChangeCallback(int callbackId) {
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_changeCallbacks.find(callbackId);
    if (it != m_changeCallbacks.end()) {
        m_changeCallbacks.erase(it);
        E2D_LOG_DEBUG("Unregistered config change callback {}", callbackId);
    }
}

/**
 * @brief 清除所有变更回调
 */
void ConfigManager::clearChangeCallbacks() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_changeCallbacks.clear();
    E2D_LOG_DEBUG("Cleared all config change callbacks");
}

/**
 * @brief 注册模块配置
 * @param moduleName 模块名称
 * @param config 模块配置指针
 */
void ConfigManager::registerModuleConfig(const std::string& moduleName, Ptr<void> config) {
    std::lock_guard<std::mutex> lock(m_mutex);

    m_moduleConfigs[moduleName] = config;
    E2D_LOG_DEBUG("Registered module config: {}", moduleName);
}

/**
 * @brief 移除模块配置
 * @param moduleName 模块名称
 */
void ConfigManager::removeModuleConfig(const std::string& moduleName) {
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_moduleConfigs.find(moduleName);
    if (it != m_moduleConfigs.end()) {
        m_moduleConfigs.erase(it);
        E2D_LOG_DEBUG("Removed module config: {}", moduleName);
    }
}

/**
 * @brief 检查模块配置是否存在
 * @param moduleName 模块名称
 * @return 如果存在返回 true
 */
bool ConfigManager::hasModuleConfig(const std::string& moduleName) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_moduleConfigs.find(moduleName) != m_moduleConfigs.end();
}

/**
 * @brief 设置配置值（字符串）
 * @param section 配置节
 * @param key 配置键
 * @param value 配置值
 */
void ConfigManager::setValue(const std::string& section, const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(m_mutex);

    ConfigChangeEvent event;
    event.section = section;
    event.field = key;
    event.newValue = value;

    m_modified = true;
    notifyChangeCallbacks(event);
}

/**
 * @brief 设置配置值（整数）
 * @param section 配置节
 * @param key 配置键
 * @param value 配置值
 */
void ConfigManager::setValue(const std::string& section, const std::string& key, int value) {
    std::ostringstream oss;
    oss << value;
    setValue(section, key, oss.str());
}

/**
 * @brief 设置配置值（浮点数）
 * @param section 配置节
 * @param key 配置键
 * @param value 配置值
 */
void ConfigManager::setValue(const std::string& section, const std::string& key, float value) {
    std::ostringstream oss;
    oss << value;
    setValue(section, key, oss.str());
}

/**
 * @brief 设置配置值（布尔值）
 * @param section 配置节
 * @param key 配置键
 * @param value 配置值
 */
void ConfigManager::setValue(const std::string& section, const std::string& key, bool value) {
    setValue(section, key, std::string(value ? "true" : "false"));
}

/**
 * @brief 获取配置值（字符串）
 * @param section 配置节
 * @param key 配置键
 * @param defaultValue 默认值
 * @return 配置值
 */
std::string ConfigManager::getValue(const std::string& section, const std::string& key, 
                                    const std::string& defaultValue) const {
    return defaultValue;
}

/**
 * @brief 获取配置值（整数）
 * @param section 配置节
 * @param key 配置键
 * @param defaultValue 默认值
 * @return 配置值
 */
int ConfigManager::getIntValue(const std::string& section, const std::string& key, int defaultValue) const {
    return defaultValue;
}

/**
 * @brief 获取配置值（浮点数）
 * @param section 配置节
 * @param key 配置键
 * @param defaultValue 默认值
 * @return 配置值
 */
float ConfigManager::getFloatValue(const std::string& section, const std::string& key, float defaultValue) const {
    return defaultValue;
}

/**
 * @brief 获取配置值（布尔值）
 * @param section 配置节
 * @param key 配置键
 * @param defaultValue 默认值
 * @return 配置值
 */
bool ConfigManager::getBoolValue(const std::string& section, const std::string& key, bool defaultValue) const {
    return defaultValue;
}

/**
 * @brief 重置配置到默认值
 */
void ConfigManager::resetToDefaults() {
    std::lock_guard<std::mutex> lock(m_mutex);

    m_appConfig = AppConfig::createDefault();
    
    if (m_platformConfig) {
        m_platformConfig->applyDefaults(m_appConfig);
    }

    m_modified = true;

    E2D_LOG_INFO("Config reset to defaults");
}

/**
 * @brief 检查配置是否有未保存的更改
 * @return 如果有未保存的更改返回 true
 */
bool ConfigManager::hasUnsavedChanges() const {
    return m_modified;
}

/**
 * @brief 标记配置为已修改
 */
void ConfigManager::markModified() {
    m_modified = true;
}

/**
 * @brief 清除修改标记
 */
void ConfigManager::clearModified() {
    m_modified = false;
}

/**
 * @brief 设置自动保存
 * @param enabled 是否启用自动保存
 * @param interval 自动保存间隔（秒）
 */
void ConfigManager::setAutoSave(bool enabled, float interval) {
    std::lock_guard<std::mutex> lock(m_mutex);

    m_autoSaveEnabled = enabled;
    m_autoSaveInterval = interval > 0.0f ? interval : 30.0f;
    m_autoSaveTimer = 0.0f;

    E2D_LOG_INFO("Auto save {} (interval: {}s)", 
                 enabled ? "enabled" : "disabled", m_autoSaveInterval);
}

/**
 * @brief 更新配置管理器（用于自动保存）
 * @param deltaTime 帧时间（秒）
 */
void ConfigManager::update(float deltaTime) {
    if (!m_autoSaveEnabled || !m_modified) {
        return;
    }

    m_autoSaveTimer += deltaTime;

    if (m_autoSaveTimer >= m_autoSaveInterval) {
        m_autoSaveTimer = 0.0f;
        saveConfig();
    }
}

/**
 * @brief 通知所有变更回调
 * @param event 配置变更事件
 */
void ConfigManager::notifyChangeCallbacks(const ConfigChangeEvent& event) {
    for (const auto& pair : m_changeCallbacks) {
        if (pair.second) {
            pair.second(event);
        }
    }
}

/**
 * @brief 将配置应用到内部存储
 * @param config 配置对象
 */
void ConfigManager::applyConfigToInternal(const AppConfig& config) {
    m_appConfig = config;
}

/**
 * @brief 从内部存储提取配置
 * @param config 输出配置对象
 */
void ConfigManager::extractConfigFromInternal(AppConfig& config) const {
    config = m_appConfig;
}

}
