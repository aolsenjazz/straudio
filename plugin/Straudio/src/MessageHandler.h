#pragma once
#include "MessageTags.h"
#include "src/Util/Logger.h"
#include "Straudio.h"
#include "src/ReceiverServer/Server.h"
#include <json.hpp>

class MessageHandler {
public:
  static void HandleMessage(Straudio* plugin, const char* jsonStr) {
    try {
      auto data = nlohmann::json::parse(jsonStr);
      
      ValidateMessage(data);
      
      int tagValue = data["tag"];
      EMessageTag tag = static_cast<EMessageTag>(tagValue);
      
      switch (tag) {
        case EMessageTag::URL_REQUEST:
          HandleUrlRequest(plugin);
          break;
        default:
          std::cerr << "Unhandled message tag: " << tagValue << std::endl;
      }
      
    } catch (const std::exception& e) {
      std::cerr << "Error: " << e.what() << std::endl;
    }
  }
  
  static void HandleUrlRequest(Straudio* plugin) {
    std::string frontendUrl = plugin->mFrontendServer->getFullUrl();
    std::string websocketUrl = plugin->mSignalServer->getFullUrl();
    std::string sessionId = plugin->mPeerConnectionManager.get()->mSessionId;
    Logger::info("Frontend: " + frontendUrl + "\nWebsocket: " + websocketUrl + "\nSession ID: " + sessionId);
  
    std::string fullHref = frontendUrl + "?s=" + sessionId + "&u=" + websocketUrl;
    
    plugin->SendArbitraryMsgFromDelegate(
                                         static_cast<int>(EMessageTag::URL_RESPONSE),
                                         static_cast<int>(fullHref.size()),
                                         fullHref.c_str()
                                         );
  }
  
private:
  static void ValidateMessage(const nlohmann::json& data) {
    if (!data.is_object()) {
      throw std::runtime_error("Invalid JSON: Expected an object");
    }
    
    if (!data.contains("tag") || !data["tag"].is_number()) {
      throw std::runtime_error("Invalid JSON: 'tag' must be a number");
    }
  }
};
