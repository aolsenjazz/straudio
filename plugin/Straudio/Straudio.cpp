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
: Plugin(info, MakeConfig(kNumParams, kNumPresets)) {
  GetParam(kGain)->InitGain("Gain", -70., -70, 0.);
  
//#ifdef DEBUG
  SetCustomUrlScheme("iplug2");
  SetEnableDevTools(true);
//#endif

  mEditorInitFunc = [&]() {
    LoadURL("http://localhost:5173/");
//  LoadIndexHtml(__FILE__, GetBundleID());
    EnableScroll(false);
  };

  MakePreset("One", -70.);
  MakePreset("Two", -30.);
  MakePreset("Three", 0.);
  
  initializeWebServer();
}

void Straudio::ProcessBlock(sample** inputs, sample** outputs, int nFrames) {
  const double gain = GetParam(kGain)->DBToAmp();
    
  mOscillator.ProcessBlock(inputs[0], nFrames); // comment for audio in

  for (int s = 0; s < nFrames; s++) {
    outputs[0][s] = inputs[0][s] * mGainSmoother.Process(gain);
    outputs[1][s] = outputs[0][s]; // copy left
  }
  
  mSender.ProcessBlock(outputs, nFrames, kCtrlTagMeter);
}

void Straudio::OnReset() {
  auto sr = GetSampleRate();
  mOscillator.SetSampleRate(sr);
  mGainSmoother.SetSmoothTime(20., sr);
}

void Straudio::OnIdle() {}

void Straudio::initializeWebServer() {
  mWebServer = std::make_unique<WebServer>();
  
  auto webRoot = FileSystem::getResourceSubdirectory("web");
  if (webRoot) {
    mWebServer->start(*webRoot);
  }
}

void Straudio::OnMessageFromWebView(const char* jsonStr) {
  MessageHandler::HandleMessage(this, jsonStr);
}
