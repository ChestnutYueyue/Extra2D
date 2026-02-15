#pragma once

#include <extra2d/config/module_config.h>
#include <extra2d/config/module_initializer.h>
#include <extra2d/config/app_config.h>
#include <extra2d/config/config_manager.h>
#include <string>

namespace extra2d {

class ConfigModuleConfig : public IModuleConfig {
public:
    std::string configPath;
    AppConfig appConfig;

    ModuleInfo getModuleInfo() const override {
        ModuleInfo info;
        info.id = 0;
        info.name = "Config";
        info.version = "1.0.0";
        info.priority = ModulePriority::Core;
        info.enabled = true;
        return info;
    }

    std::string getConfigSectionName() const override {
        return "config";
    }

    bool validate() const override {
        return true;
    }

    void resetToDefaults() override {
        configPath.clear();
        appConfig = AppConfig{};
    }

    bool loadFromJson(const void* jsonData) override;
    bool saveToJson(void* jsonData) const override;
};

class ConfigModuleInitializer : public IModuleInitializer {
public:
    ConfigModuleInitializer();
    ~ConfigModuleInitializer() override;

    ModuleId getModuleId() const override { return moduleId_; }
    ModulePriority getPriority() const override { return ModulePriority::Core; }
    std::vector<ModuleId> getDependencies() const override { return {}; }

    bool initialize(const IModuleConfig* config) override;
    void shutdown() override;
    bool isInitialized() const override { return initialized_; }

    void setModuleId(ModuleId id) { moduleId_ = id; }
    void setAppConfig(const AppConfig& config) { appConfig_ = config; }
    void setConfigPath(const std::string& path) { configPath_ = path; }

private:
    ModuleId moduleId_ = INVALID_MODULE_ID;
    bool initialized_ = false;
    AppConfig appConfig_;
    std::string configPath_;
};

ModuleId get_config_module_id();
void register_config_module();

} // namespace extra2d
