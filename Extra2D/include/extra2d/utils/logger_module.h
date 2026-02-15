#pragma once

#include <extra2d/config/module_config.h>
#include <extra2d/config/module_initializer.h>
#include <extra2d/utils/logger.h>
#include <string>

namespace extra2d {

class LoggerModuleConfig : public IModuleConfig {
public:
    LogLevel logLevel = LogLevel::Info;
    bool consoleOutput = true;
    bool fileOutput = false;
    std::string logFilePath;

    ModuleInfo getModuleInfo() const override {
        ModuleInfo info;
        info.id = 0;
        info.name = "Logger";
        info.version = "1.0.0";
        info.priority = ModulePriority::Core;
        info.enabled = true;
        return info;
    }

    std::string getConfigSectionName() const override {
        return "logger";
    }

    bool validate() const override {
        return true;
    }

    void resetToDefaults() override {
        logLevel = LogLevel::Info;
        consoleOutput = true;
        fileOutput = false;
        logFilePath.clear();
    }

    bool loadFromJson(const void* jsonData) override;
    bool saveToJson(void* jsonData) const override;
};

class LoggerModuleInitializer : public IModuleInitializer {
public:
    LoggerModuleInitializer();
    ~LoggerModuleInitializer() override;

    ModuleId getModuleId() const override { return moduleId_; }
    ModulePriority getPriority() const override { return ModulePriority::Core; }
    std::vector<ModuleId> getDependencies() const override { return {}; }

    bool initialize(const IModuleConfig* config) override;
    void shutdown() override;
    bool isInitialized() const override { return initialized_; }

    void setModuleId(ModuleId id) { moduleId_ = id; }

private:
    ModuleId moduleId_ = INVALID_MODULE_ID;
    bool initialized_ = false;
};

} // namespace extra2d
