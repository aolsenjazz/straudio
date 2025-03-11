#pragma once

#include "vendor/civetweb/civetweb.h"
#include "src/Utils/NetworkUtils.h"
#include "src/Utils/ResourceUtils.h"
#include <string>
#include <locale>
#include <codecvt>
#include <functional>
#include <unordered_map>
#include <filesystem>
#include "src/Utils/Logger.h"
#include <iostream>

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include "wdlstring.h"
#include "src/webclient.hpp"
#include "IPlugUtilities.h"

extern HMODULE g_hModule;

// DllMain.cpp

class WebServer {
public:
    using RequestHandler = std::function<std::string(const std::string& uri)>;

    WebServer() : mCtx(nullptr), preferredPort(8080), currentPort(-1) {}

    ~WebServer() {
        stop();
    }

    bool start() {
        Logger::info("start!");
        int port = findAvailablePort(preferredPort);
        //if (port == -1) {
        //    Logger::error("No available ports found. Could not start the server.");
        //    return false;
        //}
        Logger::info("sanity");
        Logger::info(std::to_string(port));

        currentPort = port;
        std::string portString = std::to_string(port);



        const char* options[] = {
            "num_threads", "4",
            "listening_ports", "0.0.0.0:9000",
            "enable_directory_listing", "no",
            NULL
        };

        mg_init_library(0);
        mCtx = mg_start(nullptr, nullptr, options);
        
        mg_set_request_handler(
            mCtx,
            "/",
            +[](mg_connection* conn, void* /*userData*/) -> int
            {

                Logger::info("haha hey");
                // 2. Serve it back as text/html
                mg_printf(conn,
                    "HTTP/1.1 200 OK\r\n"
                    "Content-Type: text/html\r\n"
                    "Content-Length: %zu\r\n\r\n",
                    419);

                mg_write(conn, CLIENT_FN, CLIENT_FN_length);
                return 1;
            },
            nullptr);

        return mCtx != nullptr;
    }

    void stop() {
        if (mCtx) {
            mg_stop(mCtx);
            mCtx = nullptr;
            currentPort = -1;
        }
    }

    void addRoute(const std::string& uri, RequestHandler handler) {
        mRoutes[uri] = handler;
    }

    int getPort() const {
        if (this->mCtx) {
            int size = 9;
            mg_server_ports ports[16];
            mg_get_server_ports(this->mCtx, size, ports);

            return ports->port;
        }
        else {
            return -1;
        }
        
    }

    std::string getFullUrl() {
        return "http://" + NetworkUtils::GetPrimaryLocalIPAddress() + ":" + std::to_string(getPort());
    }

    static std::vector<std::string> GetLocalIPAddresses() {
#ifdef _WIN32
        WSADATA wsaData;
        int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if (result != 0) {
            throw std::runtime_error("Failed to initialize Winsock (WSAStartup). Error code: " + std::to_string(result));

        }
#endif

        std::vector<std::string> addresses;


#if defined(_WIN32)
        // Windows implementation
        PIP_ADAPTER_ADDRESSES adapterAddresses = nullptr;
        ULONG outBufLen = 0;

        if (GetAdaptersAddresses(AF_UNSPEC, 0, nullptr, adapterAddresses, &outBufLen) == ERROR_BUFFER_OVERFLOW) {
            adapterAddresses = (PIP_ADAPTER_ADDRESSES)malloc(outBufLen);
            if (!adapterAddresses) throw std::runtime_error("Memory allocation failed");

            if (GetAdaptersAddresses(AF_UNSPEC, 0, nullptr, adapterAddresses, &outBufLen) == NO_ERROR) {
                for (auto adapter = adapterAddresses; adapter != nullptr; adapter = adapter->Next) {
                    if (adapter->OperStatus == IfOperStatusUp) {
                        for (auto addr = adapter->FirstUnicastAddress; addr != nullptr; addr = addr->Next) {
                            if (addr->Address.lpSockaddr->sa_family == AF_INET) {
                                sockaddr_in* sa = (sockaddr_in*)addr->Address.lpSockaddr;
                                char ip[INET_ADDRSTRLEN];
                                inet_ntop(AF_INET, &(sa->sin_addr), ip, INET_ADDRSTRLEN);

                                if (strcmp(ip, "127.0.0.1") != 0) {
                                    addresses.push_back(ip);
                                }
                            }
                        }
                    }
                }
            }
            free(adapterAddresses);
        }

#else
        // macOS/Linux implementation
        struct ifaddrs* ifaddr = nullptr;
        if (getifaddrs(&ifaddr) == -1) {
            throw std::runtime_error("getifaddrs failed");
        }

        for (struct ifaddrs* ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
            if (!ifa->ifa_addr || ifa->ifa_addr->sa_family != AF_INET) continue;

            void* sin_addr = &((struct sockaddr_in*)ifa->ifa_addr)->sin_addr;
            char ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, sin_addr, ip, INET_ADDRSTRLEN);

            if (strcmp(ip, "127.0.0.1") != 0 && (ifa->ifa_flags & IFF_RUNNING)) {
                addresses.push_back(ip);
            }
        }

        freeifaddrs(ifaddr);
#endif

        return addresses;
    }

    static std::string GetPrimaryLocalIPAddress() {
        auto addresses = GetLocalIPAddresses();
        if (addresses.empty()) {
            throw std::runtime_error("No non-loopback IPv4 addresses found");
        }
        return addresses[0];
    }

private:
    mg_context* mCtx;
    std::unordered_map<std::string, RequestHandler> mRoutes;
    int preferredPort;
    int currentPort;

    static int handleRequest(mg_connection* conn, void* cbdata) {
        WebServer* self = static_cast<WebServer*>(cbdata);
        const mg_request_info* req = mg_get_request_info(conn);

        auto it = self->mRoutes.find(req->local_uri);
        if (it != self->mRoutes.end()) {
            std::string response = it->second(req->local_uri);
            mg_printf(conn, "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n%s", response.c_str());
            return 1;
        }

        return 0; // Let CivetWeb handle static files
    }

    int findAvailablePort(int startPort) {
        for (int port = startPort; port < 65535; ++port) {

#ifdef _WIN32
            // On Windows, SOCKET is unsigned; INVALID_SOCKET is the error indicator.
            SOCKET sockfd = ::socket(AF_INET, SOCK_STREAM, 0);
            if (sockfd == INVALID_SOCKET) {
                int wsaErr = WSAGetLastError();
                std::cerr << "socket() failed with error " << wsaErr << "\n";
                continue;
            }

            // Enable address reuse
            int reuseVal = 1;
            if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuseVal, sizeof(reuseVal)) == SOCKET_ERROR) {
                int wsaErr = WSAGetLastError();
                std::cerr << "setsockopt(SO_REUSEADDR) failed with error " << wsaErr << "\n";
                closesocket(sockfd);
                continue;
            }
#else
            int sockfd = ::socket(AF_INET, SOCK_STREAM, 0);
            if (sockfd == -1) {
                std::cerr << "socket() failed on port " << port << "\n";
                continue;
            }

            // Enable address reuse on Unix-like
            int reuseVal = 1;
            if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuseVal, sizeof(reuseVal)) == -1) {
                perror("setsockopt(SO_REUSEADDR) failed");
                close(sockfd);
                continue;
            }
#endif

            // Fill out the sockaddr_in structure
            sockaddr_in sa;
            sa.sin_family = AF_INET;
            sa.sin_port = htons(port);
            sa.sin_addr.s_addr = INADDR_ANY;

            // Try binding
            if (
#ifdef _WIN32
                ::bind(sockfd, (struct sockaddr*)&sa, sizeof(sa)) != SOCKET_ERROR
#else
                ::bind(sockfd, (struct sockaddr*)&sa, sizeof(sa)) == 0
#endif
                ) {
                // Successfully bound; close and return the port
#ifdef _WIN32
                closesocket(sockfd);
#else
                close(sockfd);
#endif
                return port;
            }
            else {
                // Binding failed; retrieve and log error
#ifdef _WIN32
                int wsaErr = WSAGetLastError();
                std::cerr << "bind() failed on port " << port
                    << " with error " << wsaErr << "\n";
                closesocket(sockfd);
#else
                perror("bind() failed");
                close(sockfd);
#endif
            }
            }

        // If we reach here, no port was available in the range
        return -1;
        }
};

