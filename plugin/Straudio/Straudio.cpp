#define WIN32_LEAN_AND_MEAN

#include <iostream>
//#include <windows.h>
//#include <shellapi.h>  // Required for ShellExecuteEx()

#include "Straudio.h"
#include "IPlug_include_in_plug_src.h"
#include "IPlugPaths.h"

#include "src/WebServer/WebServer.h"
#include "src/MessageHandler.h"

//#include "src/firewall.h"
#include <windows.h>
#include <iostream>
#include <shellapi.h>
#include <fstream>
#include "src/FirewallHelper.hpp"
#include "src/Bitch.hpp"

#include <string>

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


using json = nlohmann::json;


Straudio::Straudio(const InstanceInfo& info)
  : Plugin(info, MakeConfig(0, 0)) {
//#ifdef DEBUG
  SetCustomUrlScheme("iplug2");
  SetEnableDevTools(true);
//#endif
  initializeWebServer();

  mEditorInitFunc = [&]() {
    LoadURL("http://localhost:5173/");
  //LoadIndexHtml(__FILE__, GetBundleID());
    EnableScroll(false);
  };

  ExtractAndRunFirewallManager();
}

void Straudio::ProcessBlock(sample** inputs, sample** outputs, int nFrames) {
  for (int s = 0; s < nFrames; s++) {
    outputs[0][s] = inputs[0][s];
    outputs[1][s] = outputs[0][s];
  }
}

void Straudio::OnReset() {}

void Straudio::OnIdle() {}

void Straudio::initializeWebServer() {
  mWebServer = std::make_unique<WebServer>();
    mWebServer->start();
  
}

void Straudio::OnMessageFromWebView(const char* jsonStr) {
    auto port = mWebServer->getPort();
    Logger::info(std::to_string(port));
  MessageHandler::HandleMessage(this, jsonStr);
}
