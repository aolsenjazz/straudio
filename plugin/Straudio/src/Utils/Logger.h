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
    // -----------------------------------------
    // Public Logging Methods
    // -----------------------------------------
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

private:
    // -----------------------------------------
    // Internal Enums & Data
    // -----------------------------------------
    enum class Level {
        INFO,
        WARNING,
        ERROR
    };

    inline static std::mutex mutex_;

    // -----------------------------------------
    // Logging Implementation
    // -----------------------------------------
    static void log(const std::string& message, Level level) {
        std::lock_guard<std::mutex> lock(mutex_);
        std::string levelStr = levelToString(level);
        std::string formattedMessage = "[" + levelStr + "] " + message;

        // Platform-Specific Debug Logging
#ifdef __APPLE__
        os_log_type_t osLogLevel = getOSLogLevel(level);
        os_log_with_type(OS_LOG_DEFAULT, osLogLevel, "%{public}s", formattedMessage.c_str());
#elif _WIN32
        OutputDebugStringA((formattedMessage + "\n").c_str());
#endif

        // Always print to stdout
        std::cout << formattedMessage << std::endl;
    }

    static std::string levelToString(Level level) {
        switch (level) {
        case Level::INFO:    return "INFO";
        case Level::WARNING: return "WARNING";
        case Level::ERROR:   return "ERROR";
        default:             return "UNKNOWN";
        }
    }

#ifdef __APPLE__
    static os_log_type_t getOSLogLevel(Level level) {
        switch (level) {
        case Level::ERROR:   return OS_LOG_TYPE_ERROR;
            // There is no separate OS_LOG_TYPE_WARNING, so use default
        default:             return OS_LOG_TYPE_DEFAULT;
        }
    }
#endif

    // -----------------------------------------
    // Message Building (Variadic Templates)
    // -----------------------------------------
    template<typename... Args>
    static std::string buildMessage(Args&&... args) {
        std::ostringstream oss;
        // Expand and append all arguments
        (appendArg(oss, std::forward<Args>(args)), ...);
        return oss.str();
    }

    // Convert each argument to string and append to the ostringstream
    template<typename T>
    static void appendArg(std::ostringstream& oss, T&& arg) {
        oss << toString(std::forward<T>(arg));
    }

    // -----------------------------------------
    // Conversion Functions for Various Types
    // -----------------------------------------
    // Generic template: uses ostringstream operator<< for everything else
    template<typename T>
    static std::string toString(const T& value) {
        std::ostringstream oss;
        oss << value;
        return oss.str();
    }

    // Overload for std::wstring -> UTF-8
    static std::string toString(const std::wstring& wstr) {
        return wstringToUtf8(wstr);
    }

    // -----------------------------------------
    // Cross-Platform wstring -> UTF-8 Converter
    // -----------------------------------------
    static std::string wstringToUtf8(const std::wstring& wstr) {
        // If empty, return quickly
        if (wstr.empty()) return std::string();

        // Use C++ standard wstring converter (deprecated in C++17, 
        // but still widely supported). Alternatively, on Windows, you
        // could call WideCharToMultiByte directly.
        static std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        return converter.to_bytes(wstr);
    }
};
