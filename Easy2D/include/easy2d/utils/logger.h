#pragma once

#include <easy2d/core/types.h>
#include <cstdio>
#include <cstdarg>
#include <string>
#include <sstream>
#include <type_traits>

namespace easy2d {

// ============================================================================
// 日志级别枚举
// ============================================================================
enum class LogLevel {
    Trace = 0,
    Debug = 1,
    Info = 2,
    Warn = 3,
    Error = 4,
    Fatal = 5,
    Off = 6
};

// ============================================================================
// 简单的 fmt-style {} 格式化器
// ============================================================================
namespace detail {

// 将单个参数转为字符串
template<typename T>
inline std::string to_string_arg(const T& value) {
    if constexpr (std::is_same_v<T, std::string>) {
        return value;
    } else if constexpr (std::is_same_v<T, const char*> || std::is_same_v<T, char*>) {
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
inline std::string format_impl(const char* fmt) {
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
template<typename T, typename... Args>
inline std::string format_impl(const char* fmt, const T& first, const Args&... rest) {
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
                const char* end = fmt + 2;
                while (*end && *end != '}') ++end;
                if (*end == '}') {
                    // 检查是否是十六进制格式
                    std::string spec(fmt + 2, end);
                    if (spec.find('x') != std::string::npos || spec.find('X') != std::string::npos) {
                        if constexpr (std::is_integral_v<T>) {
                            char buf[32];
                            snprintf(buf, sizeof(buf), "0x%x", static_cast<unsigned int>(first));
                            result += buf;
                        } else {
                            result += to_string_arg(first);
                        }
                    } else if (spec.find('f') != std::string::npos || spec.find('.') != std::string::npos) {
                        if constexpr (std::is_arithmetic_v<T>) {
                            // 解析精度
                            int precision = 2;
                            auto dot = spec.find('.');
                            if (dot != std::string::npos) {
                                precision = 0;
                                for (size_t i = dot + 1; i < spec.size() && spec[i] >= '0' && spec[i] <= '9'; ++i) {
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
template<typename... Args>
inline std::string e2d_format(const char* fmt, const Args&... args) {
    return detail::format_impl(fmt, args...);
}

// 无参数版本
inline std::string e2d_format(const char* fmt) {
    return std::string(fmt);
}

// ============================================================================
// Logger 类 - 简单 printf 日志
// ============================================================================
class Logger {
public:
    static void init();
    static void shutdown();
    static void setLevel(LogLevel level);
    static void setConsoleOutput(bool enable);
    static void setFileOutput(const std::string& filename);

    static LogLevel getLevel() { return level_; }

    template<typename... Args>
    static void log(LogLevel level, const char* fmt, const Args&... args) {
        if (level < level_) return;
        std::string msg = e2d_format(fmt, args...);
        const char* levelStr = "";
        switch (level) {
            case LogLevel::Trace: levelStr = "TRACE"; break;
            case LogLevel::Debug: levelStr = "DEBUG"; break;
            case LogLevel::Info:  levelStr = "INFO "; break;
            case LogLevel::Warn:  levelStr = "WARN "; break;
            case LogLevel::Error: levelStr = "ERROR"; break;
            case LogLevel::Fatal: levelStr = "FATAL"; break;
            default: break;
        }
        printf("[%s] %s\n", levelStr, msg.c_str());
    }

    // 无参数版本
    static void log(LogLevel level, const char* msg) {
        if (level < level_) return;
        const char* levelStr = "";
        switch (level) {
            case LogLevel::Trace: levelStr = "TRACE"; break;
            case LogLevel::Debug: levelStr = "DEBUG"; break;
            case LogLevel::Info:  levelStr = "INFO "; break;
            case LogLevel::Warn:  levelStr = "WARN "; break;
            case LogLevel::Error: levelStr = "ERROR"; break;
            case LogLevel::Fatal: levelStr = "FATAL"; break;
            default: break;
        }
        printf("[%s] %s\n", levelStr, msg);
    }

private:
    static LogLevel level_;
    static bool initialized_;
};

// ============================================================================
// 日志宏
// ============================================================================

#ifdef E2D_DEBUG
    #define E2D_LOG_TRACE(...) ::easy2d::Logger::log(::easy2d::LogLevel::Trace, __VA_ARGS__)
    #define E2D_LOG_DEBUG(...) ::easy2d::Logger::log(::easy2d::LogLevel::Debug, __VA_ARGS__)
#else
    #define E2D_LOG_TRACE(...)
    #define E2D_LOG_DEBUG(...)
#endif

#define E2D_LOG_INFO(...)  ::easy2d::Logger::log(::easy2d::LogLevel::Info, __VA_ARGS__)
#define E2D_LOG_WARN(...)  ::easy2d::Logger::log(::easy2d::LogLevel::Warn, __VA_ARGS__)
#define E2D_LOG_ERROR(...) ::easy2d::Logger::log(::easy2d::LogLevel::Error, __VA_ARGS__)
#define E2D_LOG_FATAL(...) ::easy2d::Logger::log(::easy2d::LogLevel::Fatal, __VA_ARGS__)

// 简化的日志宏
#define E2D_INFO(...)  E2D_LOG_INFO(__VA_ARGS__)
#define E2D_WARN(...)  E2D_LOG_WARN(__VA_ARGS__)
#define E2D_ERROR(...) E2D_LOG_ERROR(__VA_ARGS__)
#define E2D_FATAL(...) E2D_LOG_FATAL(__VA_ARGS__)
#ifdef E2D_DEBUG
    #define E2D_DEBUG_LOG(...) E2D_LOG_DEBUG(__VA_ARGS__)
    #define E2D_TRACE(...)     E2D_LOG_TRACE(__VA_ARGS__)
#else
    #define E2D_DEBUG_LOG(...)
    #define E2D_TRACE(...)
#endif

} // namespace easy2d
