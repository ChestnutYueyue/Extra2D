#pragma once

#include <string>
#include <vector>

namespace extra2d {

/**
 * @file debug_config.h
 * @brief 调试模块配置
 * 
 * 定义调试相关的配置数据结构，由 DebugModule 管理。
 */

/**
 * @brief 调试配置数据结构
 */
struct DebugConfigData {
    bool enabled = false;
    bool showFPS = false;
    bool showMemoryUsage = false;
    bool showRenderStats = false;
    bool showColliders = false;
    bool showGrid = false;
    bool logToFile = false;
    bool logToConsole = true;
    int logLevel = 2;  
    bool breakOnAssert = true;
    bool enableProfiling = false;
    std::string logFilePath;
    std::vector<std::string> debugFlags;

    /**
     * @brief 检查是否存在指定的调试标志
     * @param flag 要检查的标志名称
     * @return 如果存在返回 true
     */
    bool hasDebugFlag(const std::string& flag) const;

    /**
     * @brief 添加调试标志
     * @param flag 要添加的标志名称
     */
    void addDebugFlag(const std::string& flag);

    /**
     * @brief 移除调试标志
     * @param flag 要移除的标志名称
     */
    void removeDebugFlag(const std::string& flag);
};

} 
