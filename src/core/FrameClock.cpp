#include "FrameClock.h"

namespace Nexus {

namespace {
/// One FPS reporting window.
constexpr float kFPSWindowSeconds = 1.0f;
}

FrameClock::FrameClock(float targetFPS)
    : targetFPS_(targetFPS > 0.0f ? targetFPS : 0.0f) {
}

void FrameClock::SetTargetFPS(float fps) {
    // Anything at or below zero means "do not cap", so it is stored as exactly
    // zero and GetSleepMilliseconds never divides by it.
    targetFPS_ = fps > 0.0f ? fps : 0.0f;
}

void FrameClock::SetMaxDeltaTime(float seconds) {
    if (seconds > 0.0f) {
        maxDeltaTime_ = seconds;
    }
}

void FrameClock::Advance(float deltaSeconds) {
    if (!(deltaSeconds > 0.0f)) {
        // Catches negatives and NaN: !(x > 0) is true for NaN, where
        // (x < 0) would be false and let it through into physics.
        deltaSeconds = 0.0f;
    } else if (deltaSeconds > maxDeltaTime_) {
        deltaSeconds = maxDeltaTime_;
    }

    deltaTime_ = deltaSeconds;
    ++frameCount_;

    // FPS is measured against the clamped delta deliberately: it describes the
    // rate the loop is running the simulation at, which is what the cap makes
    // it, not the wall-clock rate it fell behind to.
    fpsAccumulator_ += deltaSeconds;
    ++fpsFrameCount_;

    if (fpsAccumulator_ >= kFPSWindowSeconds) {
        fps_ = static_cast<int>(static_cast<float>(fpsFrameCount_) / fpsAccumulator_ + 0.5f);
        fpsAccumulator_ = 0.0f;
        fpsFrameCount_ = 0;
    }
}

int FrameClock::GetSleepMilliseconds(float frameWorkSeconds) const {
    if (targetFPS_ <= 0.0f) {
        return 0;
    }

    const float targetFrameTime = 1.0f / targetFPS_;
    const float remaining = targetFrameTime - frameWorkSeconds;
    if (!(remaining > 0.0f)) {
        return 0;
    }

    // Truncating rather than rounding: oversleeping by up to a millisecond
    // every frame would cap the loop below the requested rate.
    const int milliseconds = static_cast<int>(remaining * 1000.0f);
    return milliseconds > 0 ? milliseconds : 0;
}

void FrameClock::Reset() {
    deltaTime_ = 0.0f;
    fpsAccumulator_ = 0.0f;
    fpsFrameCount_ = 0;
    fps_ = 0;
    frameCount_ = 0;
}

} // namespace Nexus
