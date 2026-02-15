#pragma once

#include <extra2d/config/app_config.h>
#include <extra2d/config/platform_config.h>
#include <extra2d/core/types.h>
#include <string>

namespace extra2d {

// ============================================================================
// 平台检测器工具类
// ============================================================================
class PlatformDetector {
public:
    /**
     * @brief 检测当前运行平台
     * @return 当前平台的类型
     */
    static PlatformType detect();

    /**
     * @brief 获取平台名称字符串
     * @return 平台名称（如 "Windows", "Linux", "macOS", "Switch"）
     */
    static const char* platformName();

    /**
     * @brief 获取指定平台类型的名称
     * @param type 平台类型
     * @return 平台名称字符串
     */
    static const char* platformName(PlatformType type);

    /**
     * @brief 检查当前平台是否为桌面平台
     * @return 如果是桌面平台返回 true
     */
    static bool isDesktopPlatform();

    /**
     * @brief 检查当前平台是否为游戏主机平台
     * @return 如果是游戏主机平台返回 true
     */
    static bool isConsolePlatform();

    /**
     * @brief 检查当前平台是否为移动平台
     * @return 如果是移动平台返回 true
     */
    static bool isMobilePlatform();

    /**
     * @brief 获取当前平台的能力
     * @return 平台能力结构
     */
    static PlatformCapabilities capabilities();

    /**
     * @brief 获取指定平台的能力
     * @param type 平台类型
     * @return 平台能力结构
     */
    static PlatformCapabilities capabilities(PlatformType type);

    /**
     * @brief 获取当前平台的默认配置
     * @return 平台默认的应用配置
     */
    static AppConfig platformDefaults();

    /**
     * @brief 获取指定平台的默认配置
     * @param type 平台类型
     * @return 平台默认的应用配置
     */
    static AppConfig platformDefaults(PlatformType type);

    /**
     * @brief 获取当前平台的推荐分辨率
     * @param width 输出宽度
     * @param height 输出高度
     */
    static void getRecommendedResolution(int& width, int& height);

    /**
     * @brief 获取当前平台的默认 DPI
     * @return 默认 DPI 值
     */
    static float getDefaultDPI();

    /**
     * @brief 检查当前平台是否支持指定功能
     * @param feature 功能名称
     * @return 如果支持返回 true
     */
    static bool supportsFeature(const std::string& feature);

    /**
     * @brief 获取系统内存大小
     * @return 系统内存大小（MB），如果无法获取返回 0
     */
    static int getSystemMemoryMB();

    /**
     * @brief 获取 CPU 核心数
     * @return CPU 核心数
     */
    static int getCPUCoreCount();

    /**
     * @brief 检查是否支持多线程渲染
     * @return 如果支持返回 true
     */
    static bool supportsMultithreadedRendering();

    /**
     * @brief 获取平台特定的配置路径
     * @param appName 应用名称
     * @return 配置文件目录路径
     */
    static std::string getConfigPath(const std::string& appName);

    /**
     * @brief 获取平台特定的存档路径
     * @param appName 应用名称
     * @return 存档文件目录路径
     */
    static std::string getSavePath(const std::string& appName);

    /**
     * @brief 获取平台特定的缓存路径
     * @param appName 应用名称
     * @return 缓存文件目录路径
     */
    static std::string getCachePath(const std::string& appName);

    /**
     * @brief 获取平台特定的日志路径
     * @param appName 应用名称
     * @return 日志文件目录路径
     */
    static std::string getLogPath(const std::string& appName);

    /**
     * @brief 获取平台特定的资源路径（Shader、纹理等）
     * Switch平台使用romfs，其他平台使用相对路径
     * @param appName 应用名称
     * @return 资源目录路径
     */
    static std::string getResourcePath(const std::string& appName = "");

    /**
     * @brief 获取平台特定的Shader路径
     * @param appName 应用名称
     * @return Shader目录路径
     */
    static std::string getShaderPath(const std::string& appName = "");

    /**
     * @brief 获取平台特定的Shader缓存路径
     * Switch平台使用sdmc，其他平台使用系统缓存目录
     * @param appName 应用名称
     * @return Shader缓存目录路径
     */
    static std::string getShaderCachePath(const std::string& appName = "");

    /**
     * @brief 检查平台是否使用romfs（只读文件系统）
     * @return 使用romfs返回true
     */
    static bool usesRomfs();

    /**
     * @brief 检查平台是否支持热重载
     * Switch平台不支持热重载（romfs只读）
     * @return 支持热重载返回true
     */
    static bool supportsHotReload();

    /**
     * @brief 检查平台是否为小端字节序
     * @return 如果是小端字节序返回 true
     */
    static bool isLittleEndian();

    /**
     * @brief 检查平台是否为大端字节序
     * @return 如果是大端字节序返回 true
     */
    static bool isBigEndian();

    /**
     * @brief 获取平台信息摘要
     * @return 平台信息字符串
     */
    static std::string getPlatformSummary();

private:
    static PlatformCapabilities getWindowsCapabilities();
    static PlatformCapabilities getLinuxCapabilities();
    static PlatformCapabilities getMacOSCapabilities();
    static PlatformCapabilities getSwitchCapabilities();

    static AppConfig getWindowsDefaults();
    static AppConfig getLinuxDefaults();
    static AppConfig getMacOSDefaults();
    static AppConfig getSwitchDefaults();
};

} 
