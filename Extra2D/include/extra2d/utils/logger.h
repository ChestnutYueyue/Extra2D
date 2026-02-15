#pragma once

#include <cstdio>
#include <extra2d/core/types.h>
#include <sstream>
#include <string>
#include <type_traits>

namespace extra2d {

enum class LogLevel {
    Trace = 0,
    Debug = 1,
    Info = 2,
    Warn = 3,
    Error = 4,
    Fatal = 5,
    Off = 6
};

namespace detail {

template <typename T> inline std::string to_string_arg(const T &value) {
    if constexpr (std::is_same_v<T, std::string>) {
        return value;
    } else if constexpr (std::is_same_v<T, const char *> ||
                         std::is_same_v<T, char *>) {
        return value ? std::string(value) : std::string("(null)");
    } else if constexpr (std::is_same_v<T, bool>) {
        return value ? "true" : "false";
    } else if constexpr (std::is_arithmetic_v<T>) {
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

inline std::string format_impl(const char *fmt) {
    std::string result;
    while (*fmt) {
        if (*fmt == '{' && *(fmt + 1) == '}') {
            result += "{}";
            fmt += 2;
        } else {
            result += *fmt;
            ++fmt;
        }
    }
    return result;
}

template <typename T, typename... Args>
inline std::string format_impl(const char *fmt, const T &first,
                               const Args &...rest) {
    std::string result;
    while (*fmt) {
        if (*fmt == '{') {
            if (*(fmt + 1) == '}') {
                result += to_string_arg(first);
                fmt += 2;
                result += format_impl(fmt, rest...);
                return result;
            } else if (*(fmt + 1) == ':') {
                const char *end = fmt + 2;
                while (*end && *end != '}')
                    ++end;
                if (*end == '}') {
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

template <typename... Args>
inline std::string e2d_format(const char *fmt, const Args &...args) {
    return detail::format_impl(fmt, args...);
}

inline std::string e2d_format(const char *fmt) { return std::string(fmt); }

class Logger {
public:
    static void init();
    static void shutdown();

    static void setLevel(LogLevel level);
    static void setConsoleOutput(bool enable);
    static void setFileOutput(const std::string &filename);

    static LogLevel getLevel() { return level_; }

    template <typename... Args>
    static void log(LogLevel level, const char *fmt, const Args &...args) {
        if (static_cast<int>(level) < static_cast<int>(level_))
            return;
        std::string msg = e2d_format(fmt, args...);
        outputLog(level, msg.c_str());
    }

    static void log(LogLevel level, const char *msg) {
        if (static_cast<int>(level) < static_cast<int>(level_))
            return;
        outputLog(level, msg);
    }

private:
    static void outputLog(LogLevel level, const char *msg);
    static const char *getLevelString(LogLevel level);
    static void writeToConsole(LogLevel level, const char *msg);
    static void writeToFile(LogLevel level, const char *msg);

    static LogLevel level_;
    static bool initialized_;
    static bool consoleOutput_;
    static bool fileOutput_;
    static std::string logFile_;
    static void *logFileHandle_;
};

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
