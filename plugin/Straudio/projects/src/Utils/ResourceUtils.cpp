#include "ResourceUtils.h"
#include <filesystem>

#if defined(__APPLE__)
#include <CoreFoundation/CoreFoundation.h>
#elif defined(_WIN32)
#include <windows.h>
#else
#include <unistd.h>
#include <linux/limits.h>
#endif

namespace fs = std::filesystem;

namespace FileSystem {

std::optional<std::string> getResourceDirectory() {
  try {
    #if defined(__APPLE__)
    // macOS implementation
    CFBundleRef mainBundle = CFBundleGetMainBundle();
    if (!mainBundle) return std::nullopt;
    
    CFURLRef resourcesURL = CFBundleCopyResourcesDirectoryURL(mainBundle);
    if (!resourcesURL) return std::nullopt;
    
    char path[PATH_MAX];
    if (!CFURLGetFileSystemRepresentation(resourcesURL, true, (UInt8*)path, PATH_MAX)) {
        CFRelease(resourcesURL);
        return std::nullopt;
    }
    
    CFRelease(resourcesURL);
    return std::string(path);

    #elif defined(_WIN32)
    // Windows implementation
    char exePath[MAX_PATH];
    GetModuleFileNameA(nullptr, exePath, MAX_PATH);
    fs::path resourcesPath = fs::path(exePath).parent_path() / "Resources";
    return resourcesPath.string();

    #else
    // Linux implementation
    char exePath[PATH_MAX];
    ssize_t count = readlink("/proc/self/exe", exePath, PATH_MAX);
    if (count <= 0) return std::nullopt;
    
    fs::path exeDir = fs::path(std::string(exePath, count)).parent_path();
    fs::path resourcesPath = exeDir / "Resources";
    
    if (!fs::exists(resourcesPath)) {
        // Try common Linux share directory
        resourcesPath = exeDir.parent_path() / "share";
    }
    
    return resourcesPath.string();
    #endif
  }
  catch (...) {
    return std::nullopt;
  }
}

std::optional<std::string> getResourceSubdirectory(const std::string& subdir) {
      auto baseDir = getResourceDirectory();
      if (!baseDir) return std::nullopt;
      
      fs::path fullPath = fs::path(*baseDir) / subdir;
      
      if (fs::exists(fullPath)) {
          return fullPath.string();
      }
      
      return std::nullopt;
  }
}
