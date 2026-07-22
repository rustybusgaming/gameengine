#ifdef NEXUS_VULKAN_ENABLED

#include "RHI/Vulkan/VulkanDescriptor.h"
#include "RHI/Vulkan/VulkanDevice.h"
#include "Logger.h"

namespace Nexus {
namespace RHI {

// VulkanDescriptorPool
VulkanDescriptorPool::VulkanDescriptorPool(VulkanDevice* device, uint32_t maxSets)
    : device_(device)
    , descriptorPool_(VK_NULL_HANDLE) {

    // Define pool sizes for different descriptor types
    std::vector<VkDescriptorPoolSize> poolSizes = {
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, maxSets * 4 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, maxSets * 4 },
        { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, maxSets * 2 },
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, maxSets * 2 }
    };

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    poolInfo.maxSets = maxSets;
    poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
    poolInfo.pPoolSizes = poolSizes.data();

    if (vkCreateDescriptorPool(device_->GetVkDevice(), &poolInfo, nullptr, &descriptorPool_) != VK_SUCCESS) {
        Logger::Error("Failed to create Vulkan descriptor pool");
    }
}

VulkanDescriptorPool::~VulkanDescriptorPool() {
    if (descriptorPool_ != VK_NULL_HANDLE) {
        vkDestroyDescriptorPool(device_->GetVkDevice(), descriptorPool_, nullptr);
    }
}

void VulkanDescriptorPool::Reset() {
    vkResetDescriptorPool(device_->GetVkDevice(), descriptorPool_, 0);
}

// VulkanDescriptorSetLayout
VulkanDescriptorSetLayout::VulkanDescriptorSetLayout(VulkanDevice* device, const std::vector<Binding>& bindings)
    : device_(device)
    , descriptorSetLayout_(VK_NULL_HANDLE) {

    std::vector<VkDescriptorSetLayoutBinding> vkBindings;
    vkBindings.reserve(bindings.size());

    for (const auto& binding : bindings) {
        VkDescriptorSetLayoutBinding layoutBinding{};
        layoutBinding.binding = binding.binding;
        layoutBinding.descriptorType = binding.type;
        layoutBinding.descriptorCount = binding.count;
        layoutBinding.stageFlags = binding.stageFlags;
        layoutBinding.pImmutableSamplers = nullptr;
        vkBindings.push_back(layoutBinding);
    }

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(vkBindings.size());
    layoutInfo.pBindings = vkBindings.data();

    if (vkCreateDescriptorSetLayout(device_->GetVkDevice(), &layoutInfo, nullptr, &descriptorSetLayout_) != VK_SUCCESS) {
        Logger::Error("Failed to create Vulkan descriptor set layout");
    }
}

VulkanDescriptorSetLayout::~VulkanDescriptorSetLayout() {
    if (descriptorSetLayout_ != VK_NULL_HANDLE) {
        vkDestroyDescriptorSetLayout(device_->GetVkDevice(), descriptorSetLayout_, nullptr);
    }
}

// VulkanDescriptorSet
VulkanDescriptorSet::VulkanDescriptorSet(VulkanDevice* device, VulkanDescriptorPool* pool, VulkanDescriptorSetLayout* layout)
    : device_(device)
    , pool_(pool)
    , descriptorSet_(VK_NULL_HANDLE) {

    VkDescriptorSetLayout vkLayout = layout->GetVkDescriptorSetLayout();

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = pool_->GetVkDescriptorPool();
    allocInfo.descriptorSetCount = 1;
    allocInfo.pSetLayouts = &vkLayout;

    if (vkAllocateDescriptorSets(device_->GetVkDevice(), &allocInfo, &descriptorSet_) != VK_SUCCESS) {
        Logger::Error("Failed to allocate Vulkan descriptor set");
    }
}

VulkanDescriptorSet::~VulkanDescriptorSet() {
    if (descriptorSet_ != VK_NULL_HANDLE) {
        vkFreeDescriptorSets(device_->GetVkDevice(), pool_->GetVkDescriptorPool(), 1, &descriptorSet_);
    }
}

void VulkanDescriptorSet::UpdateBuffer(uint32_t binding, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range) {
    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = buffer;
    bufferInfo.offset = offset;
    bufferInfo.range = range;

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = descriptorSet_;
    write.dstBinding = binding;
    write.dstArrayElement = 0;
    write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    write.descriptorCount = 1;
    write.pBufferInfo = &bufferInfo;

    vkUpdateDescriptorSets(device_->GetVkDevice(), 1, &write, 0, nullptr);
}

void VulkanDescriptorSet::UpdateTexture(uint32_t binding, VkImageView imageView, VkSampler sampler, VkImageLayout layout) {
    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = layout;
    imageInfo.imageView = imageView;
    imageInfo.sampler = sampler;

    VkWriteDescriptorSet write{};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = descriptorSet_;
    write.dstBinding = binding;
    write.dstArrayElement = 0;
    write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write.descriptorCount = 1;
    write.pImageInfo = &imageInfo;

    vkUpdateDescriptorSets(device_->GetVkDevice(), 1, &write, 0, nullptr);
}

} // namespace RHI
} // namespace Nexus

#endif // NEXUS_VULKAN_ENABLED
