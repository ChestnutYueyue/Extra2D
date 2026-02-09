#pragma once

#include <cstdarg>
#include <cstdio>
#include <extra2d/core/types.h>
#include <sstream>
#include <string>
#include <type_traits>

// SDL2 日志头文件
#include <SDL.h>

namespace extra2d {

// ============================================================================
// 日志级别枚举 - 映射到 SDL_LogPriority
// ============================================================================
enum class LogLevel {
  Trace = SDL_LOG_PRIORITY_VERBOSE,  // SDL 详细日志
  Debug = SDL_LOG_PRIORITY_DEBUG,    // SDL 调试日志
  Info = SDL_LOG_PRIORITY_INFO,      // SDL 信息日志
  Warn = SDL_LOG_PRIORITY_WARN,      // SDL 警告日志
  Error = SDL_LOG_PRIORITY_ERROR,    // SDL 错误日志
  Fatal = SDL_LOG_PRIORITY_CRITICAL, // SDL 严重日志
  Off = SDL_LOG_PRIORITY_CRITICAL + 1  // 关闭日志 (使用 Critical+1 作为关闭标记)
};

// ============================================================================
// 简单的 fmt-style {} 格式化器
// ============================================================================
namespace detail {

// 将单个参数转为字符串
template <typename T> inline std::string to_string_arg(const T &value) {
  if constexpr (std::is_same_v<T, std::string>) {
    return value;
  } else if constexpr (std::is_same_v<T, const char *> ||
                       std::is_same_v<T, char *>) {
    return value ? std::string(value) : std::string("(null)");
  } else if constexpr (std::is_same_v<T, bool>) {
    return value ? "true" : "false";
  } else if constexpr (std::is_arithmetic_v<T>) {
    // 对浮点数使用特殊格式
    if constexpr (std::is_floating_point_v<T>) {
      char buf[64];
      snprintf(buf, sizeof(buf), "%.2f", static_cast<double>(value));
      return buf;
    } else {
      return std::to_string(value);
    }
  } else {
    std::ostringstream oss;
    oss << value;
    return oss.str();
  }
}

// 格式化基础情况：没有更多参数
inline std::string format_impl(const char *fmt) {
  std::string result;
  while (*fmt) {
    if (*fmt == '{' && *(fmt + 1) == '}') {
      result += "{}"; // 无参数可替换，保留原样
      fmt += 2;
    } else {
      result += *fmt;
      ++fmt;
    }
  }
  return result;
}

// 格式化递归：替换第一个 {} 并递归处理剩余
template <typename T, typename... Args>
inline std::string format_impl(const char *fmt, const T &first,
                               const Args &...rest) {
  std::string result;
  while (*fmt) {
    if (*fmt == '{') {
      // 检查 {:#x} 等格式说明符
      if (*(fmt + 1) == '}') {
        result += to_string_arg(first);
        fmt += 2;
        result += format_impl(fmt, rest...);
        return result;
      } else if (*(fmt + 1) == ':') {
        // 跳过格式说明符直到 }
        const char *end = fmt + 2;
        while (*end && *end != '}')
          ++end;
        if (*end == '}') {
          // 检查是否是十六进制格式
          std::string spec(fmt + 2, end);
          if (spec.find('x') != std::string::npos ||
              spec.find('X') != std::string::npos) {
            if constexpr (std::is_integral_v<T>) {
              char buf[32];
              snprintf(buf, sizeof(buf), "0x%x",
                       static_cast<unsigned int>(first));
              result += buf;
            } else {
              result += to_string_arg(first);
            }
          } else if (spec.find('f') != std::string::npos ||
                     spec.find('.') != std::string::npos) {
            if constexpr (std::is_arithmetic_v<T>) {
              // 解析精度
              int precision = 2;
              auto dot = spec.find('.');
              if (dot != std::string::npos) {
                precision = 0;
                for (size_t i = dot + 1;
                     i < spec.size() && spec[i] >= '0' && spec[i] <= '9'; ++i) {
                  precision = precision * 10 + (spec[i] - '0');
                }
              }
              char fmtbuf[16];
              snprintf(fmtbuf, sizeof(fmtbuf), "%%.%df", precision);
              char buf[64];
              snprintf(buf, sizeof(buf), fmtbuf, static_cast<double>(first));
              result += buf;
            } else {
              result += to_string_arg(first);
            }
          } else {
            result += to_string_arg(first);
          }
          fmt = end + 1;
          result += format_impl(fmt, rest...);
          return result;
        }
      }
    }
    result += *fmt;
    ++fmt;
  }
  return result;
}

} // namespace detail

// 顶层格式化函数
template <typename... Args>
inline std::string e2d_format(const char *fmt, const Args &...args) {
  return detail::format_impl(fmt, args...);
}

// 无参数版本
inline std::string e2d_format(const char *fmt) { return std::string(fmt); }

// ============================================================================
// Logger 类 - 使用 SDL2 日志系统
// ============================================================================
class Logger {
public:
  /**
   * @brief 初始化日志系统
   */
  static void init();

  /**
   * @brief 关闭日志系统
   */
  static void shutdown();

  /**
   * @brief 设置日志级别
   * @param level 日志级别
   */
  static void setLevel(LogLevel level);

  /**
   * @brief 设置是否输出到控制台
   * @param enable 是否启用
   */
  static void setConsoleOutput(bool enable);

  /**
   * @brief 设置日志输出到文件
   * @param filename 日志文件名
   */
  static void setFileOutput(const std::string &filename);

  /**
   * @brief 获取当前日志级别
   * @return 当前日志级别
   */
  static LogLevel getLevel() { return level_; }

  /**
   * @brief 日志记录模板函数
   * @param level 日志级别
   * @param fmt 格式化字符串
   * @param args 可变参数
   */
  template <typename... Args>
  static void log(LogLevel level, const char *fmt, const Args &...args) {
    if (static_cast<int>(level) < static_cast<int>(level_))
      return;
    std::string msg = e2d_format(fmt, args...);
    SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION,
                   static_cast<SDL_LogPriority>(level), "[%s] %s",
                   getLevelString(level), msg.c_str());
  }

  /**
   * @brief 日志记录无参数版本
   * @param level 日志级别
   * @param msg 日志消息
   */
  static void log(LogLevel level, const char *msg) {
    if (static_cast<int>(level) < static_cast<int>(level_))
      return;
    SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION,
                   static_cast<SDL_LogPriority>(level), "[%s] %s",
                   getLevelString(level), msg);
  }

private:
  static LogLevel level_;      // 当前日志级别
  static bool initialized_;    // 是否已初始化
  static bool consoleOutput_;  // 是否输出到控制台
  static bool fileOutput_;     // 是否输出到文件
  static std::string logFile_; // 日志文件路径

  /**
   * @brief 获取日志级别字符串
   * @param level 日志级别
   * @return 级别字符串
   */
  static const char *getLevelString(LogLevel level);
};

// ============================================================================
// 日志宏
// ============================================================================

#ifdef E2D_DEBUG
#define E2D_LOG_TRACE(...)                                                     \
  ::extra2d::Logger::log(::extra2d::LogLevel::Trace, __VA_ARGS__)
#define E2D_LOG_DEBUG(...)                                                     \
  ::extra2d::Logger::log(::extra2d::LogLevel::Debug, __VA_ARGS__)
#else
#define E2D_LOG_TRACE(...)
#define E2D_LOG_DEBUG(...)
#endif

#define E2D_LOG_INFO(...)                                                      \
  ::extra2d::Logger::log(::extra2d::LogLevel::Info, __VA_ARGS__)
#define E2D_LOG_WARN(...)                                                      \
  ::extra2d::Logger::log(::extra2d::LogLevel::Warn, __VA_ARGS__)
#define E2D_LOG_ERROR(...)                                                     \
  ::extra2d::Logger::log(::extra2d::LogLevel::Error, __VA_ARGS__)
#define E2D_LOG_FATAL(...)                                                     \
  ::extra2d::Logger::log(::extra2d::LogLevel::Fatal, __VA_ARGS__)

// 简化的日志宏
#define E2D_INFO(...) E2D_LOG_INFO(__VA_ARGS__)
#define E2D_WARN(...) E2D_LOG_WARN(__VA_ARGS__)
#define E2D_ERROR(...) E2D_LOG_ERROR(__VA_ARGS__)
#define E2D_FATAL(...) E2D_LOG_FATAL(__VA_ARGS__)
#ifdef E2D_DEBUG
#define E2D_DEBUG_LOG(...) E2D_LOG_DEBUG(__VA_ARGS__)
#define E2D_TRACE(...) E2D_LOG_TRACE(__VA_ARGS__)
#else
#define E2D_DEBUG_LOG(...)
#define E2D_TRACE(...)
#endif

} // namespace extra2d
