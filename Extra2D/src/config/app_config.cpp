#include <extra2d/config/app_config.h>
#include <extra2d/utils/logger.h>

namespace extra2d {

AppConfig AppConfig::createDefault() {
    AppConfig config;
    
    config.appName = "Extra2D App";
    config.appVersion = "1.0.0";
    config.organization = "";
    config.configFile = "config.json";
    config.targetPlatform = PlatformType::Auto;
    
    return config;
}

bool AppConfig::validate() const {
    if (appName.empty()) {
        E2D_LOG_ERROR("Config validation failed: app name cannot be empty");
        return false;
    }
    if (appVersion.empty()) {
        E2D_LOG_ERROR("Config validation failed: app version cannot be empty");
        return false;
    }
    if (configFile.empty()) {
        E2D_LOG_ERROR("Config validation failed: config file cannot be empty");
        return false;
    }
    
    return true;
}

void AppConfig::reset() {
    *this = createDefault();
    E2D_LOG_INFO("App config reset to defaults");
}

void AppConfig::merge(const AppConfig& other) {
    if (other.appName != "Extra2D App") {
        appName = other.appName;
    }
    if (other.appVersion != "1.0.0") {
        appVersion = other.appVersion;
    }
    if (!other.organization.empty()) {
        organization = other.organization;
    }
    if (other.configFile != "config.json") {
        configFile = other.configFile;
    }
    if (other.targetPlatform != PlatformType::Auto) {
        targetPlatform = other.targetPlatform;
    }
    
    E2D_LOG_INFO("Merged app config");
}

} 
