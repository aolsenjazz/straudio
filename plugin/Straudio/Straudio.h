#pragma once

#include "IPlug_include_in_plug_hdr.h"
#include "Oscillator.h"
#include "Smoothers.h"
#include "ISender.h"
#include "src/WebServer/WebServer.h"

using namespace iplug;

class Straudio final : public Plugin
{
public:
  Straudio(const InstanceInfo& info);
  
  void ProcessBlock(sample** inputs, sample** outputs, int nFrames) override;
  void OnReset() override;
  void OnIdle() override;
  void OnMessageFromWebView(const char* json) override;
  
  std::unique_ptr<WebServer> mWebServer;

private:
  void initializeWebServer();
  std::string mPluginFilePath;
};
