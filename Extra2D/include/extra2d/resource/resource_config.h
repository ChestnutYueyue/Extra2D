#pragma once

#include <string>
#include <vector>

namespace extra2d {

/**
 * @file resource_config.h
 * @brief 资源模块配置
 * 
 * 定义资源相关的配置数据结构，由 ResourceModule 管理。
 */

/**
 * @brief 资源配置数据结构
 */
struct ResourceConfigData {
    std::string assetRootPath = "assets";
    std::string cachePath = "cache";
    std::string savePath = "saves";
    std::string configPath = "config";
    std::string logPath = "logs";
    bool useAssetCache = true;
    int maxCacheSize = 512;  
    bool hotReloadEnabled = false;
    float hotReloadInterval = 1.0f;  
    bool compressTextures = false;
    bool preloadCommonAssets = true;
    std::vector<std::string> searchPaths;

    /**
     * @brief 添加资源搜索路径
     * @param path 要添加的搜索路径
     */
    void addSearchPath(const std::string& path);

    /**
     * @brief 移除资源搜索路径
     * @param path 要移除的搜索路径
     */
    void removeSearchPath(const std::string& path);

    /**
     * @brief 检查是否存在指定的搜索路径
     * @param path 要检查的路径
     * @return 如果存在返回 true
     */
    bool hasSearchPath(const std::string& path) const;
};

} 
