#pragma once

#include <atomic>
#include <string>

#include "src/SignalClient.h"
#include "src/PeerConnectionRegistry.h"

class MultiPeerConnectionManager {
public:
  std::string mSessionId = "1";
  
  explicit MultiPeerConnectionManager(std::string wsUrl)
  : mWsUrl(std::move(wsUrl)),
  mSignal(mWsUrl),
  mPeers([this](const nlohmann::json &j) { mSignal.send(j); }) {}
  
  ~MultiPeerConnectionManager() { stop(); }
  
  inline bool start() {
    if (mRunning) return true;
    mSignal.connect([this](const nlohmann::json& j) {
      onSignal(j);
    });
    
    nlohmann::json createMsg = {{"type", "createSession"},
      {"timeSent", Util::currentTimestamp()},
      {"payload", nlohmann::json::object()}};
    mSignal.send(createMsg);
    mRunning = true;
    return true;
  }
  
  inline void stop() {
    if (!mRunning.exchange(false)) return;
    mSignal.close();
  }
  
  const std::string &sessionId() const { return mSessionId; }
  
private:
  inline void onSignal(const nlohmann::json &j) {
    std::string type = j.value("type", "");
    if (type == "sessionCreated") {
      // The trivial solution is to hardcode a sessionId; this is acceptable right now.
//      mSessionId = j["payload"].value("sessionId", "");
      Logger::info("Session created: " + mSessionId);
    } else if (type == "offer") {
      mPeers.handleOffer(j["payload"]);
    } else if (type == "answer") {
      mPeers.handleAnswer(j["payload"]);
    } else if (type == "candidate") {
      mPeers.handleCandidate(j["payload"]);
    } else if (type == "error") {
      Logger::error("Server error: " + j.dump());
    } else {
      // ping/pong already handled inside SignalClient, ignore
    }
  }
  
  std::string               mWsUrl;
  std::atomic<bool>         mRunning {false};
  
  SignalClient              mSignal;
  PeerConnectionRegistry    mPeers;
};
