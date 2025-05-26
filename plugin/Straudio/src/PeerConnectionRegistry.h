#pragma once

#include <functional>
#include <memory>
#include <unordered_map>
#include <rtc/rtc.hpp>
#include <nlohmann/json.hpp>

#include "src/Util/Util.h"
#include "src/Util/Logger.h" // adjust include path

class PeerConnectionRegistry {
public:
  using Json           = nlohmann::json;
  using SendSignalFunc = std::function<void(const Json &)>; // <- outward via SignalClient

  explicit PeerConnectionRegistry(SendSignalFunc sendCb) : mSend(std::move(sendCb)) {}

  // inbound from signaling channel
  inline void handleOffer(const Json &pl) {
    std::string from = pl.value("sourceClientId", "");
    if (from.empty()) return;
    auto &ctx = ensurePeer(from);
    ctx.pc->setRemoteDescription(rtc::Description(pl["description"], "offer"));
//    ctx.pc->createAnswer();
  }

  inline void handleAnswer(const Json &pl) {
    std::string from = pl.value("sourceClientId", "");
    if (from.empty()) return;
    auto &ctx = ensurePeer(from);
    ctx.pc->setRemoteDescription(rtc::Description(pl["description"], "answer"));
  }

  inline void handleCandidate(const Json &pl) {
    std::string from = pl.value("sourceClientId", "");
    if (from.empty()) return;
    auto &ctx = ensurePeer(from);
    ctx.pc->addRemoteCandidate(rtc::Candidate(pl["candidate"], pl["mid"]));
  }

private:
  struct PeerCtx {
    std::shared_ptr<rtc::PeerConnection> pc;
  };

  inline PeerCtx &ensurePeer(const std::string &id) {
    auto it = mPeers.find(id);
    if (it == mPeers.end()) {
      it = mPeers.emplace(id, PeerCtx{createPc(id)}).first;
    }
    return it->second;
  }

  inline std::shared_ptr<rtc::PeerConnection> createPc(const std::string &rid) {
    auto pc = std::make_shared<rtc::PeerConnection>(rtc::Configuration{});

    pc->onLocalDescription([this, rid](rtc::Description desc) {
      Json msg = {
        {"type", desc.typeString()},
        {"timeSent", Util::currentTimestamp()},
        {"payload", {{"targetClientId", rid}, {"description", std::string(desc)}}}
      };
      mSend(msg);
    });

    pc->onLocalCandidate([this, rid](rtc::Candidate cand) {
      Json msg = {
        {"type", "candidate"},
        {"timeSent", Util::currentTimestamp()},
        {"payload", {{"targetClientId", rid}, {"candidate", std::string(cand)}, {"mid", cand.mid()}}}
      };
      mSend(msg);
    });

    pc->onStateChange([rid](rtc::PeerConnection::State st) {
      Logger::info("[PC " + rid + "] state=" + std::to_string(static_cast<int>(st)));
    });

    return pc;
  }

  SendSignalFunc                              mSend;
  std::unordered_map<std::string, PeerCtx>    mPeers;
};
