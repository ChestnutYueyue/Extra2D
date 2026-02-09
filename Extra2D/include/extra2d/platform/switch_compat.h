#pragma once

/**
 * @file switch_compat.h
 * @brief Nintendo Switch 兼容性头文件 (已弃用)
 *
 * @deprecated 请使用 platform_compat.h 替代
 * 此文件保留用于向后兼容
 */

// 包含新的跨平台兼容性头文件
#include "platform_compat.h"

// 发出弃用警告（仅在非 Switch 平台或调试模式下）
#if !defined(__SWITCH__) && defined(E2D_DEBUG)
    #warning "switch_compat.h is deprecated, use platform_compat.h instead"
#endif
