#pragma once

#ifdef __APPLE__
#include <os/log.h>
#elif _WIN32
#include <windows.h>
#endif

#include <iostream>
#include <mutex>
#include <sstream>
#include <string>
#include <codecvt>
#include <locale>

#undef ERROR

class Logger {
public:
    enum class Level { VERBOSE, INFO, WARNING, ERROR };

    template<typename... Args>
    static void verbose(Args&&... args) {
        log(buildMessage(std::forward<Args>(args)...), Level::VERBOSE);
    }
    template<typename... Args>
    static void info(Args&&... args) {
        log(buildMessage(std::forward<Args>(args)...), Level::INFO);
    }
    template<typename... Args>
    static void warning(Args&&... args) {
        log(buildMessage(std::forward<Args>(args)...), Level::WARNING);
    }
    template<typename... Args>
    static void error(Args&&... args) {
        log(buildMessage(std::forward<Args>(args)...), Level::ERROR);
    }

    static void setMinLevel(Level lvl) { minLevel_ = lvl; }

private:
    inline static std::mutex mutex_;
    inline static std::atomic<Level> minLevel_{Level::INFO};

    static void log(const std::string& message, Level level) {
        if (level < minLevel_) return;
        std::lock_guard<std::mutex> lock(mutex_);
        std::string formatted = "[" + levelToString(level) + "] " + message;
#ifdef __APPLE__
        os_log_with_type(OS_LOG_DEFAULT, getOSLogLevel(level), "%{public}s", formatted.c_str());
#elif _WIN32
        OutputDebugStringA((formatted + "\n").c_str());
#else
        std::cout << formatted << std::endl;
#endif
    }

    static std::string levelToString(Level level) {
        switch (level) {
        case Level::VERBOSE: return "VERBOSE";
        case Level::INFO:    return "INFO";
        case Level::WARNING: return "WARNING";
        case Level::ERROR:   return "ERROR";
        default:             return "UNKNOWN";
        }
    }

#ifdef __APPLE__
    static os_log_type_t getOSLogLevel(Level level) {
        return level == Level::ERROR ? OS_LOG_TYPE_ERROR : OS_LOG_TYPE_DEFAULT;
    }
#endif

    template<typename... Args>
    static std::string buildMessage(Args&&... args) {
        std::ostringstream oss;
        (appendArg(oss, std::forward<Args>(args)), ...);
        return oss.str();
    }

    template<typename T>
    static void appendArg(std::ostringstream& oss, T&& arg) {
        oss << toString(std::forward<T>(arg));
    }

    template<typename T>
    static std::string toString(const T& value) {
        std::ostringstream oss;
        oss << value;
        return oss.str();
    }
    static std::string toString(const std::wstring& wstr) {
        if (wstr.empty()) return {};
        static std::wstring_convert<std::codecvt_utf8<wchar_t>> cv;
        return cv.to_bytes(wstr);
    }
};
