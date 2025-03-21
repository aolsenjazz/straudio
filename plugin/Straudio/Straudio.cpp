#define WIN32_LEAN_AND_MEAN

#include <iostream>

#include "Straudio.h"
#include "IPlug_include_in_plug_src.h"
#include "IPlugPaths.h"
#include "src/PluginUI/PluginUI.hpp"
#include "src/Utils/FileUtils.h"

#include "src/WebServer/WebServer.h"
#include "src/MessageHandler.h"

using json = nlohmann::json;


Straudio::Straudio(const InstanceInfo& info)
  : Plugin(info, MakeConfig(0, 0)) {
//#ifdef DEBUG
  SetEnableDevTools(true);
//#endif
  

  mEditorInitFunc = [&]() {
    LoadFile(mPluginFilePath.c_str(), nullptr);
    EnableScroll(false);
  };
    
  mPluginFilePath = AppDataFileHelper::WriteDataToAppDir("plugin-ui.html", PLUGIN_UI, PLUGIN_UI_length);
  initializeWebServer();
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
