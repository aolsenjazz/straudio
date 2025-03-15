
#pragma once

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <sstream>
#include <string>

namespace AppDataFileHelper {

    /**
     * Writes the given data to a file in the Local AppData folder.
     *
     * @param filename The name of the file to create (e.g. "pluginUI.html").
     * @param data Pointer to the raw data bytes.
     * @param length Number of bytes to write.
     * @param subDir (Optional) Subdirectory under LOCALAPPDATA to use (default: "MyPlugin").
     *
     * @return A std::string containing the full path to the written file.
     *
     * @throws std::runtime_error if the LOCALAPPDATA environment variable isn't set,
     *         or if directories cannot be created or the file cannot be written.
     */
    inline std::string WriteDataToAppDir(const std::string& filename,
        const uint8_t* data,
        size_t length,
        const std::string& subDir = "Straudio")
    {
        namespace fs = std::filesystem;

        // Retrieve the LOCALAPPDATA environment variable.
        const char* localAppData = std::getenv("LOCALAPPDATA");
        if (!localAppData)
        {
            throw std::runtime_error("LOCALAPPDATA environment variable not found.");
        }

        // Construct the target directory path (e.g. LOCALAPPDATA/MyPlugin).
        fs::path appDataPath(localAppData);
        fs::path targetDir = appDataPath / subDir;

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

}
