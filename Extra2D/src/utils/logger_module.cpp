#include <extra2d/utils/logger_module.h>
#include <extra2d/config/module_registry.h>
#include <extra2d/utils/logger.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace extra2d {

bool LoggerModuleConfig::loadFromJson(const void* jsonData) {
    if (!jsonData) return false;
    
    try {
        const json& j = *static_cast<const json*>(jsonData);
        
        if (j.contains("logLevel")) {
            int level = j["logLevel"].get<int>();
            if (level >= 0 && level <= 6) {
                logLevel = static_cast<LogLevel>(level);
            }
        }
        
        if (j.contains("consoleOutput")) {
            consoleOutput = j["consoleOutput"].get<bool>();
        }
        
        if (j.contains("fileOutput")) {
            fileOutput = j["fileOutput"].get<bool>();
        }
        
        if (j.contains("logFilePath")) {
            logFilePath = j["logFilePath"].get<std::string>();
        }
        
        return true;
    } catch (...) {
        return false;
    }
}

bool LoggerModuleConfig::saveToJson(void* jsonData) const {
    if (!jsonData) return false;
    
    try {
        json& j = *static_cast<json*>(jsonData);
        j["logLevel"] = static_cast<int>(logLevel);
        j["consoleOutput"] = consoleOutput;
        j["fileOutput"] = fileOutput;
        j["logFilePath"] = logFilePath;
        return true;
    } catch (...) {
        return false;
    }
}

LoggerModuleInitializer::LoggerModuleInitializer()
    : moduleId_(INVALID_MODULE_ID)
    , initialized_(false) {
}

LoggerModuleInitializer::~LoggerModuleInitializer() {
    if (initialized_) {
        shutdown();
    }
}

bool LoggerModuleInitializer::initialize(const IModuleConfig* config) {
    if (initialized_) return true;
    
    const LoggerModuleConfig* loggerConfig = dynamic_cast<const LoggerModuleConfig*>(config);
    
    Logger::init();
    
    if (loggerConfig) {
        Logger::setLevel(loggerConfig->logLevel);
        Logger::setConsoleOutput(loggerConfig->consoleOutput);
        if (loggerConfig->fileOutput && !loggerConfig->logFilePath.empty()) {
            Logger::setFileOutput(loggerConfig->logFilePath);
        }
    }
    
    initialized_ = true;
    E2D_LOG_INFO("Logger module initialized");
    return true;
}

void LoggerModuleInitializer::shutdown() {
    if (!initialized_) return;
    
    E2D_LOG_INFO("Logger module shutting down");
    Logger::shutdown();
    initialized_ = false;
}

namespace {
    static ModuleId s_loggerModuleId = INVALID_MODULE_ID;
    
    struct LoggerModuleRegistrar {
        LoggerModuleRegistrar() {
            s_loggerModuleId = ModuleRegistry::instance().registerModule(
                makeUnique<LoggerModuleConfig>(),
                []() -> UniquePtr<IModuleInitializer> {
                    auto initializer = makeUnique<LoggerModuleInitializer>();
                    initializer->setModuleId(s_loggerModuleId);
                    return initializer;
                }
            );
        }
    };
    
    static LoggerModuleRegistrar s_registrar;
}

} // namespace extra2d
