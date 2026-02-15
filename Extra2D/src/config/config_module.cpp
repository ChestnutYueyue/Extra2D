#include <extra2d/config/config_module.h>
#include <extra2d/config/module_registry.h>
#include <extra2d/utils/logger.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace extra2d {

static ModuleId s_configModuleId = INVALID_MODULE_ID;

ModuleId get_config_module_id() {
    return s_configModuleId;
}

bool ConfigModuleConfig::loadFromJson(const void* jsonData) {
    if (!jsonData) return false;
    
    try {
        const json& j = *static_cast<const json*>(jsonData);
        
        if (j.contains("configPath")) {
            configPath = j["configPath"].get<std::string>();
        }
        
        return true;
    } catch (...) {
        return false;
    }
}

bool ConfigModuleConfig::saveToJson(void* jsonData) const {
    if (!jsonData) return false;
    
    try {
        json& j = *static_cast<json*>(jsonData);
        j["configPath"] = configPath;
        return true;
    } catch (...) {
        return false;
    }
}

ConfigModuleInitializer::ConfigModuleInitializer()
    : moduleId_(INVALID_MODULE_ID)
    , initialized_(false) {
}

ConfigModuleInitializer::~ConfigModuleInitializer() {
    if (initialized_) {
        shutdown();
    }
}

bool ConfigModuleInitializer::initialize(const IModuleConfig* config) {
    if (initialized_) return true;
    
    const ConfigModuleConfig* configModule = dynamic_cast<const ConfigModuleConfig*>(config);
    
    if (!configPath_.empty()) {
        if (!ConfigManager::instance().initialize(configPath_)) {
            if (!ConfigManager::instance().initialize()) {
                return false;
            }
        }
    } else {
        if (!ConfigManager::instance().initialize()) {
            return false;
        }
    }
    
    if (configModule && !configModule->appConfig.appName.empty()) {
        ConfigManager::instance().setAppConfig(configModule->appConfig);
    } else if (!appConfig_.appName.empty()) {
        ConfigManager::instance().setAppConfig(appConfig_);
    }
    
    initialized_ = true;
    E2D_LOG_INFO("Config module initialized");
    return true;
}

void ConfigModuleInitializer::shutdown() {
    if (!initialized_) return;
    
    E2D_LOG_INFO("Config module shutting down");
    ConfigManager::instance().shutdown();
    initialized_ = false;
}

void register_config_module() {
    if (s_configModuleId != INVALID_MODULE_ID) return;
    
    s_configModuleId = ModuleRegistry::instance().registerModule(
        makeUnique<ConfigModuleConfig>(),
        []() -> UniquePtr<IModuleInitializer> {
            auto initializer = makeUnique<ConfigModuleInitializer>();
            initializer->setModuleId(s_configModuleId);
            return initializer;
        }
    );
}

namespace {
    struct ConfigModuleAutoRegister {
        ConfigModuleAutoRegister() {
            register_config_module();
        }
    };
    
    static ConfigModuleAutoRegister s_autoRegister;
}

} // namespace extra2d
