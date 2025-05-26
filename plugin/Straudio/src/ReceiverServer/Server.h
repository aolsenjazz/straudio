#pragma once

#include "vendor/civetweb/civetweb.h"
#include <string>
#include <functional>
#include <unordered_map>
#include <iostream>
#include "src/Util/NetworkUtil.h"
#include "src/ReceiverServer/Routes.h"

class ReceiverServer
{
public:
  using RequestHandler = std::function<std::string(const std::string& uri)>;

  ReceiverServer()
    : mCtx(nullptr)
    , preferredPort(57441)
    , currentPort(-1)
  {}

  ~ReceiverServer()
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
      std::cerr << "Failed to start FrontendServer." << std::endl;
      return false;
    }

    // Only register the index (frontend) handler.
    mg_set_request_handler(mCtx, "/", &IndexRoute::handleRequest, this);

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
  int preferredPort;
  int currentPort;

  bool verifyPortAccess() {
    // Check firewall or OS access if needed.
    // This is where the Windows Firewall BS goes
    return true;
  }
};
