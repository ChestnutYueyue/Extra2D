#include "hello_module.h"
#include <extra2d/config/module_registry.h>
#include <extra2d/utils/logger.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace extra2d {

static ModuleId s_helloModuleId = INVALID_MODULE_ID;

/**
 * @brief 获取Hello模块标识符
 */
ModuleId get_hello_module_id() {
    return s_helloModuleId;
}

/**
 * @brief 从JSON加载配置
 */
bool HelloModuleConfig::loadFromJson(const void* jsonData) {
    if (!jsonData) return false;
    
    try {
        const json& j = *static_cast<const json*>(jsonData);
        
        if (j.contains("greeting")) {
            config.greeting = j["greeting"].get<std::string>();
        }
        if (j.contains("repeatCount")) {
            config.repeatCount = j["repeatCount"].get<int>();
        }
        if (j.contains("enableLogging")) {
            config.enableLogging = j["enableLogging"].get<bool>();
        }
        
        return true;
    } catch (...) {
        return false;
    }
}

/**
 * @brief 保存配置到JSON
 */
bool HelloModuleConfig::saveToJson(void* jsonData) const {
    if (!jsonData) return false;
    
    try {
        json& j = *static_cast<json*>(jsonData);
        j["greeting"] = config.greeting;
        j["repeatCount"] = config.repeatCount;
        j["enableLogging"] = config.enableLogging;
        return true;
    } catch (...) {
        return false;
    }
}

/**
 * @brief 构造函数
 */
HelloModuleInitializer::HelloModuleInitializer()
    : moduleId_(INVALID_MODULE_ID)
    , initialized_(false) {
}

/**
 * @brief 析构函数
 */
HelloModuleInitializer::~HelloModuleInitializer() {
    if (initialized_) {
        shutdown();
    }
}

/**
 * @brief 获取模块依赖列表
 */
std::vector<ModuleId> HelloModuleInitializer::getDependencies() const {
    return {};
}

/**
 * @brief 初始化模块
 */
bool HelloModuleInitializer::initialize(const IModuleConfig* config) {
    if (initialized_) {
        E2D_LOG_WARN("HelloModule already initialized");
        return true;
    }
    
    if (!config) {
        E2D_LOG_ERROR("HelloModule config is null");
        return false;
    }
    
    const HelloModuleConfig* helloConfig = dynamic_cast<const HelloModuleConfig*>(config);
    if (!helloConfig) {
        E2D_LOG_ERROR("Invalid HelloModule config type");
        return false;
    }
    
    if (!helloConfig->validate()) {
        E2D_LOG_ERROR("HelloModule config validation failed");
        return false;
    }
    
    config_ = helloConfig->config;
    
    initialized_ = true;
    
    E2D_LOG_INFO("HelloModule initialized");
    E2D_LOG_INFO("  Greeting: {}", config_.greeting);
    E2D_LOG_INFO("  Repeat Count: {}", config_.repeatCount);
    E2D_LOG_INFO("  Logging Enabled: {}", config_.enableLogging);
    
    sayHello();
    
    return true;
}

/**
 * @brief 关闭模块
 */
void HelloModuleInitializer::shutdown() {
    if (!initialized_) return;
    
    if (config_.enableLogging) {
        E2D_LOG_INFO("HelloModule shutdown - Goodbye!");
    }
    
    initialized_ = false;
}

/**
 * @brief 执行问候操作
 */
void HelloModuleInitializer::sayHello() const {
    if (!config_.enableLogging) return;
    
    for (int i = 0; i < config_.repeatCount; ++i) {
        E2D_LOG_INFO("[HelloModule] {}", config_.greeting);
    }
}

/**
 * @brief 注册Hello模块
 */
void register_hello_module() {
    if (s_helloModuleId != INVALID_MODULE_ID) {
        E2D_LOG_WARN("HelloModule already registered");
        return;
    }
    
    s_helloModuleId = ModuleRegistry::instance().registerModule(
        makeUnique<HelloModuleConfig>(),
        []() -> UniquePtr<IModuleInitializer> {
            auto initializer = makeUnique<HelloModuleInitializer>();
            initializer->setModuleId(s_helloModuleId);
            return initializer;
        }
    );
    
    E2D_LOG_DEBUG("HelloModule registered with id: {}", s_helloModuleId);
}

namespace {
    /**
     * @brief 自动注册器
     * 在程序启动时自动注册模块
     */
    struct HelloModuleAutoRegister {
        HelloModuleAutoRegister() {
            register_hello_module();
        }
    };
    
    static HelloModuleAutoRegister s_autoRegister;
}

} // namespace extra2d
