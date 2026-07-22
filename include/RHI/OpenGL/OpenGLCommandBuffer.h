/**
 * @file OpenGLCommandBuffer.h
 * @brief OpenGL command buffer (state tracking wrapper)
 * @author Nexus Engine Team
 * @date 2026-07-22
 */

#pragma once

#ifdef NEXUS_OPENGL_ENABLED

#include "../RHIDevice.h"

namespace Nexus {
namespace RHI {

class OpenGLDevice;

class OpenGLCommandBuffer : public RHICommandBuffer {
public:
    OpenGLCommandBuffer(OpenGLDevice* device);
    ~OpenGLCommandBuffer() override;

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

private:
    OpenGLDevice* device_;
    RHIPipeline* currentPipeline_;
};

} // namespace RHI
} // namespace Nexus

#endif // NEXUS_OPENGL_ENABLED
