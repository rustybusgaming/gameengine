/**
 * @file VulkanDescriptor.h
 * @brief Vulkan descriptor set management for uniform buffers and textures
 * @author Nexus Engine Team
 * @date 2026-07-22
 */

#pragma once

#ifdef NEXUS_VULKAN_ENABLED

#include <vulkan/vulkan.h>
#include <vector>
#include <memory>

namespace Nexus {
namespace RHI {

class VulkanDevice;

/**
 * @class VulkanDescriptorPool
 * @brief Manages allocation of descriptor sets
 */
class VulkanDescriptorPool {
public:
    VulkanDescriptorPool(VulkanDevice* device, uint32_t maxSets = 1000);
    ~VulkanDescriptorPool();

    VkDescriptorPool GetVkDescriptorPool() const { return descriptorPool_; }

    void Reset();

private:
    VulkanDevice* device_;
    VkDescriptorPool descriptorPool_;
};

/**
 * @class VulkanDescriptorSetLayout
 * @brief Defines the layout of a descriptor set
 */
class VulkanDescriptorSetLayout {
public:
    struct Binding {
        uint32_t binding;
        VkDescriptorType type;
        uint32_t count;
        VkShaderStageFlags stageFlags;
    };

    VulkanDescriptorSetLayout(VulkanDevice* device, const std::vector<Binding>& bindings);
    ~VulkanDescriptorSetLayout();

    VkDescriptorSetLayout GetVkDescriptorSetLayout() const { return descriptorSetLayout_; }

private:
    VulkanDevice* device_;
    VkDescriptorSetLayout descriptorSetLayout_;
};

/**
 * @class VulkanDescriptorSet
 * @brief A set of descriptors (uniforms, textures, etc.)
 */
class VulkanDescriptorSet {
public:
    VulkanDescriptorSet(VulkanDevice* device, VulkanDescriptorPool* pool, VulkanDescriptorSetLayout* layout);
    ~VulkanDescriptorSet();

    VkDescriptorSet GetVkDescriptorSet() const { return descriptorSet_; }

    void UpdateBuffer(uint32_t binding, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range);
    void UpdateTexture(uint32_t binding, VkImageView imageView, VkSampler sampler, VkImageLayout layout);

private:
    VulkanDevice* device_;
    VulkanDescriptorPool* pool_;
    VkDescriptorSet descriptorSet_;
};

using VulkanDescriptorPoolPtr = std::shared_ptr<VulkanDescriptorPool>;
using VulkanDescriptorSetLayoutPtr = std::shared_ptr<VulkanDescriptorSetLayout>;
using VulkanDescriptorSetPtr = std::shared_ptr<VulkanDescriptorSet>;

} // namespace RHI
} // namespace Nexus

#endif // NEXUS_VULKAN_ENABLED
