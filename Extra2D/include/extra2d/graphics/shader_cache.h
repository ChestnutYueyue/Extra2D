#pragma once

#include <extra2d/core/types.h>
#include <string>
#include <unordered_map>
#include <vector>

namespace extra2d {

// ============================================================================
// Shader缓存条目
// ============================================================================
struct ShaderCacheEntry {
    std::string name;
    std::string sourceHash;
    uint64_t compileTime = 0;
    std::vector<uint8_t> binary;
    std::vector<std::string> dependencies;
};

// ============================================================================
// Shader缓存管理器
// ============================================================================
class ShaderCache {
public:
    /**
     * @brief 获取单例实例
     * @return 缓存管理器实例引用
     */
    static ShaderCache& getInstance();

    /**
     * @brief 初始化缓存系统
     * @param cacheDir 缓存目录路径
     * @return 初始化成功返回true，失败返回false
     */
    bool init(const std::string& cacheDir);

    /**
     * @brief 关闭缓存系统
     */
    void shutdown();

    /**
     * @brief 检查缓存是否有效
     * @param name Shader名称
     * @param sourceHash 源码哈希值
     * @return 缓存有效返回true，否则返回false
     */
    bool hasValidCache(const std::string& name, const std::string& sourceHash);

    /**
     * @brief 加载缓存的二进制数据
     * @param name Shader名称
     * @return 缓存条目指针，不存在返回nullptr
     */
    Ptr<ShaderCacheEntry> loadCache(const std::string& name);

    /**
     * @brief 保存编译结果到缓存
     * @param entry 缓存条目
     * @return 保存成功返回true，失败返回false
     */
    bool saveCache(const ShaderCacheEntry& entry);

    /**
     * @brief 使缓存失效
     * @param name Shader名称
     */
    void invalidate(const std::string& name);

    /**
     * @brief 清除所有缓存
     */
    void clearAll();

    /**
     * @brief 计算源码哈希值
     * @param vertSource 顶点着色器源码
     * @param fragSource 片段着色器源码
     * @return 哈希值字符串
     */
    static std::string computeHash(const std::string& vertSource,
                                   const std::string& fragSource);

    /**
     * @brief 检查是否已初始化
     * @return 已初始化返回true，否则返回false
     */
    bool isInitialized() const { return initialized_; }

private:
    ShaderCache() = default;
    ~ShaderCache() = default;
    ShaderCache(const ShaderCache&) = delete;
    ShaderCache& operator=(const ShaderCache&) = delete;

    std::string cacheDir_;
    std::unordered_map<std::string, ShaderCacheEntry> cacheMap_;
    bool initialized_ = false;

    /**
     * @brief 加载缓存索引
     * @return 加载成功返回true，失败返回false
     */
    bool loadCacheIndex();

    /**
     * @brief 保存缓存索引
     * @return 保存成功返回true，失败返回false
     */
    bool saveCacheIndex();

    /**
     * @brief 获取缓存文件路径
     * @param name Shader名称
     * @return 缓存文件完整路径
     */
    std::string getCachePath(const std::string& name) const;

    /**
     * @brief 确保缓存目录存在
     * @return 目录存在或创建成功返回true，否则返回false
     */
    bool ensureCacheDirectory();
};

// 便捷宏
#define E2D_SHADER_CACHE() ::extra2d::ShaderCache::getInstance()

} // namespace extra2d
