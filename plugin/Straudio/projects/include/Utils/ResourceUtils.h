#pragma once
#include <string>
#include <optional>

namespace FileSystem {
    /**
     * @brief Get the platform-specific resource root directory
     * @return std::optional<std::string> Path if found, nullopt otherwise
     *
     * Returns:
     * - macOS: <Bundle>/Contents/Resources/
     * - Windows: <EXE_dir>/Resources/
     * - Linux: <EXE_dir>/../share/ or <EXE_dir>/Resources/
     */
    std::optional<std::string> getResourceDirectory();
    
    /**
     * @brief Get a subdirectory within resources
     * @param subdir The subdirectory name (e.g. "web")
     * @return std::optional<std::string> Full path if exists, nullopt otherwise
     */
    std::optional<std::string> getResourceSubdirectory(const std::string& subdir);
}
