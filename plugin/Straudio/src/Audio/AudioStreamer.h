#pragma once
#include <thread>
#include <condition_variable>
#include <atomic>
#include <vector>
#include <functional>
#include <cstdint>
#include <algorithm>
#include "AudioRing.hpp"

class AudioStreamer {
public:
  using PushFn = std::function<void(const int16_t*, const int16_t*, size_t)>;

  explicit AudioStreamer(PushFn cb,
                         int sr = 48000,
                         int blk = 480,
                         int ring = 16384)
  : mPush(std::move(cb)),
    mSR(sr), mBlk(blk), mRing(ring) {}

  ~AudioStreamer() { stop(); }

  void start() {
    if (mRun.exchange(true)) return;
    mTh = std::thread([this] { run(); });
  }

  void stop() {
    if (!mRun.exchange(false)) return;
    mCv.notify_all();
    if (mTh.joinable()) mTh.join();
  }

  void writeAudio(const float* L, const float* R, size_t n) noexcept {
    mRing.write(L, R, n);
    mCv.notify_one();
  }

private:
  void run() {
    std::vector<float>   fL(mBlk), fR(mBlk);
    std::vector<int16_t> iL(mBlk), iR(mBlk);

    while (mRun) {
      {
        std::unique_lock lk(mMx);
        mCv.wait(lk, [&] { return !mRun || mRing.available() >= mBlk; });
        if (!mRun) break;
      }
      mRing.read(fL.data(), fR.data(), mBlk);
      for (int i = 0; i < mBlk; ++i) {
        iL[i] = static_cast<int16_t>(
                  std::clamp(fL[i] * 32767.f, -32768.f, 32767.f));
        iR[i] = static_cast<int16_t>(
                  std::clamp(fR[i] * 32767.f, -32768.f, 32767.f));
      }
      mPush(iL.data(), iR.data(), mBlk);
    }
  }

  PushFn                       mPush;
  const int                    mSR, mBlk;
  AudioRing                    mRing;
  std::atomic<bool>            mRun{false};
  std::thread                  mTh;
  std::condition_variable_any  mCv;
  std::mutex                   mMx;
};
