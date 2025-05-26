#pragma once

#include <functional>
#include <string>
#include <thread>
#include <atomic>
#include "vendor/civetweb/civetweb.h"
#include <nlohmann/json.hpp>

#include "src/Util/Util.h"
#include "src/Util/Logger.h" // adjust include path as needed

class SignalClient {
public:
  using Json      = nlohmann::json;
  using OnMessage = std::function<void(const Json &)>; // called from WS thread
  
  explicit SignalClient(std::string url)
  : mUrl(std::move(url)) {}
  
  ~SignalClient() { close(); }
  
  SignalClient(const SignalClient &)            = delete;
  SignalClient &operator=(const SignalClient &) = delete;
  
  // ‑‑‑ API --------------------------------------------------------------
  inline bool connect(OnMessage handler) {
    if (mConn) return true;
    mOnMessage = std::move(handler);
    
    std::string host, path; int port;
    Util::parseWebSocketUrl(mUrl, host, port, path);
    
    char err[100] = {0};
    mConn = mg_connect_websocket_client(host.c_str(), port, 0, err, sizeof(err),
                                        path.c_str(), nullptr, &onWsData, &onWsClose, this);
    if (!mConn) {
      Logger::error(std::string("WS connect failed: ") + err);
      return false;
    }
    
    mRunning = true;
    startPingLoop();
    return true;
  }
  
  inline void send(const Json &msg) {
    if (!mConn) return;
    std::string s = msg.dump();
    mg_websocket_client_write(mConn, MG_WEBSOCKET_OPCODE_TEXT, s.c_str(), s.size());
  }
  
  inline void close() {
    if (!mRunning.exchange(false)) return;
    mKeepPinging = false;
    
    if (mPingThread.joinable()) mPingThread.join();
    if (mConn) {
      mConn = nullptr;
    }
  }
  
private:
  // ‑‑‑ websocket callbacks (C‑style) -----------------------------------
  static int onWsData(mg_connection *c, int flags, char *data, size_t len, void *ud) {
    auto *self = static_cast<SignalClient *>(ud);
    int opcode = flags & 0x0F;
    if (opcode == MG_WEBSOCKET_OPCODE_TEXT) {
      try {
        Json j = Json::parse(std::string(data, len));
        self->mOnMessage(j);
      } catch (std::exception &e) {
        Logger::error(std::string("Bad JSON from server: ") + e.what());
      }
    } else if (opcode == MG_WEBSOCKET_OPCODE_PING) {
      mg_websocket_client_write(c, MG_WEBSOCKET_OPCODE_PONG, data, len);
    }
    return 1;
  }
  
  static void onWsClose(const mg_connection *, void *ud) {
    auto *self = static_cast<SignalClient *>(ud);
    self->mConn   = nullptr;
    self->mRunning = false;
    Logger::info("WebSocket closed by server");
  }
  
  inline void startPingLoop() {
    mKeepPinging = true;
    mPingThread  = std::thread([this] {
      while (mKeepPinging) {
        Json ping = {{"type", "ping"}, {"timeSent", Util::currentTimestamp()}};
        send(ping);
        std::this_thread::sleep_for(std::chrono::seconds(1));
      }
    });
  }
  
  // ‑‑‑ members ---------------------------------------------------------
  std::string       mUrl;
  mg_connection *   mConn {nullptr};
  OnMessage         mOnMessage;
  std::atomic<bool> mRunning {false};
  std::atomic<bool> mKeepPinging {false};
  std::thread       mPingThread;
};
