#pragma once
#include <atomic>
#include <vector>
#include <cstddef>
#include <algorithm>

class AudioRing {
public:
  explicit AudioRing(size_t framesPow2 = 16384)
  : mMask(framesPow2 - 1),
    mL(framesPow2), mR(framesPow2) {}

  size_t write(const float* L, const float* R, size_t n) noexcept {
    size_t w = mWrite.load(std::memory_order_relaxed);
    size_t space = mMask + 1 - (w - mRead.load(std::memory_order_acquire));
    n = std::min(n, space);
    for (size_t i = 0; i < n; ++i) {
      size_t idx = (w + i) & mMask;
      mL[idx] = L[i];
      mR[idx] = R[i];
    }
    mWrite.store(w + n, std::memory_order_release);
    return n;
  }

  size_t read(float* L, float* R, size_t n) noexcept {
    size_t r = mRead.load(std::memory_order_relaxed);
    size_t avail = mWrite.load(std::memory_order_acquire) - r;
    n = std::min(n, avail);
    for (size_t i = 0; i < n; ++i) {
      size_t idx = (r + i) & mMask;
      L[i] = mL[idx];
      R[i] = mR[idx];
    }
    mRead.store(r + n, std::memory_order_release);
    return n;
  }

  size_t available() const noexcept {
    return mWrite.load(std::memory_order_acquire) -
           mRead.load(std::memory_order_acquire);
  }
private:
  const size_t            mMask;
  std::vector<float>      mL, mR;
  std::atomic<size_t>     mWrite{0}, mRead{0};
};
