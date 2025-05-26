#pragma once

#include "civetweb.h"
#include <json.hpp>
#include <mutex>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <random>
#include <string>
#include <chrono>
#include <thread>
#include <atomic>

#include "src/Util/Util.h"
#include "src/Util/Logger.h"

using json = nlohmann::json;

namespace SignallingRoute {

static std::mutex sessionMutex;
struct PingMeta { bool awaiting{false}; };
static std::unordered_map<const mg_connection*, PingMeta> pingState;


// Simple data for each WebSocket connection
struct ConnectionInfo {
  std::string connectionId;
  std::string sessionId;
  bool isHost = false;
};

// Each session has a list of mg_connection* that have joined
struct Session {
  std::vector<mg_connection*> clientConnections;
};

static std::unordered_map<const mg_connection*, ConnectionInfo> connectionToInfo;
static std::unordered_map<std::string, Session> sessionIdToSession;

static std::thread               gPingThread;
static std::atomic<bool>         gKeepPinging{false};
static std::condition_variable   gPingCv;
static std::mutex                gPingMx;

void websocketCloseHandler(const struct mg_connection* conn, void* cbdata);


inline void dropConn(mg_connection* c)
{
  mg_websocket_write(c, MG_WEBSOCKET_OPCODE_CONNECTION_CLOSE, "", 0);

  {
    std::lock_guard<std::mutex> lock(sessionMutex);
    pingState.erase(c);
  }

  mg_close_connection(c);
}

static void startPingThread()
{
  if (gKeepPinging.exchange(true)) return;
  
  gPingThread = std::thread([] {
    std::unique_lock lk(gPingMx);
    while (gKeepPinging) {
      if (gPingCv.wait_for(lk, std::chrono::seconds(5),
                           [] { return !gKeepPinging.load(); }))
        break;
      
      std::vector<mg_connection*> toDrop;
      
      { /* gather under sessionMutex */
        std::lock_guard<std::mutex> guard(sessionMutex);
        for (auto& kv : connectionToInfo) {
          const mg_connection* c = kv.first;
          auto& meta = pingState[c];
          
          if (meta.awaiting) {
            toDrop.push_back(const_cast<mg_connection*>(c));
            continue;
          }
          json ping = {{"type","ping"},{"timeSent",Util::currentTimestamp()}};
          std::string s = ping.dump();
          mg_websocket_write(const_cast<mg_connection*>(c),
                             MG_WEBSOCKET_OPCODE_TEXT, s.c_str(), s.size());
          meta.awaiting = true;
        }
      }
      
      for (auto* c : toDrop) {
        Logger::info("Unanswered ping. Closing...");
        dropConn(c);
      }
    }
  });
}


inline void stopPingThread()
{
  gKeepPinging.store(false);
  gPingCv.notify_all();
  if (gPingThread.joinable())
    gPingThread.join();
}

static std::string getHostIdFromSession(const Session &session) {
  for (mg_connection* cl : session.clientConnections) {
    auto it = connectionToInfo.find(cl);
    if (it != connectionToInfo.end() && it->second.isHost) {
      return it->second.connectionId;
    }
  }
  return "";
}

static void routeSignalingMessage(const std::string &sessionId,
                                  mg_connection* sender,
                                  const std::string &message,
                                  const std::string &targetClientId)
{
  std::lock_guard<std::mutex> lock(sessionMutex);
  
  if (targetClientId.empty()) {
    Logger::error("RouteSignalingMessage: targetClientId is empty.");
    return;
  }
  
  auto sit = sessionIdToSession.find(sessionId);
  if (sit == sessionIdToSession.end()) {
    Logger::error("Unable to find sessionId: " + sessionId);
    return;
  }
  Session &session = sit->second;
  
  mg_connection* targetConn = nullptr;
  for (mg_connection* conn : session.clientConnections) {
    auto infoIt = connectionToInfo.find(conn);
    if (infoIt != connectionToInfo.end() &&
        infoIt->second.connectionId == targetClientId) {
      targetConn = conn;
      break;
    }
  }
  
  if (targetConn && targetConn != sender) {
    mg_websocket_write(targetConn,
                       MG_WEBSOCKET_OPCODE_TEXT,
                       message.c_str(),
                       message.size());
  } else {
    Logger::error("Target connection not found or target equals sender.");
  }
}

int websocketConnectHandler(const struct mg_connection *conn, void *cbdata) {
  Logger::info("Received new WebSocket connection request. Accepting...");
  return 0;
}

void websocketReadyHandler(struct mg_connection *conn, void *cbdata) {
  Logger::info("WebSocket handshake complete. Ready to receive messages.");
  
  {
    std::lock_guard<std::mutex> lock(sessionMutex);
    ConnectionInfo info;
    info.connectionId = Util::generateRandomId();
    connectionToInfo[conn] = info;
  }
  
  startPingThread();
}

int websocketDataHandler(struct mg_connection *conn,
                         int bits,
                         char *data,
                         size_t len,
                         void *cbdata) {
  bool isText = ((bits & 0x0F) == MG_WEBSOCKET_OPCODE_TEXT);
  if (!isText) {
    Logger::info("Received non-text frame (binary?) - ignoring.");
    return 1;
  }
  
  std::string message(data, len);
  Logger::verbose("Received message: " + message);
  
  try {
    json j = json::parse(message);
    std::string type = j.value("type", "");
    
    ConnectionInfo connInfo;
    {
      std::lock_guard<std::mutex> lock(sessionMutex);
      auto it = connectionToInfo.find(conn);
      if (it == connectionToInfo.end()) {
        Logger::error("Connection has no ConnectionInfo! Ignoring...");
        return 1;
      }
      connInfo = it->second;
    }
    
    if (type == "createSession") {
      std::string newSessionId = "1";
      {
        std::lock_guard<std::mutex> lock(sessionMutex);
        connInfo.sessionId = newSessionId;
        connInfo.isHost = true;
        connectionToInfo[conn] = connInfo;
        Session newSession;
        newSession.clientConnections.push_back(conn);
        sessionIdToSession[newSessionId] = newSession;
      }
      
      json resp = {
        {"type", "sessionCreated"},
        {"payload", {
          {"sessionId", newSessionId},
          {"role", "host"}
        }},
        {"timeSent", Util::currentTimestamp()}
      };
      std::string respStr = resp.dump();
      mg_websocket_write(conn, MG_WEBSOCKET_OPCODE_TEXT,
                         respStr.c_str(), respStr.size());
    }
    else if (type == "joinSession") {
      auto payload = j.value("payload", json::object());
      std::string sessionId = payload.value("sessionId", "");
      if (sessionId.empty()) {
        Logger::error("joinSession message missing sessionId in payload.");
        return 1;
      }
      
      {
        std::lock_guard<std::mutex> lock(sessionMutex);
        auto sit = sessionIdToSession.find(sessionId);
        if (sit == sessionIdToSession.end()) {
          Logger::error("Session not found: " + sessionId);
          json errResp = {
            {"type", "error"},
            {"payload", { {"message", "Session not found"} }},
            {"timeSent", Util::currentTimestamp()}
          };
          std::string errStr = errResp.dump();
          mg_websocket_write(conn, MG_WEBSOCKET_OPCODE_TEXT,
                             errStr.c_str(), errStr.size());
        } else {
          connInfo.sessionId = sessionId;
          connInfo.isHost = false;
          connectionToInfo[conn] = connInfo;
          Session &session = sit->second;
          session.clientConnections.push_back(conn);
          
          json joinResp = {
            {"type", "joinedSession"},
            {"payload", {
              {"sessionId", sessionId},
              {"role", "guest"},
              {"hostId", getHostIdFromSession(session)}
            }},
            {"timeSent", Util::currentTimestamp()}
          };
          std::string joinStr = joinResp.dump();
          mg_websocket_write(conn, MG_WEBSOCKET_OPCODE_TEXT,
                             joinStr.c_str(), joinStr.size());
        }
      }
    }
    else if (type == "offer" || type == "answer" || type == "candidate") {
      auto payload = j.value("payload", json::object());
      payload["sourceClientId"] = connInfo.connectionId;
      std::string targetClientId = payload.value("targetClientId", "");
      json forwardedMsg = {
        {"type", type},
        {"payload", payload},
        {"timeSent", Util::currentTimestamp()},
      };
      std::string updatedMsg = forwardedMsg.dump();
      routeSignalingMessage(connInfo.sessionId, conn, updatedMsg, targetClientId);
    }
    else if (type == "ping") {
      json pongMsg;
      pongMsg["type"] = "pong";
      pongMsg["timeSent"] = Util::currentTimestamp();
      std::string pongStr = pongMsg.dump();
      mg_websocket_write(conn, MG_WEBSOCKET_OPCODE_TEXT, pongStr.c_str(), pongStr.size());
    } else if (type == "pong") {
      std::lock_guard<std::mutex> lock(sessionMutex);
      pingState[conn].awaiting = false;
    }
    else {
      Logger::error("Unknown signaling type: " + type);
    }
  }
  catch (std::exception &e) {
    Logger::error(std::string("Exception parsing signaling message: ") + e.what());
    Logger::error(message);
  }
  
  return 1;
}

void websocketCloseHandler(const struct mg_connection *conn, void *cbdata) {
  Logger::info("WebSocket connection closed.");
  
  std::lock_guard<std::mutex> lock(sessionMutex);
  
  auto it = connectionToInfo.find(conn);
  if (it != connectionToInfo.end()) {
    ConnectionInfo cinfo = it->second;
    std::string sessionId = cinfo.sessionId;
    connectionToInfo.erase(it);
    if (!sessionId.empty()) {
      auto sit = sessionIdToSession.find(sessionId);
      if (sit != sessionIdToSession.end()) {
        Session &session = sit->second;
        auto &clientConns = session.clientConnections;
        clientConns.erase(std::remove(clientConns.begin(), clientConns.end(), conn),
                          clientConns.end());
        if (clientConns.empty()) {
          sessionIdToSession.erase(sit);
        }
      }
    }
  }
}

}


