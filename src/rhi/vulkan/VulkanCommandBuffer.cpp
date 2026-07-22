#ifdef NEXUS_VULKAN_ENABLED

#include "RHI/Vulkan/VulkanCommandBuffer.h"
#include "RHI/Vulkan/VulkanDevice.h"
#include "RHI/Vulkan/VulkanResource.h"
#include "Logger.h"

namespace Nexus {
namespace RHI {

VulkanCommandBuffer::VulkanCommandBuffer(VulkanDevice* device, VkCommandBuffer commandBuffer)
    : device_(device)
    , commandBuffer_(commandBuffer) {
}

VulkanCommandBuffer::~VulkanCommandBuffer() {
}

void VulkanCommandBuffer::Begin() {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer_, &beginInfo);
}

void VulkanCommandBuffer::End() {
    vkEndCommandBuffer(commandBuffer_);
}

void VulkanCommandBuffer::BeginRenderPass(RHIRenderPass* renderPass, RHIFramebuffer* framebuffer) {
    VulkanRenderPass* vkRenderPass = static_cast<VulkanRenderPass*>(renderPass);
    VulkanFramebuffer* vkFramebuffer = static_cast<VulkanFramebuffer*>(framebuffer);

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = vkRenderPass->GetVkRenderPass();
    renderPassInfo.framebuffer = vkFramebuffer->GetVkFramebuffer();
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = {vkFramebuffer->GetWidth(), vkFramebuffer->GetHeight()};

    VkClearValue clearValues[2];
    clearValues[0].color = {{0.0f, 0.0f, 0.0f, 1.0f}};
    clearValues[1].depthStencil = {1.0f, 0};

    renderPassInfo.clearValueCount = 2;
    renderPassInfo.pClearValues = clearValues;

    vkCmdBeginRenderPass(commandBuffer_, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void VulkanCommandBuffer::EndRenderPass() {
    vkCmdEndRenderPass(commandBuffer_);
}

void VulkanCommandBuffer::SetViewport(const Viewport& viewport) {
    VkViewport vkViewport{};
    vkViewport.x = viewport.x;
    vkViewport.y = viewport.y;
    vkViewport.width = viewport.width;
    vkViewport.height = viewport.height;
    vkViewport.minDepth = viewport.minDepth;
    vkViewport.maxDepth = viewport.maxDepth;

    vkCmdSetViewport(commandBuffer_, 0, 1, &vkViewport);
}

void VulkanCommandBuffer::SetScissor(const Scissor& scissor) {
    VkRect2D vkScissor{};
    vkScissor.offset = {scissor.x, scissor.y};
    vkScissor.extent = {scissor.width, scissor.height};

    vkCmdSetScissor(commandBuffer_, 0, 1, &vkScissor);
}

void VulkanCommandBuffer::SetPipeline(RHIPipeline* pipeline) {
    VulkanPipeline* vkPipeline = static_cast<VulkanPipeline*>(pipeline);
    vkCmdBindPipeline(commandBuffer_, VK_PIPELINE_BIND_POINT_GRAPHICS, vkPipeline->GetVkPipeline());
}

void VulkanCommandBuffer::SetVertexBuffer(RHIBuffer* buffer, uint32_t binding, uint64_t offset) {
    VulkanBuffer* vkBuffer = static_cast<VulkanBuffer*>(buffer);
    VkBuffer vertexBuffers[] = {vkBuffer->GetVkBuffer()};
    VkDeviceSize offsets[] = {offset};
    vkCmdBindVertexBuffers(commandBuffer_, binding, 1, vertexBuffers, offsets);
}

void VulkanCommandBuffer::SetIndexBuffer(RHIBuffer* buffer, uint64_t offset) {
    VulkanBuffer* vkBuffer = static_cast<VulkanBuffer*>(buffer);
    vkCmdBindIndexBuffer(commandBuffer_, vkBuffer->GetVkBuffer(), offset, VK_INDEX_TYPE_UINT32);
}

void VulkanCommandBuffer::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) {
    vkCmdDraw(commandBuffer_, vertexCount, instanceCount, firstVertex, firstInstance);
}

void VulkanCommandBuffer::DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) {
    vkCmdDrawIndexed(commandBuffer_, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void VulkanCommandBuffer::ClearRenderTarget(RHITexture* target, const ClearColor& color) {
    Logger::Warning("VulkanCommandBuffer::ClearRenderTarget not yet implemented");
}

void VulkanCommandBuffer::ClearDepthStencil(RHITexture* target, float depth, uint8_t stencil) {
    Logger::Warning("VulkanCommandBuffer::ClearDepthStencil not yet implemented");
}

void VulkanCommandBuffer::CopyBuffer(RHIBuffer* src, RHIBuffer* dst, uint64_t size, uint64_t srcOffset, uint64_t dstOffset) {
    VulkanBuffer* vkSrc = static_cast<VulkanBuffer*>(src);
    VulkanBuffer* vkDst = static_cast<VulkanBuffer*>(dst);

    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = srcOffset;
    copyRegion.dstOffset = dstOffset;
    copyRegion.size = size;

    vkCmdCopyBuffer(commandBuffer_, vkSrc->GetVkBuffer(), vkDst->GetVkBuffer(), 1, &copyRegion);
}

void VulkanCommandBuffer::CopyTexture(RHITexture* src, RHITexture* dst) {
    Logger::Warning("VulkanCommandBuffer::CopyTexture not yet implemented");
}

} // namespace RHI
} // namespace Nexus

#endif // NEXUS_VULKAN_ENABLED
