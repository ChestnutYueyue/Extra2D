#pragma once

#include <extra2d/config/module_config.h>
#include <extra2d/config/module_initializer.h>
#include <extra2d/config/platform_config.h>

namespace extra2d {

class PlatformModuleConfig : public IModuleConfig {
public:
    PlatformType targetPlatform = PlatformType::Auto;

    ModuleInfo getModuleInfo() const override {
        ModuleInfo info;
        info.id = 0;
        info.name = "Platform";
        info.version = "1.0.0";
        info.priority = ModulePriority::Core;
        info.enabled = true;
        return info;
    }

    std::string getConfigSectionName() const override {
        return "platform";
    }

    bool validate() const override {
        return true;
    }

    void resetToDefaults() override {
        targetPlatform = PlatformType::Auto;
    }

    bool loadFromJson(const void* jsonData) override;
    bool saveToJson(void* jsonData) const override;
};

class PlatformModuleInitializer : public IModuleInitializer {
public:
    PlatformModuleInitializer();
    ~PlatformModuleInitializer() override;

    ModuleId getModuleId() const override { return moduleId_; }
    ModulePriority getPriority() const override { return ModulePriority::Core; }
    std::vector<ModuleId> getDependencies() const override { return {}; }

    bool initialize(const IModuleConfig* config) override;
    void shutdown() override;
    bool isInitialized() const override { return initialized_; }

    void setModuleId(ModuleId id) { moduleId_ = id; }
    void setPlatform(PlatformType platform) { targetPlatform_ = platform; }

    PlatformType getPlatform() const { return resolvedPlatform_; }
    PlatformConfig* getPlatformConfig() const { return platformConfig_.get(); }

private:
    bool initSwitch();
    void shutdownSwitch();

    ModuleId moduleId_ = INVALID_MODULE_ID;
    bool initialized_ = false;
    PlatformType targetPlatform_ = PlatformType::Auto;
    PlatformType resolvedPlatform_ = PlatformType::Windows;
    UniquePtr<PlatformConfig> platformConfig_;
};

ModuleId get_platform_module_id();
void register_platform_module();

} // namespace extra2d
