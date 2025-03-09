#pragma once

#include <string>
#include <optional>
#include <filesystem>
#include <algorithm>
#include "IPlugConstants.h"
#include "WDLString.h"
#include "IPlugPaths.h"

namespace fs = std::filesystem;

namespace FileSystem {

inline std::optional<std::string> getResourceDirectory(const char* bundleID) {
    WDL_String bundlePath;

    // Get the bundle path using IPlug's infrastructure
    iplug::BundleResourcePath(bundlePath, bundleID);

    if (bundlePath.GetLength() > 0) {
        std::string pathStr(bundlePath.Get());

        // Normalize path separators for Windows
        #if defined(_WIN32)
        std::replace(pathStr.begin(), pathStr.end(), '/', '\\');
        #endif

        return pathStr;
    }

    return std::nullopt;
}

inline bool iequals(const std::string& a, const std::string& b) {
    return a.length() == b.length() &&
           std::equal(a.begin(), a.end(), b.begin(),
               [](char c1, char c2) {
                   return std::tolower(static_cast<unsigned char>(c1)) ==
                          std::tolower(static_cast<unsigned char>(c2));
               });
}

inline std::optional<std::string> getResourceSubdirectory(const char* bundleID, const std::string& subdir) {
    auto baseDir = getResourceDirectory(bundleID);
    
    if (!baseDir) return std::nullopt;
    
    try {
        fs::path fullPath = fs::path(*baseDir) / subdir;

        // First try exact match
        if (fs::exists(fullPath)) {
            return fullPath.string();
        }

        // Case-insensitive fallback for macOS
        #if defined(__APPLE__)
        for (const auto& entry : fs::directory_iterator(*baseDir)) {
            if (fs::is_directory(entry) && iequals(entry.path().filename().string(), subdir)) {
                return entry.path().string();
            }
        }
        #endif
    }
    catch (...) {
        // Filesystem errors
    }
    
    return std::nullopt;
}

}
