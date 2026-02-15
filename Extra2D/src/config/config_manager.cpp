#include <extra2d/config/config_manager.h>
#include <extra2d/config/platform_config.h>
#include <extra2d/config/platform_detector.h>
#include <extra2d/utils/logger.h>
#include <sstream>

namespace extra2d {

ConfigManager::ConfigManager()
    : m_nextCallbackId(1)
    , m_autoSaveEnabled(false)
    , m_autoSaveInterval(30.0f)
    , m_autoSaveTimer(0.0f)
{
    m_appConfig = AppConfig::createDefault();
}

ConfigManager::~ConfigManager() {
    if (m_initialized) {
        shutdown();
    }
}

ConfigManager& ConfigManager::instance() {
    static ConfigManager instance;
    return instance;
}

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

    m_initialized = true;
    m_modified = false;

    E2D_LOG_INFO("ConfigManager initialized for platform: {}", 
                 m_platformConfig->platformName());
    return true;
}

void ConfigManager::shutdown() {
    if (!m_initialized) {
        return;
    }

    std::lock_guard<std::mutex> lock(m_mutex);

    if (m_autoSaveEnabled && m_modified) {
        saveConfig();
    }

    m_changeCallbacks.clear();
    m_rawValues.clear();
    m_loader.reset();
    m_platformConfig.reset();

    m_initialized = false;
    m_modified = false;

    E2D_LOG_INFO("ConfigManager shutdown complete");
}

bool ConfigManager::isInitialized() const {
    return m_initialized;
}

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

ConfigLoadResult ConfigManager::loadConfigWithModules(const std::string& filepath) {
    std::lock_guard<std::mutex> lock(m_mutex);

    std::string path = filepath.empty() ? m_configPath : filepath;
    if (path.empty()) {
        return ConfigLoadResult::error("No config file path specified");
    }

    if (!m_loader) {
        return ConfigLoadResult::error("Config loader not initialized");
    }

    ConfigLoadResult result = m_loader->loadWithModules(path);

    if (result.success) {
        m_configPath = path;
        m_modified = false;
        E2D_LOG_INFO("Full config (with modules) loaded from: {}", path);
    } else {
        E2D_LOG_ERROR("Failed to load full config from {}: {}", path, result.errorMessage);
    }

    return result;
}

ConfigSaveResult ConfigManager::saveConfigWithModules(const std::string& filepath) {
    std::lock_guard<std::mutex> lock(m_mutex);

    std::string path = filepath.empty() ? m_configPath : filepath;
    if (path.empty()) {
        return ConfigSaveResult::error("No config file path specified");
    }

    if (!m_loader) {
        return ConfigSaveResult::error("Config loader not initialized");
    }

    ConfigSaveResult result = m_loader->saveWithModules(path);

    if (result.success) {
        m_configPath = path;
        m_modified = false;
        E2D_LOG_INFO("Full config (with modules) saved to: {}", path);
    } else {
        E2D_LOG_ERROR("Failed to save full config to {}: {}", path, result.errorMessage);
    }

    return result;
}

ConfigLoadResult ConfigManager::reload() {
    return loadConfig(m_configPath);
}

const AppConfig& ConfigManager::appConfig() const {
    return m_appConfig;
}

AppConfig& ConfigManager::appConfig() {
    m_modified = true;
    return m_appConfig;
}

void ConfigManager::setAppConfig(const AppConfig& config) {
    std::lock_guard<std::mutex> lock(m_mutex);

    m_appConfig = config;
    m_modified = true;

    E2D_LOG_INFO("App config updated");
}

PlatformConfig* ConfigManager::platformConfig() {
    return m_platformConfig.get();
}

const PlatformConfig* ConfigManager::platformConfig() const {
    return m_platformConfig.get();
}

int ConfigManager::registerChangeCallback(ConfigChangeCallback callback) {
    std::lock_guard<std::mutex> lock(m_mutex);

    int id = m_nextCallbackId++;
    m_changeCallbacks[id] = std::move(callback);

    E2D_LOG_DEBUG("Registered config change callback with id {}", id);
    return id;
}

void ConfigManager::unregisterChangeCallback(int callbackId) {
    std::lock_guard<std::mutex> lock(m_mutex);

    auto it = m_changeCallbacks.find(callbackId);
    if (it != m_changeCallbacks.end()) {
        m_changeCallbacks.erase(it);
        E2D_LOG_DEBUG("Unregistered config change callback {}", callbackId);
    }
}

void ConfigManager::clearChangeCallbacks() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_changeCallbacks.clear();
    E2D_LOG_DEBUG("Cleared all config change callbacks");
}

void ConfigManager::setValue(const std::string& section, const std::string& key, const std::string& value) {
    std::lock_guard<std::mutex> lock(m_mutex);

    std::string fullKey = section + "." + key;
    
    ConfigChangeEvent event;
    event.section = section;
    event.field = key;
    event.oldValue = m_rawValues[fullKey];
    event.newValue = value;

    m_rawValues[fullKey] = value;
    m_modified = true;
    notifyChangeCallbacks(event);
}

void ConfigManager::setValue(const std::string& section, const std::string& key, int value) {
    std::ostringstream oss;
    oss << value;
    setValue(section, key, oss.str());
}

void ConfigManager::setValue(const std::string& section, const std::string& key, float value) {
    std::ostringstream oss;
    oss << value;
    setValue(section, key, oss.str());
}

void ConfigManager::setValue(const std::string& section, const std::string& key, bool value) {
    setValue(section, key, std::string(value ? "true" : "false"));
}

std::string ConfigManager::getValue(const std::string& section, const std::string& key, 
                                    const std::string& defaultValue) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    std::string fullKey = section + "." + key;
    auto it = m_rawValues.find(fullKey);
    if (it != m_rawValues.end()) {
        return it->second;
    }
    return defaultValue;
}

int ConfigManager::getIntValue(const std::string& section, const std::string& key, int defaultValue) const {
    std::string value = getValue(section, key);
    if (!value.empty()) {
        try {
            return std::stoi(value);
        } catch (...) {
            return defaultValue;
        }
    }
    return defaultValue;
}

float ConfigManager::getFloatValue(const std::string& section, const std::string& key, float defaultValue) const {
    std::string value = getValue(section, key);
    if (!value.empty()) {
        try {
            return std::stof(value);
        } catch (...) {
            return defaultValue;
        }
    }
    return defaultValue;
}

bool ConfigManager::getBoolValue(const std::string& section, const std::string& key, bool defaultValue) const {
    std::string value = getValue(section, key);
    if (!value.empty()) {
        if (value == "true" || value == "1") return true;
        if (value == "false" || value == "0") return false;
    }
    return defaultValue;
}

void ConfigManager::resetToDefaults() {
    std::lock_guard<std::mutex> lock(m_mutex);

    m_appConfig = AppConfig::createDefault();
    m_rawValues.clear();
    m_modified = true;

    E2D_LOG_INFO("Config reset to defaults");
}

bool ConfigManager::hasUnsavedChanges() const {
    return m_modified;
}

void ConfigManager::markModified() {
    m_modified = true;
}

void ConfigManager::clearModified() {
    m_modified = false;
}

void ConfigManager::setAutoSave(bool enabled, float interval) {
    std::lock_guard<std::mutex> lock(m_mutex);

    m_autoSaveEnabled = enabled;
    m_autoSaveInterval = interval > 0.0f ? interval : 30.0f;
    m_autoSaveTimer = 0.0f;

    E2D_LOG_INFO("Auto save {} (interval: {}s)", 
                 enabled ? "enabled" : "disabled", m_autoSaveInterval);
}

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

void ConfigManager::notifyChangeCallbacks(const ConfigChangeEvent& event) {
    for (const auto& pair : m_changeCallbacks) {
        if (pair.second) {
            pair.second(event);
        }
    }
}

}
