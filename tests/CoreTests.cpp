// ---------------------------------------------------------------------------
// Tests for the portable engine core: entity/component storage, the transform
// component, timing, and the platform layer.
// ---------------------------------------------------------------------------

#include "TestFramework.h"

#include "ECS.h"
#include "Components.h"
#include "FrameClock.h"
#include "Timer.h"
#include "Platform.h"

#include <chrono>
#include <cstdint>
#include <limits>
#include <set>
#include <thread>

using namespace Nexus;

namespace {

constexpr float kEps = 1e-4f;

// --- Entity lifecycle ------------------------------------------------------

NEXUS_TEST(EntitiesAreUniqueWhileAlive) {
    ECS ecs;
    ecs.Init();

    std::set<Entity> seen;
    for (int i = 0; i < 100; ++i) {
        const Entity e = ecs.CreateEntity();
        CHECK(seen.insert(e).second);   // insert fails on a duplicate id
    }

    CHECK_EQ(ecs.GetLivingEntityCount(), 100u);
}

NEXUS_TEST(DestroyingAnEntityDecrementsTheLivingCount) {
    ECS ecs;
    ecs.Init();

    const Entity a = ecs.CreateEntity();
    ecs.CreateEntity();
    CHECK_EQ(ecs.GetLivingEntityCount(), 2u);

    ecs.DestroyEntity(a);
    CHECK_EQ(ecs.GetLivingEntityCount(), 1u);
}

NEXUS_TEST(DestroyedEntityIdsAreRecycled) {
    // The entity pool is bounded, so ids must return to it or a long-running
    // scene eventually exhausts them.
    ECS ecs;
    ecs.Init();

    const Entity first = ecs.CreateEntity();
    ecs.DestroyEntity(first);
    ecs.CreateEntity();

    CHECK_EQ(ecs.GetLivingEntityCount(), 1u);
}

// --- Component storage -----------------------------------------------------

NEXUS_TEST(ComponentsRoundTripThroughStorage) {
    ECS ecs;
    ecs.Init();
    ecs.RegisterComponent<TransformComponent>();

    const Entity e = ecs.CreateEntity();

    TransformComponent t;
    t.position = {1.0f, 2.0f, 3.0f};
    ecs.AddComponent(e, t);

    CHECK(ecs.HasComponent<TransformComponent>(e));
    CHECK_NEAR(ecs.GetComponent<TransformComponent>(e).position.x, 1.0f, kEps);
    CHECK_NEAR(ecs.GetComponent<TransformComponent>(e).position.z, 3.0f, kEps);
}

NEXUS_TEST(ComponentsAreMutableInPlace) {
    ECS ecs;
    ecs.Init();
    ecs.RegisterComponent<TransformComponent>();

    const Entity e = ecs.CreateEntity();
    ecs.AddComponent(e, TransformComponent{});

    ecs.GetComponent<TransformComponent>(e).position.y = 42.0f;
    CHECK_NEAR(ecs.GetComponent<TransformComponent>(e).position.y, 42.0f, kEps);
}

NEXUS_TEST(RemovingAComponentClearsIt) {
    ECS ecs;
    ecs.Init();
    ecs.RegisterComponent<TransformComponent>();

    const Entity e = ecs.CreateEntity();
    ecs.AddComponent(e, TransformComponent{});
    CHECK(ecs.HasComponent<TransformComponent>(e));

    ecs.RemoveComponent<TransformComponent>(e);
    CHECK(!ecs.HasComponent<TransformComponent>(e));
}

NEXUS_TEST(ComponentsAreIndependentPerEntity) {
    // The dense component array swaps entries on removal, so it is worth
    // confirming that entity-to-index bookkeeping keeps the values together.
    ECS ecs;
    ecs.Init();
    ecs.RegisterComponent<TransformComponent>();

    const Entity a = ecs.CreateEntity();
    const Entity b = ecs.CreateEntity();
    const Entity c = ecs.CreateEntity();

    TransformComponent ta; ta.position = {1.0f, 0.0f, 0.0f};
    TransformComponent tb; tb.position = {2.0f, 0.0f, 0.0f};
    TransformComponent tc; tc.position = {3.0f, 0.0f, 0.0f};

    ecs.AddComponent(a, ta);
    ecs.AddComponent(b, tb);
    ecs.AddComponent(c, tc);

    // Removing the middle entry moves the last element into its slot.
    ecs.RemoveComponent<TransformComponent>(b);

    CHECK_NEAR(ecs.GetComponent<TransformComponent>(a).position.x, 1.0f, kEps);
    CHECK_NEAR(ecs.GetComponent<TransformComponent>(c).position.x, 3.0f, kEps);
    CHECK(!ecs.HasComponent<TransformComponent>(b));
}

NEXUS_TEST(DestroyingAnEntityRemovesItsComponents) {
    ECS ecs;
    ecs.Init();
    ecs.RegisterComponent<TransformComponent>();

    const Entity e = ecs.CreateEntity();
    ecs.AddComponent(e, TransformComponent{});
    ecs.DestroyEntity(e);

    CHECK(!ecs.HasComponent<TransformComponent>(e));
}

NEXUS_TEST(DistinctComponentTypesGetDistinctIds) {
    ECS ecs;
    ecs.Init();
    ecs.RegisterComponent<TransformComponent>();
    ecs.RegisterComponent<NameComponent>();

    CHECK(ecs.GetComponentType<TransformComponent>() != ecs.GetComponentType<NameComponent>());
}

NEXUS_TEST(AnEntityCanHoldSeveralComponentTypes) {
    ECS ecs;
    ecs.Init();
    ecs.RegisterComponent<TransformComponent>();
    ecs.RegisterComponent<NameComponent>();

    const Entity e = ecs.CreateEntity();

    NameComponent name;
    name.name = "Player";
    ecs.AddComponent(e, TransformComponent{});
    ecs.AddComponent(e, name);

    CHECK(ecs.HasComponent<TransformComponent>(e));
    CHECK(ecs.HasComponent<NameComponent>(e));
    CHECK_EQ(ecs.GetComponent<NameComponent>(e).name, std::string("Player"));

    // Removing one must not disturb the other.
    ecs.RemoveComponent<TransformComponent>(e);
    CHECK(ecs.HasComponent<NameComponent>(e));
}

// --- TransformComponent ----------------------------------------------------

NEXUS_TEST(DefaultTransformIsIdentity) {
    // Scale defaults to one, not zero - a zero-scale default would make every
    // freshly created entity invisible.
    const TransformComponent t;
    DirectX::XMFLOAT4X4 m;
    DirectX::XMStoreFloat4x4(&m, t.GetMatrix());

    CHECK_NEAR(m._11, 1.0f, kEps);
    CHECK_NEAR(m._22, 1.0f, kEps);
    CHECK_NEAR(m._33, 1.0f, kEps);
    CHECK_NEAR(m._44, 1.0f, kEps);
    CHECK_NEAR(m._41, 0.0f, kEps);
}

NEXUS_TEST(TransformAppliesScaleThenRotationThenTranslation) {
    TransformComponent t;
    t.position = {10.0f, 0.0f, 0.0f};
    t.scale = {2.0f, 2.0f, 2.0f};

    const DirectX::XMVECTOR p =
        DirectX::XMVector3Transform(DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), t.GetMatrix());

    // Scaled to 2 and then translated by 10 - not translated and then scaled,
    // which would give 22.
    CHECK_NEAR(DirectX::XMVectorGetX(p), 12.0f, kEps);
}

NEXUS_TEST(TransformRotationIsInDegrees) {
    TransformComponent t;
    t.rotation = {0.0f, 0.0f, 90.0f};

    const DirectX::XMVECTOR v =
        DirectX::XMVector3TransformNormal(DirectX::XMVectorSet(1.0f, 0.0f, 0.0f, 0.0f), t.GetMatrix());

    CHECK_NEAR(DirectX::XMVectorGetY(v), 1.0f, kEps);
}

// --- FrameClock ------------------------------------------------------------
//
// The engine's main loop used to read its delta time from a timer it had just
// reset, so every frame advanced the simulation by a few microseconds no
// matter how long the frame actually took. These pin down the timing policy
// without any test having to sleep.

NEXUS_TEST(FrameClockStartsAtZero) {
    const FrameClock clock;

    CHECK_NEAR(clock.GetDeltaTime(), 0.0f, kEps);
    CHECK_EQ(clock.GetFPS(), 0);
    CHECK_EQ(clock.GetFrameCount(), std::uint64_t{0});
}

NEXUS_TEST(FrameClockReportsTheDeltaItWasGiven) {
    FrameClock clock;

    clock.Advance(0.016f);
    CHECK_NEAR(clock.GetDeltaTime(), 0.016f, kEps);

    clock.Advance(0.033f);
    CHECK_NEAR(clock.GetDeltaTime(), 0.033f, kEps);

    CHECK_EQ(clock.GetFrameCount(), std::uint64_t{2});
}

NEXUS_TEST(FrameClockClampsALongStallSoPhysicsStaysSimulable) {
    FrameClock clock;
    clock.SetMaxDeltaTime(0.1f);

    // A five-second stall - a breakpoint, or a window being dragged.
    clock.Advance(5.0f);

    CHECK_NEAR(clock.GetDeltaTime(), 0.1f, kEps);
}

NEXUS_TEST(FrameClockTreatsNegativeAndNonFiniteDeltasAsZero) {
    FrameClock clock;

    clock.Advance(-1.0f);
    CHECK_NEAR(clock.GetDeltaTime(), 0.0f, kEps);

    clock.Advance(std::numeric_limits<float>::quiet_NaN());
    CHECK_NEAR(clock.GetDeltaTime(), 0.0f, kEps);
}

NEXUS_TEST(FrameClockCountsFPSOverAOneSecondWindow) {
    FrameClock clock;

    // 1/64 is exactly representable in binary floating point, so 64 of them
    // accumulate to exactly 1.0 and the window closes on the 64th frame.
    // 1/60 is not, and sums to a hair under a second.
    for (int i = 0; i < 64; ++i) {
        clock.Advance(1.0f / 64.0f);
    }

    CHECK_EQ(clock.GetFPS(), 64);
    CHECK_EQ(clock.GetFrameCount(), std::uint64_t{64});
}

NEXUS_TEST(FrameClockFPSIsZeroBeforeTheFirstFullSecond) {
    FrameClock clock;

    for (int i = 0; i < 10; ++i) {
        clock.Advance(1.0f / 60.0f);   // ~0.17s total
    }

    CHECK_EQ(clock.GetFPS(), 0);
}

NEXUS_TEST(FrameClockFPSTracksAChangeInRate) {
    FrameClock clock;

    for (int i = 0; i < 64; ++i) {
        clock.Advance(1.0f / 64.0f);
    }
    CHECK_EQ(clock.GetFPS(), 64);

    // Now run a second of slower frames; the window should re-measure.
    for (int i = 0; i < 32; ++i) {
        clock.Advance(1.0f / 32.0f);
    }
    CHECK_EQ(clock.GetFPS(), 32);
}

NEXUS_TEST(FrameClockSleepsForTheRemainderOfTheFrameBudget) {
    FrameClock clock(60.0f);   // 16.67ms budget

    // A frame whose work took 6ms leaves ~10ms.
    CHECK_EQ(clock.GetSleepMilliseconds(0.006f), 10);
}

NEXUS_TEST(FrameClockDoesNotSleepWhenTheFrameOverran) {
    FrameClock clock(60.0f);

    CHECK_EQ(clock.GetSleepMilliseconds(0.020f), 0);
    CHECK_EQ(clock.GetSleepMilliseconds(1.0f), 0);
}

NEXUS_TEST(FrameClockDoesNotSleepWhenUncapped) {
    FrameClock clock;
    CHECK_NEAR(clock.GetTargetFPS(), 0.0f, kEps);
    CHECK_EQ(clock.GetSleepMilliseconds(0.001f), 0);

    // A non-positive target means "uncapped", and must never divide by zero.
    clock.SetTargetFPS(-30.0f);
    CHECK_NEAR(clock.GetTargetFPS(), 0.0f, kEps);
    CHECK_EQ(clock.GetSleepMilliseconds(0.001f), 0);
}

NEXUS_TEST(FrameClockResetClearsStateButKeepsConfiguration) {
    FrameClock clock(60.0f);
    clock.SetMaxDeltaTime(0.1f);

    for (int i = 0; i < 64; ++i) {
        clock.Advance(1.0f / 64.0f);
    }
    CHECK_EQ(clock.GetFPS(), 64);

    clock.Reset();

    CHECK_EQ(clock.GetFPS(), 0);
    CHECK_EQ(clock.GetFrameCount(), std::uint64_t{0});
    CHECK_NEAR(clock.GetDeltaTime(), 0.0f, kEps);
    CHECK_NEAR(clock.GetTargetFPS(), 60.0f, kEps);
    CHECK_NEAR(clock.GetMaxDeltaTime(), 0.1f, kEps);
}

NEXUS_TEST(FrameClockDrivenByTheTimerSeesARealInterval) {
    // Ties FrameClock back to the clock the engine actually feeds it with:
    // Timer::Tick(), not the reset-then-read that produced a ~0 delta.
    FrameClock clock;
    Timer timer;

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    clock.Advance(timer.Tick());

    CHECK(clock.GetDeltaTime() >= 0.015f);
    CHECK(clock.GetDeltaTime() < 1.0f);
}

// --- Timer -----------------------------------------------------------------

NEXUS_TEST(TimerStartsNearZero) {
    const Timer timer;
    CHECK(timer.GetElapsedTime() >= 0.0f);
    CHECK(timer.GetElapsedTime() < 1.0f);
}

NEXUS_TEST(TimerAdvancesMonotonically) {
    const Timer timer;
    const float first = timer.GetElapsedTime();
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    const float second = timer.GetElapsedTime();

    CHECK(second > first);
    CHECK(second >= 0.015f);   // allow for scheduler slack below the 20ms sleep
}

NEXUS_TEST(TimerResetReturnsToZero) {
    Timer timer;
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    CHECK(timer.GetElapsedTime() > 0.0f);

    timer.Reset();
    CHECK(timer.GetElapsedTime() < 0.010f);
}

NEXUS_TEST(TimerTickMeasuresTheIntervalBetweenCalls) {
    Timer timer;
    timer.Tick();   // establish the baseline

    std::this_thread::sleep_for(std::chrono::milliseconds(20));
    const float delta = timer.Tick();
    CHECK(delta >= 0.015f);

    // A second Tick with no sleep must report a much smaller delta, not the
    // total elapsed time.
    CHECK(timer.Tick() < delta);
}

NEXUS_TEST(TimerSecondsMatchesFloatElapsed) {
    const Timer timer;
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    CHECK_NEAR(timer.GetElapsedSeconds(), timer.GetElapsedTime(), 0.01);
}

// --- Platform --------------------------------------------------------------

NEXUS_TEST(PlatformReportsTheHostItIsRunningOn) {
    const std::string name = Platform::GetPlatformName();
    CHECK(!name.empty());
    CHECK(name != "Unknown");

#if defined(_WIN32)
    CHECK_EQ(name, std::string("Windows"));
#elif defined(__linux__)
    CHECK_EQ(name, std::string("Linux"));
#elif defined(__APPLE__)
    CHECK_EQ(name, std::string("macOS"));
#endif
}

NEXUS_TEST(PlatformTimeAdvances) {
    const double first = Platform::GetTime();
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    CHECK(Platform::GetTime() > first);
}

NEXUS_TEST(PlatformSleepWaitsAtLeastTheRequestedTime) {
    const double before = Platform::GetTime();
    Platform::Sleep(20);
    CHECK(Platform::GetTime() - before >= 0.015);
}

NEXUS_TEST(PlatformSleepOfZeroReturnsImmediately) {
    const double before = Platform::GetTime();
    Platform::Sleep(0);
    Platform::Sleep(-5);
    CHECK(Platform::GetTime() - before < 0.5);
}

NEXUS_TEST(PlatformFileExistsDistinguishesPresentFromAbsent) {
    CHECK(!Platform::FileExists("/definitely/not/a/real/path/nexus-test-xyz"));

    // A directory is not a regular file.
    CHECK(!Platform::FileExists("."));
}

NEXUS_TEST(PlatformMemoryUsageIsAPercentageOrUnknown) {
    const int percent = Platform::GetSystemMemoryUsagePercent();

    // -1 means "cannot determine here"; anything else must be a real percentage.
    // A value of 0 would be indistinguishable from failure if this returned 0
    // on error, which is why the contract uses -1.
    CHECK(percent == -1 || (percent >= 0 && percent <= 100));
}

} // namespace

int main() {
    RUN_TEST(EntitiesAreUniqueWhileAlive);
    RUN_TEST(DestroyingAnEntityDecrementsTheLivingCount);
    RUN_TEST(DestroyedEntityIdsAreRecycled);

    RUN_TEST(ComponentsRoundTripThroughStorage);
    RUN_TEST(ComponentsAreMutableInPlace);
    RUN_TEST(RemovingAComponentClearsIt);
    RUN_TEST(ComponentsAreIndependentPerEntity);
    RUN_TEST(DestroyingAnEntityRemovesItsComponents);
    RUN_TEST(DistinctComponentTypesGetDistinctIds);
    RUN_TEST(AnEntityCanHoldSeveralComponentTypes);

    RUN_TEST(DefaultTransformIsIdentity);
    RUN_TEST(TransformAppliesScaleThenRotationThenTranslation);
    RUN_TEST(TransformRotationIsInDegrees);

    RUN_TEST(FrameClockStartsAtZero);
    RUN_TEST(FrameClockReportsTheDeltaItWasGiven);
    RUN_TEST(FrameClockClampsALongStallSoPhysicsStaysSimulable);
    RUN_TEST(FrameClockTreatsNegativeAndNonFiniteDeltasAsZero);
    RUN_TEST(FrameClockCountsFPSOverAOneSecondWindow);
    RUN_TEST(FrameClockFPSIsZeroBeforeTheFirstFullSecond);
    RUN_TEST(FrameClockFPSTracksAChangeInRate);
    RUN_TEST(FrameClockSleepsForTheRemainderOfTheFrameBudget);
    RUN_TEST(FrameClockDoesNotSleepWhenTheFrameOverran);
    RUN_TEST(FrameClockDoesNotSleepWhenUncapped);
    RUN_TEST(FrameClockResetClearsStateButKeepsConfiguration);
    RUN_TEST(FrameClockDrivenByTheTimerSeesARealInterval);

    RUN_TEST(TimerStartsNearZero);
    RUN_TEST(TimerAdvancesMonotonically);
    RUN_TEST(TimerResetReturnsToZero);
    RUN_TEST(TimerTickMeasuresTheIntervalBetweenCalls);
    RUN_TEST(TimerSecondsMatchesFloatElapsed);

    RUN_TEST(PlatformReportsTheHostItIsRunningOn);
    RUN_TEST(PlatformTimeAdvances);
    RUN_TEST(PlatformSleepWaitsAtLeastTheRequestedTime);
    RUN_TEST(PlatformSleepOfZeroReturnsImmediately);
    RUN_TEST(PlatformFileExistsDistinguishesPresentFromAbsent);
    RUN_TEST(PlatformMemoryUsageIsAPercentageOrUnknown);

    return NexusTest::Summarize("CoreTests");
}
