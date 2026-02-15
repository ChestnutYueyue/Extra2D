#include <extra2d/config/module_registry.h>
#include <algorithm>
#include <cstdio>

namespace extra2d {

/**
 * @brief 获取单例实例
 * 使用静态局部变量实现线程安全的单例模式
 * @return 模块注册表实例引用
 */
ModuleRegistry& ModuleRegistry::instance() {
    static ModuleRegistry instance;
    return instance;
}

/**
 * @brief 注册模块
 * 将模块配置和初始化器工厂添加到注册表
 * @param config 模块配置
 * @param initializerFactory 初始化器工厂函数（可选）
 * @return 分配的模块标识符
 */
ModuleId ModuleRegistry::registerModule(
    UniquePtr<IModuleConfig> config,
    ModuleInitializerFactory initializerFactory
) {
    if (!config) {
        std::fprintf(stderr, "[ERROR] Cannot register null module config\n");
        return INVALID_MODULE_ID;
    }

    std::lock_guard<std::mutex> lock(mutex_);

    ModuleInfo info = config->getModuleInfo();
    
    if (nameToId_.find(info.name) != nameToId_.end()) {
        std::fprintf(stderr, "[ERROR] Module '%s' already registered\n", info.name.c_str());
        return INVALID_MODULE_ID;
    }

    ModuleId id = generateId();
    
    ModuleEntry entry;
    entry.id = id;
    entry.config = std::move(config);
    entry.initializerFactory = std::move(initializerFactory);
    entry.initialized = false;

    modules_[id] = std::move(entry);
    nameToId_[info.name] = id;

    return id;
}

/**
 * @brief 注销模块
 * 从注册表中移除指定模块
 * @param id 模块标识符
 * @return 注销成功返回 true
 */
bool ModuleRegistry::unregisterModule(ModuleId id) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = modules_.find(id);
    if (it == modules_.end()) {
        return false;
    }

    ModuleInfo info = it->second.config->getModuleInfo();
    nameToId_.erase(info.name);
    modules_.erase(it);

    return true;
}

/**
 * @brief 获取模块配置
 * @param id 模块标识符
 * @return 模块配置指针，不存在返回 nullptr
 */
IModuleConfig* ModuleRegistry::getModuleConfig(ModuleId id) const {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = modules_.find(id);
    if (it != modules_.end()) {
        return it->second.config.get();
    }
    return nullptr;
}

/**
 * @brief 根据名称获取模块配置
 * @param name 模块名称
 * @return 模块配置指针，不存在返回 nullptr
 */
IModuleConfig* ModuleRegistry::getModuleConfigByName(const std::string& name) const {
    std::lock_guard<std::mutex> lock(mutex_);

    auto nameIt = nameToId_.find(name);
    if (nameIt == nameToId_.end()) {
        return nullptr;
    }

    auto moduleIt = modules_.find(nameIt->second);
    if (moduleIt != modules_.end()) {
        return moduleIt->second.config.get();
    }
    return nullptr;
}

/**
 * @brief 获取或创建模块初始化器
 * @param id 模块标识符
 * @return 初始化器指针，不存在返回 nullptr
 */
IModuleInitializer* ModuleRegistry::getInitializer(ModuleId id) {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = modules_.find(id);
    if (it == modules_.end() || !it->second.initializerFactory) {
        return nullptr;
    }

    if (!it->second.initializer) {
        it->second.initializer = it->second.initializerFactory();
    }

    return it->second.initializer.get();
}

/**
 * @brief 获取所有已注册模块标识符
 * @return 模块标识符列表
 */
std::vector<ModuleId> ModuleRegistry::getAllModules() const {
    std::lock_guard<std::mutex> lock(mutex_);

    std::vector<ModuleId> result;
    result.reserve(modules_.size());

    for (const auto& pair : modules_) {
        result.push_back(pair.first);
    }

    return result;
}

/**
 * @brief 获取模块初始化顺序
 * 根据优先级和依赖关系计算初始化顺序
 * @return 按初始化顺序排列的模块标识符列表
 */
std::vector<ModuleId> ModuleRegistry::getInitializationOrder() const {
    std::lock_guard<std::mutex> lock(mutex_);

    std::vector<std::pair<ModuleId, int>> modulePriorities;
    modulePriorities.reserve(modules_.size());

    for (const auto& pair : modules_) {
        ModuleInfo info = pair.second.config->getModuleInfo();
        if (info.enabled) {
            modulePriorities.emplace_back(pair.first, static_cast<int>(info.priority));
        }
    }

    std::sort(modulePriorities.begin(), modulePriorities.end(),
        [](const auto& a, const auto& b) {
            return a.second < b.second;
        });

    std::vector<ModuleId> result;
    result.reserve(modulePriorities.size());

    for (const auto& pair : modulePriorities) {
        result.push_back(pair.first);
    }

    return result;
}

/**
 * @brief 检查模块是否存在
 * @param id 模块标识符
 * @return 存在返回 true
 */
bool ModuleRegistry::hasModule(ModuleId id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return modules_.find(id) != modules_.end();
}

/**
 * @brief 清空所有注册的模块
 */
void ModuleRegistry::clear() {
    std::lock_guard<std::mutex> lock(mutex_);

    modules_.clear();
    nameToId_.clear();
    nextId_ = 1;
}

/**
 * @brief 生成新的模块标识符
 * @return 新的模块标识符
 */
ModuleId ModuleRegistry::generateId() {
    return nextId_++;
}

}
