#include <extra2d/platform/platform_init_module.h>
#include <extra2d/config/module_registry.h>
#include <extra2d/config/config_manager.h>
#include <extra2d/utils/logger.h>
#include <nlohmann/json.hpp>

#ifdef __SWITCH__
#include <switch.h>
#endif

using json = nlohmann::json;

namespace extra2d {

static ModuleId s_platformModuleId = INVALID_MODULE_ID;

ModuleId get_platform_module_id() {
    return s_platformModuleId;
}

bool PlatformModuleConfig::loadFromJson(const void* jsonData) {
    if (!jsonData) return false;
    
    try {
        const json& j = *static_cast<const json*>(jsonData);
        
        if (j.contains("targetPlatform")) {
            int platform = j["targetPlatform"].get<int>();
            if (platform >= 0 && platform <= 5) {
                targetPlatform = static_cast<PlatformType>(platform);
            }
        }
        
        return true;
    } catch (...) {
        return false;
    }
}

bool PlatformModuleConfig::saveToJson(void* jsonData) const {
    if (!jsonData) return false;
    
    try {
        json& j = *static_cast<json*>(jsonData);
        j["targetPlatform"] = static_cast<int>(targetPlatform);
        return true;
    } catch (...) {
        return false;
    }
}

PlatformModuleInitializer::PlatformModuleInitializer()
    : moduleId_(INVALID_MODULE_ID)
    , initialized_(false)
    , targetPlatform_(PlatformType::Auto)
    , resolvedPlatform_(PlatformType::Windows) {
}

PlatformModuleInitializer::~PlatformModuleInitializer() {
    if (initialized_) {
        shutdown();
    }
}

bool PlatformModuleInitializer::initialize(const IModuleConfig* config) {
    if (initialized_) return true;
    
    const PlatformModuleConfig* platformConfig = dynamic_cast<const PlatformModuleConfig*>(config);
    if (platformConfig) {
        targetPlatform_ = platformConfig->targetPlatform;
    }
    
    resolvedPlatform_ = targetPlatform_;
    if (resolvedPlatform_ == PlatformType::Auto) {
#ifdef __SWITCH__
        resolvedPlatform_ = PlatformType::Switch;
#else
#ifdef _WIN32
        resolvedPlatform_ = PlatformType::Windows;
#elif defined(__linux__)
        resolvedPlatform_ = PlatformType::Linux;
#elif defined(__APPLE__)
        resolvedPlatform_ = PlatformType::macOS;
#else
        resolvedPlatform_ = PlatformType::Windows;
#endif
#endif
    }
    
    platformConfig_ = createPlatformConfig(resolvedPlatform_);
    if (!platformConfig_) {
        E2D_LOG_ERROR("Failed to create platform config");
        return false;
    }
    
    auto& appConfig = ConfigManager::instance().appConfig();
    appConfig.applyPlatformConstraints(*platformConfig_);
    
    if (resolvedPlatform_ == PlatformType::Switch) {
        if (!initSwitch()) {
            return false;
        }
    }
    
    initialized_ = true;
    E2D_LOG_INFO("Platform module initialized ({})", getPlatformTypeName(resolvedPlatform_));
    return true;
}

bool PlatformModuleInitializer::initSwitch() {
#ifdef __SWITCH__
    Result rc;
    rc = romfsInit();
    if (R_SUCCEEDED(rc)) {
        E2D_LOG_INFO("RomFS initialized successfully");
    } else {
        E2D_LOG_WARN("romfsInit failed: {:#08X}, will use regular filesystem", rc);
    }

    rc = socketInitializeDefault();
    if (R_FAILED(rc)) {
        E2D_LOG_WARN("socketInitializeDefault failed, nxlink will not be available");
    }
#endif
    return true;
}

void PlatformModuleInitializer::shutdown() {
    if (!initialized_) return;
    
    E2D_LOG_INFO("Platform module shutting down");
    
    if (resolvedPlatform_ == PlatformType::Switch) {
        shutdownSwitch();
    }
    
    platformConfig_.reset();
    initialized_ = false;
}

void PlatformModuleInitializer::shutdownSwitch() {
#ifdef __SWITCH__
    romfsExit();
    socketExit();
#endif
}

void register_platform_module() {
    if (s_platformModuleId != INVALID_MODULE_ID) return;
    
    s_platformModuleId = ModuleRegistry::instance().registerModule(
        makeUnique<PlatformModuleConfig>(),
        []() -> UniquePtr<IModuleInitializer> {
            auto initializer = makeUnique<PlatformModuleInitializer>();
            initializer->setModuleId(s_platformModuleId);
            return initializer;
        }
    );
}

namespace {
    struct PlatformModuleAutoRegister {
        PlatformModuleAutoRegister() {
            register_platform_module();
        }
    };
    
    static PlatformModuleAutoRegister s_autoRegister;
}

} // namespace extra2d
