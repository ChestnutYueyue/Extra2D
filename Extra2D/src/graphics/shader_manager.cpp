#include <extra2d/graphics/shader_manager.h>
#include <extra2d/utils/logger.h>

namespace extra2d {

/**
 * @brief 获取单例实例
 * @return Shader管理器实例引用
 */
ShaderManager& ShaderManager::getInstance() {
    static ShaderManager instance;
    return instance;
}

/**
 * @brief 使用平台默认路径初始化Shader系统
 * 自动检测平台并使用正确的路径（romfs/sdmc/相对路径）
 * @param factory 渲染后端Shader工厂
 * @param appName 应用名称（用于缓存目录）
 * @return 初始化成功返回true，失败返回false
 */
bool ShaderManager::init(Ptr<IShaderFactory> factory, const std::string& appName) {
    std::string shaderDir = PlatformDetector::getShaderPath(appName);
    std::string cacheDir = PlatformDetector::getShaderCachePath(appName);
    
    hotReloadSupported_ = PlatformDetector::supportsHotReload();
    
    E2D_LOG_INFO("Platform: {} (HotReload: {})", 
                 PlatformDetector::platformName(),
                 hotReloadSupported_ ? "supported" : "not supported");
    
    return init(shaderDir, cacheDir, factory);
}

/**
 * @brief 初始化Shader系统
 * @param shaderDir Shader文件目录
 * @param cacheDir 缓存目录
 * @param factory 渲染后端Shader工厂
 * @return 初始化成功返回true，失败返回false
 */
bool ShaderManager::init(const std::string& shaderDir,
                         const std::string& cacheDir,
                         Ptr<IShaderFactory> factory) {
    if (initialized_) {
        E2D_LOG_WARN("ShaderManager already initialized");
        return true;
    }

    if (!factory) {
        E2D_LOG_ERROR("Shader factory is null");
        return false;
    }

    shaderDir_ = shaderDir;
    cacheDir_ = cacheDir;
    factory_ = factory;

    hotReloadSupported_ = PlatformDetector::supportsHotReload();

#ifdef __SWITCH__
    if (!ShaderCache::getInstance().init(cacheDir_)) {
        E2D_LOG_WARN("Failed to initialize shader cache on Switch");
    }
#else
    if (!ShaderCache::getInstance().init(cacheDir_)) {
        E2D_LOG_WARN("Failed to initialize shader cache, caching disabled");
    }
#endif

    if (hotReloadSupported_) {
        if (!ShaderHotReloader::getInstance().init()) {
            E2D_LOG_WARN("Failed to initialize hot reloader");
        }
    }

    loader_.addIncludePath(shaderDir_ + "common");

    initialized_ = true;
    E2D_LOG_INFO("ShaderManager initialized");
    E2D_LOG_INFO("  Shader directory: {}", shaderDir_);
    E2D_LOG_INFO("  Cache directory: {}", cacheDir_);
    E2D_LOG_INFO("  Hot reload: {}", hotReloadSupported_ ? "supported" : "not supported");

    return true;
}

/**
 * @brief 关闭Shader系统
 */
void ShaderManager::shutdown() {
    if (!initialized_) {
        return;
    }

    if (hotReloadSupported_) {
        ShaderHotReloader::getInstance().shutdown();
    }
    ShaderCache::getInstance().shutdown();

    shaders_.clear();
    factory_.reset();
    initialized_ = false;

    E2D_LOG_INFO("ShaderManager shutdown");
}

/**
 * @brief 从分离文件加载Shader
 * @param name Shader名称
 * @param vertPath 顶点着色器文件路径
 * @param fragPath 片段着色器文件路径
 * @return 加载的Shader实例
 */
Ptr<IShader> ShaderManager::loadFromFiles(const std::string& name,
                                          const std::string& vertPath,
                                          const std::string& fragPath) {
    if (!initialized_) {
        E2D_LOG_ERROR("ShaderManager not initialized");
        return nullptr;
    }

    auto it = shaders_.find(name);
    if (it != shaders_.end()) {
        return it->second.shader;
    }

    ShaderLoadResult result = loader_.loadFromSeparateFiles(name, vertPath, fragPath);
    if (!result.success) {
        E2D_LOG_ERROR("Failed to load shader files: {} - {}", vertPath, fragPath);
        return nullptr;
    }

    std::string sourceHash = ShaderCache::computeHash(result.vertSource, result.fragSource);
    Ptr<IShader> shader = loadFromCache(name, sourceHash, result.vertSource, result.fragSource);

    if (!shader) {
        shader = factory_->createFromSource(name, result.vertSource, result.fragSource);
        if (!shader) {
            E2D_LOG_ERROR("Failed to create shader from source: {}", name);
            return nullptr;
        }

        std::vector<uint8_t> binary;
        if (factory_->getShaderBinary(*shader, binary)) {
            ShaderCacheEntry entry;
            entry.name = name;
            entry.sourceHash = sourceHash;
            entry.binary = binary;
            entry.dependencies = result.dependencies;
            ShaderCache::getInstance().saveCache(entry);
        }
    }

    ShaderInfo info;
    info.shader = shader;
    info.vertSource = result.vertSource;
    info.fragSource = result.fragSource;
    info.filePaths = {vertPath, fragPath};
    info.filePaths.insert(info.filePaths.end(), result.dependencies.begin(), result.dependencies.end());

    info.metadata.name = name;
    info.metadata.vertPath = vertPath;
    info.metadata.fragPath = fragPath;

    shaders_[name] = std::move(info);

    if (hotReloadEnabled_ && hotReloadSupported_) {
        auto callback = [this, name](const FileChangeEvent& event) {
            this->handleFileChange(name, event);
        };
        ShaderHotReloader::getInstance().watch(name, shaders_[name].filePaths, callback);
    }

    E2D_LOG_DEBUG("Shader loaded: {}", name);
    return shader;
}

/**
 * @brief 从组合文件加载Shader
 * @param path 组合Shader文件路径
 * @return 加载的Shader实例
 */
Ptr<IShader> ShaderManager::loadFromCombinedFile(const std::string& path) {
    if (!initialized_) {
        E2D_LOG_ERROR("ShaderManager not initialized");
        return nullptr;
    }

    ShaderMetadata metadata = loader_.getMetadata(path);
    std::string name = metadata.name.empty() ? path : metadata.name;

    auto it = shaders_.find(name);
    if (it != shaders_.end()) {
        return it->second.shader;
    }

    ShaderLoadResult result = loader_.loadFromCombinedFile(path);
    if (!result.success) {
        E2D_LOG_ERROR("Failed to load combined shader file: {}", path);
        return nullptr;
    }

    std::string sourceHash = ShaderCache::computeHash(result.vertSource, result.fragSource);
    Ptr<IShader> shader = loadFromCache(name, sourceHash, result.vertSource, result.fragSource);

    if (!shader) {
        shader = factory_->createFromSource(name, result.vertSource, result.fragSource);
        if (!shader) {
            E2D_LOG_ERROR("Failed to create shader from source: {}", name);
            return nullptr;
        }

        std::vector<uint8_t> binary;
        if (factory_->getShaderBinary(*shader, binary)) {
            ShaderCacheEntry entry;
            entry.name = name;
            entry.sourceHash = sourceHash;
            entry.binary = binary;
            entry.dependencies = result.dependencies;
            ShaderCache::getInstance().saveCache(entry);
        }
    }

    ShaderInfo info;
    info.shader = shader;
    info.vertSource = result.vertSource;
    info.fragSource = result.fragSource;
    info.filePaths = {path};
    info.filePaths.insert(info.filePaths.end(), result.dependencies.begin(), result.dependencies.end());
    info.metadata = metadata;

    shaders_[name] = std::move(info);

    if (hotReloadEnabled_ && hotReloadSupported_) {
        auto callback = [this, name](const FileChangeEvent& event) {
            this->handleFileChange(name, event);
        };
        ShaderHotReloader::getInstance().watch(name, shaders_[name].filePaths, callback);
    }

    E2D_LOG_DEBUG("Shader loaded from combined file: {}", name);
    return shader;
}

/**
 * @brief 从源码加载Shader
 * @param name Shader名称
 * @param vertSource 顶点着色器源码
 * @param fragSource 片段着色器源码
 * @return 加载的Shader实例
 */
Ptr<IShader> ShaderManager::loadFromSource(const std::string& name,
                                           const std::string& vertSource,
                                           const std::string& fragSource) {
    if (!initialized_) {
        E2D_LOG_ERROR("ShaderManager not initialized");
        return nullptr;
    }

    auto it = shaders_.find(name);
    if (it != shaders_.end()) {
        return it->second.shader;
    }

    Ptr<IShader> shader = factory_->createFromSource(name, vertSource, fragSource);
    if (!shader) {
        E2D_LOG_ERROR("Failed to create shader from source: {}", name);
        return nullptr;
    }

    ShaderInfo info;
    info.shader = shader;
    info.vertSource = vertSource;
    info.fragSource = fragSource;
    info.metadata.name = name;

    shaders_[name] = std::move(info);

    E2D_LOG_DEBUG("Shader loaded from source: {}", name);
    return shader;
}

/**
 * @brief 获取已加载的Shader
 * @param name Shader名称
 * @return Shader实例，不存在返回nullptr
 */
Ptr<IShader> ShaderManager::get(const std::string& name) const {
    auto it = shaders_.find(name);
    if (it != shaders_.end()) {
        return it->second.shader;
    }
    return nullptr;
}

/**
 * @brief 检查Shader是否存在
 * @param name Shader名称
 * @return 存在返回true，否则返回false
 */
bool ShaderManager::has(const std::string& name) const {
    return shaders_.find(name) != shaders_.end();
}

/**
 * @brief 移除Shader
 * @param name Shader名称
 */
void ShaderManager::remove(const std::string& name) {
    auto it = shaders_.find(name);
    if (it != shaders_.end()) {
        ShaderHotReloader::getInstance().unwatch(name);
        shaders_.erase(it);
        E2D_LOG_DEBUG("Shader removed: {}", name);
    }
}

/**
 * @brief 清除所有Shader
 */
void ShaderManager::clear() {
    if (hotReloadSupported_) {
        for (const auto& pair : shaders_) {
            ShaderHotReloader::getInstance().unwatch(pair.first);
        }
    }
    shaders_.clear();
    E2D_LOG_DEBUG("All shaders cleared");
}

/**
 * @brief 注册重载回调
 * @param name Shader名称
 * @param callback 重载回调函数
 */
void ShaderManager::setReloadCallback(const std::string& name, ShaderReloadCallback callback) {
    auto it = shaders_.find(name);
    if (it != shaders_.end()) {
        it->second.reloadCallback = callback;
    }
}

/**
 * @brief 启用/禁用热重载
 * @param enabled 是否启用
 */
void ShaderManager::setHotReloadEnabled(bool enabled) {
    if (!hotReloadSupported_) {
        E2D_LOG_WARN("Hot reload not supported on this platform");
        return;
    }
    hotReloadEnabled_ = enabled;
    ShaderHotReloader::getInstance().setEnabled(enabled);
    E2D_LOG_INFO("Hot reload {}", enabled ? "enabled" : "disabled");
}

/**
 * @brief 检查热重载是否启用
 * @return 启用返回true，否则返回false
 */
bool ShaderManager::isHotReloadEnabled() const {
    return hotReloadEnabled_ && hotReloadSupported_;
}

/**
 * @brief 更新热重载系统（主循环调用）
 */
void ShaderManager::update() {
    if (hotReloadEnabled_ && hotReloadSupported_) {
        ShaderHotReloader::getInstance().update();
    }
}

/**
 * @brief 手动重载Shader
 * @param name Shader名称
 * @return 重载成功返回true，失败返回false
 */
bool ShaderManager::reload(const std::string& name) {
    auto it = shaders_.find(name);
    if (it == shaders_.end()) {
        E2D_LOG_WARN("Shader not found for reload: {}", name);
        return false;
    }

    ShaderInfo& info = it->second;

    std::string vertSource = info.vertSource;
    std::string fragSource = info.fragSource;

    if (!info.metadata.vertPath.empty() && !info.metadata.fragPath.empty()) {
        ShaderLoadResult result = loader_.loadFromSeparateFiles(name, info.metadata.vertPath, info.metadata.fragPath);
        if (result.success) {
            vertSource = result.vertSource;
            fragSource = result.fragSource;
        }
    } else if (!info.metadata.combinedPath.empty()) {
        ShaderLoadResult result = loader_.loadFromCombinedFile(info.metadata.combinedPath);
        if (result.success) {
            vertSource = result.vertSource;
            fragSource = result.fragSource;
        }
    }

    Ptr<IShader> newShader = factory_->createFromSource(name, vertSource, fragSource);
    if (!newShader) {
        E2D_LOG_ERROR("Failed to reload shader: {}", name);
        return false;
    }

    info.shader = newShader;
    info.vertSource = vertSource;
    info.fragSource = fragSource;

    if (info.reloadCallback) {
        info.reloadCallback(newShader);
    }

    E2D_LOG_INFO("Shader reloaded: {}", name);
    return true;
}

/**
 * @brief 获取内置Shader
 * @param name 内置Shader名称
 * @return Shader实例
 */
Ptr<IShader> ShaderManager::getBuiltin(const std::string& name) {
    Ptr<IShader> shader = get(name);
    if (shader) {
        return shader;
    }

    std::string path = shaderDir_ + "builtin/" + name + ".shader";
    return loadFromCombinedFile(path);
}

/**
 * @brief 加载所有内置Shader
 * @return 加载成功返回true，失败返回false
 */
bool ShaderManager::loadBuiltinShaders() {
    if (!initialized_) {
        E2D_LOG_ERROR("ShaderManager not initialized");
        return false;
    }

    bool allSuccess = true;

    const char* builtinNames[] = {
        "sprite",
        "particle",
        "shape",
        "postprocess",
        "font"
    };

    for (const char* name : builtinNames) {
        std::string path = shaderDir_ + "builtin/" + name + ".shader";
        std::string shaderName = std::string("builtin_") + name;

        if (!loadFromCombinedFile(path)) {
            E2D_LOG_ERROR("Failed to load builtin {} shader from: {}", name, path);
            allSuccess = false;
        } else {
            auto it = shaders_.find(name);
            if (it != shaders_.end()) {
                shaders_[shaderName] = it->second;
                shaders_.erase(it);
            }
        }
    }

    if (allSuccess) {
        E2D_LOG_INFO("All builtin shaders loaded");
    }

    return allSuccess;
}

/**
 * @brief 从缓存加载Shader
 * @param name Shader名称
 * @param sourceHash 源码哈希值
 * @param vertSource 顶点着色器源码
 * @param fragSource 片段着色器源码
 * @return Shader实例
 */
Ptr<IShader> ShaderManager::loadFromCache(const std::string& name,
                                          const std::string& sourceHash,
                                          const std::string& vertSource,
                                          const std::string& fragSource) {
    if (!ShaderCache::getInstance().isInitialized()) {
        return nullptr;
    }

    if (!ShaderCache::getInstance().hasValidCache(name, sourceHash)) {
        return nullptr;
    }

    Ptr<ShaderCacheEntry> entry = ShaderCache::getInstance().loadCache(name);
    if (!entry || entry->binary.empty()) {
        return nullptr;
    }

    Ptr<IShader> shader = factory_->createFromBinary(name, entry->binary);
    if (shader) {
        E2D_LOG_DEBUG("Shader loaded from cache: {}", name);
    }

    return shader;
}

/**
 * @brief 处理文件变化事件
 * @param shaderName Shader名称
 * @param event 文件变化事件
 */
void ShaderManager::handleFileChange(const std::string& shaderName, const FileChangeEvent& event) {
    E2D_LOG_DEBUG("Shader file changed: {} -> {}", shaderName, event.filepath);
    reload(shaderName);
}

} // namespace extra2d
