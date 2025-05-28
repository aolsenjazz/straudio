#pragma once

#include "IPlug_include_in_plug_hdr.h"
#include "Oscillator.h"
#include "Smoothers.h"
#include "ISender.h"
#include "src/ReceiverServer/Server.h"
#include "src/SignalServer/Server.h"
#include "src/PeerConnectionManager/PeerConnectionManager.h"
#include "src/ConnectivityManager.h"

using namespace iplug;

class Straudio final : public Plugin
{
public:
  Straudio(const InstanceInfo& info);
  
  void ProcessBlock(sample** inputs, sample** outputs, int nFrames) override;
  void OnReset() override;
  void OnIdle() override;
  void OnMessageFromWebView(const char* json) override;
  ~Straudio();
  
  std::unique_ptr<ReceiverServer> mFrontendServer;
  std::unique_ptr<SignalServer> mSignalServer;
  std::unique_ptr<MultiPeerConnectionManager> mPeerConnectionManager;

private:
  void initializeWebServers();
  std::string mPluginFilePath;
  std::unique_ptr<ConnectivityManager> mConnectivity;
  void shutdownWebServers();
};
