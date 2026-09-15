#include "Engine.h"
#include "GraphicsDevice.h"
#include "AudioDevice.h"
#include "AudioSystem.h"
#include "InputManager.h"
#include "Timer.h"
#include "Logger.h"
#include "ScriptingEngine.h"
#include "ResourceManager.h"
#include "PhysicsEngine.h"
#include "AISystem.h"
#include "LightingEngine.h"
#include "AnimationSystem.h"
#include "ParticleSystem.h"
#include "MotionControlSystem.h"
#include "EngineUI.h"
#include "EngineErrorRecovery.h"
#if defined(_WIN32)
    #include <windowsx.h>   // GET_X_LPARAM and friends; Win32 message crackers only.
#endif
#include <chrono>
#include <stdexcept>
#include <sstream>

namespace Nexus {

#if defined(_WIN32)
// Provided by ImGui's Win32 backend (thirdparty/imgui/backends). Declared here
// rather than included so this file needs no ImGui headers.
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace {
/// Gives the UI first look at native window messages. Installed only once the
/// UI has initialised successfully - ImGui's handler must not be called before
/// its backend exists.
bool ForwardMessageToUI(WindowHandle window, unsigned int message,
                        std::uintptr_t wParam, std::intptr_t lParam) {
    return ImGui_ImplWin32_WndProcHandler(window, static_cast<UINT>(message),
                                          static_cast<WPARAM>(wParam),
                                          static_cast<LPARAM>(lParam)) != 0;
}
} // namespace
#endif

// The window class, window procedure and message loop that used to live here
// now belong to the platform layer (Platform::CreateGameWindow and friends).
// They were the only Windows-specific code in this file, and therefore the only
// reason the engine core could not be built anywhere else.

Engine::Engine() {
    // Subsystems are constructed here so every accessor returns a usable
    // pointer immediately. They previously were never constructed at all:
    // Initialize() went straight to graphics_->Initialize(...) through a null
    // unique_ptr, so starting the engine was undefined behaviour on every
    // platform, Windows included.
    CreateSubsystems();
}

Engine::~Engine() {
    SafeShutdown();
}

void Engine::CreateSubsystems() {
    // Idempotent, so Initialize() after a Shutdown() (which releases them)
    // brings the engine back up rather than dereferencing null.
    if (!graphics_)      graphics_      = std::make_unique<GraphicsDevice>();
    if (!audio_)         audio_         = std::make_unique<AudioDevice>();
    if (!audioSystem_)   audioSystem_   = std::make_unique<AudioSystem>();
    if (!input_)         input_         = std::make_unique<InputManager>();
#ifdef NEXUS_PYTHON_ENABLED
    if (!scripting_)     scripting_     = std::make_unique<ScriptingEngine>();
#endif
    if (!resources_)     resources_     = std::make_unique<ResourceManager>();
    if (!physics_)       physics_       = std::make_unique<PhysicsEngine>();
    if (!ai_)            ai_            = std::make_unique<AIManager>();
    if (!lighting_)      lighting_      = std::make_unique<LightingEngine>();
    if (!animation_)     animation_     = std::make_unique<AnimationSystem>();
    if (!particles_)     particles_     = std::make_unique<ParticleSystem>();
    if (!motionControl_) motionControl_ = std::make_unique<MotionControlSystem>();
    if (!textRenderer_)  textRenderer_  = std::make_unique<TextRenderer>();
    if (!ui_)            ui_            = std::make_unique<EngineUI>();
    if (!errorRecovery_) errorRecovery_ = std::make_unique<EngineErrorRecovery>();
}

bool Engine::Initialize(const std::string& configFile) {
    InitParams params;
    params.configFile = configFile;
    return Initialize(params);
}

bool Engine::Initialize(const InitParams& params) {
    if (initialized_) return true;

    Logger::Info("Initializing Nexus Engine...");

    // InitParams was declared but never read: window size, title and fullscreen
    // were whatever the uninitialised members happened to hold.
    width_      = params.width  > 0 ? params.width  : 1280;
    height_     = params.height > 0 ? params.height : 720;
    fullscreen_ = params.fullscreen;
    title_      = params.title.empty() ? std::string("Nexus Engine") : params.title;

    if (!params.configFile.empty()) {
        // Accepted by the signature since the beginning and silently dropped.
        // Say so rather than letting a caller believe their settings applied.
        Logger::Warning("Config file '" + params.configFile +
                        "' ignored: engine configuration files are not implemented yet");
    }

    try {
        // Subsystems may have been released by a previous Shutdown().
        CreateSubsystems();

        // Initialize platform
        if (!Platform::Initialize()) {
            Logger::Error("Failed to initialize platform");
            return false;
        }

        // The platform layer owns the window class, the window procedure and
        // the message pump; width_/height_ describe the client area.
        window_ = Platform::CreateGameWindow(title_, width_, height_);
        if (!window_) {
            Logger::Error("Failed to create window");
            return false;
        }

        // Initialize graphics first
        if (!graphics_->Initialize(window_, width_, height_, fullscreen_)) {
            Logger::Error("Failed to initialize graphics device");
            return false;
        }

        // Initialize input
        if (!input_->Initialize(window_)) {
            Logger::Error("Failed to initialize input manager");
            return false;
        }

        // Initialize resource manager with graphics device
        if (!resources_->Initialize(graphics_->GetDevice())) {
            Logger::Error("Failed to initialize resource manager");
            return false;
        }

        // Initialize audio
        if (!audio_->Initialize()) {
            Logger::Error("Failed to initialize audio device");
            return false;
        }

        // Fix: Use correct AudioSystem::Initialize signature (takes int parameters, not AudioDevice*)
        if (!audioSystem_->Initialize(44100, 2, 16, AudioSystem::AudioChannelLayout::Stereo)) {
            Logger::Error("Failed to initialize audio system");
            return false;
        }

#ifdef NEXUS_PYTHON_ENABLED
        // Initialize scripting
        if (!scripting_->Initialize()) {
            Logger::Warning("Python scripting not available");
        }
#endif

        // Initialize physics
        if (!physics_->Initialize()) {
            Logger::Error("Failed to initialize physics engine");
            return false;
        }

        // Initialize AI system
        if (!ai_->Initialize()) {
            Logger::Error("Failed to initialize AI system");
            return false;
        }

        // Initialize lighting engine
        if (!lighting_->Initialize(graphics_->GetDevice(), graphics_->GetContext(), width_, height_)) {
            Logger::Error("Failed to initialize lighting engine");
            return false;
        }

        // Fix: AnimationSystem::Initialize takes device and context parameters
        if (!animation_->Initialize(graphics_->GetDevice(), graphics_->GetContext())) {
            Logger::Error("Failed to initialize animation system");
            return false;
        }

        // Initialize particle system
        if (!particles_->Initialize(graphics_->GetDevice(), graphics_->GetContext())) {
            Logger::Error("Failed to initialize particle system");
            return false;
        }

        // Fix: MotionControlSystem::Initialize takes no parameters
        if (!motionControl_->Initialize()) {
            Logger::Warning("Motion control initialization failed - continuing without motion control");
        }

        // The remaining subsystems are optional: the engine runs without them,
        // so each is released on failure rather than left half-initialised.
        // Update() and Render() skip whatever is null, which is what makes
        // "initialise or drop" safe.

        if (!textRenderer_->Initialize(graphics_->GetDevice(), graphics_->GetContext())) {
            Logger::Warning("Failed to initialize text renderer - continuing without on-screen text");
            textRenderer_.reset();
        }

        if (!ui_->Initialize(this, graphics_->GetDevice(), graphics_->GetContext())) {
            Logger::Warning("Failed to initialize engine UI - continuing without it");
            ui_.reset();
        } else {
#if defined(_WIN32)
            // Only now that ImGui's backend exists is it safe to route native
            // messages into it.
            Platform::SetWindowMessageHook(&ForwardMessageToUI);
#endif
        }

        if (!errorRecovery_->Initialize(this)) {
            Logger::Warning("Failed to initialize error recovery - continuing without it");
            errorRecovery_.reset();
        }

        // Create physics demo
        physics_->CreatePhysicsDemo();

        Logger::Info("Engine initialized successfully");
        initialized_ = true;
        shouldExit_ = false;   // IsRunning() is true from here.
        return true;

    } catch (const std::exception& e) {
        Logger::Error("Exception during engine initialization: " + std::string(e.what()));
        return false;
    }
}

void Engine::Run() {
    if (!initialized_) {
        Logger::Error("Engine::Run called before a successful Initialize()");
        return;
    }

    Logger::Info("Starting main engine loop...");

    shouldExit_ = false;
    frameClock_.Reset();

    // Two clocks with different jobs: frameTimer measures the whole frame
    // including the pacing sleep, which is the delta the simulation advances
    // by; workTimer measures just this frame's update and render, which is
    // what the pacing calculation needs.
    //
    // Delta time used to be read from a timer that had just been reset, so it
    // was a few microseconds every frame regardless of how long the frame took
    // - physics, animation and particles all effectively stood still.
    Timer frameTimer;
    Timer workTimer;

    try {
        while (IsRunning()) {
            frameClock_.Advance(frameTimer.Tick());
            const float deltaTime = frameClock_.GetDeltaTime();

            workTimer.Reset();

            // Pump the host windowing system. Platform::ProcessMessages owns
            // the platform-specific details and reports false once the window
            // has asked the application to close.
            if (!Platform::ProcessMessages()) {
                RequestExit();
                break;
            }

            // Check for ESC to exit. Routed through InputManager rather than
            // GetAsyncKeyState so this honours the engine's own input state -
            // and so it works off Windows.
            if (input_ && input_->IsKeyDown(KeyCode::Escape)) {
                RequestExit();
                break;
            }

            try {
                Update(deltaTime);
            } catch (const std::exception& e) {
                Logger::Error("Exception during update: " + std::string(e.what()));
                RequestExit();
                break;
            }
            perfStats_.updateTime = workTimer.GetElapsedTime();

            try {
                Render();
            } catch (const std::exception& e) {
                Logger::Error("Exception during render: " + std::string(e.what()));
                RequestExit();
                break;
            }

            const float workSeconds = workTimer.GetElapsedTime();
            perfStats_.renderTime = workSeconds - perfStats_.updateTime;
            perfStats_.frameTime = deltaTime;

            // Give the rest of the machine the remainder of the frame budget.
            const int sleepMs = frameClock_.GetSleepMilliseconds(workSeconds);
            if (sleepMs > 0) {
                Platform::Sleep(sleepMs);
            }
        }
    } catch (const std::exception& e) {
        Logger::Error("Exception in main loop: " + std::string(e.what()));
        RequestExit();
    }

    Logger::Info("Main loop ended after " + std::to_string(frameClock_.GetFrameCount()) + " frames");
}

void Engine::Update(float deltaTime) {
    // Update input first
    if (input_) {
        input_->Update();
    }
    
    // Update physics
    if (physics_) {
        physics_->Update(deltaTime);
    }
    
    // Update AI
    if (ai_) {
        ai_->Update(deltaTime);
    }
    
    // Update audio
    if (audioSystem_) {
        audioSystem_->Update(deltaTime);
    }
    
    // Update animation
    if (animation_) {
        animation_->Update(deltaTime);
    }
    
    // Update particles
    if (particles_) {
        particles_->Update(deltaTime);
    }
    
    // Update motion control
    if (motionControl_) {
        motionControl_->Update(deltaTime);
    }
    
#ifdef NEXUS_PYTHON_ENABLED
    // Update scripting
    if (scripting_) {
        scripting_->Update(deltaTime);
    }
#endif
    
    // Update UI
    if (ui_) {
        // UI updates happen during Render() phase
    }
    
    // Update error recovery
    if (errorRecovery_) {
        errorRecovery_->Update(deltaTime);
    }
}

void Engine::Render() {
    if (!graphics_) return;

    graphics_->BeginFrame();

    // ImGui requires NewFrame before any widget call and EndFrame after the
    // scene. Only ui_->Render() was being called, which would have tripped
    // ImGui's own assertions the moment the UI was ever initialised.
    if (ui_) {
        ui_->NewFrame();
    }

    
    // Render physics objects
    if (physics_) {
        const auto& renderObjects = physics_->GetRenderObjects();
        if (!renderObjects.empty()) {
            static bool firstRender = true;
            if (firstRender) {
                Logger::Info("Rendering " + std::to_string(renderObjects.size()) + " physics objects");
                firstRender = false;
            }
            for (const auto& obj : renderObjects) {
                switch (obj.shapeType) {
                    case CollisionShape::Type::Box:
                        graphics_->RenderBox(obj.position, obj.scale, obj.color);
                        break;
                    case CollisionShape::Type::Sphere:
                        graphics_->RenderSphere(obj.position, obj.scale.x, obj.color);
                        break;
                    case CollisionShape::Type::Capsule:
                        graphics_->RenderCapsule(obj.position, obj.scale.x, obj.scale.y, obj.color);
                        break;

                    case CollisionShape::Type::Cylinder:
                    case CollisionShape::Type::Cone:
                    case CollisionShape::Type::Mesh:
                    case CollisionShape::Type::Plane:
                    case CollisionShape::Type::Heightfield:
                        // The debug renderer has no primitive for these yet.
                        // Listed individually rather than behind a default so
                        // that adding a new shape type still warns here.
                        break;
                }
            }
        }
        
        // Render UI text (basic status information)
        if (textRenderer_) {
            using namespace DirectX;
            textRenderer_->RenderText("Nexus Engine v1.0", 10.0f, 10.0f, 1.0f, XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
            textRenderer_->RenderText("FPS: " + std::to_string(GetFPS()), 10.0f, 30.0f, 1.0f, XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f));
            textRenderer_->RenderText("Objects: " + std::to_string(physics_ ? physics_->GetRenderObjects().size() : 0), 10.0f, 50.0f, 1.0f, XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f));
        }
    }
    
    // Render UI on top of the scene, then let ImGui submit its draw data
    // before the frame is presented.
    if (ui_) {
        ui_->Render();
        ui_->EndFrame();
    }

    graphics_->EndFrame();
    graphics_->Present();
}

void Engine::Shutdown() {
    // Idempotent: the destructor calls this via SafeShutdown(), and callers
    // routinely call it explicitly first.
    if (!initialized_ && !window_) {
        return;
    }

    Logger::Info("Shutting down engine...");

    shouldExit_ = true;

#if defined(_WIN32)
    // Stop routing messages into ImGui before its backend is torn down.
    Platform::SetWindowMessageHook(nullptr);
#endif

    // Shutdown subsystems in reverse order
    errorRecovery_.reset();
    ui_.reset();
    textRenderer_.reset();
    motionControl_.reset();
    particles_.reset();
    animation_.reset();
    lighting_.reset();
    ai_.reset();
    physics_.reset();

#ifdef NEXUS_PYTHON_ENABLED
    scripting_.reset();
#endif

    resources_.reset();
    input_.reset();
    audioSystem_.reset();
    audio_.reset();
    graphics_.reset();

    // The window outlives the subsystems that render into it.
    if (window_) {
        Platform::DestroyGameWindow(window_);
        window_ = nullptr;
    }

    Platform::Shutdown();

    // Without this, a second Initialize() returned true immediately and handed
    // back an engine whose subsystems had all been released.
    initialized_ = false;

    Logger::Info("Engine shutdown complete");
}

void Engine::SafeShutdown() noexcept {
    // Declared and called from the destructor since the beginning but never
    // defined, so linking anything that constructed an Engine failed. A
    // destructor is implicitly noexcept, so nothing may escape from here.
    try {
        Shutdown();
    } catch (const std::exception& e) {
        try {
            Logger::Error("Exception during engine shutdown: " + std::string(e.what()));
        } catch (...) {
        }
    } catch (...) {
        try {
            Logger::Error("Unknown exception during engine shutdown");
        } catch (...) {
        }
    }
}

} // namespace Nexus
