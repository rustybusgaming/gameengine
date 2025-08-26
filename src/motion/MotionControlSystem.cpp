#include "MotionControlSystem.h"
#include "Logger.h"
#include <algorithm>
#include <cmath>
#include <chrono>

namespace Nexus {

MotionControlSystem::MotionControlSystem()
    : kinectSensor_(nullptr)
    , coordinateMapper_(nullptr)
    , bodyFrameReader_(nullptr)
    , gestureRecognizer_(nullptr)
    , isInitialized_(false)
    , calibrationMode_(false)
    , motionSensitivity_(1.0f)
    , aimingSmoothing_(0.8f)
    , gestureThreshold_(0.7f)
    , lastFrameTime_(0.0f)
    , trackingQuality_(TrackingQuality::Medium)
    , errorCount_(0)
    , consecutiveTrackingFailures_(0)
    , trackingEnabled_(true)
{
    // Configure default filter settings based on tracking quality
    ConfigureFilteringByQuality(trackingQuality_);
    
    // Initialize aiming direction
    currentAimDirection_ = DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f);
}

MotionControlSystem::~MotionControlSystem() {
    Shutdown();
}

bool MotionControlSystem::Initialize() {
    Logger::Info("MotionControlSystem::Initialize starting...");
    
    // Initialize Kinect sensor
    if (!InitializeKinect()) {
        Logger::Warning("Kinect not available - motion control features disabled");
        // Attempt to initialize fallback tracking system
        if (!InitializeFallbackTracking()) {
            Logger::Error("Motion control system initialization failed - no tracking system available");
            return false;
        }
        Logger::Info("Fallback tracking system initialized");
    } else {
        Logger::Info("Kinect initialization completed");
    }
    
    // Initialize gesture recognition
    if (!InitializeGestureRecognition()) {
        Logger::Warning("Gesture recognition initialization failed - continuing with limited functionality");
    } else {
        Logger::Info("Gesture recognition initialization completed");
        // Define standard gesture patterns
        DefineGesturePatterns();
    }
    
    // Initialize motion tracking
    Logger::Info("About to initialize motion tracking...");
    if (!InitializeMotionTracking()) {
        Logger::Warning("Motion tracking initialization encountered issues - some features may be limited");
    } else {
        Logger::Info("Motion tracking initialization completed");
    }
    
    // Record initialization time
    lastFrameTime_ = static_cast<float>(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count()) / 1000.0f;
        
    isInitialized_ = true;
    Logger::Info("Motion control system initialized successfully");
    return true;
}

void MotionControlSystem::Shutdown() {
    // Clean up Kinect resources
    if (bodyFrameReader_) {
        static_cast<IUnknown*>(bodyFrameReader_)->Release();
        bodyFrameReader_ = nullptr;
    }
    
    if (coordinateMapper_) {
        static_cast<IUnknown*>(coordinateMapper_)->Release();
        coordinateMapper_ = nullptr;
    }
    
    if (kinectSensor_) {
        // For Kinect sensor, we need to close it first
        // static_cast<IKinectSensor*>(kinectSensor_)->Close();
        static_cast<IUnknown*>(kinectSensor_)->Release();
        kinectSensor_ = nullptr;
    }
    
    // Clean up gesture recognition
    if (gestureRecognizer_) {
        delete gestureRecognizer_;
        gestureRecognizer_ = nullptr;
    }
    
    isInitialized_ = false;
    Logger::Info("Motion control system shut down");
}

void MotionControlSystem::Update(float deltaTime) {
    if (!isInitialized_ || !trackingEnabled_) return;
    
    try {
        // Calculate actual time since last update for better timing consistency
        float currentTime = static_cast<float>(std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count()) / 1000.0f;
        float actualDeltaTime = currentTime - lastFrameTime_;
        lastFrameTime_ = currentTime;
        
        // Cap delta time to avoid large jumps after hiccups
        actualDeltaTime = std::min(actualDeltaTime, 0.1f);
        
        // Update Kinect body tracking
        if (!UpdateBodyTracking()) {
            consecutiveTrackingFailures_++;
            if (consecutiveTrackingFailures_ > MAX_TRACKING_FAILURES) {
                Logger::Warning("Multiple consecutive tracking failures detected - attempting recovery");
                AttemptTrackingRecovery();
            }
        } else {
            consecutiveTrackingFailures_ = 0;
        }
        
        // Update gesture recognition
        UpdateGestureRecognition(actualDeltaTime);
        
        // Update motion-based aiming
        UpdateMotionAiming(actualDeltaTime);
        
        // Update motion controls
        UpdateMotionControls(actualDeltaTime);
        
        // Process calibration if in calibration mode
        if (calibrationMode_) {
            UpdateCalibration(actualDeltaTime);
        }
    } catch (const std::exception& e) {
        Logger::Error("Exception in MotionControlSystem::Update: " + std::string(e.what()));
        errorCount_++;
        
        // If too many errors occur, try to recover
        if (errorCount_ > MAX_ERROR_THRESHOLD) {
            AttemptSystemRecovery();
        }
    }
}

bool MotionControlSystem::InitializeKinect() {
    // Initialize Kinect sensor (simplified for demonstration)
    // In a real implementation, this would use Kinect SDK
    
    Logger::Info("Initializing Kinect sensor...");
    
    // Simulated Kinect initialization
    // HRESULT hr = GetDefaultKinectSensor(&kinectSensor_);
    // if (FAILED(hr)) {
    //     return false;
    // }
    
    // For demonstration, we'll simulate successful initialization
    Logger::Info("Kinect sensor initialized (simulated)");
    return true;
}

bool MotionControlSystem::InitializeGestureRecognition() {
    // Temporarily disable gesture recognition to isolate crash
    Logger::Info("Gesture recognition initialized (disabled for debugging)");
    return true;
}

bool MotionControlSystem::InitializeMotionTracking() {
    try {
        // Configure motion filter based on tracking quality
        switch (trackingQuality_) {
            case TrackingQuality::Low:
                motionFilter_.windowSize = 10; // Larger window for more aggressive smoothing
                motionFilter_.weights = { 0.05f, 0.05f, 0.1f, 0.1f, 0.1f, 0.2f, 0.1f, 0.1f, 0.1f, 0.05f };
                break;
                
            case TrackingQuality::Medium:
                motionFilter_.windowSize = 5;
                motionFilter_.weights = { 0.1f, 0.2f, 0.4f, 0.2f, 0.1f };
                break;
                
            case TrackingQuality::High:
                motionFilter_.windowSize = 3; // Smaller window for more responsive tracking
                motionFilter_.weights = { 0.2f, 0.6f, 0.2f };
                break;
                
            case TrackingQuality::Ultra:
                motionFilter_.windowSize = 2; // Minimal filtering for maximum responsiveness
                motionFilter_.weights = { 0.3f, 0.7f };
                break;
        }
        
        // Initialize joint history for velocity calculation
        motionFilter_.positions.clear();
        motionFilter_.velocities.clear();
        motionFilter_.timestamps.clear();
        
        // Preallocate memory for filter buffers
        motionFilter_.positions.reserve(motionFilter_.windowSize);
        motionFilter_.velocities.reserve(motionFilter_.windowSize);
        motionFilter_.timestamps.reserve(motionFilter_.windowSize);
        
        // Initialize joint tracking with proper vector sizes
        int jointCount = static_cast<int>(JointType::Count);
        trackedJoints_.resize(jointCount);
        jointConfidence_.resize(jointCount);
        jointVelocities_.resize(jointCount);
        jointHistory_.resize(jointCount);
        
        // Initialize all joint data
        for (int i = 0; i < jointCount; i++) {
            trackedJoints_[i] = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
            jointVelocities_[i] = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
            jointConfidence_[i] = 0.0f;
            
            // Initialize history for each joint (for advanced filtering)
            jointHistory_[i].resize(motionFilter_.windowSize);
            for (auto& pos : jointHistory_[i]) {
                pos = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
            }
        }
        
        // Initialize skeletal constraints for improved tracking
        InitializeSkeletalConstraints();
        
        return true;
    } catch (const std::exception& e) {
        Logger::Error("Failed to initialize motion tracking: " + std::string(e.what()));
        return false;
    }
}

bool MotionControlSystem::UpdateBodyTracking() {
    // Update body tracking data from Kinect
    // This would normally read from Kinect body frame
    
    try {
        // Simulated body tracking update
        static float time = 0.0f;
        time += 0.016f; // Simulate 60 FPS
        
        float handMovementScale = 0.5f;
        float torsoMovementScale = 0.2f;
        
        // Simulate a more complete skeleton with realistic constraints
        
        // Head and neck
        trackedJoints_[static_cast<int>(JointType::Head)] = DirectX::XMFLOAT3(
            sinf(time * 0.3f) * 0.1f,
            1.7f + sinf(time * 0.2f) * 0.05f, 
            2.0f
        );
        
        trackedJoints_[static_cast<int>(JointType::Neck)] = DirectX::XMFLOAT3(
            sinf(time * 0.3f) * 0.05f,
            1.5f + sinf(time * 0.2f) * 0.03f, 
            2.0f
        );
        
        // Torso
        trackedJoints_[static_cast<int>(JointType::SpineShoulder)] = DirectX::XMFLOAT3(
            sinf(time * 0.3f) * 0.03f,
            1.4f + sinf(time * 0.2f) * 0.02f, 
            2.0f
        );
        
        trackedJoints_[static_cast<int>(JointType::SpineMid)] = DirectX::XMFLOAT3(
            sinf(time * 0.3f) * torsoMovementScale * 0.05f,
            1.2f + sinf(time * 0.2f) * 0.01f, 
            2.0f
        );
        
        trackedJoints_[static_cast<int>(JointType::SpineBase)] = DirectX::XMFLOAT3(
            sinf(time * 0.3f) * torsoMovementScale * 0.02f,
            1.0f,
            2.0f
        );
        
        // Right arm
        trackedJoints_[static_cast<int>(JointType::ShoulderRight)] = DirectX::XMFLOAT3(
            0.2f + sinf(time * 0.3f) * 0.03f,
            1.4f + sinf(time * 0.2f) * 0.02f, 
            2.0f
        );
        
        trackedJoints_[static_cast<int>(JointType::ElbowRight)] = DirectX::XMFLOAT3(
            0.25f + sinf(time * 0.6f) * 0.1f,
            1.2f + sinf(time * 0.3f) * 0.1f, 
            2.0f
        );
        
        // Right hand with more complex motion pattern
        trackedJoints_[static_cast<int>(JointType::HandRight)] = DirectX::XMFLOAT3(
            0.3f + sinf(time) * handMovementScale,
            1.0f + cosf(time * 0.5f) * 0.3f, 
            2.0f + sinf(time * 0.7f) * 0.2f
        );
        
        // Left arm
        trackedJoints_[static_cast<int>(JointType::ShoulderLeft)] = DirectX::XMFLOAT3(
            -0.2f + sinf(time * 0.3f) * 0.03f,
            1.4f + sinf(time * 0.2f) * 0.02f, 
            2.0f
        );
        
        trackedJoints_[static_cast<int>(JointType::ElbowLeft)] = DirectX::XMFLOAT3(
            -0.25f + sinf(time * 0.5f) * 0.1f,
            1.2f + sinf(time * 0.3f) * 0.1f, 
            2.0f
        );
        
        // Left hand with different motion pattern from right hand
        trackedJoints_[static_cast<int>(JointType::HandLeft)] = DirectX::XMFLOAT3(
            -0.3f - sinf(time * 1.1f) * handMovementScale,
            1.0f + cosf(time * 0.6f) * 0.3f, 
            2.0f + sinf(time * 0.8f) * 0.2f
        );
        
        // Hip joints
        trackedJoints_[static_cast<int>(JointType::HipRight)] = DirectX::XMFLOAT3(
            0.1f + sinf(time * 0.3f) * 0.01f,
            0.9f, 
            2.0f
        );
        
        trackedJoints_[static_cast<int>(JointType::HipLeft)] = DirectX::XMFLOAT3(
            -0.1f + sinf(time * 0.3f) * 0.01f,
            0.9f, 
            2.0f
        );
        
        // Legs
        trackedJoints_[static_cast<int>(JointType::KneeRight)] = DirectX::XMFLOAT3(
            0.1f + sinf(time * 0.3f) * 0.01f,
            0.6f, 
            2.0f
        );
        
        trackedJoints_[static_cast<int>(JointType::KneeLeft)] = DirectX::XMFLOAT3(
            -0.1f + sinf(time * 0.3f) * 0.01f,
            0.6f, 
            2.0f
        );
        
        trackedJoints_[static_cast<int>(JointType::AnkleRight)] = DirectX::XMFLOAT3(
            0.1f + sinf(time * 0.3f) * 0.01f,
            0.1f, 
            2.0f
        );
        
        trackedJoints_[static_cast<int>(JointType::AnkleLeft)] = DirectX::XMFLOAT3(
            -0.1f + sinf(time * 0.3f) * 0.01f,
            0.1f, 
            2.0f
        );
        
        trackedJoints_[static_cast<int>(JointType::FootRight)] = DirectX::XMFLOAT3(
            0.1f + sinf(time * 0.3f) * 0.01f,
            0.0f, 
            2.1f
        );
        
        trackedJoints_[static_cast<int>(JointType::FootLeft)] = DirectX::XMFLOAT3(
            -0.1f + sinf(time * 0.3f) * 0.01f,
            0.0f, 
            2.1f
        );
        
        // Hands details
        trackedJoints_[static_cast<int>(JointType::HandTipRight)] = DirectX::XMFLOAT3(
            0.32f + sinf(time) * handMovementScale,
            0.95f + cosf(time * 0.5f) * 0.3f, 
            2.05f + sinf(time * 0.7f) * 0.2f
        );
        
        trackedJoints_[static_cast<int>(JointType::HandTipLeft)] = DirectX::XMFLOAT3(
            -0.32f - sinf(time * 1.1f) * handMovementScale,
            0.95f + cosf(time * 0.6f) * 0.3f, 
            2.05f + sinf(time * 0.8f) * 0.2f
        );
        
        // Calculate velocities for all joints
        static float lastUpdateTime = time - 0.016f;
        float deltaT = time - lastUpdateTime;
        lastUpdateTime = time;
        
        // Update confidence values with some variation
        float confidenceBase = 0.5f + sinf(time * 0.1f) * 0.1f;
        
        // Update joint data and calculate velocities
        for (int i = 0; i < static_cast<int>(JointType::Count); i++) {
            // Add joint history
            if (!jointHistory_[i].empty()) {
                // Calculate velocity
                DirectX::XMFLOAT3 prevPos = jointHistory_[i].front();
                jointVelocities_[i] = DirectX::XMFLOAT3(
                    (trackedJoints_[i].x - prevPos.x) / deltaT,
                    (trackedJoints_[i].y - prevPos.y) / deltaT,
                    (trackedJoints_[i].z - prevPos.z) / deltaT
                );
                
                // Shift history and add new position
                for (size_t j = jointHistory_[i].size() - 1; j > 0; j--) {
                    jointHistory_[i][j] = jointHistory_[i][j-1];
                }
                jointHistory_[i][0] = trackedJoints_[i];
            } else {
                jointVelocities_[i] = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
            }
            
            // Set confidence based on joint type (hands and important joints higher confidence)
            if (i == static_cast<int>(JointType::HandRight) ||
                i == static_cast<int>(JointType::HandLeft) ||
                i == static_cast<int>(JointType::Head)) {
                jointConfidence_[i] = std::min(0.95f, confidenceBase + 0.3f);
            } else {
                jointConfidence_[i] = std::min(0.9f, confidenceBase);
            }
        }
        
        // Apply skeletal constraints to ensure physically valid poses
        ApplySkeletalConstraints();
        
        return true;
    } catch (const std::exception& e) {
        Logger::Error("Error in body tracking update: " + std::string(e.what()));
        return false;
    }
}

void MotionControlSystem::UpdateGestureRecognition(float deltaTime) {
    if (!gestureRecognizer_) return;
    
    // Collect joint positions for gesture recognition
    std::vector<DirectX::XMFLOAT3> keyJoints;
    keyJoints.push_back(trackedJoints_[static_cast<int>(JointType::HandRight)]);
    keyJoints.push_back(trackedJoints_[static_cast<int>(JointType::HandLeft)]);
    keyJoints.push_back(trackedJoints_[static_cast<int>(JointType::Head)]);
    
    // Update gesture recognition
    gestureRecognizer_->Update(keyJoints, deltaTime);
    
    // Check for recognized gestures
    auto recognizedGestures = gestureRecognizer_->GetRecognizedGestures();
    for (const auto& gesture : recognizedGestures) {
        if (gesture.confidence > gestureThreshold_) {
            HandleGesture(gesture);
        }
    }
}

void MotionControlSystem::UpdateMotionAiming(float deltaTime) {
    // Get hand position for aiming
    DirectX::XMFLOAT3 rightHand = trackedJoints_[static_cast<int>(JointType::HandRight)];
    
    // Apply motion filtering for smoother aiming
    ApplyMotionFiltering(rightHand);
    
    // Convert hand position to aiming direction
    DirectX::XMFLOAT3 aimDirection = CalculateAimDirection(rightHand);
    
    // Apply smoothing
    currentAimDirection_ = LerpVector3(currentAimDirection_, aimDirection, 1.0f - aimingSmoothing_);
    
    // Update aiming data
    aimingData_.direction = currentAimDirection_;
    aimingData_.confidence = jointConfidence_[static_cast<int>(JointType::HandRight)];
    aimingData_.isActive = (aimingData_.confidence > 0.5f);
}

void MotionControlSystem::UpdateMotionControls(float deltaTime) {
    // Update motion-based movement controls
    UpdateMovementControls();
    
    // Update motion-based interaction controls
    UpdateInteractionControls();
}

void MotionControlSystem::DefineGesturePatterns() {
    if (!gestureRecognizer_) return;
    
    // Define punch gesture
    GesturePattern punchPattern;
    punchPattern.name = "Punch";
    punchPattern.jointSequence = { JointType::HandRight };
    punchPattern.motionThreshold = 2.0f;
    punchPattern.timeWindow = 0.5f;
    gestureRecognizer_->AddPattern(punchPattern);
    
    // Define grab gesture
    GesturePattern grabPattern;
    grabPattern.name = "Grab";
    grabPattern.jointSequence = { JointType::HandRight, JointType::HandLeft };
    grabPattern.motionThreshold = 0.5f;
    grabPattern.timeWindow = 1.0f;
    gestureRecognizer_->AddPattern(grabPattern);
    
    // Define wave gesture
    GesturePattern wavePattern;
    wavePattern.name = "Wave";
    wavePattern.jointSequence = { JointType::HandRight };
    wavePattern.motionThreshold = 1.0f;
    wavePattern.timeWindow = 2.0f;
    gestureRecognizer_->AddPattern(wavePattern);
}

void MotionControlSystem::HandleGesture(const RecognizedGesture& gesture) {
    Logger::Info("Gesture recognized: " + gesture.name + " (confidence: " + std::to_string(gesture.confidence) + ")");
    
    // Handle different gesture types
    if (gesture.name == "Punch") {
        HandlePunchGesture(gesture);
    } else if (gesture.name == "Grab") {
        HandleGrabGesture(gesture);
    } else if (gesture.name == "Wave") {
        HandleWaveGesture(gesture);
    }
}

void MotionControlSystem::HandlePunchGesture(const RecognizedGesture& gesture) {
    // Trigger punch action
    MotionEvent event;
    event.type = MotionEventType::Punch;
    event.position = trackedJoints_[static_cast<int>(JointType::HandRight)];
    event.confidence = gesture.confidence;
    event.timestamp = static_cast<float>(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count()) / 1000.0f;
    
    motionEvents_.push_back(event);
}

void MotionControlSystem::HandleGrabGesture(const RecognizedGesture& gesture) {
    // Trigger grab action
    MotionEvent event;
    event.type = MotionEventType::Grab;
    event.position = trackedJoints_[static_cast<int>(JointType::HandRight)];
    event.confidence = gesture.confidence;
    event.timestamp = static_cast<float>(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count()) / 1000.0f;
    
    motionEvents_.push_back(event);
}

void MotionControlSystem::HandleWaveGesture(const RecognizedGesture& gesture) {
    // Trigger wave action
    MotionEvent event;
    event.type = MotionEventType::Wave;
    event.position = trackedJoints_[static_cast<int>(JointType::HandRight)];
    event.confidence = gesture.confidence;
    event.timestamp = static_cast<float>(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count()) / 1000.0f;
    
    motionEvents_.push_back(event);
}

void MotionControlSystem::ApplyMotionFiltering(DirectX::XMFLOAT3& position) {
    // Get current timestamp for velocity-aware filtering
    float currentTime = static_cast<float>(std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count()) / 1000.0f;
    
    // Store position and timestamp
    motionFilter_.positions.push_back(position);
    motionFilter_.timestamps.push_back(currentTime);
    
    // Keep buffer size within limit
    while (motionFilter_.positions.size() > motionFilter_.windowSize) {
        motionFilter_.positions.erase(motionFilter_.positions.begin());
        motionFilter_.timestamps.erase(motionFilter_.timestamps.begin());
    }
    
    // Advanced filtering based on tracking quality setting
    switch (trackingQuality_) {
        case TrackingQuality::Ultra:
            // Minimal filtering - just use latest position with slight smoothing
            if (motionFilter_.positions.size() >= 2) {
                DirectX::XMFLOAT3 latestPos = motionFilter_.positions.back();
                DirectX::XMFLOAT3 prevPos = motionFilter_.positions[motionFilter_.positions.size() - 2];
                position = LerpVector3(prevPos, latestPos, 0.7f); // Favor latest position
            }
            break;
            
        case TrackingQuality::High:
            // Simple weighted average with bias toward more recent samples
            ApplyWeightedFiltering(position, 0.8f); // Higher responsiveness
            break;
            
        case TrackingQuality::Medium:
            // Default - balanced weighted filtering
            ApplyWeightedFiltering(position, 0.6f);
            break;
            
        case TrackingQuality::Low:
            // Apply more aggressive filtering with prediction based on velocity
            ApplyPredictiveFiltering(position);
            break;
    }
    
    // Apply jitter reduction for smoother movement
    ApplyJitterReduction(position);
}

void MotionControlSystem::ApplyWeightedFiltering(DirectX::XMFLOAT3& position, float responsiveness) {
    if (motionFilter_.positions.empty()) return;
    
    // Calculate weighted average with temporal decay
    DirectX::XMFLOAT3 filteredPosition(0.0f, 0.0f, 0.0f);
    float totalWeight = 0.0f;
    float latestTime = motionFilter_.timestamps.back();
    
    for (size_t i = 0; i < motionFilter_.positions.size(); i++) {
        // Calculate weight based on both predefined weights and temporal decay
        float baseWeight = (i < motionFilter_.weights.size()) ? motionFilter_.weights[i] : 1.0f;
        float timeOffset = latestTime - motionFilter_.timestamps[i];
        float temporalDecay = exp(-timeOffset * 5.0f * responsiveness); // Faster decay for higher responsiveness
        
        float weight = baseWeight * temporalDecay;
        
        filteredPosition.x += motionFilter_.positions[i].x * weight;
        filteredPosition.y += motionFilter_.positions[i].y * weight;
        filteredPosition.z += motionFilter_.positions[i].z * weight;
        totalWeight += weight;
    }
    
    if (totalWeight > 0.0f) {
        filteredPosition.x /= totalWeight;
        filteredPosition.y /= totalWeight;
        filteredPosition.z /= totalWeight;
        position = filteredPosition;
    }
}

void MotionControlSystem::ApplyPredictiveFiltering(DirectX::XMFLOAT3& position) {
    if (motionFilter_.positions.size() < 3) return;
    
    // Calculate average velocity over the last few positions
    DirectX::XMFLOAT3 avgVelocity(0.0f, 0.0f, 0.0f);
    float totalTime = 0.0f;
    
    for (size_t i = 1; i < motionFilter_.positions.size(); i++) {
        float dt = motionFilter_.timestamps[i] - motionFilter_.timestamps[i-1];
        if (dt > 0.0001f) { // Avoid division by very small numbers
            DirectX::XMFLOAT3 velocity(
                (motionFilter_.positions[i].x - motionFilter_.positions[i-1].x) / dt,
                (motionFilter_.positions[i].y - motionFilter_.positions[i-1].y) / dt,
                (motionFilter_.positions[i].z - motionFilter_.positions[i-1].z) / dt
            );
            
            avgVelocity.x += velocity.x;
            avgVelocity.y += velocity.y;
            avgVelocity.z += velocity.z;
            totalTime += dt;
        }
    }
    
    // If we have valid velocity data, perform filtering with prediction
    if (totalTime > 0.0f) {
        // Calculate final average velocity
        avgVelocity.x /= (motionFilter_.positions.size() - 1);
        avgVelocity.y /= (motionFilter_.positions.size() - 1);
        avgVelocity.z /= (motionFilter_.positions.size() - 1);
        
        // Store velocity for future use
        motionFilter_.velocities.push_back(avgVelocity);
        
        while (motionFilter_.velocities.size() > motionFilter_.windowSize) {
            motionFilter_.velocities.erase(motionFilter_.velocities.begin());
        }
        
        // Perform standard weighted filtering
        ApplyWeightedFiltering(position, 0.4f); // Use lower responsiveness for more smoothing
        
        // Add prediction component based on velocity (look ahead by a small amount)
        float lookAheadTime = 0.05f; // 50ms prediction
        position.x += avgVelocity.x * lookAheadTime;
        position.y += avgVelocity.y * lookAheadTime;
        position.z += avgVelocity.z * lookAheadTime;
    }
}

void MotionControlSystem::ApplyJitterReduction(DirectX::XMFLOAT3& position) {
    if (motionFilter_.positions.size() < 2) return;
    
    // Get the previous filtered position
    DirectX::XMFLOAT3 prevFiltered = motionFilter_.positions[motionFilter_.positions.size() - 2];
    
    // Calculate distance between current and previous position
    float dx = position.x - prevFiltered.x;
    float dy = position.y - prevFiltered.y;
    float dz = position.z - prevFiltered.z;
    float distance = sqrtf(dx*dx + dy*dy + dz*dz);
    
    // If the movement is very small (likely jitter), reduce its impact
    if (distance < 0.01f) { // Small movement threshold
        // Increase smoothing for small movements (reduce jitter)
        position = LerpVector3(prevFiltered, position, 0.3f); // 30% new, 70% old
    } else if (distance > 0.1f) {
        // Fast movement - reduce smoothing for better responsiveness
        position = LerpVector3(prevFiltered, position, 0.7f); // 70% new, 30% old
    } else {
        // Normal movement - standard smoothing
        position = LerpVector3(prevFiltered, position, 0.5f); // 50-50 blend
    }
}

DirectX::XMFLOAT3 MotionControlSystem::CalculateAimDirection(const DirectX::XMFLOAT3& handPosition) {
    // Calculate aiming direction based on hand position
    // This is a simplified calculation - in reality, you'd use both hands and body orientation
    
    DirectX::XMFLOAT3 centerPosition(0.0f, 1.0f, 0.0f); // Assume center of screen/body
    
    DirectX::XMFLOAT3 direction;
    direction.x = (handPosition.x - centerPosition.x) * motionSensitivity_;
    direction.y = (handPosition.y - centerPosition.y) * motionSensitivity_;
    direction.z = 1.0f; // Forward direction
    
    // Normalize direction
    DirectX::XMVECTOR dir = DirectX::XMLoadFloat3(&direction);
    dir = DirectX::XMVector3Normalize(dir);
    DirectX::XMStoreFloat3(&direction, dir);
    
    return direction;
}

DirectX::XMFLOAT3 MotionControlSystem::LerpVector3(const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b, float t) {
    return DirectX::XMFLOAT3(
        a.x + (b.x - a.x) * t,
        a.y + (b.y - a.y) * t,
        a.z + (b.z - a.z) * t
    );
}

void MotionControlSystem::UpdateMovementControls() {
    // Use body lean for movement
    DirectX::XMFLOAT3 spine = trackedJoints_[static_cast<int>(JointType::SpineBase)];
    DirectX::XMFLOAT3 head = trackedJoints_[static_cast<int>(JointType::Head)];
    
    // Calculate lean direction
    DirectX::XMFLOAT3 leanDirection;
    leanDirection.x = head.x - spine.x;
    leanDirection.y = 0.0f; // Don't use vertical lean for movement
    leanDirection.z = head.z - spine.z;
    
    // Apply movement based on lean
    if (abs(leanDirection.x) > 0.1f || abs(leanDirection.z) > 0.1f) {
        MotionEvent event;
        event.type = MotionEventType::Movement;
        event.direction = leanDirection;
        event.confidence = jointConfidence_[static_cast<int>(JointType::Head)];
        event.timestamp = static_cast<float>(std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count()) / 1000.0f;
        
        motionEvents_.push_back(event);
    }
}

void MotionControlSystem::UpdateInteractionControls() {
    // Check for interaction gestures
    DirectX::XMFLOAT3 leftHand = trackedJoints_[static_cast<int>(JointType::HandLeft)];
    DirectX::XMFLOAT3 rightHand = trackedJoints_[static_cast<int>(JointType::HandRight)];
    
    // Check for pointing gesture
    if (rightHand.y > 1.2f && jointConfidence_[static_cast<int>(JointType::HandRight)] > 0.8f) {
        MotionEvent event;
        event.type = MotionEventType::Point;
        event.position = rightHand;
        event.confidence = jointConfidence_[static_cast<int>(JointType::HandRight)];
        event.timestamp = static_cast<float>(std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count()) / 1000.0f;
        
        motionEvents_.push_back(event);
    }
}

// Public interface methods
DirectX::XMFLOAT3 MotionControlSystem::GetJointPosition(JointType joint) const {
    int index = static_cast<int>(joint);
    if (index >= 0 && index < static_cast<int>(JointType::Count)) {
        return trackedJoints_[index];
    }
    return DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
}

float MotionControlSystem::GetJointConfidence(JointType joint) const {
    int index = static_cast<int>(joint);
    if (index >= 0 && index < static_cast<int>(JointType::Count)) {
        return jointConfidence_[index];
    }
    return 0.0f;
}

MotionAimingData MotionControlSystem::GetAimingData() const {
    return aimingData_;
}

std::vector<MotionEvent> MotionControlSystem::GetMotionEvents() {
    std::vector<MotionEvent> events = motionEvents_;
    motionEvents_.clear(); // Clear events after retrieval
    return events;
}

void MotionControlSystem::SetMotionSensitivity(float sensitivity) {
    motionSensitivity_ = std::clamp(sensitivity, 0.1f, 5.0f);
}

void MotionControlSystem::SetAimingSmoothing(float smoothing) {
    aimingSmoothing_ = std::clamp(smoothing, 0.0f, 0.95f);
}

void MotionControlSystem::SetGestureThreshold(float threshold) {
    gestureThreshold_ = std::clamp(threshold, 0.1f, 1.0f);
}

void MotionControlSystem::StartCalibration() {
    calibrationMode_ = true;
    Logger::Info("Motion control calibration started");
}

void MotionControlSystem::StopCalibration() {
    calibrationMode_ = false;
    Logger::Info("Motion control calibration completed");
}

bool MotionControlSystem::IsCalibrating() const {
    return calibrationMode_;
}

// GestureRecognizer implementations
void GestureRecognizer::AddPattern(const GesturePattern& pattern) {
    patterns_.push_back(pattern);
}

std::vector<RecognizedGesture> GestureRecognizer::GetRecognizedGestures() const {
    return recognizedGestures_;
}

void GestureRecognizer::ProcessJoints(const std::vector<DirectX::XMFLOAT3>& joints) {
    currentJoints_ = joints;
    // Basic gesture processing logic would go here
}

void GestureRecognizer::Update(const std::vector<DirectX::XMFLOAT3>& joints, float deltaTime) {
    ProcessJoints(joints);
    
    // Simple gesture recognition logic
    recognizedGestures_.clear();
    
    // For now, just a placeholder implementation
    // Real gesture recognition would analyze joint movements over time
}

void GestureRecognizer::Reset() {
    recognizedGestures_.clear();
    currentJoints_.clear();
    gestureStartTime_ = 0.0f;
    isRecognizing_ = false;
}

// New methods added for improved motion control system

void MotionControlSystem::InitializeSkeletalConstraints() {
    // Define physical constraints between joints to ensure valid poses
    // These constraints define valid ranges of motion between connected joints
    
    // Structure: joint pair and their valid distance range (min/max)
    skeletalConstraints_.push_back({
        JointType::Head, JointType::Neck,
        0.08f, 0.25f // Head to neck distance range in meters
    });
    
    skeletalConstraints_.push_back({
        JointType::Neck, JointType::SpineShoulder,
        0.05f, 0.15f
    });
    
    skeletalConstraints_.push_back({
        JointType::SpineShoulder, JointType::SpineMid,
        0.1f, 0.3f
    });
    
    skeletalConstraints_.push_back({
        JointType::SpineMid, JointType::SpineBase,
        0.1f, 0.35f
    });
    
    skeletalConstraints_.push_back({
        JointType::SpineShoulder, JointType::ShoulderRight,
        0.1f, 0.25f
    });
    
    skeletalConstraints_.push_back({
        JointType::ShoulderRight, JointType::ElbowRight,
        0.2f, 0.4f
    });
    
    skeletalConstraints_.push_back({
        JointType::ElbowRight, JointType::HandRight,
        0.2f, 0.4f
    });
    
    skeletalConstraints_.push_back({
        JointType::SpineShoulder, JointType::ShoulderLeft,
        0.1f, 0.25f
    });
    
    skeletalConstraints_.push_back({
        JointType::ShoulderLeft, JointType::ElbowLeft,
        0.2f, 0.4f
    });
    
    skeletalConstraints_.push_back({
        JointType::ElbowLeft, JointType::HandLeft,
        0.2f, 0.4f
    });
}

void MotionControlSystem::ApplySkeletalConstraints() {
    // Apply constraints to ensure physically valid poses
    for (const auto& constraint : skeletalConstraints_) {
        int jointA = static_cast<int>(constraint.jointA);
        int jointB = static_cast<int>(constraint.jointB);
        
        // Calculate current distance between joints
        DirectX::XMFLOAT3 posA = trackedJoints_[jointA];
        DirectX::XMFLOAT3 posB = trackedJoints_[jointB];
        
        float dx = posA.x - posB.x;
        float dy = posA.y - posB.y;
        float dz = posA.z - posB.z;
        float distance = sqrtf(dx*dx + dy*dy + dz*dz);
        
        // Check if constraint is violated
        if (distance < constraint.minDistance || distance > constraint.maxDistance) {
            // Constraint violated, adjust positions
            float targetDistance = std::clamp(distance, constraint.minDistance, constraint.maxDistance);
            float ratio = targetDistance / (distance + FLT_EPSILON); // Avoid division by zero
            
            // Calculate adjustment vector
            float adjustX = dx * (1.0f - ratio) * 0.5f;
            float adjustY = dy * (1.0f - ratio) * 0.5f;
            float adjustZ = dz * (1.0f - ratio) * 0.5f;
            
            // Apply adjustment based on confidence (joints with higher confidence move less)
            float confidenceA = jointConfidence_[jointA];
            float confidenceB = jointConfidence_[jointB];
            float totalConfidence = confidenceA + confidenceB;
            
            if (totalConfidence > 0.0f) {
                float weightA = confidenceB / totalConfidence;
                float weightB = confidenceA / totalConfidence;
                
                // Update positions to satisfy constraint
                trackedJoints_[jointA].x -= adjustX * weightA;
                trackedJoints_[jointA].y -= adjustY * weightA;
                trackedJoints_[jointA].z -= adjustZ * weightA;
                
                trackedJoints_[jointB].x += adjustX * weightB;
                trackedJoints_[jointB].y += adjustY * weightB;
                trackedJoints_[jointB].z += adjustZ * weightB;
            }
        }
    }
}

void MotionControlSystem::AttemptTrackingRecovery() {
    Logger::Warning("Attempting to recover tracking system...");
    
    // Reset tracking failures counter
    consecutiveTrackingFailures_ = 0;
    
    // Try to reinitialize Kinect if it's the primary tracking system
    bool success = InitializeKinect();
    
    if (!success) {
        Logger::Warning("Failed to recover Kinect tracking, switching to fallback tracking");
        success = InitializeFallbackTracking();
    }
    
    if (success) {
        Logger::Info("Tracking recovery successful");
    } else {
        Logger::Error("Tracking recovery failed - disabling tracking temporarily");
        trackingEnabled_ = false;
        
        // Schedule automatic retry after a delay
        // In a real implementation, this would use a timer or thread
        Logger::Info("Will attempt to re-enable tracking automatically after delay");
    }
}

void MotionControlSystem::AttemptSystemRecovery() {
    Logger::Warning("Motion Control System encountered multiple errors - attempting recovery");
    
    // Reset error counter
    errorCount_ = 0;
    
    // Temporarily disable tracking during recovery
    trackingEnabled_ = false;
    
    try {
        // Clean up resources
        Shutdown();
        
        // Wait briefly
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        // Try to reinitialize
        if (Initialize()) {
            Logger::Info("Motion control system recovery successful");
            trackingEnabled_ = true;
        } else {
            Logger::Error("Motion control system recovery failed");
        }
    } catch (const std::exception& e) {
        Logger::Error("Exception during system recovery: " + std::string(e.what()));
    }
}

bool MotionControlSystem::InitializeFallbackTracking() {
    // Initialize alternative tracking method when Kinect is not available
    Logger::Info("Initializing fallback tracking system");
    
    // In a real implementation, this would initialize webcam tracking, 
    // mouse/keyboard simulation, or other fallback systems
    
    // For this example, we'll just set up the simulated tracking
    InitializeMotionTracking();
    
    return true;
}

void MotionControlSystem::ConfigureFilteringByQuality(TrackingQuality quality) {
    switch (quality) {
        case TrackingQuality::Ultra:
            motionSensitivity_ = 1.5f;
            aimingSmoothing_ = 0.5f; // Less smoothing for more responsiveness
            break;
            
        case TrackingQuality::High:
            motionSensitivity_ = 1.2f;
            aimingSmoothing_ = 0.7f;
            break;
            
        case TrackingQuality::Medium:
            motionSensitivity_ = 1.0f;
            aimingSmoothing_ = 0.8f;
            break;
            
        case TrackingQuality::Low:
            motionSensitivity_ = 0.8f;
            aimingSmoothing_ = 0.9f; // More smoothing for stability
            break;
    }
}

void MotionControlSystem::UpdateCalibration(float deltaTime) {
    static float calibrationTime = 0.0f;
    calibrationTime += deltaTime;
    
    // Get head and hand positions for calibration
    DirectX::XMFLOAT3 head = trackedJoints_[static_cast<int>(JointType::Head)];
    DirectX::XMFLOAT3 rightHand = trackedJoints_[static_cast<int>(JointType::HandRight)];
    DirectX::XMFLOAT3 leftHand = trackedJoints_[static_cast<int>(JointType::HandLeft)];
    
    // Record range of motion
    UpdateCalibrationRange(head, rightHand, leftHand);
    
    // After 5 seconds of calibration, finalize
    if (calibrationTime > 5.0f) {
        FinalizeCalibration();
        calibrationMode_ = false;
        calibrationTime = 0.0f;
    }
}

void MotionControlSystem::UpdateCalibrationRange(const DirectX::XMFLOAT3& head, 
                                              const DirectX::XMFLOAT3& rightHand, 
                                              const DirectX::XMFLOAT3& leftHand) {
    // Update min/max ranges for calibration
    // In a real implementation, this would track the user's range of motion
    // during calibration to optimize sensitivity settings
    
    // For now, just simulate the calibration process
    Logger::Info("Calibrating: Please move your arms through full range of motion...");
}

void MotionControlSystem::FinalizeCalibration() {
    Logger::Info("Calibration complete - optimizing tracking parameters");
    
    // In a real implementation, this would analyze the calibration data
    // and set optimal sensitivity/smoothing parameters
    
    // For demonstration, just set to medium quality
    SetTrackingQuality(TrackingQuality::Medium);
}

void MotionControlSystem::SetTrackingQuality(TrackingQuality quality) {
    trackingQuality_ = quality;
    ConfigureFilteringByQuality(quality);
    Logger::Info("Tracking quality set to: " + std::to_string(static_cast<int>(quality)));
}

} // namespace Nexus
