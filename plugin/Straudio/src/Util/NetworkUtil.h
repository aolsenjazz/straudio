#pragma once

#include <string>
#include <vector>
#include <stdexcept>

#if defined(_WIN32)
    #include <winsock2.h>
    #include <iphlpapi.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "iphlpapi.lib")
#else
    #include <ifaddrs.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <net/if.h>
    #include <unistd.h>
#endif

namespace NetworkUtils {

inline std::vector<std::string> GetLocalIPAddresses() {
    std::vector<std::string> addresses;

#if defined(_WIN32)
    // Windows implementation
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        throw std::runtime_error("Failed to initialize Winsock");
    
    PIP_ADAPTER_ADDRESSES adapterAddresses = nullptr;
    ULONG outBufLen = 0;
    
    if (GetAdaptersAddresses(AF_UNSPEC, 0, nullptr, adapterAddresses, &outBufLen) == ERROR_BUFFER_OVERFLOW) {
        adapterAddresses = (PIP_ADAPTER_ADDRESSES)malloc(outBufLen);
        if (!adapterAddresses)
            throw std::runtime_error("Memory allocation failed for adapter addresses");
        
        if (GetAdaptersAddresses(AF_UNSPEC, 0, nullptr, adapterAddresses, &outBufLen) == NO_ERROR) {
            for (auto adapter = adapterAddresses; adapter != nullptr; adapter = adapter->Next) {
                if (adapter->OperStatus == IfOperStatusUp) {
                    for (auto addr = adapter->FirstUnicastAddress; addr != nullptr; addr = addr->Next) {
                        if (addr->Address.lpSockaddr->sa_family == AF_INET) {
                            sockaddr_in* sa = reinterpret_cast<sockaddr_in*>(addr->Address.lpSockaddr);
                            char ip[INET_ADDRSTRLEN];
                            inet_ntop(AF_INET, &(sa->sin_addr), ip, INET_ADDRSTRLEN);
                            if (strcmp(ip, "127.0.0.1") != 0)
                                addresses.push_back(ip);
                        }
                    }
                }
            }
        }
        free(adapterAddresses);
    }
    
    WSACleanup();
#else
    // macOS/Linux implementation
    struct ifaddrs* ifaddr = nullptr;
    if (getifaddrs(&ifaddr) == -1)
        throw std::runtime_error("getifaddrs failed");
    
    for (struct ifaddrs* ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr || ifa->ifa_addr->sa_family != AF_INET)
            continue;
        void* sin_addr = &((struct sockaddr_in*)ifa->ifa_addr)->sin_addr;
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, sin_addr, ip, INET_ADDRSTRLEN);
        if (strcmp(ip, "127.0.0.1") != 0 && (ifa->ifa_flags & IFF_RUNNING))
            addresses.push_back(ip);
    }
    
    freeifaddrs(ifaddr);
#endif

    return addresses;
}

inline std::string GetPrimaryLocalIPAddress() {
    auto addresses = GetLocalIPAddresses();
    if (addresses.empty())
        throw std::runtime_error("No non-loopback IPv4 addresses found");
    return addresses.front();
}

inline int FindAvailablePort(int startPort) {
    for (int port = startPort; port < 65535; ++port) {
#if defined(_WIN32)
        SOCKET sockfd = ::socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd == INVALID_SOCKET)
            continue;
        int reuseVal = 1;
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuseVal, sizeof(reuseVal)) == SOCKET_ERROR) {
            closesocket(sockfd);
            continue;
        }
#else
        int sockfd = ::socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd == -1)
            continue;
        int reuseVal = 1;
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuseVal, sizeof(reuseVal)) == -1) {
            close(sockfd);
            continue;
        }
#endif
        sockaddr_in sa;
        sa.sin_family = AF_INET;
        sa.sin_port = htons(port);
        sa.sin_addr.s_addr = INADDR_ANY;
#if defined(_WIN32)
        if (::bind(sockfd, reinterpret_cast<struct sockaddr*>(&sa), sizeof(sa)) != SOCKET_ERROR) {
            closesocket(sockfd);
            return port;
        }
        closesocket(sockfd);
#else
        if (::bind(sockfd, reinterpret_cast<struct sockaddr*>(&sa), sizeof(sa)) == 0) {
            close(sockfd);
            return port;
        }
        close(sockfd);
#endif
    }
    return -1; // No available port found.
}

} // namespace NetworkUtils
