#pragma once
#include <thread>
#include <atomic>
#include <functional>
#include <condition_variable>
#include <chrono>
#include "src/Util/Logger.h"
#include "src/Util/NetworkUtil.h"

class ConnectivityManager {
public:
  using Fn = std::function<void()>;

  ConnectivityManager(Fn onOffline, Fn onOnline, int periodSec = 1)
  : offlineCb(std::move(onOffline)),
    onlineCb (std::move(onOnline)),
    period(periodSec) {}

  ~ConnectivityManager() { stop(); }

  ConnectivityManager(const ConnectivityManager&)            = delete;
  ConnectivityManager& operator=(const ConnectivityManager&) = delete;

  void start()
  {
    if (running.exchange(true)) return;
    th = std::thread([this] { loop(); });
  }

  void stop()
  {
    if (!running.exchange(false)) return;
    cv.notify_all();
    if (th.joinable()) th.join();
  }

private:
  void loop()
  {
    bool wasOnline = false;              // assume up at start
    std::unique_lock lk(mx);
    while (running) {
      try {
        (void)NetworkUtils::GetPrimaryLocalIPAddress();
        if (!wasOnline) { onlineCb(); wasOnline = true; }
      } catch (...) {
        if (wasOnline)  { offlineCb(); wasOnline = false; }
      }
      cv.wait_for(lk, std::chrono::seconds(period),
                  [this]{ return !running.load(); });
    }
  }

  Fn                       offlineCb;
  Fn                       onlineCb;
  const int                period;
  std::thread              th;
  std::atomic<bool>        running{false};
  std::condition_variable_any cv;
  std::mutex               mx;
};
