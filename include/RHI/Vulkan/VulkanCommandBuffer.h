#pragma once

#ifdef NEXUS_VULKAN_ENABLED

#include "../RHIDevice.h"
#include <vulkan/vulkan.h>

namespace Nexus {
namespace RHI {

class VulkanDevice;

class VulkanCommandBuffer : public RHICommandBuffer {
public:
    VulkanCommandBuffer(VulkanDevice* device, VkCommandBuffer commandBuffer);
    ~VulkanCommandBuffer() override;

    void Begin() override;
    void End() override;

    void BeginRenderPass(RHIRenderPass* renderPass, RHIFramebuffer* framebuffer) override;
    void EndRenderPass() override;

    void SetViewport(const Viewport& viewport) override;
    void SetScissor(const Scissor& scissor) override;

    void SetPipeline(RHIPipeline* pipeline) override;
    void SetVertexBuffer(RHIBuffer* buffer, uint32_t binding = 0, uint64_t offset = 0) override;
    void SetIndexBuffer(RHIBuffer* buffer, uint64_t offset = 0) override;

    void Draw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t firstVertex = 0, uint32_t firstInstance = 0) override;
    void DrawIndexed(uint32_t indexCount, uint32_t instanceCount = 1, uint32_t firstIndex = 0, int32_t vertexOffset = 0, uint32_t firstInstance = 0) override;

    void ClearRenderTarget(RHITexture* target, const ClearColor& color) override;
    void ClearDepthStencil(RHITexture* target, float depth, uint8_t stencil) override;

    void CopyBuffer(RHIBuffer* src, RHIBuffer* dst, uint64_t size, uint64_t srcOffset = 0, uint64_t dstOffset = 0) override;
    void CopyTexture(RHITexture* src, RHITexture* dst) override;

    VkCommandBuffer GetVkCommandBuffer() const { return commandBuffer_; }

private:
    VulkanDevice* device_;
    VkCommandBuffer commandBuffer_;
};

} // namespace RHI
} // namespace Nexus

#endif // NEXUS_VULKAN_ENABLED
