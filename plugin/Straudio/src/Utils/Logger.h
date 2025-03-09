#pragma once

#ifdef __APPLE__
#include <os/log.h>
#elif _WIN32
#include <windows.h>
#endif

#include <iostream>
#include <mutex>

class Logger {
public:
    static void info(const std::string& message) {
        log(message, Level::INFO);
    }

    static void warning(const std::string& message) {
        log(message, Level::WARNING);
    }

    static void error(const std::string& message) {
        log(message, Level::ERROR);
    }

private:
    enum class Level {
        INFO,
        WARNING,
        ERROR
    };

    inline static std::mutex mutex_;

    static void log(const std::string& message, Level level) {
        std::lock_guard<std::mutex> lock(mutex_);
        std::string levelStr = levelToString(level);
        std::string formattedMessage = "[" + levelStr + "] " + message;

#ifdef __APPLE__
        os_log_type_t osLogLevel = getOSLogLevel(level);
        os_log_with_type(OS_LOG_DEFAULT, osLogLevel, "%{public}s", formattedMessage.c_str());
#elif _WIN32
        OutputDebugStringA((formattedMessage + "\n").c_str());
#endif

        std::cout << formattedMessage << std::endl;
    }

    static std::string levelToString(Level level) {
        switch (level) {
            case Level::INFO: return "INFO";
            case Level::WARNING: return "WARNING";
            case Level::ERROR: return "ERROR";
            default: return "UNKNOWN";
        }
    }

#ifdef __APPLE__
    static os_log_type_t getOSLogLevel(Level level) {
        switch (level) {
            case Level::INFO: return OS_LOG_TYPE_DEFAULT;
            case Level::ERROR: return OS_LOG_TYPE_ERROR;
            default: return OS_LOG_TYPE_DEFAULT;
        }
    }
#endif
};
