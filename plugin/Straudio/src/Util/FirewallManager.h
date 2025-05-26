//
//  FirewallManager.h
//  Straudio-macOS
//
//  Created by Alexander Olsen on 3/15/25.
//

#ifndef FirewallManager_h
#define FirewallManager_h

#endif /* FirewallManager_h */

#include <windows.h>
#include <iostream>
#include <shellapi.h>
#include <fstream>
#include "src/FirewallHelper.hpp"
#include "src/Bitch.hpp"
#pragma comment(lib, "Shell32.lib")

void ExtractAndRunFirewallManager() {
    // Get the system's temp folder
    wchar_t tempPath[MAX_PATH];
    GetTempPathW(MAX_PATH, tempPath);
    std::wstring exePath = std::wstring(tempPath) + L"Bitch.exe";
    Logger::info(exePath);
    // Write binary data to a temp file
    std::ofstream outFile(exePath, std::ios::binary);
    if (!outFile) {
        std::cerr << "Failed to create temp file!" << std::endl;
        return;
    }

    outFile.write(reinterpret_cast<const char*>(BITCH), BITCH_length);
    outFile.close();

    // Ensure the file is executable
    SetFileAttributesW(exePath.c_str(), FILE_ATTRIBUTE_NORMAL);

    // Execute the extracted file with admin privileges
    SHELLEXECUTEINFOW sei = { sizeof(sei) };
    sei.lpVerb = L"runas";  // Correct: Wide string
    sei.lpFile = exePath.c_str();  // Correct: Wide string
    sei.nShow = SW_SHOWNORMAL;
    sei.fMask = SEE_MASK_NOCLOSEPROCESS;

    if (!ShellExecuteExW(&sei)) {
        DWORD dwError = GetLastError();
        std::cerr << "Failed to execute FirewallManager! Error: " << dwError << std::endl;
        return;
    }

    // Wait for process to complete before cleanup (optional)
    if (sei.hProcess) {
        WaitForSingleObject(sei.hProcess, INFINITE);
        CloseHandle(sei.hProcess);
    }

    // OPTIONAL: Delete the executable after execution
    //DeleteFileW(exePath.c_str());
}
