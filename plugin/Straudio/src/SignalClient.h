#pragma once
#include <functional>
#include <string>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <chrono>
#include "vendor/civetweb/civetweb.h"
#include <nlohmann/json.hpp>
#include "src/Util/Util.h"
#include "src/Util/Logger.h"

class SignalClient {
public:
  using Json      = nlohmann::json;
  using OnMessage = std::function<void(const Json&)>;
  std::vector<Json> pending;

  explicit SignalClient(std::string url)
  : mUrl(std::move(url))
  {
    pingThread  = std::thread([this]{ pingLoop(); });
    reconThread = std::thread([this]{ reconnectLoop(); });
  }

  ~SignalClient() { close(); }

  SignalClient(const SignalClient&)            = delete;
  SignalClient& operator=(const SignalClient&) = delete;

  void connect(OnMessage handler)
  {
    mOnMessage      = std::move(handler);
    allowReconnect  = true;
    reconCv.notify_all();            // wake reconnect loop immediately
  }

  void send(const Json& msg)
  {
    if (!conn) {                   // not yet open
      pending.push_back(msg);
      return;
    }
    std::string s = msg.dump();
    mg_websocket_client_write(conn, MG_WEBSOCKET_OPCODE_TEXT, s.c_str(), s.size());
  }

  void close()
  {
    allowReconnect = false;
    running        = false;
    alive          = false;

    pingCv.notify_all();
    reconCv.notify_all();

    if (pingThread.joinable())   pingThread.join();
    if (reconThread.joinable())  reconThread.join();

    if (conn) {
      mg_websocket_client_write(conn, MG_WEBSOCKET_OPCODE_CONNECTION_CLOSE, "", 0);
      mg_close_connection(conn);
      conn = nullptr;
    }
  }

private:
  /* --------------- ws callbacks ---------------- */
  static int onWsData(mg_connection* c,int f,char* d,size_t n,void* u)
  {
    auto* s = static_cast<SignalClient*>(u);
    int op = f & 0x0F;
    if (op==MG_WEBSOCKET_OPCODE_TEXT) {
      try {
        Json j = Json::parse(std::string(d,n));
        std::string type=j.value("type","");
        if (type=="ping") { s->send({{"type","pong"},{"timeSent",Util::currentTimestamp()}}); return 1;}
        if (type=="pong") { s->awaitingPong=false; return 1; }
        if (s->mOnMessage) s->mOnMessage(j);
      } catch(...) { Logger::error("Bad JSON from server"); }
    }
    return 1;
  }

  static void onWsClose(const mg_connection*, void* u)
  {
    auto* s = static_cast<SignalClient*>(u);
    s->conn   = nullptr;
    s->running= false;
    s->awaitingPong = false;
    s->reconCv.notify_all();
  }

  /* --------------- ping loop ------------------- */
  void pingLoop()
  {
    std::unique_lock lk(pingMx);
    while (alive) {
      pingCv.wait_for(lk, std::chrono::seconds(5));
      if (!alive) break;
      if (!running) continue;

      if (awaitingPong) {           // timeout
        Logger::error("Ping timeout; closing socket");
        closeSocketOnly();
        continue;
      }
      send({{"type","ping"},{"timeSent",Util::currentTimestamp()}});
      awaitingPong = true;
    }
  }

  /* --------------- reconnect loop -------------- */
  void reconnectLoop()
  {
    std::unique_lock lk(reconMx);
    int attempt=0;
    while (alive) {
      reconCv.wait_for(lk, std::chrono::milliseconds(100));
      if (!alive) break;
      if (!allowReconnect || running) continue;

      int delay = std::min(1000*(1<<attempt), 30000);
      lk.unlock();
      std::this_thread::sleep_for(std::chrono::milliseconds(delay));
      lk.lock();
      if (!allowReconnect || running) { attempt=0; continue; }

      Logger::info("WS reconnect attempt ",attempt+1);
      if (openSocket()) { attempt=0; continue; }
      ++attempt;
    }
  }

  /* --------------- socket helpers -------------- */
  bool openSocket()
  {
    std::string host,path; int port;
    Util::parseWebSocketUrl(mUrl,host,port,path);
    char err[100]{};
    conn = mg_connect_websocket_client(host.c_str(),port,0,err,sizeof(err),
                                       path.c_str(),nullptr,
                                       &onWsData,&onWsClose,this);
    if (!conn) { Logger::warning("WS connect failed: "+std::string(err)); return false; }
    running = true;
    
    for (auto& p : pending)
      send(p);
    pending.clear();
    
    awaitingPong = false;
    pingCv.notify_all();
    return true;
  }

  void closeSocketOnly()
  {
    running        = false;
    awaitingPong   = false;
    if (conn) {
      mg_websocket_client_write(conn, MG_WEBSOCKET_OPCODE_CONNECTION_CLOSE,"",0);
      mg_close_connection(conn);
      conn=nullptr;
    }
  }

  /* --------------- members --------------------- */
  std::string          mUrl;
  mg_connection*       conn{nullptr};
  OnMessage            mOnMessage;

  std::atomic<bool>    running{false};
  std::atomic<bool>    allowReconnect{false};
  std::atomic<bool>    awaitingPong{false};
  std::atomic<bool>    alive{true};

  std::thread          pingThread;
  std::thread          reconThread;

  std::condition_variable_any pingCv;
  std::condition_variable_any reconCv;
  std::mutex          pingMx;
  std::mutex          reconMx;
};
