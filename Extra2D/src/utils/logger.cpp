#include <extra2d/utils/logger.h>

namespace extra2d {

// 静态成员定义
LogLevel Logger::level_ = LogLevel::Info;
bool Logger::initialized_ = false;
bool Logger::consoleOutput_ = true;
bool Logger::fileOutput_ = false;
std::string Logger::logFile_;

/**
 * @brief 获取日志级别字符串
 * @param level 日志级别
 * @return 级别字符串
 */
const char *Logger::getLevelString(LogLevel level) {
  switch (level) {
  case LogLevel::Trace:
    return "TRACE";
  case LogLevel::Debug:
    return "DEBUG";
  case LogLevel::Info:
    return "INFO ";
  case LogLevel::Warn:
    return "WARN ";
  case LogLevel::Error:
    return "ERROR";
  case LogLevel::Fatal:
    return "FATAL";
  default:
    return "UNKNOWN";
  }
}

/**
 * @brief 初始化日志系统
 */
void Logger::init() {
  if (initialized_) {
    return;
  }

  // 设置 SDL 日志级别为详细模式（允许所有级别的日志）
  SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_VERBOSE);

  initialized_ = true;
  log(LogLevel::Info, "Logger initialized with SDL2");
}

/**
 * @brief 关闭日志系统
 */
void Logger::shutdown() {
  if (initialized_) {
    log(LogLevel::Info, "Logger shutting down");
  }
  initialized_ = false;
}

/**
 * @brief 设置日志级别
 * @param level 日志级别
 */
void Logger::setLevel(LogLevel level) {
  level_ = level;
  // 同时设置 SDL 的日志级别
  if (level != LogLevel::Off) {
    SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION,
                       static_cast<SDL_LogPriority>(level));
  }
}

/**
 * @brief 设置是否输出到控制台
 * @param enable 是否启用
 */
void Logger::setConsoleOutput(bool enable) {
  consoleOutput_ = enable;
  // SDL2 日志默认输出到控制台，通过设置日志优先级控制
  if (!enable) {
    SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_CRITICAL);
  } else {
    SDL_LogSetPriority(SDL_LOG_CATEGORY_APPLICATION,
                       static_cast<SDL_LogPriority>(level_));
  }
}

/**
 * @brief 设置日志输出到文件
 * @param filename 日志文件名
 */
void Logger::setFileOutput(const std::string &filename) {
  logFile_ = filename;
  fileOutput_ = !filename.empty();

  if (fileOutput_) {
    // SDL2 使用 SDL_LogSetOutputFunction 可以重定向日志输出
    // 这里我们记录文件路径，实际文件输出可以通过自定义回调实现
    log(LogLevel::Info, "File output configured: {}", filename);
  }
}

} // namespace extra2d
