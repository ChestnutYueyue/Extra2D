#pragma once

#include <cstdint>
#include <functional>
#include <memory>
#include <string>

namespace extra2d {

// ---------------------------------------------------------------------------
// 宏定义
// ---------------------------------------------------------------------------
#define E2D_CONCAT_IMPL(a, b) a##b
#define E2D_CONCAT(a, b) E2D_CONCAT_IMPL(a, b)

// ---------------------------------------------------------------------------
// 智能指针别名
// ---------------------------------------------------------------------------
template <typename T> using Ptr = std::shared_ptr<T>;
template <typename T> using SharedPtr = std::shared_ptr<T>;

template <typename T> using UniquePtr = std::unique_ptr<T>;

template <typename T> using WeakPtr = std::weak_ptr<T>;

/// 创建 shared_ptr 的便捷函数
template <typename T, typename... Args> inline Ptr<T> makePtr(Args &&...args) {
  return std::make_shared<T>(std::forward<Args>(args)...);
}

template <typename T, typename... Args> inline SharedPtr<T> makeShared(Args &&...args) {
  return std::make_shared<T>(std::forward<Args>(args)...);
}

/// 创建 unique_ptr 的便捷函数
template <typename T, typename... Args>
inline UniquePtr<T> makeUnique(Args &&...args) {
  return std::make_unique<T>(std::forward<Args>(args)...);
}

// ---------------------------------------------------------------------------
// 函数别名
// ---------------------------------------------------------------------------
template <typename Sig> using Function = std::function<Sig>;

// ---------------------------------------------------------------------------
// 基础类型别名
// ---------------------------------------------------------------------------
using int8 = std::int8_t;
using int16 = std::int16_t;
using int32 = std::int32_t;
using int64 = std::int64_t;
using uint8 = std::uint8_t;
using uint16 = std::uint16_t;
using uint32 = std::uint32_t;
using uint64 = std::uint64_t;

} // namespace extra2d
