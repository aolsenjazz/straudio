// WebServer.h
#pragma once
#include "civetweb.h"
#include <string>
#include <functional>
#include <unordered_map>

class WebServer {
public:
  using RequestHandler = std::function<std::string(const std::string& uri)>;
  
  WebServer();
  ~WebServer();
  
  bool start(const std::string& webRoot = "");
  void stop();
  
  void addRoute(const std::string& uri, RequestHandler handler);
  void addStaticRoute(const std::string& uri, const std::string& filePath);
  
  int getPort() const;  // New function to get the port the server is using
  std::string getFullUrl();

private:
  mg_context* mCtx;
  std::unordered_map<std::string, RequestHandler> mRoutes;
  int preferredPort;  // The preferred port to start the server on
  int currentPort;    // The actual port the server is using
  
  static int handleRequest(mg_connection* conn, void* cbdata);
  
  int findAvailablePort(int startPort);  // Function to find an available port
};
