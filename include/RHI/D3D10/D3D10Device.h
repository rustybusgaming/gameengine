#pragma once

#ifdef NEXUS_DIRECTX10_ENABLED

#include "../RHIDevice.h"
#include <d3d10.h>
#include <dxgi.h>
#include <vector>

namespace Nexus {
namespace RHI {

class D3D10Device : public RHIDevice {
public:
    D3D10Device();
    ~D3D10Device() override;

    bool Initialize(void* windowHandle, const DeviceDesc& desc) override;
    void Shutdown() override;
    void WaitIdle() override;

    // Command buffer creation
    RHICommandBuffer* CreateCommandBuffer() override;
    void DestroyCommandBuffer(RHICommandBuffer* commandBuffer) override;

    // Resource creation
    RHIBuffer* CreateBuffer(const BufferDesc& desc, const void* initialData) override;
    RHITexture* CreateTexture(const TextureDesc& desc, const void* initialData) override;
    RHISampler* CreateSampler(const SamplerDesc& desc) override;
    RHIShader* CreateShader(ShaderStage stage, const void* bytecode, size_t size) override;
    RHIPipeline* CreateGraphicsPipeline(const GraphicsPipelineDesc& desc) override;
    RHIPipeline* CreateComputePipeline(const ComputePipelineDesc& desc) override;
    RHIRenderPass* CreateRenderPass(const std::vector<TextureFormat>& colorFormats,
                                     TextureFormat depthFormat) override;
    RHIFramebuffer* CreateFramebuffer(const std::vector<RHITexture*>& colorTargets,
                                      RHITexture* depthTarget, uint32_t width, uint32_t height) override;

    // Resource destruction
    void DestroyBuffer(RHIBuffer* buffer) override;
    void DestroyTexture(RHITexture* texture) override;
    void DestroySampler(RHISampler* sampler) override;
    void DestroyShader(RHIShader* shader) override;
    void DestroyPipeline(RHIPipeline* pipeline) override;
    void DestroyRenderPass(RHIRenderPass* renderPass) override;
    void DestroyFramebuffer(RHIFramebuffer* framebuffer) override;

    // Command submission
    void SubmitCommandBuffer(RHICommandBuffer* commandBuffer) override;
    void Present() override;

    // Device queries
    bool IsDeviceLost() const override;
    bool ResetDevice() override;

    // Direct3D 10 specific
    ID3D10Device* GetD3D10Device() const { return device_; }
    IDXGISwapChain* GetSwapChain() const { return swapChain_; }

private:
    bool CreateD3D10Device(void* windowHandle, const DeviceDesc& desc);
    void ReleaseD3D10Resources();

    ID3D10Device* device_;
    IDXGISwapChain* swapChain_;
    ID3D10RenderTargetView* renderTargetView_;
    ID3D10DepthStencilView* depthStencilView_;
    ID3D10Texture2D* backBuffer_;
    ID3D10Texture2D* depthStencilBuffer_;
    bool deviceLost_;
};

} // namespace RHI
} // namespace Nexus

#endif // NEXUS_DIRECTX10_ENABLED
