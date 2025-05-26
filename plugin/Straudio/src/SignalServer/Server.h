#pragma once

#include "vendor/civetweb/civetweb.h"
#include <string>
#include <iostream>
#include "src/SignalServer/Routes.h"
#include "src/Util/NetworkUtil.h"

class SignalServer
{
public:
  SignalServer()
  : mCtx(nullptr)
  , preferredPort(57442) // Use a different preferred port from FrontendServer.
  , currentPort(-1)
  {}
  
  ~SignalServer()
  {
    stop();
  }
  
  bool start()
  {
    int port = NetworkUtils::FindAvailablePort(preferredPort);
    currentPort = port;
    std::string portString = std::to_string(port);
    
    if (!verifyPortAccess())
    {
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
      std::cerr << "Failed to start SignalServer." << std::endl;
      return false;
    }
    
    // Register the WebSocket handler on the /signal endpoint.
    mg_set_websocket_handler(
                             mCtx,
                             "/",
                             &SignallingRoute::websocketConnectHandler,
                             &SignallingRoute::websocketReadyHandler,
                             &SignallingRoute::websocketDataHandler,
                             &SignallingRoute::websocketCloseHandler,
                             this
                             );
    
    return true;
  }
  
  void stop()
  {
    
    SignallingRoute::gKeepPinging.store(false);
    SignallingRoute::stopPingThread();
    
    if (mCtx)
    {
      mg_stop(mCtx);
      mg_exit_library();
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
    return "ws://" + NetworkUtils::GetPrimaryLocalIPAddress() + ":" + std::to_string(getPort()) + "/";
  }
  
private:
  mg_context* mCtx;
  int preferredPort;
  int currentPort;
  
  bool verifyPortAccess() {
    // Check for firewall or OS access if needed.
    return true;
  }
};
