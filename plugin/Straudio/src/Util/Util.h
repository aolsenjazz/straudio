#pragma once

#include <string>
#include <random>
#include <chrono>

namespace Util {

/**
 * Generates a random ID of the specified length.
 */
inline std::string generateRandomId(size_t length = 8) {
  static const char* chars =
  "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
  thread_local static std::mt19937 rg{ std::random_device{}() };
  thread_local static std::uniform_int_distribution<size_t> pick(0, 61);
  
  std::string s;
  s.reserve(length);
  for (size_t i = 0; i < length; i++) {
    s.push_back(chars[pick(rg)]);
  }
  return s;
}

/**
 * Returns the current timestamp in milliseconds.
 */
inline long long currentTimestamp() {
  auto now = std::chrono::system_clock::now();
  return std::chrono::duration_cast<std::chrono::milliseconds>(
                                                               now.time_since_epoch())
  .count();
}

inline void parseWebSocketUrl(const std::string &url,
                              std::string &host,
                              int &port,
                              std::string &path) {
  constexpr const char *kPrefix = "ws://";
  if (url.rfind(kPrefix, 0) != 0)
    throw std::invalid_argument("URL must begin with ws://");
  
  size_t pos   = std::strlen(kPrefix);
  size_t colon = url.find(':', pos);
  if (colon == std::string::npos)
    throw std::invalid_argument("URL must contain : after host");
  
  host = url.substr(pos, colon - pos);
  size_t slash = url.find('/', colon + 1);
  if (slash == std::string::npos)
    throw std::invalid_argument("URL must contain / after port");
  
  port = std::stoi(url.substr(colon + 1, slash - colon - 1));
  path = url.substr(slash);
}


}

