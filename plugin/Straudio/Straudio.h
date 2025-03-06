#pragma once

#include "IPlug_include_in_plug_hdr.h"
#include "Oscillator.h"
#include "Smoothers.h"
#include "ISender.h"
#include "WebServer.h"

using namespace iplug;

const int kNumPresets = 3;

enum EParams
{
  kGain = 0,
  kNumParams
};

enum EMsgTags
{
  kMsgTagButton1 = 0,
  kMsgTagButton2 = 1,
  kMsgTagButton3 = 2,
  kMsgTagBinaryTest = 3
};

enum EControlTags
{
  kCtrlTagMeter = 0,
};

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
  iplug::IPeakSender<2> mSender;
  FastSinOscillator<sample> mOscillator {0., 440.};
  LogParamSmooth<sample, 1> mGainSmoother;
  
  
  void initializeWebServer();
};
