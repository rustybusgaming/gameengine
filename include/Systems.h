#pragma once

#include "Components.h"
#include "ECS.h"
#include <DirectXMath.h>
#include <algorithm>

namespace Nexus {

// Character Controller System - handles player movement
class CharacterControllerSystem : public System {
public:
    void Update(float deltaTime) override {
        for (auto entity : entities) {
            auto& transform = ecs_->GetComponent<TransformComponent>(entity);
            auto& controller = ecs_->GetComponent<CharacterControllerComponent>(entity);
            auto& rigidbody = ecs_->GetComponent<RigidbodyComponent>(entity);

            // Apply movement input
            DirectX::XMFLOAT3 movement = controller.moveDirection;
            float speed = controller.isSprinting ? controller.sprintSpeed : controller.moveSpeed;

            // Normalize movement vector
            DirectX::XMVECTOR movementVec = DirectX::XMLoadFloat3(&movement);
            if (DirectX::XMVectorGetX(DirectX::XMVector3Length(movementVec)) > 0.1f) {
                movementVec = DirectX::XMVector3Normalize(movementVec);
                DirectX::XMStoreFloat3(&movement, movementVec);

                // Apply horizontal movement
                rigidbody.velocity.x = movement.x * speed;
                rigidbody.velocity.z = movement.z * speed;
            } else {
                // Apply drag when not moving
                rigidbody.velocity.x *= 0.9f;
                rigidbody.velocity.z *= 0.9f;
            }

            // Apply gravity
            if (!controller.isGrounded) {
                controller.verticalVelocity -= controller.gravity * deltaTime;
            } else {
                controller.verticalVelocity = 0.0f;

                // Handle jump
                if (ecs_->HasComponent<PlayerInputComponent>(entity)) {
                    auto& input = ecs_->GetComponent<PlayerInputComponent>(entity);
                    if (input.jumpPressed) {
                        controller.verticalVelocity = sqrtf(2.0f * controller.gravity * controller.jumpHeight);
                        controller.isGrounded = false;
                    }
                }
            }

            // Apply vertical velocity
            rigidbody.velocity.y = controller.verticalVelocity;

            // Update position
            transform.position.x += rigidbody.velocity.x * deltaTime;
            transform.position.y += rigidbody.velocity.y * deltaTime;
            transform.position.z += rigidbody.velocity.z * deltaTime;

            // Simple ground check (y <= 0)
            if (transform.position.y <= 0.0f) {
                transform.position.y = 0.0f;
                controller.isGrounded = true;
            } else {
                controller.isGrounded = false;
            }
        }
    }

    void SetECS(ECS* ecs) { ecs_ = ecs; }

private:
    ECS* ecs_ = nullptr;
};

// Player Input System - processes input and updates PlayerInputComponent
class PlayerInputSystem : public System {
public:
    void Update(float deltaTime) override {
        for (auto entity : entities) {
            auto& input = ecs_->GetComponent<PlayerInputComponent>(entity);
            auto& controller = ecs_->GetComponent<CharacterControllerComponent>(entity);

            // Convert 2D input to 3D movement direction
            controller.moveDirection.x = input.moveInput.x;
            controller.moveDirection.y = 0.0f;
            controller.moveDirection.z = input.moveInput.y;

            controller.isSprinting = input.sprintPressed;

            // Reset one-frame inputs
            input.jumpPressed = false;
        }
    }

    void SetECS(ECS* ecs) { ecs_ = ecs; }

private:
    ECS* ecs_ = nullptr;
};

// Simple Physics System - updates transforms based on velocities
class PhysicsSystem : public System {
public:
    void Update(float deltaTime) override {
        for (auto entity : entities) {
            auto& transform = ecs_->GetComponent<TransformComponent>(entity);
            auto& rigidbody = ecs_->GetComponent<RigidbodyComponent>(entity);

            if (rigidbody.isKinematic) continue;

            // Apply gravity
            if (rigidbody.useGravity) {
                rigidbody.velocity.y -= 9.81f * deltaTime;
            }

            // Apply drag
            rigidbody.velocity.x *= (1.0f - rigidbody.drag);
            rigidbody.velocity.y *= (1.0f - rigidbody.drag);
            rigidbody.velocity.z *= (1.0f - rigidbody.drag);

            // Update position
            transform.position.x += rigidbody.velocity.x * deltaTime;
            transform.position.y += rigidbody.velocity.y * deltaTime;
            transform.position.z += rigidbody.velocity.z * deltaTime;
        }
    }

    void SetECS(ECS* ecs) { ecs_ = ecs; }

private:
    ECS* ecs_ = nullptr;
};

// Lifetime System - destroys entities after their lifetime expires
class LifetimeSystem : public System {
public:
    void Update(float deltaTime) override {
        std::vector<Entity> toDestroy;

        for (auto entity : entities) {
            auto& lifetime = ecs_->GetComponent<LifetimeComponent>(entity);

            if (lifetime.ShouldDestroy(deltaTime)) {
                toDestroy.push_back(entity);
            }
        }

        // Destroy expired entities
        for (auto entity : toDestroy) {
            ecs_->DestroyEntity(entity);
        }
    }

    void SetECS(ECS* ecs) { ecs_ = ecs; }

private:
    ECS* ecs_ = nullptr;
};

// Simple AI System - basic AI behaviors
class AISystem : public System {
public:
    void Update(float deltaTime) override {
        for (auto entity : entities) {
            auto& ai = ecs_->GetComponent<AIComponent>(entity);
            auto& transform = ecs_->GetComponent<TransformComponent>(entity);

            switch (ai.currentState) {
                case AIComponent::State::Idle:
                    // Just wait
                    break;

                case AIComponent::State::Patrol:
                    if (!ai.patrolPoints.empty()) {
                        DirectX::XMFLOAT3 target = ai.patrolPoints[ai.currentPatrolPoint];
                        MoveTowards(transform, target, ai.moveSpeed, deltaTime);

                        // Check if reached patrol point
                        float dist = Distance(transform.position, target);
                        if (dist < 1.0f) {
                            ai.currentPatrolPoint = (ai.currentPatrolPoint + 1) % ai.patrolPoints.size();
                        }
                    }
                    break;

                case AIComponent::State::Chase:
                    if (ai.targetEntity != NULL_ENTITY && ecs_->HasComponent<TransformComponent>(ai.targetEntity)) {
                        auto& targetTransform = ecs_->GetComponent<TransformComponent>(ai.targetEntity);
                        float dist = Distance(transform.position, targetTransform.position);

                        if (dist > ai.detectionRange) {
                            ai.currentState = AIComponent::State::Patrol;
                        } else if (dist < ai.attackRange) {
                            ai.currentState = AIComponent::State::Attack;
                        } else {
                            MoveTowards(transform, targetTransform.position, ai.moveSpeed, deltaTime);
                        }
                    }
                    break;

                case AIComponent::State::Attack:
                    // Attack logic would go here
                    break;

                case AIComponent::State::Flee:
                    if (ai.targetEntity != NULL_ENTITY && ecs_->HasComponent<TransformComponent>(ai.targetEntity)) {
                        auto& targetTransform = ecs_->GetComponent<TransformComponent>(ai.targetEntity);
                        DirectX::XMFLOAT3 fleeDir = {
                            transform.position.x - targetTransform.position.x,
                            0.0f,
                            transform.position.z - targetTransform.position.z
                        };
                        MoveInDirection(transform, fleeDir, ai.moveSpeed, deltaTime);
                    }
                    break;
            }
        }
    }

    void SetECS(ECS* ecs) { ecs_ = ecs; }

private:
    ECS* ecs_ = nullptr;

    void MoveTowards(TransformComponent& transform, const DirectX::XMFLOAT3& target, float speed, float deltaTime) {
        DirectX::XMFLOAT3 direction = {
            target.x - transform.position.x,
            0.0f,
            target.z - transform.position.z
        };

        DirectX::XMVECTOR dirVec = DirectX::XMLoadFloat3(&direction);
        dirVec = DirectX::XMVector3Normalize(dirVec);
        DirectX::XMStoreFloat3(&direction, dirVec);

        transform.position.x += direction.x * speed * deltaTime;
        transform.position.z += direction.z * speed * deltaTime;
    }

    void MoveInDirection(TransformComponent& transform, const DirectX::XMFLOAT3& direction, float speed, float deltaTime) {
        DirectX::XMVECTOR dirVec = DirectX::XMLoadFloat3(&direction);
        dirVec = DirectX::XMVector3Normalize(dirVec);
        DirectX::XMFLOAT3 normalizedDir;
        DirectX::XMStoreFloat3(&normalizedDir, dirVec);

        transform.position.x += normalizedDir.x * speed * deltaTime;
        transform.position.z += normalizedDir.z * speed * deltaTime;
    }

    float Distance(const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b) {
        float dx = b.x - a.x;
        float dz = b.z - a.z;
        return sqrtf(dx * dx + dz * dz);
    }
};

// Particle Update System - updates particle emitters
class ParticleSystem : public System {
public:
    void Update(float deltaTime) override {
        for (auto entity : entities) {
            auto& emitter = ecs_->GetComponent<ParticleEmitterComponent>(entity);

            if (!emitter.isActive) continue;

            // Particle spawning and update logic would go here
            // This is a placeholder for the full particle system
        }
    }

    void SetECS(ECS* ecs) { ecs_ = ecs; }

private:
    ECS* ecs_ = nullptr;
};

// Audio System - updates 3D audio sources
class AudioSystem : public System {
public:
    void Update(float deltaTime) override {
        Entity listenerEntity = NULL_ENTITY;
        DirectX::XMFLOAT3 listenerPos = {0.0f, 0.0f, 0.0f};

        // Find listener
        for (auto entity : entities) {
            if (ecs_->HasComponent<AudioListenerComponent>(entity)) {
                auto& listener = ecs_->GetComponent<AudioListenerComponent>(entity);
                if (listener.isActive) {
                    listenerEntity = entity;
                    auto& transform = ecs_->GetComponent<TransformComponent>(entity);
                    listenerPos = transform.position;
                    break;
                }
            }
        }

        // Update audio sources
        for (auto entity : entities) {
            if (!ecs_->HasComponent<AudioSourceComponent>(entity)) continue;

            auto& source = ecs_->GetComponent<AudioSourceComponent>(entity);
            auto& transform = ecs_->GetComponent<TransformComponent>(entity);

            if (source.is3D && listenerEntity != NULL_ENTITY) {
                // Calculate distance attenuation
                float dist = Distance(transform.position, listenerPos);
                float attenuation = 1.0f;

                if (dist > source.minDistance) {
                    attenuation = source.minDistance / dist;
                }

                if (dist > source.maxDistance) {
                    attenuation = 0.0f;
                }

                // Update volume based on distance (actual audio API calls would go here)
            }
        }
    }

    void SetECS(ECS* ecs) { ecs_ = ecs; }

private:
    ECS* ecs_ = nullptr;

    float Distance(const DirectX::XMFLOAT3& a, const DirectX::XMFLOAT3& b) {
        float dx = b.x - a.x;
        float dy = b.y - a.y;
        float dz = b.z - a.z;
        return sqrtf(dx * dx + dy * dy + dz * dz);
    }
};

} // namespace Nexus
