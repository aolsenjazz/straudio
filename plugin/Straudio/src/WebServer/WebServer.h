#pragma once

#include "vendor/civetweb/civetweb.h"
#include <string>
#include <functional>
#include <unordered_map>
#include <iostream>
#include "src/ReceiverUI/ReceiverUI.hpp"

// Only include WinSock and IP helper APIs on Windows
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")
#else
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <unistd.h>
#endif

class WebServer
{
public:
  using RequestHandler = std::function<std::string(const std::string& uri)>;
  
  WebServer()
  : mCtx(nullptr)
  , preferredPort(8080)
  , currentPort(-1)
  {}
  
  ~WebServer()
  {
    stop();
  }
  
  bool start()
  {
    int port = findAvailablePort(preferredPort);
    currentPort = port;
    
    const char* options[] = {
      "num_threads", "4",
      "listening_ports", "0.0.0.0:9000",
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
    
    // Use our private static callback for the root path
    mg_set_request_handler(mCtx, "/", &WebServer::handleRequest, this);
    
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
  
  void addRoute(const std::string& uri, RequestHandler handler)
  {
    mRoutes[uri] = handler;
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
  
  // For convenience, return something like "http://192.168.x.x:9000"
  std::string getFullUrl()
  {
    return "http://" + GetPrimaryLocalIPAddress() + ":" + std::to_string(getPort());
  }
  
  // Retrieve the first non-loopback local IPv4 address
  static std::string GetPrimaryLocalIPAddress()
  {
    auto addresses = GetLocalIPAddresses();
    if (addresses.empty())
    {
      throw std::runtime_error("No non-loopback IPv4 addresses found");
    }
    return addresses.front();
  }
  
private:
  mg_context* mCtx;
  std::unordered_map<std::string, RequestHandler> mRoutes;
  int preferredPort;
  int currentPort;
  
  // The request callback used in 'start()'
  static int handleRequest(mg_connection* conn, void* cbdata)
  {
    auto* self = static_cast<WebServer*>(cbdata);
    
    // If you have multiple routes, you can look them up here
    // using req->local_uri and self->mRoutes
    // For now, just serve the embedded HTML file
    
    mg_printf(conn,
              "HTTP/1.1 200 OK\r\n"
              "Content-Type: text/html\r\n"
              "Content-Length: %u\r\n\r\n",
              RECEIVER_UI_length);
    
    mg_write(conn, RECEIVER_UI, RECEIVER_UI_length);
    return 1;
  }
  
  static std::vector<std::string> GetLocalIPAddresses()
  {
#ifdef _WIN32
    // Initialize WinSock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
      throw std::runtime_error("Failed to initialize Winsock");
    }
    
    std::vector<std::string> addresses;
    PIP_ADAPTER_ADDRESSES adapterAddresses = nullptr;
    ULONG outBufLen = 0;
    
    // First call to determine buffer size
    if (GetAdaptersAddresses(AF_UNSPEC, 0, nullptr, adapterAddresses, &outBufLen) == ERROR_BUFFER_OVERFLOW)
    {
      adapterAddresses = (PIP_ADAPTER_ADDRESSES)malloc(outBufLen);
      if (!adapterAddresses)
      {
        WSACleanup();
        throw std::runtime_error("Memory allocation failed for adapter addresses");
      }
      
      if (GetAdaptersAddresses(AF_UNSPEC, 0, nullptr, adapterAddresses, &outBufLen) == NO_ERROR)
      {
        for (auto adapter = adapterAddresses; adapter; adapter = adapter->Next)
        {
          if (adapter->OperStatus == IfOperStatusUp)
          {
            for (auto addr = adapter->FirstUnicastAddress; addr; addr = addr->Next)
            {
              if (addr->Address.lpSockaddr->sa_family == AF_INET)
              {
                auto sa = reinterpret_cast<sockaddr_in*>(addr->Address.lpSockaddr);
                char ip[INET_ADDRSTRLEN];
                inet_ntop(AF_INET, &(sa->sin_addr), ip, INET_ADDRSTRLEN);
                
                if (strcmp(ip, "127.0.0.1") != 0)
                {
                  addresses.push_back(ip);
                }
              }
            }
          }
        }
      }
      free(adapterAddresses);
    }
    
    WSACleanup();
    return addresses;
#else
    // macOS/Linux implementation
    std::vector<std::string> addresses;
    struct ifaddrs* ifaddr = nullptr;
    
    if (getifaddrs(&ifaddr) == -1)
    {
      throw std::runtime_error("getifaddrs failed");
    }
    
    for (auto* ifa = ifaddr; ifa; ifa = ifa->ifa_next)
    {
      if (!ifa->ifa_addr || ifa->ifa_addr->sa_family != AF_INET) continue;
      
      void* sin_addr = &((struct sockaddr_in*)ifa->ifa_addr)->sin_addr;
      char ip[INET_ADDRSTRLEN];
      inet_ntop(AF_INET, sin_addr, ip, INET_ADDRSTRLEN);
      
      if (strcmp(ip, "127.0.0.1") != 0 && (ifa->ifa_flags & IFF_RUNNING))
      {
        addresses.push_back(ip);
      }
    }
    
    freeifaddrs(ifaddr);
    return addresses;
#endif
  }
  
  // Search for an available port, starting from 'startPort'
  int findAvailablePort(int startPort)
  {
    for (int port = startPort; port < 65535; ++port)
    {
#ifdef _WIN32
      SOCKET sockfd = ::socket(AF_INET, SOCK_STREAM, 0);
      if (sockfd == INVALID_SOCKET)
      {
        continue;
      }
      
      int reuseVal = 1;
      if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,
                     (const char*)&reuseVal, sizeof(reuseVal)) == SOCKET_ERROR)
      {
        closesocket(sockfd);
        continue;
      }
#else
      int sockfd = ::socket(AF_INET, SOCK_STREAM, 0);
      if (sockfd == -1)
      {
        continue;
      }
      
      int reuseVal = 1;
      if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,
                     &reuseVal, sizeof(reuseVal)) == -1)
      {
        close(sockfd);
        continue;
      }
#endif
      sockaddr_in sa;
      sa.sin_family = AF_INET;
      sa.sin_port = htons(port);
      sa.sin_addr.s_addr = INADDR_ANY;
      
#ifdef _WIN32
      if (::bind(sockfd, (struct sockaddr*)&sa, sizeof(sa)) != SOCKET_ERROR)
      {
        closesocket(sockfd);
        return port;
      }
      closesocket(sockfd);
#else
      if (::bind(sockfd, (struct sockaddr*)&sa, sizeof(sa)) == 0)
      {
        close(sockfd);
        return port;
      }
      close(sockfd);
#endif
    }
    
    // No ports available in range
    return -1;
  }
};
