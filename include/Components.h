#pragma once

#include "ECS.h"
#include <DirectXMath.h>
#include <string>

namespace Nexus {

// ============================================
// CORE COMPONENTS
// ============================================

struct TransformComponent {
    DirectX::XMFLOAT3 position = {0.0f, 0.0f, 0.0f};
    DirectX::XMFLOAT3 rotation = {0.0f, 0.0f, 0.0f}; // Euler angles in degrees
    DirectX::XMFLOAT3 scale = {1.0f, 1.0f, 1.0f};

    DirectX::XMMATRIX GetMatrix() const {
        DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(position.x, position.y, position.z);
        DirectX::XMMATRIX rotationX = DirectX::XMMatrixRotationX(DirectX::XMConvertToRadians(rotation.x));
        DirectX::XMMATRIX rotationY = DirectX::XMMatrixRotationY(DirectX::XMConvertToRadians(rotation.y));
        DirectX::XMMATRIX rotationZ = DirectX::XMMatrixRotationZ(DirectX::XMConvertToRadians(rotation.z));
        DirectX::XMMATRIX scaling = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);

        return scaling * rotationX * rotationY * rotationZ * translation;
    }
};

struct NameComponent {
    std::string name = "Entity";
};

struct TagComponent {
    std::vector<std::string> tags;

    void AddTag(const std::string& tag) {
        tags.push_back(tag);
    }

    bool HasTag(const std::string& tag) const {
        return std::find(tags.begin(), tags.end(), tag) != tags.end();
    }
};

// ============================================
// RENDERING COMPONENTS
// ============================================

struct MeshComponent {
    uint32_t meshId = 0;
    uint32_t materialId = 0;
    bool visible = true;
    bool castsShadows = true;
    bool receivesShadows = true;
};

struct LightComponent {
    enum class Type {
        Directional,
        Point,
        Spot
    };

    Type type = Type::Point;
    DirectX::XMFLOAT3 color = {1.0f, 1.0f, 1.0f};
    float intensity = 1.0f;
    float range = 10.0f;
    float spotAngle = 45.0f;
    bool castsShadows = true;
};

struct CameraComponent {
    float fov = 60.0f;
    float nearPlane = 0.1f;
    float farPlane = 1000.0f;
    bool isActive = true;

    DirectX::XMMATRIX GetProjectionMatrix(float aspectRatio) const {
        return DirectX::XMMatrixPerspectiveFovLH(
            DirectX::XMConvertToRadians(fov),
            aspectRatio,
            nearPlane,
            farPlane
        );
    }
};

// ============================================
// PHYSICS COMPONENTS
// ============================================

struct RigidbodyComponent {
    DirectX::XMFLOAT3 velocity = {0.0f, 0.0f, 0.0f};
    DirectX::XMFLOAT3 angularVelocity = {0.0f, 0.0f, 0.0f};
    float mass = 1.0f;
    float drag = 0.01f;
    float angularDrag = 0.05f;
    bool useGravity = true;
    bool isKinematic = false;
    void* physicsBody = nullptr; // Jolt physics body
};

struct ColliderComponent {
    enum class Type {
        Box,
        Sphere,
        Capsule,
        Mesh
    };

    Type type = Type::Box;
    DirectX::XMFLOAT3 center = {0.0f, 0.0f, 0.0f};
    DirectX::XMFLOAT3 size = {1.0f, 1.0f, 1.0f};
    float radius = 0.5f;
    float height = 2.0f;
    bool isTrigger = false;
};

// ============================================
// AI COMPONENTS
// ============================================

struct AIComponent {
    enum class State {
        Idle,
        Patrol,
        Chase,
        Attack,
        Flee
    };

    State currentState = State::Idle;
    Entity targetEntity = NULL_ENTITY;
    float detectionRange = 10.0f;
    float attackRange = 2.0f;
    float moveSpeed = 3.0f;
    std::vector<DirectX::XMFLOAT3> patrolPoints;
    int currentPatrolPoint = 0;
};

struct NavMeshAgentComponent {
    DirectX::XMFLOAT3 destination = {0.0f, 0.0f, 0.0f};
    float speed = 3.5f;
    float acceleration = 8.0f;
    float stoppingDistance = 0.5f;
    bool hasPath = false;
    std::vector<DirectX::XMFLOAT3> path;
    int currentPathNode = 0;
};

// ============================================
// GAMEPLAY COMPONENTS
// ============================================

struct HealthComponent {
    float maxHealth = 100.0f;
    float currentHealth = 100.0f;
    bool isAlive = true;

    void TakeDamage(float damage) {
        currentHealth -= damage;
        if (currentHealth <= 0.0f) {
            currentHealth = 0.0f;
            isAlive = false;
        }
    }

    void Heal(float amount) {
        currentHealth += amount;
        if (currentHealth > maxHealth) {
            currentHealth = maxHealth;
        }
    }

    float GetHealthPercent() const {
        return currentHealth / maxHealth;
    }
};

struct CharacterControllerComponent {
    float moveSpeed = 5.0f;
    float sprintSpeed = 8.0f;
    float jumpHeight = 2.0f;
    float gravity = 9.81f;
    bool isGrounded = false;
    bool isSprinting = false;
    DirectX::XMFLOAT3 moveDirection = {0.0f, 0.0f, 0.0f};
    float verticalVelocity = 0.0f;
};

struct PlayerInputComponent {
    DirectX::XMFLOAT2 moveInput = {0.0f, 0.0f};
    DirectX::XMFLOAT2 lookInput = {0.0f, 0.0f};
    bool jumpPressed = false;
    bool sprintPressed = false;
    bool firePressed = false;
};

// ============================================
// PARTICLE COMPONENTS
// ============================================

struct ParticleEmitterComponent {
    uint32_t maxParticles = 1000;
    uint32_t particlesPerSecond = 100;
    float particleLifetime = 2.0f;
    DirectX::XMFLOAT3 emissionDirection = {0.0f, 1.0f, 0.0f};
    float emissionAngle = 15.0f;
    float startSpeed = 5.0f;
    DirectX::XMFLOAT3 startColor = {1.0f, 1.0f, 1.0f};
    DirectX::XMFLOAT3 endColor = {1.0f, 1.0f, 1.0f};
    float startSize = 1.0f;
    float endSize = 0.0f;
    bool useGravity = false;
    bool isActive = true;
    void* particleSystemHandle = nullptr;
};

// ============================================
// AUDIO COMPONENTS
// ============================================

struct AudioSourceComponent {
    uint32_t soundId = 0;
    float volume = 1.0f;
    float pitch = 1.0f;
    bool is3D = true;
    bool isLooping = false;
    bool isPlaying = false;
    float minDistance = 1.0f;
    float maxDistance = 100.0f;
};

struct AudioListenerComponent {
    bool isActive = true;
};

// ============================================
// ANIMATION COMPONENTS
// ============================================

struct AnimatorComponent {
    uint32_t currentAnimation = 0;
    float animationTime = 0.0f;
    float playbackSpeed = 1.0f;
    bool isPlaying = true;
    bool isLooping = true;
    std::vector<uint32_t> animations;
};

// ============================================
// SCRIPT COMPONENTS
// ============================================

struct ScriptComponent {
    std::string scriptPath;
    void* scriptInstance = nullptr;
    bool isActive = true;
};

// ============================================
// UTILITY COMPONENTS
// ============================================

struct LifetimeComponent {
    float lifetime = 5.0f;
    float elapsed = 0.0f;

    bool ShouldDestroy(float deltaTime) {
        elapsed += deltaTime;
        return elapsed >= lifetime;
    }
};

struct ParentComponent {
    Entity parent = NULL_ENTITY;
};

struct ChildrenComponent {
    std::vector<Entity> children;

    void AddChild(Entity child) {
        children.push_back(child);
    }

    void RemoveChild(Entity child) {
        children.erase(std::remove(children.begin(), children.end(), child), children.end());
    }
};

} // namespace Nexus
