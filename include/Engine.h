#pragma once

#include <memory>
#include <string>
#include <vector>
#include <chrono>

#include "FrameClock.h"
#include "Platform.h"     // WindowHandle, and the window/message-pump API.
#include "TextRenderer.h"

namespace Nexus {

class GraphicsDevice;
class AudioDevice;
class AudioSystem;
class InputManager;
class ScriptingEngine;
class ResourceManager;
class PhysicsEngine;
class AIManager;
class LightingEngine;
class AnimationSystem;
class ParticleSystem;
class MotionControlSystem;
class EngineUI;
class UISystem;
class EngineErrorRecovery;

struct InitParams {
    std::string configFile;
    int width = 1280;
    int height = 720;
    bool fullscreen = false;
    std::string title = "Nexus Engine";
};

/**
 * Main engine class that manages all subsystems
 */
class Engine {
public:
    Engine();
    ~Engine();

    // Core lifecycle

    /// Initialises every subsystem against a window of the requested size.
    /// Safe to call twice; the second call is a no-op returning true.
    bool Initialize(const InitParams& params);

    /// Convenience overload: default window settings, optionally with a config
    /// file. Equivalent to filling in InitParams::configFile and nothing else.
    bool Initialize(const std::string& configFile = "");
    void Run();
    void Shutdown();

    // Subsystem access
    GraphicsDevice* GetGraphics() const { return graphics_.get(); }
    AudioDevice* GetAudio() const { return audio_.get(); }
    AudioSystem* GetAudioSystem() const { return audioSystem_.get(); }
    InputManager* GetInput() const { return input_.get(); }
#ifdef NEXUS_PYTHON_ENABLED
    ScriptingEngine* GetScripting() const { return scripting_.get(); }
#else
    ScriptingEngine* GetScripting() const { return nullptr; }
#endif
    ResourceManager* GetResources() const { return resources_.get(); }
    PhysicsEngine* GetPhysics() const { return physics_.get(); }
    AIManager* GetAI() const { return ai_.get(); }
    LightingEngine* GetLighting() const { return lighting_.get(); }
    AnimationSystem* GetAnimation() const { return animation_.get(); }
    ParticleSystem* GetParticles() const { return particles_.get(); }
    MotionControlSystem* GetMotionControl() const { return motionControl_.get(); }
    EngineUI* GetUI() const { return ui_.get(); }
    EngineErrorRecovery* GetErrorRecovery() const { return errorRecovery_.get(); }

    // Frame control

    /// Caps the main loop; 0 or below runs uncapped.
    void SetTargetFPS(float fps) { frameClock_.SetTargetFPS(fps); }
    float GetTargetFPS() const { return frameClock_.GetTargetFPS(); }

    /// Frames per second over the last completed second, 0 before then.
    int GetFPS() const { return frameClock_.GetFPS(); }
    float GetDeltaTime() const { return frameClock_.GetDeltaTime(); }

    // State
    bool IsInitialized() const { return initialized_; }

    /// True once Initialize() has succeeded and until an exit is requested or
    /// Shutdown() runs.
    ///
    /// Derived rather than stored: it used to be a flag set only inside Run(),
    /// so the hand-written loops in the C API and main_debug.cpp - which drive
    /// the engine themselves rather than calling Run() - tested it straight
    /// after Initialize(), found it false, and never executed a single frame.
    bool IsRunning() const { return initialized_ && !shouldExit_; }

    /// Asks the main loop to stop at the end of the current frame. Safe to
    /// call from a subsystem mid-frame: the frame still completes, so nothing
    /// tears down underneath code that is still running.
    void RequestExit() { shouldExit_ = true; }

    /// The engine's window, or nullptr before Initialize().
    WindowHandle GetWindow() const { return window_; }

private:
    void Update(float deltaTime);
    void Render();

    /// Shutdown() that cannot throw, for use from the destructor.
    void SafeShutdown() noexcept;

    /// Constructs any subsystem that is not already allocated. Called from the
    /// constructor and again from Initialize(), so an Initialize() following a
    /// Shutdown() brings the engine back up instead of dereferencing null.
    void CreateSubsystems();
    
    // Core subsystems
    std::unique_ptr<GraphicsDevice> graphics_;
    std::unique_ptr<AudioDevice> audio_;
    std::unique_ptr<AudioSystem> audioSystem_;
    std::unique_ptr<InputManager> input_;
#ifdef NEXUS_PYTHON_ENABLED
    std::unique_ptr<ScriptingEngine> scripting_;
#endif
    std::unique_ptr<ResourceManager> resources_;
    
    // Advanced subsystems
    std::unique_ptr<PhysicsEngine> physics_;
    std::unique_ptr<AIManager> ai_;
    std::unique_ptr<LightingEngine> lighting_;
    std::unique_ptr<AnimationSystem> animation_;
    std::unique_ptr<ParticleSystem> particles_;
    std::unique_ptr<MotionControlSystem> motionControl_;
    std::unique_ptr<TextRenderer> textRenderer_;
    std::unique_ptr<EngineUI> ui_;
    std::unique_ptr<EngineErrorRecovery> errorRecovery_;

    // Window and initialization
    //
    // Every member here carries a default initialiser. width_, height_,
    // fullscreen_ and the frame-rate target were previously absent from the
    // constructor's initialiser list and then read by Initialize(), so the
    // window was created at an indeterminate size.
    WindowHandle window_ = nullptr;
    int width_ = 1280;
    int height_ = 720;
    bool fullscreen_ = false;
    std::string title_ = "Nexus Engine";

    // Engine state
    bool initialized_ = false;
    bool shouldExit_ = false;
    bool recoveringFromError_ = false;

    /// Delta time, FPS and frame pacing. Owns what were loose members plus a
    /// function-local static inside GetFPS().
    FrameClock frameClock_{60.0f};

    // Performance stats
    struct PerfStats {
        float frameTime = 0.0f;
        float updateTime = 0.0f;
        float renderTime = 0.0f;
        int memoryUsage = 0;
    } perfStats_;
};

} // namespace Nexus
