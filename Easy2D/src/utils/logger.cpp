#include <easy2d/utils/logger.h>

namespace easy2d {

// 静态成员定义
LogLevel Logger::level_ = LogLevel::Trace;
bool Logger::initialized_ = false;

void Logger::init() {
    if (initialized_) {
        return;
    }

    initialized_ = true;
    log(LogLevel::Info, "Logger initialized");
}

void Logger::shutdown() {
    if (initialized_) {
        log(LogLevel::Info, "Logger shutting down");
    }
    initialized_ = false;
}

void Logger::setLevel(LogLevel level) {
    level_ = level;
}

void Logger::setConsoleOutput(bool /*enable*/) {
    // On Switch, console output always goes to nxlink stdout
    // Nothing to configure
}

void Logger::setFileOutput(const std::string& /*filename*/) {
    // File output not supported on Switch
    // Could potentially write to sdmc:/ in the future
}

} // namespace easy2d
