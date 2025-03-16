#pragma once

#include "vendor/civetweb/civetweb.h"
#include <string>
#include <functional>
#include <unordered_map>
#include <iostream>
#include "src/Utils/NetworkUtils.h"
#include "src/WebServer/IndexRoute.h"
#include "src/WebServer/SignallingRoute.h"

class WebServer
{
public:
  using RequestHandler = std::function<std::string(const std::string& uri)>;
  
  WebServer()
  : mCtx(nullptr)
  , preferredPort(57441)
  , currentPort(-1)
  {}
  
  ~WebServer()
  {
    stop();
  }
  
  bool start()
  {
    int port = NetworkUtils::FindAvailablePort(preferredPort);
    currentPort = port;
    std:: string portString = std::to_string(port);
    
    Boolean hasAccess = verifyPortAccess();
    if (!hasAccess) {
      return false;
    }
    
    const char* options[] = {
      "num_threads", "4",
      "listening_ports", portString.c_str(),
      "enable_directory_listing", "no",
      nullptr
    };
    
    mg_init_library(0);
    mCtx = mg_start(nullptr, nullptr, options);
    
    if (!mCtx)
    {
      std::cerr << "Failed to start CivetWeb server." << std::endl;
      return false;
    }
    
    mg_set_request_handler(mCtx, "/", &IndexRoute::handleRequest, this);
    mg_set_request_handler(mCtx, "/", &SignallingRoute::handleRequest, this);
    
    return true;
  }
  
  void stop()
  {
    if (mCtx)
    {
      mg_stop(mCtx);
      mCtx = nullptr;
      currentPort = -1;
    }
  }
  
  int getPort() const
  {
    if (mCtx)
    {
      mg_server_ports ports[16];
      int size = 16;
      mg_get_server_ports(mCtx, size, ports);
      return ports->port;
    }
    return -1;
  }
  
  std::string getFullUrl()
  {
    return "http://" + NetworkUtils::GetPrimaryLocalIPAddress() + ":" + std::to_string(getPort());
  }
  
private:
  mg_context* mCtx;
  std::unordered_map<std::string, RequestHandler> mRoutes;
  int preferredPort;
  int currentPort;
  
  bool verifyPortAccess() {
    // at this point, we know that the port is available on the host OS
    // we still need to verify:
    // has firewall access been granted to local devices for this port?
    // reachable from local devices
    // THIS IS THE ENTRYPOINT FOR WINDOWS FIREWALL BS
    return true;
  }
  
  // The request callback used in 'start()'
  static int handleRequest(mg_connection* conn, void* cbdata)
  {
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Access-Control-Allow-Origin: *\r\n"
              "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
              "Content-Type: text/html\r\n"
              "Content-Length: %u\r\n\r\n",
              RECEIVER_UI_length);
    
    mg_write(conn, RECEIVER_UI, RECEIVER_UI_length);
    return 1;
  }
};
