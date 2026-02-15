#include <extra2d/graphics/shader_hot_reloader.h>
#include <extra2d/utils/logger.h>
#include <chrono>
#include <filesystem>

namespace extra2d {

namespace fs = std::filesystem;

/**
 * @brief 获取单例实例
 * @return 热重载管理器实例引用
 */
ShaderHotReloader& ShaderHotReloader::getInstance() {
    static ShaderHotReloader instance;
    return instance;
}

/**
 * @brief 初始化热重载系统
 * @return 初始化成功返回true，失败返回false
 */
bool ShaderHotReloader::init() {
    if (initialized_) {
        return true;
    }

#ifdef _WIN32
    buffer_.resize(4096);
#endif

    initialized_ = true;
    E2D_LOG_INFO("Shader hot reloader initialized");
    return true;
}

/**
 * @brief 关闭热重载系统
 */
void ShaderHotReloader::shutdown() {
    if (!initialized_) {
        return;
    }

#ifdef _WIN32
    if (watchHandle_ != nullptr) {
        FindCloseChangeNotification(watchHandle_);
        watchHandle_ = nullptr;
    }
#endif

    watchMap_.clear();
    initialized_ = false;
    enabled_ = false;
    E2D_LOG_INFO("Shader hot reloader shutdown");
}

/**
 * @brief 注册Shader文件监视
 * @param shaderName Shader名称
 * @param filePaths 要监视的文件列表
 * @param callback 文件变化时的回调
 */
void ShaderHotReloader::watch(const std::string& shaderName,
                              const std::vector<std::string>& filePaths,
                              FileChangeCallback callback) {
    if (!initialized_) {
        E2D_LOG_WARN("Hot reloader not initialized");
        return;
    }

    WatchInfo info;
    info.filePaths = filePaths;
    info.callback = callback;

    for (const auto& path : filePaths) {
        info.modifiedTimes[path] = getFileModifiedTime(path);
    }

    watchMap_[shaderName] = std::move(info);
    E2D_LOG_DEBUG("Watching shader: {} ({} files)", shaderName, filePaths.size());
}

/**
 * @brief 取消监视
 * @param shaderName Shader名称
 */
void ShaderHotReloader::unwatch(const std::string& shaderName) {
    auto it = watchMap_.find(shaderName);
    if (it != watchMap_.end()) {
        watchMap_.erase(it);
        E2D_LOG_DEBUG("Stopped watching shader: {}", shaderName);
    }
}

/**
 * @brief 更新文件监视（在主循环中调用）
 */
void ShaderHotReloader::update() {
    if (!initialized_ || !enabled_) {
        return;
    }

    pollChanges();
}

/**
 * @brief 启用/禁用热重载
 * @param enabled 是否启用
 */
void ShaderHotReloader::setEnabled(bool enabled) {
    enabled_ = enabled;
    E2D_LOG_DEBUG("Hot reload {}", enabled ? "enabled" : "disabled");
}

/**
 * @brief 轮询检查文件变化
 */
void ShaderHotReloader::pollChanges() {
    auto now = static_cast<uint64_t>(
        std::chrono::system_clock::now().time_since_epoch().count());

    for (auto& pair : watchMap_) {
        WatchInfo& info = pair.second;

        for (const auto& filePath : info.filePaths) {
            uint64_t currentModTime = getFileModifiedTime(filePath);
            uint64_t lastModTime = info.modifiedTimes[filePath];

            if (currentModTime != 0 && lastModTime != 0 && currentModTime != lastModTime) {
                info.modifiedTimes[filePath] = currentModTime;

                FileChangeEvent event;
                event.filepath = filePath;
                event.type = FileChangeEvent::Type::Modified;
                event.timestamp = now;

                E2D_LOG_DEBUG("Shader file changed: {}", filePath);

                if (info.callback) {
                    info.callback(event);
                }
            }
        }
    }
}

/**
 * @brief 获取文件修改时间
 * @param filepath 文件路径
 * @return 修改时间戳
 */
uint64_t ShaderHotReloader::getFileModifiedTime(const std::string& filepath) {
    try {
        auto ftime = fs::last_write_time(filepath);
        auto sctp = std::chrono::time_point_cast<std::chrono::seconds>(
            ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now());
        return static_cast<uint64_t>(sctp.time_since_epoch().count());
    } catch (...) {
        return 0;
    }
}

} // namespace extra2d
