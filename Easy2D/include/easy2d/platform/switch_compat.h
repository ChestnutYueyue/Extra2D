#pragma once

/**
 * @file switch_compat.h
 * @brief Nintendo Switch 兼容性头文件
 *
 * 提供 Switch 平台特定的兼容性定义和宏
 */

#ifdef __SWITCH__

// ============================================================================
// Switch 平台包含
// ============================================================================
#include <switch.h>
#include <malloc.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <string>

// ============================================================================
// Switch 特定的内存操作
// ============================================================================
// 不需要特殊处理，libnx已提供malloc/free

// ============================================================================
// Switch 文件系统相关
// ============================================================================
// RomFS路径前缀
#define SWITCH_ROMFS_PREFIX "romfs:/"

// RomFS 根路径常量
namespace easy2d {
namespace romfs {
    static constexpr const char* ROMFS_ROOT = "romfs:/";

    // 检查文件是否存在于 romfs 中
    inline bool fileExists(const char* path) {
        struct stat st;
        return stat(path, &st) == 0;
    }

    // 检查路径是否为 romfs 路径
    inline bool isRomfsPath(const char* path) {
        return path && (strncmp(path, "romfs:/", 7) == 0 || strncmp(path, "romfs:\\", 7) == 0);
    }

    // 构建 romfs 完整路径
    inline std::string makePath(const char* relativePath) {
        std::string result = ROMFS_ROOT;
        result += relativePath;
        return result;
    }
} // namespace romfs
} // namespace easy2d

// ============================================================================
// Switch 调试输出
// ============================================================================
#ifdef E2D_DEBUG
    #define SWITCH_DEBUG_PRINTF(fmt, ...) printf("[Easy2D] " fmt "\n", ##__VA_ARGS__)
#else
    #define SWITCH_DEBUG_PRINTF(fmt, ...) ((void)0)
#endif

// ============================================================================
// Switch 特定的编译器属性
// ============================================================================
#define SWITCH_LIKELY(x)   __builtin_expect(!!(x), 1)
#define SWITCH_UNLIKELY(x) __builtin_expect(!!(x), 0)

// ============================================================================
// Switch 特定的平台检查宏
// ============================================================================
#define IS_SWITCH_PLATFORM 1

#endif // __SWITCH__
