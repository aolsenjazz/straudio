#pragma once
#include <string>
#include <vector>

namespace NetworkUtils {
    // Get all non-loopback IPv4 addresses
    std::vector<std::string> GetLocalIPAddresses();
    
    // Get the first non-loopback IPv4 address
    std::string GetPrimaryLocalIPAddress();
}
