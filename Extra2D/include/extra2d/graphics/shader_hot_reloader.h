#pragma once

#include <extra2d/core/types.h>
#include <functional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#ifdef _WIN32
#include <windows.h>
#endif

namespace extra2d {

// ============================================================================
// 文件变化事件
// ============================================================================
struct FileChangeEvent {
    std::string filepath;
    
    enum class Type {
        Created,
        Modified,
        Deleted,
        Renamed
    } type;
    
    uint64_t timestamp = 0;
};

// ============================================================================
// 文件变化回调
// ============================================================================
using FileChangeCallback = std::function<void(const FileChangeEvent&)>;

// ============================================================================
// Shader热重载管理器
// ============================================================================
class ShaderHotReloader {
public:
    /**
     * @brief 获取单例实例
     * @return 热重载管理器实例引用
     */
    static ShaderHotReloader& getInstance();

    /**
     * @brief 初始化热重载系统
     * @return 初始化成功返回true，失败返回false
     */
    bool init();

    /**
     * @brief 关闭热重载系统
     */
    void shutdown();

    /**
     * @brief 注册Shader文件监视
     * @param shaderName Shader名称
     * @param filePaths 要监视的文件列表
     * @param callback 文件变化时的回调
     */
    void watch(const std::string& shaderName,
               const std::vector<std::string>& filePaths,
               FileChangeCallback callback);

    /**
     * @brief 取消监视
     * @param shaderName Shader名称
     */
    void unwatch(const std::string& shaderName);

    /**
     * @brief 更新文件监视（在主循环中调用）
     */
    void update();

    /**
     * @brief 启用/禁用热重载
     * @param enabled 是否启用
     */
    void setEnabled(bool enabled);

    /**
     * @brief 检查是否启用
     * @return 启用返回true，否则返回false
     */
    bool isEnabled() const { return enabled_; }

    /**
     * @brief 检查是否已初始化
     * @return 已初始化返回true，否则返回false
     */
    bool isInitialized() const { return initialized_; }

private:
    ShaderHotReloader() = default;
    ~ShaderHotReloader() = default;
    ShaderHotReloader(const ShaderHotReloader&) = delete;
    ShaderHotReloader& operator=(const ShaderHotReloader&) = delete;

    bool enabled_ = false;
    bool initialized_ = false;

    struct WatchInfo {
        std::vector<std::string> filePaths;
        FileChangeCallback callback;
        std::unordered_map<std::string, uint64_t> modifiedTimes;
    };
    std::unordered_map<std::string, WatchInfo> watchMap_;

#ifdef _WIN32
    HANDLE watchHandle_ = nullptr;
    std::vector<uint8_t> buffer_;
    std::string watchDir_;
    bool watching_ = false;
#endif

    /**
     * @brief 轮询检查文件变化
     */
    void pollChanges();

    /**
     * @brief 获取文件修改时间
     * @param filepath 文件路径
     * @return 修改时间戳
     */
    static uint64_t getFileModifiedTime(const std::string& filepath);
};

// 便捷宏
#define E2D_SHADER_HOT_RELOADER() ::extra2d::ShaderHotReloader::getInstance()

} // namespace extra2d
