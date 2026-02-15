#include <extra2d/utils/logger.h>
#include <cstdio>
#include <ctime>

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef __SWITCH__
#include <switch.h>
#endif

namespace extra2d {

LogLevel Logger::level_ = LogLevel::Info;
bool Logger::initialized_ = false;
bool Logger::consoleOutput_ = true;
bool Logger::fileOutput_ = false;
std::string Logger::logFile_;
void *Logger::logFileHandle_ = nullptr;

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

void Logger::writeToConsole(LogLevel level, const char *msg) {
    const char *levelStr = getLevelString(level);
    
#ifdef _WIN32
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    WORD color = 7;
    switch (level) {
    case LogLevel::Trace:    color = 8; break;
    case LogLevel::Debug:    color = 8; break;
    case LogLevel::Info:     color = 7; break;
    case LogLevel::Warn:     color = 14; break;
    case LogLevel::Error:    color = 12; break;
    case LogLevel::Fatal:    color = 12 | FOREGROUND_INTENSITY; break;
    default: break;
    }
    SetConsoleTextAttribute(hConsole, color);
    printf("[%s] %s\n", levelStr, msg);
    SetConsoleTextAttribute(hConsole, 7);
#else
    const char *colorCode = "\033[0m";
    switch (level) {
    case LogLevel::Trace:    colorCode = "\033[90m"; break;
    case LogLevel::Debug:    colorCode = "\033[90m"; break;
    case LogLevel::Info:     colorCode = "\033[0m"; break;
    case LogLevel::Warn:     colorCode = "\033[33m"; break;
    case LogLevel::Error:    colorCode = "\033[31m"; break;
    case LogLevel::Fatal:    colorCode = "\033[1;31m"; break;
    default: break;
    }
    printf("%s[%s] %s\033[0m\n", colorCode, levelStr, msg);
#endif
}

void Logger::writeToFile(LogLevel level, const char *msg) {
    if (!logFileHandle_) return;
    
    FILE *fp = static_cast<FILE *>(logFileHandle_);
    
    time_t now = time(nullptr);
    struct tm *tm_info = localtime(&now);
    char timeBuf[32];
    strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", tm_info);
    
    fprintf(fp, "[%s] [%s] %s\n", timeBuf, getLevelString(level), msg);
    fflush(fp);
}

void Logger::outputLog(LogLevel level, const char *msg) {
    if (consoleOutput_) {
        writeToConsole(level, msg);
    }
    if (fileOutput_) {
        writeToFile(level, msg);
    }
}

void Logger::init() {
    if (initialized_) {
        return;
    }

#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD mode = 0;
        if (GetConsoleMode(hOut, &mode)) {
            mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
            SetConsoleMode(hOut, mode);
        }
    }
#endif

#ifdef __SWITCH__
    consoleInit(NULL);
#endif

    initialized_ = true;
    log(LogLevel::Info, "Logger initialized");
}

void Logger::shutdown() {
    if (initialized_) {
        log(LogLevel::Info, "Logger shutting down");
    }
    
    if (logFileHandle_) {
        fclose(static_cast<FILE *>(logFileHandle_));
        logFileHandle_ = nullptr;
    }
    
#ifdef __SWITCH__
    consoleExit(NULL);
#endif
    
    initialized_ = false;
    fileOutput_ = false;
}

void Logger::setLevel(LogLevel level) {
    level_ = level;
}

void Logger::setConsoleOutput(bool enable) {
    consoleOutput_ = enable;
}

void Logger::setFileOutput(const std::string &filename) {
    if (logFileHandle_) {
        fclose(static_cast<FILE *>(logFileHandle_));
        logFileHandle_ = nullptr;
    }
    
    logFile_ = filename;
    fileOutput_ = !filename.empty();
    
    if (fileOutput_) {
#ifdef _WIN32
        FILE *fp = nullptr;
        fopen_s(&fp, filename.c_str(), "a");
#else
        FILE *fp = fopen(filename.c_str(), "a");
#endif
        logFileHandle_ = fp;
        
        if (fp) {
            time_t now = time(nullptr);
            struct tm *tm_info = localtime(&now);
            char timeBuf[32];
            strftime(timeBuf, sizeof(timeBuf), "%Y-%m-%d %H:%M:%S", tm_info);
            fprintf(fp, "\n=== Log session started at %s ===\n", timeBuf);
            fflush(fp);
        }
    }
}

} // namespace extra2d
