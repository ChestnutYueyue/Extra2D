#pragma once

#include <extra2d/config/platform_detector.h>
#include <extra2d/graphics/shader_cache.h>
#include <extra2d/graphics/shader_hot_reloader.h>
#include <extra2d/graphics/shader_interface.h>
#include <extra2d/graphics/shader_loader.h>
#include <functional>
#include <unordered_map>

namespace extra2d {

// ============================================================================
// Shader重载回调
// ============================================================================
using ShaderReloadCallback = std::function<void(Ptr<IShader> newShader)>;

// ============================================================================
// Shader管理器 - 统一入口
// ============================================================================
class ShaderManager {
public:
    /**
     * @brief 获取单例实例
     * @return Shader管理器实例引用
     */
    static ShaderManager& getInstance();

    // ------------------------------------------------------------------------
    // 初始化和关闭
    // ------------------------------------------------------------------------

    /**
     * @brief 使用平台默认路径初始化Shader系统
     * 自动检测平台并使用正确的路径（romfs/sdmc/相对路径）
     * @param factory 渲染后端Shader工厂
     * @param appName 应用名称（用于缓存目录）
     * @return 初始化成功返回true，失败返回false
     */
    bool init(Ptr<IShaderFactory> factory, const std::string& appName = "extra2d");

    /**
     * @brief 初始化Shader系统
     * @param shaderDir Shader文件目录
     * @param cacheDir 缓存目录
     * @param factory 渲染后端Shader工厂
     * @return 初始化成功返回true，失败返回false
     */
    bool init(const std::string& shaderDir,
              const std::string& cacheDir,
              Ptr<IShaderFactory> factory);

    /**
     * @brief 关闭Shader系统
     */
    void shutdown();

    /**
     * @brief 检查是否已初始化
     * @return 已初始化返回true，否则返回false
     */
    bool isInitialized() const { return initialized_; }

    /**
     * @brief 检查当前平台是否支持热重载
     * Switch平台使用romfs，不支持热重载
     * @return 支持热重载返回true
     */
    bool isHotReloadSupported() const { return hotReloadSupported_; }

    // ------------------------------------------------------------------------
    // Shader加载
    // ------------------------------------------------------------------------

    /**
     * @brief 从分离文件加载Shader
     * @param name Shader名称
     * @param vertPath 顶点着色器文件路径
     * @param fragPath 片段着色器文件路径
     * @return 加载的Shader实例
     */
    Ptr<IShader> loadFromFiles(const std::string& name,
                               const std::string& vertPath,
                               const std::string& fragPath);

    /**
     * @brief 从组合文件加载Shader
     * @param path 组合Shader文件路径
     * @return 加载的Shader实例
     */
    Ptr<IShader> loadFromCombinedFile(const std::string& path);

    /**
     * @brief 从源码加载Shader
     * @param name Shader名称
     * @param vertSource 顶点着色器源码
     * @param fragSource 片段着色器源码
     * @return 加载的Shader实例
     */
    Ptr<IShader> loadFromSource(const std::string& name,
                                const std::string& vertSource,
                                const std::string& fragSource);

    /**
     * @brief 获取已加载的Shader
     * @param name Shader名称
     * @return Shader实例，不存在返回nullptr
     */
    Ptr<IShader> get(const std::string& name) const;

    /**
     * @brief 检查Shader是否存在
     * @param name Shader名称
     * @return 存在返回true，否则返回false
     */
    bool has(const std::string& name) const;

    /**
     * @brief 移除Shader
     * @param name Shader名称
     */
    void remove(const std::string& name);

    /**
     * @brief 清除所有Shader
     */
    void clear();

    // ------------------------------------------------------------------------
    // 热重载
    // ------------------------------------------------------------------------

    /**
     * @brief 注册重载回调
     * @param name Shader名称
     * @param callback 重载回调函数
     */
    void setReloadCallback(const std::string& name, ShaderReloadCallback callback);

    /**
     * @brief 启用/禁用热重载
     * @param enabled 是否启用
     */
    void setHotReloadEnabled(bool enabled);

    /**
     * @brief 检查热重载是否启用
     * @return 启用返回true，否则返回false
     */
    bool isHotReloadEnabled() const;

    /**
     * @brief 更新热重载系统（主循环调用）
     */
    void update();

    /**
     * @brief 手动重载Shader
     * @param name Shader名称
     * @return 重载成功返回true，失败返回false
     */
    bool reload(const std::string& name);

    // ------------------------------------------------------------------------
    // 内置Shader
    // ------------------------------------------------------------------------

    /**
     * @brief 获取内置Shader
     * @param name 内置Shader名称
     * @return Shader实例
     */
    Ptr<IShader> getBuiltin(const std::string& name);

    /**
     * @brief 加载所有内置Shader
     * @return 加载成功返回true，失败返回false
     */
    bool loadBuiltinShaders();

    // ------------------------------------------------------------------------
    // 工具方法
    // ------------------------------------------------------------------------

    /**
     * @brief 获取Shader目录
     * @return Shader目录路径
     */
    const std::string& getShaderDir() const { return shaderDir_; }

    /**
     * @brief 获取ShaderLoader
     * @return ShaderLoader引用
     */
    ShaderLoader& getLoader() { return loader_; }

private:
    ShaderManager() = default;
    ~ShaderManager() = default;
    ShaderManager(const ShaderManager&) = delete;
    ShaderManager& operator=(const ShaderManager&) = delete;

    std::string shaderDir_;
    std::string cacheDir_;
    Ptr<IShaderFactory> factory_;
    ShaderLoader loader_;

    struct ShaderInfo {
        Ptr<IShader> shader;
        ShaderMetadata metadata;
        ShaderReloadCallback reloadCallback;
        std::string vertSource;
        std::string fragSource;
        std::vector<std::string> filePaths;
    };
    std::unordered_map<std::string, ShaderInfo> shaders_;

    bool initialized_ = false;
    bool hotReloadEnabled_ = false;
    bool hotReloadSupported_ = true;

    /**
     * @brief 从缓存加载Shader
     * @param name Shader名称
     * @param sourceHash 源码哈希值
     * @param vertSource 顶点着色器源码
     * @param fragSource 片段着色器源码
     * @return Shader实例
     */
    Ptr<IShader> loadFromCache(const std::string& name,
                               const std::string& sourceHash,
                               const std::string& vertSource,
                               const std::string& fragSource);

    /**
     * @brief 创建内置Shader源码
     */
    void createBuiltinShaderSources();

    /**
     * @brief 处理文件变化事件
     * @param shaderName Shader名称
     * @param event 文件变化事件
     */
    void handleFileChange(const std::string& shaderName, const FileChangeEvent& event);
};

// 便捷宏
#define E2D_SHADER_MANAGER() ::extra2d::ShaderManager::getInstance()

} // namespace extra2d
