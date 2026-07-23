#pragma once

#ifdef NEXUS_DIRECTX9_ENABLED

#include "../RHIDevice.h"
#include <d3d9.h>
#include <vector>

namespace Nexus {
namespace RHI {

class D3D9Device : public RHIDevice {
public:
    D3D9Device();
    ~D3D9Device() override;

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

    // Direct3D 9 specific
    IDirect3DDevice9* GetD3D9Device() const { return device_; }
    IDirect3D9* GetD3D9() const { return d3d9_; }

private:
    bool CreateD3D9Device(void* windowHandle, const DeviceDesc& desc);
    void ReleaseD3D9Resources();

    IDirect3D9* d3d9_;
    IDirect3DDevice9* device_;
    D3DPRESENT_PARAMETERS presentParams_;
    bool deviceLost_;
};

} // namespace RHI
} // namespace Nexus

#endif // NEXUS_DIRECTX9_ENABLED
