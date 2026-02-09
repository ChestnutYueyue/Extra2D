#pragma once

/**
 * @file platform_compat.h
 * @brief 跨平台兼容性头文件
 *
 * 提供所有支持平台的兼容性定义和宏
 * 支持平台: Nintendo Switch, Windows, Linux, macOS
 */

// ============================================================================
// 平台检测
// ============================================================================

#ifdef __SWITCH__
    // Nintendo Switch 平台
    #define PLATFORM_SWITCH 1
    #define PLATFORM_NAME "Nintendo Switch"

#elif defined(_WIN32)
    // Windows 平台
    #define PLATFORM_PC 1
    #define PLATFORM_WINDOWS 1
    #define PLATFORM_NAME "Windows"
    #ifndef NOMINMAX
    #define NOMINMAX
    #endif
    #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
    #endif
    // 禁用 SDL 的 main 重定义，使用标准 main 函数
    #define SDL_MAIN_HANDLED

#elif defined(__linux__)
    // Linux 平台
    #define PLATFORM_PC 1
    #define PLATFORM_LINUX 1
    #define PLATFORM_NAME "Linux"

#elif defined(__APPLE__) && defined(__MACH__)
    // macOS 平台
    #define PLATFORM_PC 1
    #define PLATFORM_MACOS 1
    #define PLATFORM_NAME "macOS"

#else
    #error "Unsupported platform"
#endif

// ============================================================================
// Nintendo Switch 平台包含
// ============================================================================

#ifdef PLATFORM_SWITCH

#include <switch.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <string>

// RomFS路径前缀
#define ROMFS_PREFIX "romfs:/"

// Switch 特定的编译器属性
#define E2D_LIKELY(x)   __builtin_expect(!!(x), 1)
#define E2D_UNLIKELY(x) __builtin_expect(!!(x), 0)

// Switch 调试输出
#ifdef E2D_DEBUG
    #define E2D_PLATFORM_LOG(fmt, ...) printf("[Extra2D] " fmt "\n", ##__VA_ARGS__)
#else
    #define E2D_PLATFORM_LOG(fmt, ...) ((void)0)
#endif

#endif // PLATFORM_SWITCH

// ============================================================================
// PC 平台包含 (Windows/Linux/macOS)
// ============================================================================

#ifdef PLATFORM_PC

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <string>
#include <cstdint>

// PC 平台使用标准文件路径
#define ROMFS_PREFIX ""

// PC 平台编译器属性
#if defined(__GNUC__) || defined(__clang__)
    #define E2D_LIKELY(x)   __builtin_expect(!!(x), 1)
    #define E2D_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
    #define E2D_LIKELY(x)   (x)
    #define E2D_UNLIKELY(x) (x)
#endif

// PC 调试输出
#ifdef E2D_DEBUG
    #ifdef PLATFORM_WINDOWS
        #include <windows.h>
        #define E2D_PLATFORM_LOG(fmt, ...) OutputDebugStringA((std::string("[Extra2D] ") + fmt + "\n").c_str())
    #else
        #define E2D_PLATFORM_LOG(fmt, ...) printf("[Extra2D] " fmt "\n", ##__VA_ARGS__)
    #endif
#else
    #define E2D_PLATFORM_LOG(fmt, ...) ((void)0)
#endif

#endif // PLATFORM_PC

// ============================================================================
// 跨平台通用定义
// ============================================================================

namespace extra2d {
namespace platform {

/**
 * @brief 获取当前平台名称
 * @return 平台名称字符串
 */
inline const char* getPlatformName() {
    return PLATFORM_NAME;
}

/**
 * @brief 检查当前是否为 Switch 平台
 * @return true 如果是 Switch 平台
 */
inline bool isSwitch() {
#ifdef PLATFORM_SWITCH
    return true;
#else
    return false;
#endif
}

/**
 * @brief 检查当前是否为 PC 平台
 * @return true 如果是 PC 平台 (Windows/Linux/macOS)
 */
inline bool isPC() {
#ifdef PLATFORM_PC
    return true;
#else
    return false;
#endif
}

/**
 * @brief 检查当前是否为 Windows 平台
 * @return true 如果是 Windows 平台
 */
inline bool isWindows() {
#ifdef PLATFORM_WINDOWS
    return true;
#else
    return false;
#endif
}

/**
 * @brief 检查当前是否为 Linux 平台
 * @return true 如果是 Linux 平台
 */
inline bool isLinux() {
#ifdef PLATFORM_LINUX
    return true;
#else
    return false;
#endif
}

/**
 * @brief 检查当前是否为 macOS 平台
 * @return true 如果是 macOS 平台
 */
inline bool isMacOS() {
#ifdef PLATFORM_MACOS
    return true;
#else
    return false;
#endif
}

} // namespace platform

// ============================================================================
// 文件系统路径工具
// ============================================================================

namespace romfs {

/**
 * @brief RomFS 根路径常量
 */
#ifdef PLATFORM_SWITCH
    static constexpr const char* ROOT = "romfs:/";
#else
    static constexpr const char* ROOT = "";
#endif

/**
 * @brief 检查文件是否存在
 * @param path 文件路径
 * @return true 如果文件存在
 */
inline bool fileExists(const char* path) {
    struct stat st;
    return stat(path, &st) == 0;
}

/**
 * @brief 检查路径是否为 romfs 路径 (Switch) 或普通路径 (PC)
 * @param path 文件路径
 * @return true 如果是 romfs 格式路径
 */
inline bool isRomfsPath(const char* path) {
    return path && (strncmp(path, "romfs:/", 7) == 0 || strncmp(path, "romfs:\\", 7) == 0);
}

/**
 * @brief 构建完整路径
 * @param relativePath 相对路径
 * @return 完整路径 (Switch 添加 romfs:/ 前缀，PC 保持原样)
 */
inline std::string makePath(const char* relativePath) {
#ifdef PLATFORM_SWITCH
    std::string result = ROOT;
    result += relativePath;
    return result;
#else
    return std::string(relativePath);
#endif
}

} // namespace romfs

} // namespace extra2d

// ============================================================================
// 向后兼容：保留 switch_compat.h 的宏定义
// ============================================================================

#ifdef PLATFORM_SWITCH
    #define IS_SWITCH_PLATFORM 1
    #define SWITCH_ROMFS_PREFIX "romfs:/"
    #define SWITCH_LIKELY(x) E2D_LIKELY(x)
    #define SWITCH_UNLIKELY(x) E2D_UNLIKELY(x)
    #ifdef E2D_DEBUG
        #define SWITCH_DEBUG_PRINTF(fmt, ...) E2D_PLATFORM_LOG(fmt, ##__VA_ARGS__)
    #else
        #define SWITCH_DEBUG_PRINTF(fmt, ...) ((void)0)
    #endif
#endif
