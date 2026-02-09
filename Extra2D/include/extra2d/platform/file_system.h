#pragma once

/**
 * @file file_system.h
 * @brief 跨平台文件系统工具
 *
 * 提供统一的文件路径处理和文件操作接口
 * 支持平台: Nintendo Switch, Windows, Linux, macOS
 */

#include <extra2d/platform/platform_compat.h>
#include <string>
#include <vector>

namespace extra2d {

// ============================================================================
// 文件系统工具类
// ============================================================================
class FileSystem {
public:
    /**
     * @brief 获取资源根目录
     * @return 资源根目录路径
     * 
     * Switch: "romfs:/"
     * PC: 可执行文件所在目录 + "/assets/" 或当前工作目录
     */
    static std::string getResourceRoot();

    /**
     * @brief 解析资源路径
     * @param relativePath 相对路径 (例如 "assets/font.ttf")
     * @return 完整路径
     * 
     * Switch: "romfs:/assets/font.ttf"
     * PC: "./assets/font.ttf" 或 exe目录/assets/font.ttf
     */
    static std::string resolvePath(const std::string& relativePath);

    /**
     * @brief 检查文件是否存在
     * @param path 文件路径
     * @return true 如果文件存在
     */
    static bool fileExists(const std::string& path);

    /**
     * @brief 检查目录是否存在
     * @param path 目录路径
     * @return true 如果目录存在
     */
    static bool directoryExists(const std::string& path);

    /**
     * @brief 获取可执行文件所在目录
     * @return 可执行文件目录路径
     */
    static std::string getExecutableDirectory();

    /**
     * @brief 获取当前工作目录
     * @return 当前工作目录路径
     */
    static std::string getCurrentWorkingDirectory();

    /**
     * @brief 组合路径
     * @param base 基础路径
     * @param relative 相对路径
     * @return 组合后的路径
     */
    static std::string combinePath(const std::string& base, const std::string& relative);

    /**
     * @brief 获取文件名（不含路径）
     * @param path 完整路径
     * @return 文件名
     */
    static std::string getFileName(const std::string& path);

    /**
     * @brief 获取文件扩展名
     * @param path 文件路径
     * @return 扩展名（包含点，例如 ".ttf"）
     */
    static std::string getFileExtension(const std::string& path);

    /**
     * @brief 获取目录名
     * @param path 文件路径
     * @return 目录路径
     */
    static std::string getDirectoryName(const std::string& path);

    /**
     * @brief 规范化路径（统一分隔符，去除冗余）
     * @param path 原始路径
     * @return 规范化后的路径
     */
    static std::string normalizePath(const std::string& path);

    /**
     * @brief 读取文件内容为字符串
     * @param path 文件路径
     * @return 文件内容，失败返回空字符串
     */
    static std::string readFileText(const std::string& path);

    /**
     * @brief 读取文件内容为字节数组
     * @param path 文件路径
     * @return 文件内容，失败返回空vector
     */
    static std::vector<uint8_t> readFileBytes(const std::string& path);

    /**
     * @brief 获取文件大小
     * @param path 文件路径
     * @return 文件大小（字节），失败返回 -1
     */
    static int64_t getFileSize(const std::string& path);

    /**
     * @brief 创建目录
     * @param path 目录路径
     * @return true 如果成功
     */
    static bool createDirectory(const std::string& path);

    /**
     * @brief 创建多级目录
     * @param path 目录路径
     * @return true 如果成功
     */
    static bool createDirectories(const std::string& path);

private:
    // 禁止实例化
    FileSystem() = delete;
    ~FileSystem() = delete;
};

// ============================================================================
// 便捷函数
// ============================================================================

/**
 * @brief 解析资源路径的便捷函数
 * @param path 相对路径
 * @return 完整路径
 */
inline std::string resolvePath(const std::string& path) {
    return FileSystem::resolvePath(path);
}

/**
 * @brief 检查文件是否存在的便捷函数
 * @param path 文件路径
 * @return true 如果文件存在
 */
inline bool fileExists(const std::string& path) {
    return FileSystem::fileExists(path);
}

} // namespace extra2d
