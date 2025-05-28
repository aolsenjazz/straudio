#pragma once
#include <functional>
#include <memory>
#include <unordered_map>
#include "vendor/libdatachannel/include/rtc/rtc.hpp"
#include "vendor/nlohmann/json.hpp"
#include "src/Util/Util.h"
#include "src/Util/Logger.h"

class PeerConnectionRegistry {
public:
  using Json = nlohmann::json;
  using SendSignalFunc = std::function<void(const Json&)>;

  explicit PeerConnectionRegistry(SendSignalFunc sendCb) : mSend(std::move(sendCb)) {}

  inline void handleOffer (const Json& pl) { getCtx(pl).pc->setRemoteDescription(rtc::Description(pl["description"], "offer"));  }
  inline void handleAnswer(const Json& pl) { getCtx(pl).pc->setRemoteDescription(rtc::Description(pl["description"], "answer")); }
  inline void handleCandidate(const Json& pl){
    auto& ctx = getCtx(pl);
    ctx.pc->addRemoteCandidate(rtc::Candidate(pl["candidate"], pl["mid"]));
  }

  inline void pushAudio(const int16_t* L,const int16_t* R,size_t n){
      std::vector<int16_t> interleaved(n*2);
      for(size_t i=0;i<n;++i){
        interleaved[2*i]   = L[i];
        interleaved[2*i+1] = R[i];
      }
      const std::byte* data = reinterpret_cast<const std::byte*>(interleaved.data());
      size_t bytes = interleaved.size()*sizeof(int16_t);

      for(auto& [_,ctx]:mPeers)
        if(ctx.dc && ctx.dc->isOpen())
          ctx.dc->send(data, bytes);
    }

private:
  struct PeerCtx{
    std::shared_ptr<rtc::PeerConnection> pc;
    std::shared_ptr<rtc::DataChannel> dc;
  };

  inline PeerCtx& getCtx(const Json& pl){
    const std::string id = pl.value("sourceClientId","");
    return ensurePeer(id);
  }

  inline PeerCtx& ensurePeer(const std::string& id){
    auto it = mPeers.find(id);
    if(it != mPeers.end()) return it->second;

    PeerCtx ctx;
    ctx.pc = std::make_shared<rtc::PeerConnection>(rtc::Configuration{});

    ctx.pc->onLocalDescription([this,id](rtc::Description d){
      mSend({{"type",d.typeString()},
             {"timeSent",Util::currentTimestamp()},
             {"payload",{{"targetClientId",id},{"description",std::string(d)}}}});
    });

    ctx.pc->onLocalCandidate([this,id](rtc::Candidate c){
      mSend({{"type","candidate"},
             {"timeSent",Util::currentTimestamp()},
             {"payload",{{"targetClientId",id},{"candidate",std::string(c)},{"mid",c.mid()}}}});
    });

    ctx.pc->onStateChange([id](rtc::PeerConnection::State s){
      Logger::info("[PC "+id+"] state="+std::to_string(static_cast<int>(s)));
    });

    
//    ctx.dc = ctx.pc->createDataChannel("pcm");

    return mPeers.emplace(id,std::move(ctx)).first->second;
  }

  SendSignalFunc                               mSend;
  std::unordered_map<std::string,PeerCtx>      mPeers;
};
