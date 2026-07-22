#pragma once

#include "RHIDefinitions.h"
#include "RHIResource.h"
#include <memory>
#include <vector>

namespace Nexus {
namespace RHI {

struct SwapChainDesc {
    void* windowHandle;
    uint32_t width;
    uint32_t height;
    uint32_t bufferCount;
    TextureFormat format;
    bool vsync;
    bool fullscreen;
};

class RHICommandBuffer {
public:
    virtual ~RHICommandBuffer() = default;

    virtual void Begin() = 0;
    virtual void End() = 0;

    virtual void BeginRenderPass(RHIRenderPass* renderPass, RHIFramebuffer* framebuffer) = 0;
    virtual void EndRenderPass() = 0;

    virtual void SetViewport(const Viewport& viewport) = 0;
    virtual void SetScissor(const Scissor& scissor) = 0;

    virtual void SetPipeline(RHIPipeline* pipeline) = 0;
    virtual void SetVertexBuffer(RHIBuffer* buffer, uint32_t binding = 0, uint64_t offset = 0) = 0;
    virtual void SetIndexBuffer(RHIBuffer* buffer, uint64_t offset = 0) = 0;

    virtual void Draw(uint32_t vertexCount, uint32_t instanceCount = 1, uint32_t firstVertex = 0, uint32_t firstInstance = 0) = 0;
    virtual void DrawIndexed(uint32_t indexCount, uint32_t instanceCount = 1, uint32_t firstIndex = 0, int32_t vertexOffset = 0, uint32_t firstInstance = 0) = 0;

    virtual void ClearRenderTarget(RHITexture* target, const ClearColor& color) = 0;
    virtual void ClearDepthStencil(RHITexture* target, float depth, uint8_t stencil) = 0;

    virtual void CopyBuffer(RHIBuffer* src, RHIBuffer* dst, uint64_t size, uint64_t srcOffset = 0, uint64_t dstOffset = 0) = 0;
    virtual void CopyTexture(RHITexture* src, RHITexture* dst) = 0;
};

using RHICommandBufferPtr = std::unique_ptr<RHICommandBuffer>;

class RHIDevice {
public:
    virtual ~RHIDevice() = default;

    static std::unique_ptr<RHIDevice> Create(GraphicsAPI api = GraphicsAPI::Auto);

    virtual bool Initialize(const SwapChainDesc& swapChainDesc) = 0;
    virtual void Shutdown() = 0;

    virtual GraphicsAPI GetAPI() const = 0;
    virtual const char* GetAPIName() const = 0;

    virtual void BeginFrame() = 0;
    virtual void EndFrame() = 0;
    virtual void Present() = 0;

    virtual RHICommandBufferPtr CreateCommandBuffer() = 0;
    virtual void SubmitCommandBuffer(RHICommandBuffer* commandBuffer) = 0;
    virtual void WaitIdle() = 0;

    virtual RHIBufferPtr CreateBuffer(const BufferDesc& desc, const void* initialData = nullptr) = 0;
    virtual RHITexturePtr CreateTexture(const TextureDesc& desc, const void* initialData = nullptr) = 0;
    virtual RHISamplerPtr CreateSampler(const SamplerDesc& desc) = 0;

    virtual RHIShaderPtr CreateShader(ShaderStage stage, const void* bytecode, size_t size) = 0;
    virtual RHIShaderPtr CreateShaderFromSource(ShaderStage stage, const std::string& source, const std::string& entryPoint = "main") = 0;

    virtual RHIPipelinePtr CreateGraphicsPipeline(
        const std::vector<RHIShader*>& shaders,
        const BlendStateDesc& blendState,
        const DepthStencilStateDesc& depthStencilState,
        const RasterizerStateDesc& rasterizerState,
        PrimitiveTopology topology
    ) = 0;

    virtual RHIRenderPassPtr CreateRenderPass(
        const std::vector<TextureFormat>& colorFormats,
        TextureFormat depthFormat = TextureFormat::D24_UNORM_S8_UINT
    ) = 0;

    virtual RHIFramebufferPtr CreateFramebuffer(
        RHIRenderPass* renderPass,
        const std::vector<RHITexture*>& colorAttachments,
        RHITexture* depthAttachment = nullptr
    ) = 0;

    virtual RHITexture* GetBackBuffer() = 0;
    virtual uint32_t GetBackBufferWidth() const = 0;
    virtual uint32_t GetBackBufferHeight() const = 0;

    virtual void ResizeSwapChain(uint32_t width, uint32_t height) = 0;

    virtual bool IsDeviceLost() = 0;
    virtual bool ResetDevice() = 0;
};

using RHIDevicePtr = std::unique_ptr<RHIDevice>;

} // namespace RHI
} // namespace Nexus
