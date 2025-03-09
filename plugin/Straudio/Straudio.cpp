#include <iostream>

#include "Straudio.h"
#include "IPlug_include_in_plug_src.h"
#include "IPlugPaths.h"

#include "ResourceUtils.h"
#include "MessageHandler.h"
#include "json.hpp"
#include "civetweb.h"

using json = nlohmann::json;

Straudio::Straudio(const InstanceInfo& info)
  : Plugin(info, MakeConfig(0, 0)) {
//#ifdef DEBUG
  SetCustomUrlScheme("iplug2");
  SetEnableDevTools(true);
//#endif

  mEditorInitFunc = [&]() {
    LoadURL("http://localhost:5173/");
//  LoadIndexHtml(__FILE__, GetBundleID());
    EnableScroll(false);
  };

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
  
  const char* bundleId = GetBundleID();
  auto webRoot = FileSystem::getResourceSubdirectory(bundleId, "web");
  if (webRoot) {
    mWebServer->start(*webRoot);
  }
}

void Straudio::OnMessageFromWebView(const char* jsonStr) {
  MessageHandler::HandleMessage(this, jsonStr);
}
