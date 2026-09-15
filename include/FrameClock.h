#pragma once

#include <cstdint>

namespace Nexus {

/**
 * Per-frame timing policy: the delta time handed to the subsystems, a
 * once-a-second FPS counter, and the frame-pacing calculation.
 *
 * It is *fed* deltas rather than reading a clock itself. Keeping the policy
 * (how dt is clamped, when FPS updates, how long to sleep) separate from the
 * measurement is what makes it testable: a test can push a stream of deltas
 * through it and assert on the result without sleeping, and Engine::Run stays
 * a thin loop over Timer::Tick().
 */
class FrameClock {
public:
    /// Frame-rate cap in frames per second; 0 or below means uncapped.
    explicit FrameClock(float targetFPS = 0.0f);

    void SetTargetFPS(float fps);
    float GetTargetFPS() const { return targetFPS_; }

    /// Largest delta ever reported to the rest of the engine, in seconds.
    ///
    /// A frame that stalls - a breakpoint, a window drag, a slow asset load -
    /// would otherwise hand physics a multi-second dt and push every moving
    /// object straight through the geometry it should have collided with.
    /// Clamping trades a momentary slowdown for staying simulable.
    void SetMaxDeltaTime(float seconds);
    float GetMaxDeltaTime() const { return maxDeltaTime_; }

    /// Records one completed frame. `deltaSeconds` is the raw measured
    /// interval; GetDeltaTime() reports it clamped to the maximum above.
    /// Negative deltas are treated as zero - a clock should never hand back
    /// time, but the loop must not propagate it if one ever does.
    void Advance(float deltaSeconds);

    /// Seconds since the previous frame, clamped. Zero before the first
    /// Advance(), so a subsystem read during startup sees a still world
    /// rather than an uninitialised value.
    float GetDeltaTime() const { return deltaTime_; }

    /// Frames per second over the last completed one-second window, or 0 until
    /// a full second has been accumulated.
    int GetFPS() const { return fps_; }

    /// Total frames recorded since construction or the last Reset().
    std::uint64_t GetFrameCount() const { return frameCount_; }

    /// Whole milliseconds to sleep so that a frame whose work took
    /// `frameWorkSeconds` still lands on the target frame time. Returns 0 when
    /// capping is disabled or the frame has already overrun its budget.
    int GetSleepMilliseconds(float frameWorkSeconds) const;

    /// Clears delta, FPS and frame count. The target FPS and maximum delta are
    /// configuration, not state, so they survive.
    void Reset();

private:
    float targetFPS_;
    float maxDeltaTime_ = 0.25f;   ///< 4 FPS: slow, but still a simulable step.

    float deltaTime_ = 0.0f;

    // FPS is counted over a sliding one-second window rather than smoothed per
    // frame, so the number on screen is a rate that was actually achieved.
    float fpsAccumulator_ = 0.0f;
    int fpsFrameCount_ = 0;
    int fps_ = 0;

    std::uint64_t frameCount_ = 0;
};

} // namespace Nexus
