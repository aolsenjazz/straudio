#define WIN32_LEAN_AND_MEAN

#include <iostream>

#include "Straudio.h"
#include "IPlug_include_in_plug_src.h"
#include "IPlugPaths.h"
#include "src/PluginUI/PluginUI.hpp"
#include "src/Util/FileUtil.h"

#include "src/ReceiverServer/Server.h"
#include "src/SignalServer/Server.h"
#include "src/PeerConnectionManager/PeerConnectionManager.h"
#include "src/MessageHandler.h"

using json = nlohmann::json;

Straudio::Straudio(const InstanceInfo& info)
: Plugin(info, MakeConfig(0, 0)) {
  //#ifdef DEBUG
  SetEnableDevTools(true);
  //#endif
  
  mEditorInitFunc = [&]() {
    //    LoadFile(mPluginFilePath.c_str(), nullptr);
    LoadURL("http://localhost:5173");
    EnableScroll(false);
  };
  
  mPluginFilePath = AppDataFileHelper::WriteDataToAppDir("plugin-ui.html", PLUGIN_UI, PLUGIN_UI_length);
  initializeWebServers();
}

Straudio::~Straudio()
{
  if (mPeerConnectionManager) mPeerConnectionManager->stop();
  if (mSignalServer)         mSignalServer->stop();
  if (mFrontendServer)       mFrontendServer->stop();
}

void Straudio::ProcessBlock(sample** inputs, sample** outputs, int nFrames) {
  for (int s = 0; s < nFrames; s++) {
    outputs[0][s] = inputs[0][s];
    outputs[1][s] = outputs[0][s];
  }
}

void Straudio::OnReset() {}

void Straudio::OnIdle() {}

void Straudio::initializeWebServers() {
  mFrontendServer = std::make_unique<ReceiverServer>();
  Boolean frontendSuccess = mFrontendServer->start();
  
  mSignalServer = std::make_unique<SignalServer>();
  Boolean signalSuccess = mSignalServer->start();
  
  if (!frontendSuccess || !signalSuccess) {
    // long-term the correct thing todo will be pass error to
    // frontend, to be handled and give user feedback
    abort();
  }
  
  mPeerConnectionManager = std::make_unique<MultiPeerConnectionManager>(mSignalServer->getFullUrl());
  Boolean peerSuccess = mPeerConnectionManager->start();
  
  if (!peerSuccess) {
    abort();
  }
}

void Straudio::OnMessageFromWebView(const char* jsonStr) {
  MessageHandler::HandleMessage(this, jsonStr);
}
