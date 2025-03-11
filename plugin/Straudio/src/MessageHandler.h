#pragma once
#include "MessageTags.h"
#include "src/Utils/Logger.h"
#include "Straudio.h"
#include "src/WebServer/WebServer.h"
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
          HandleUrlRequest(plugin, data);
          break;
        default:
          std::cerr << "Unhandled message tag: " << tagValue << std::endl;
      }

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
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
  
  static void HandleUrlRequest(Straudio* plugin, const nlohmann::json& data) {
    std::string url = plugin->mWebServer->getFullUrl();
    Logger::info(url);


    plugin->SendArbitraryMsgFromDelegate(
      static_cast<int>(EMessageTag::URL_RESPONSE),
     static_cast<int>(url.size()),
      url.c_str()
    );
  }
};
