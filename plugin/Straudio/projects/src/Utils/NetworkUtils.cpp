#include "NetworkUtils.h"
#include <stdexcept>

#if defined(_WIN32)
#include <winsock2.h>
#include <iphlpapi.h>
#pragma comment(lib, "iphlpapi.lib")
#else
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>  // Include for IFF_RUNNING on Linux/macOS
#endif

namespace NetworkUtils {

std::vector<std::string> GetLocalIPAddresses() {
    std::vector<std::string> addresses;

#if defined(_WIN32)
    // Windows implementation
    PIP_ADAPTER_ADDRESSES adapterAddresses = nullptr;
    ULONG outBufLen = 0;

    // First call to get buffer size
    if(GetAdaptersAddresses(AF_UNSPEC, 0, nullptr, adapterAddresses, &outBufLen) == ERROR_BUFFER_OVERFLOW) {
        adapterAddresses = (PIP_ADAPTER_ADDRESSES)malloc(outBufLen);
        if(!adapterAddresses) throw std::runtime_error("Memory allocation failed");

        // Second call to get actual data
        if(GetAdaptersAddresses(AF_UNSPEC, 0, nullptr, adapterAddresses, &outBufLen) == NO_ERROR) {
            for(auto adapter = adapterAddresses; adapter != nullptr; adapter = adapter->Next) {
                if(adapter->OperStatus == IfOperStatusUp) {
                    for(auto addr = adapter->FirstUnicastAddress; addr != nullptr; addr = addr->Next) {
                        if(addr->Address.lpSockaddr->sa_family == AF_INET) {
                            sockaddr_in* sa = (sockaddr_in*)addr->Address.lpSockaddr;
                            char ip[INET_ADDRSTRLEN];
                            inet_ntop(AF_INET, &(sa->sin_addr), ip, INET_ADDRSTRLEN);

                            if(strcmp(ip, "127.0.0.1") != 0) {
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
    if(getifaddrs(&ifaddr) == -1) {
        throw std::runtime_error("getifaddrs failed");
    }

    for(struct ifaddrs* ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
        if(!ifa->ifa_addr || ifa->ifa_addr->sa_family != AF_INET) continue;

        void* sin_addr = &((struct sockaddr_in*)ifa->ifa_addr)->sin_addr;
        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, sin_addr, ip, INET_ADDRSTRLEN);

        // Check if the interface is running, only for non-loopback addresses
        if(strcmp(ip, "127.0.0.1") != 0 && (ifa->ifa_flags & IFF_RUNNING)) {
            addresses.push_back(ip);
        }
    }

    freeifaddrs(ifaddr);
#endif

    return addresses;
}

std::string GetPrimaryLocalIPAddress() {
    auto addresses = GetLocalIPAddresses();
    if(addresses.empty()) {
        throw std::runtime_error("No non-loopback IPv4 addresses found");
    }
    return addresses[0]; // Return first available address
}

} // namespace NetworkUtils
