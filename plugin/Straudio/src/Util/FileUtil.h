#pragma once

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <sstream>
#include <string>

namespace AppDataFileHelper {

    /**
     * Writes the given data to a file in the persistent application directory.
     *
     * On Windows, this uses the LOCALAPPDATA folder. On macOS, it uses
     * ~/Library/Application Support.
     *
     * @param filename The name of the file to create (e.g. "pluginUI.html").
     * @param data Pointer to the raw data bytes.
     * @param length Number of bytes to write.
     * @param subDir (Optional) Subdirectory under the base directory to use (default: "Straudio").
     *
     * @return A std::string containing the full path to the written file.
     *
     * @throws std::runtime_error if the required environment variable isn't set,
     *         or if directories cannot be created or the file cannot be written.
     */
    inline std::string WriteDataToAppDir(const std::string& filename,
                                           const uint8_t* data,
                                           size_t length,
                                           const std::string& subDir = "Straudio")
    {
        namespace fs = std::filesystem;
        fs::path baseDir;
#ifdef _WIN32
        // Retrieve the LOCALAPPDATA environment variable.
        const char* localAppData = std::getenv("LOCALAPPDATA");
        if (!localAppData)
        {
            throw std::runtime_error("LOCALAPPDATA environment variable not found.");
        }
        baseDir = fs::path(localAppData);
#else
        // On macOS, use HOME and append Library/Application Support.
        const char* homeDir = std::getenv("HOME");
        if (!homeDir)
        {
            throw std::runtime_error("HOME environment variable not found.");
        }
        baseDir = fs::path(homeDir) / "Library" / "Application Support";
#endif

        // Construct the target directory path.
        fs::path targetDir = baseDir / subDir;

        // Create the directory (and any intermediate directories) if they don't exist.
        std::error_code ec;
        fs::create_directories(targetDir, ec);
        if (ec)
        {
            std::stringstream ss;
            ss << "Failed to create directory " << targetDir.string() << ": " << ec.message();
            throw std::runtime_error(ss.str());
        }

        // Construct the full file path.
        fs::path filePath = targetDir / filename;

        // Write the data to the file.
        std::ofstream ofs(filePath, std::ios::binary);
        if (!ofs)
        {
            throw std::runtime_error("Failed to open file for writing: " + filePath.string());
        }
        ofs.write(reinterpret_cast<const char*>(data), length);
        ofs.close();

        return filePath.string();
    }

} // namespace AppDataFileHelper
