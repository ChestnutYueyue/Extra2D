#include <extra2d/config/config_loader.h>
#include <extra2d/config/module_registry.h>
#include <extra2d/utils/logger.h>

#include <nlohmann/json.hpp>
#include <fstream>
#include <sstream>

using json = nlohmann::json;

namespace extra2d {

ConfigLoadResult JsonConfigLoader::load(const std::string& filepath, AppConfig& config) {
    E2D_LOG_INFO("正在从 JSON 文件加载应用配置: {}", filepath);
    
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

ConfigSaveResult JsonConfigLoader::save(const std::string& filepath, const AppConfig& config) {
    E2D_LOG_INFO("正在保存应用配置到 JSON 文件: {}", filepath);
    
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

ConfigLoadResult JsonConfigLoader::loadFromString(const std::string& content, AppConfig& config) {
    json root;
    
    try {
        root = json::parse(content);
    } catch (const json::parse_error& e) {
        E2D_LOG_ERROR("JSON 解析错误: {}", e.what());
        return ConfigLoadResult::error(std::string("JSON 解析错误: ") + e.what(), 
                                       static_cast<int>(e.byte));
    }
    
    if (root.contains("appName") && root["appName"].is_string()) {
        config.appName = root["appName"].get<std::string>();
    }
    
    if (root.contains("appVersion") && root["appVersion"].is_string()) {
        config.appVersion = root["appVersion"].get<std::string>();
    }
    
    if (root.contains("organization") && root["organization"].is_string()) {
        config.organization = root["organization"].get<std::string>();
    }
    
    if (root.contains("configFile") && root["configFile"].is_string()) {
        config.configFile = root["configFile"].get<std::string>();
    }
    
    if (root.contains("targetPlatform") && root["targetPlatform"].is_number_integer()) {
        config.targetPlatform = static_cast<PlatformType>(root["targetPlatform"].get<int>());
    }
    
    E2D_LOG_INFO("JSON 应用配置加载成功");
    return ConfigLoadResult::ok();
}

std::string JsonConfigLoader::saveToString(const AppConfig& config) {
    json root;
    
    root["appName"] = config.appName;
    root["appVersion"] = config.appVersion;
    root["organization"] = config.organization;
    root["configFile"] = config.configFile;
    root["targetPlatform"] = static_cast<int>(config.targetPlatform);
    
    return root.dump(4);
}

ConfigLoadResult JsonConfigLoader::loadWithModules(const std::string& filepath) {
    E2D_LOG_INFO("正在从 JSON 文件加载完整配置（含模块）: {}", filepath);
    
    std::ifstream file(filepath);
    if (!file.is_open()) {
        E2D_LOG_ERROR("无法打开配置文件: {}", filepath);
        return ConfigLoadResult::error("无法打开配置文件: " + filepath);
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    file.close();
    
    json root;
    try {
        root = json::parse(content);
    } catch (const json::parse_error& e) {
        E2D_LOG_ERROR("JSON 解析错误: {}", e.what());
        return ConfigLoadResult::error(std::string("JSON 解析错误: ") + e.what(), 
                                       static_cast<int>(e.byte));
    }
    
    auto& registry = ModuleRegistry::instance();
    auto moduleIds = registry.getAllModules();
    
    for (ModuleId moduleId : moduleIds) {
        IModuleConfig* moduleConfig = registry.getModuleConfig(moduleId);
        if (!moduleConfig) continue;
        
        std::string sectionName = moduleConfig->getConfigSectionName();
        if (sectionName.empty()) continue;
        
        if (root.contains(sectionName)) {
            if (!moduleConfig->loadFromJson(&root[sectionName])) {
                E2D_LOG_WARN("模块 {} 配置加载失败", moduleConfig->getModuleInfo().name);
            } else {
                E2D_LOG_DEBUG("模块 {} 配置加载成功", moduleConfig->getModuleInfo().name);
            }
        }
    }
    
    E2D_LOG_INFO("完整配置加载成功");
    return ConfigLoadResult::ok();
}

ConfigSaveResult JsonConfigLoader::saveWithModules(const std::string& filepath) {
    E2D_LOG_INFO("正在保存完整配置（含模块）到 JSON 文件: {}", filepath);
    
    json root;
    
    auto& registry = ModuleRegistry::instance();
    auto moduleIds = registry.getAllModules();
    
    for (ModuleId moduleId : moduleIds) {
        IModuleConfig* moduleConfig = registry.getModuleConfig(moduleId);
        if (!moduleConfig) continue;
        
        std::string sectionName = moduleConfig->getConfigSectionName();
        if (sectionName.empty()) continue;
        
        json sectionJson;
        if (moduleConfig->saveToJson(&sectionJson)) {
            root[sectionName] = sectionJson;
        }
    }
    
    std::ofstream file(filepath);
    if (!file.is_open()) {
        E2D_LOG_ERROR("无法创建配置文件: {}", filepath);
        return ConfigSaveResult::error("无法创建配置文件: " + filepath);
    }
    
    file << root.dump(4);
    file.close();
    
    E2D_LOG_INFO("完整配置已成功保存到: {}", filepath);
    return ConfigSaveResult::ok();
}

bool JsonConfigLoader::supportsFile(const std::string& filepath) const {
    if (filepath.length() >= 5) {
        std::string ext = filepath.substr(filepath.length() - 5);
        for (char& c : ext) c = static_cast<char>(std::tolower(c));
        return ext == ".json";
    }
    return false;
}

UniquePtr<ConfigLoader> JsonConfigLoader::clone() const {
    return makeUnique<JsonConfigLoader>();
}

}
