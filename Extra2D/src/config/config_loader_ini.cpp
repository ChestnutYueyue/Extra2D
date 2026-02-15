#include <extra2d/config/config_loader.h>
#include <extra2d/config/config_manager.h>
#include <extra2d/config/module_registry.h>
#include <extra2d/utils/logger.h>

#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <map>

namespace extra2d {

/**
 * @brief 去除字符串首尾空白字符
 * @param str 输入字符串
 * @return 去除空白后的字符串
 */
static std::string trim(const std::string& str) {
    size_t start = 0;
    while (start < str.length() && std::isspace(static_cast<unsigned char>(str[start]))) {
        ++start;
    }
    size_t end = str.length();
    while (end > start && std::isspace(static_cast<unsigned char>(str[end - 1]))) {
        --end;
    }
    return str.substr(start, end - start);
}

/**
 * @brief 将字符串转换为小写
 * @param str 输入字符串
 * @return 小写字符串
 */
static std::string toLower(const std::string& str) {
    std::string result = str;
    for (char& c : result) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return result;
}

/**
 * @brief INI 数据存储结构
 */
using IniData = std::map<std::string, std::map<std::string, std::string>>;

/**
 * @brief 解析 INI 内容到数据结构
 * @param content INI 内容字符串
 * @param data 输出的 INI 数据
 * @return 解析结果
 */
static ConfigLoadResult parseIniContent(const std::string& content, IniData& data) {
    std::istringstream stream(content);
    std::string line;
    std::string currentSection;
    int lineNumber = 0;
    
    while (std::getline(stream, line)) {
        ++lineNumber;
        line = trim(line);
        
        if (line.empty() || line[0] == ';' || line[0] == '#') {
            continue;
        }
        
        if (line[0] == '[') {
            size_t endBracket = line.find(']');
            if (endBracket == std::string::npos) {
                return ConfigLoadResult::error("INI 解析错误: 缺少右括号", lineNumber);
            }
            currentSection = trim(line.substr(1, endBracket - 1));
            if (data.find(currentSection) == data.end()) {
                data[currentSection] = std::map<std::string, std::string>();
            }
        } else {
            size_t equalPos = line.find('=');
            if (equalPos == std::string::npos) {
                continue;
            }
            
            std::string key = trim(line.substr(0, equalPos));
            std::string value = trim(line.substr(equalPos + 1));
            
            if (currentSection.empty()) {
                return ConfigLoadResult::error("INI 解析错误: 键值对不在任何节中", lineNumber);
            }
            
            data[currentSection][key] = value;
        }
    }
    
    return ConfigLoadResult::ok();
}

/**
 * @brief 获取 INI 值（带默认值）
 * @param data INI 数据
 * @param section 节名
 * @param key 键名
 * @param defaultValue 默认值
 * @return 值字符串
 */
static std::string getIniValue(const IniData& data, const std::string& section, 
                               const std::string& key, const std::string& defaultValue = "") {
    auto sectionIt = data.find(section);
    if (sectionIt == data.end()) {
        return defaultValue;
    }
    auto keyIt = sectionIt->second.find(key);
    if (keyIt == sectionIt->second.end()) {
        return defaultValue;
    }
    return keyIt->second;
}

/**
 * @brief 检查 INI 值是否存在
 * @param data INI 数据
 * @param section 节名
 * @param key 键名
 * @return 是否存在
 */
static bool hasIniValue(const IniData& data, const std::string& section, const std::string& key) {
    auto sectionIt = data.find(section);
    if (sectionIt == data.end()) {
        return false;
    }
    return sectionIt->second.find(key) != sectionIt->second.end();
}

ConfigLoadResult IniConfigLoader::load(const std::string& filepath, AppConfig& config) {
    E2D_LOG_INFO("正在从 INI 文件加载应用配置: {}", filepath);
    
    std::ifstream file(filepath);
    if (!file.is_open()) {
        E2D_LOG_ERROR("无法打开配置文件: {}", filepath);
        return ConfigLoadResult::error("无法打开配置文件: " + filepath);
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    file.close();
    
    return loadFromString(content, config);
}

ConfigSaveResult IniConfigLoader::save(const std::string& filepath, const AppConfig& config) {
    E2D_LOG_INFO("正在保存应用配置到 INI 文件: {}", filepath);
    
    std::string content = saveToString(config);
    
    std::ofstream file(filepath);
    if (!file.is_open()) {
        E2D_LOG_ERROR("无法创建配置文件: {}", filepath);
        return ConfigSaveResult::error("无法创建配置文件: " + filepath);
    }
    
    file << content;
    file.close();
    
    E2D_LOG_INFO("配置已成功保存到: {}", filepath);
    return ConfigSaveResult::ok();
}

ConfigLoadResult IniConfigLoader::loadFromString(const std::string& content, AppConfig& config) {
    IniData data;
    auto result = parseIniContent(content, data);
    if (result.hasError()) {
        return result;
    }
    
    if (hasIniValue(data, "app", "name")) {
        config.appName = getIniValue(data, "app", "name");
    }
    if (hasIniValue(data, "app", "version")) {
        config.appVersion = getIniValue(data, "app", "version");
    }
    if (hasIniValue(data, "app", "organization")) {
        config.organization = getIniValue(data, "app", "organization");
    }
    if (hasIniValue(data, "app", "configFile")) {
        config.configFile = getIniValue(data, "app", "configFile");
    }
    if (hasIniValue(data, "app", "targetPlatform")) {
        int value;
        auto res = parseInt(getIniValue(data, "app", "targetPlatform"), value, "app.targetPlatform");
        if (res.isOk()) {
            config.targetPlatform = static_cast<PlatformType>(value);
        }
    }
    
    E2D_LOG_INFO("INI 应用配置加载成功");
    return ConfigLoadResult::ok();
}

std::string IniConfigLoader::saveToString(const AppConfig& config) {
    std::ostringstream oss;
    
    oss << "[app]\n";
    oss << "name=" << config.appName << "\n";
    oss << "version=" << config.appVersion << "\n";
    oss << "organization=" << config.organization << "\n";
    oss << "configFile=" << config.configFile << "\n";
    oss << "targetPlatform=" << static_cast<int>(config.targetPlatform) << "\n";
    
    return oss.str();
}

ConfigLoadResult IniConfigLoader::loadWithModules(const std::string& filepath) {
    E2D_LOG_INFO("正在从 INI 文件加载完整配置（含模块）: {}", filepath);
    
    std::ifstream file(filepath);
    if (!file.is_open()) {
        E2D_LOG_ERROR("无法打开配置文件: {}", filepath);
        return ConfigLoadResult::error("无法打开配置文件: " + filepath);
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    file.close();
    
    IniData data;
    auto result = parseIniContent(content, data);
    if (result.hasError()) {
        return result;
    }
    
    auto& registry = ModuleRegistry::instance();
    auto moduleIds = registry.getAllModules();
    
    for (ModuleId moduleId : moduleIds) {
        IModuleConfig* moduleConfig = registry.getModuleConfig(moduleId);
        if (!moduleConfig) continue;
        
        std::string sectionName = moduleConfig->getConfigSectionName();
        if (sectionName.empty()) continue;
        
        if (data.find(sectionName) != data.end()) {
            E2D_LOG_DEBUG("加载模块 {} 的 INI 配置", moduleConfig->getModuleInfo().name);
        }
    }
    
    E2D_LOG_INFO("完整配置加载成功");
    return ConfigLoadResult::ok();
}

ConfigSaveResult IniConfigLoader::saveWithModules(const std::string& filepath) {
    E2D_LOG_INFO("正在保存完整配置（含模块）到 INI 文件: {}", filepath);
    
    std::ostringstream oss;
    
    oss << saveToString(ConfigManager::instance().appConfig());
    
    auto& registry = ModuleRegistry::instance();
    auto moduleIds = registry.getAllModules();
    
    for (ModuleId moduleId : moduleIds) {
        IModuleConfig* moduleConfig = registry.getModuleConfig(moduleId);
        if (!moduleConfig) continue;
        
        std::string sectionName = moduleConfig->getConfigSectionName();
        if (sectionName.empty()) continue;
        
        oss << "\n[" << sectionName << "]\n";
    }
    
    std::ofstream file(filepath);
    if (!file.is_open()) {
        E2D_LOG_ERROR("无法创建配置文件: {}", filepath);
        return ConfigSaveResult::error("无法创建配置文件: " + filepath);
    }
    
    file << oss.str();
    file.close();
    
    E2D_LOG_INFO("完整配置已成功保存到: {}", filepath);
    return ConfigSaveResult::ok();
}

bool IniConfigLoader::supportsFile(const std::string& filepath) const {
    if (filepath.length() >= 4) {
        std::string ext = filepath.substr(filepath.length() - 4);
        for (char& c : ext) c = static_cast<char>(std::tolower(c));
        return ext == ".ini";
    }
    return false;
}

UniquePtr<ConfigLoader> IniConfigLoader::clone() const {
    return makeUnique<IniConfigLoader>();
}

std::string IniConfigLoader::sectionKey(const std::string& section, const std::string& key) const {
    return section + "." + key;
}

ConfigLoadResult IniConfigLoader::parseInt(const std::string& value, int& result, const std::string& fieldName) {
    try {
        size_t pos;
        result = std::stoi(value, &pos);
        if (pos != value.length()) {
            return ConfigLoadResult::error("无法解析整数值: " + value, -1, fieldName);
        }
        return ConfigLoadResult::ok();
    } catch (const std::exception& e) {
        return ConfigLoadResult::error(std::string("解析整数失败: ") + e.what(), -1, fieldName);
    }
}

ConfigLoadResult IniConfigLoader::parseFloat(const std::string& value, float& result, const std::string& fieldName) {
    try {
        size_t pos;
        result = std::stof(value, &pos);
        if (pos != value.length()) {
            return ConfigLoadResult::error("无法解析浮点数值: " + value, -1, fieldName);
        }
        return ConfigLoadResult::ok();
    } catch (const std::exception& e) {
        return ConfigLoadResult::error(std::string("解析浮点数失败: ") + e.what(), -1, fieldName);
    }
}

ConfigLoadResult IniConfigLoader::parseBool(const std::string& value, bool& result, const std::string& fieldName) {
    std::string lower = toLower(value);
    if (lower == "true" || lower == "1" || lower == "yes" || lower == "on") {
        result = true;
        return ConfigLoadResult::ok();
    }
    if (lower == "false" || lower == "0" || lower == "no" || lower == "off") {
        result = false;
        return ConfigLoadResult::ok();
    }
    return ConfigLoadResult::error("无法解析布尔值: " + value, -1, fieldName);
}

}
