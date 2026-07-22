#pragma once

#ifdef NEXUS_DIRECTX11_ENABLED

#include "../RHIDevice.h"
#include <d3d11.h>
#include <dxgi.h>
#include <wrl/client.h>
#include <vector>

namespace Nexus {
namespace RHI {

using Microsoft::WRL::ComPtr;

class D3D11Device : public RHIDevice {
public:
    D3D11Device();
    ~D3D11Device() override;

    bool Initialize(const SwapChainDesc& swapChainDesc) override;
    void Shutdown() override;

    GraphicsAPI GetAPI() const override { return GraphicsAPI::DirectX11; }
    const char* GetAPIName() const override { return "DirectX 11"; }

    void BeginFrame() override;
    void EndFrame() override;
    void Present() override;

    RHICommandBufferPtr CreateCommandBuffer() override;
    void SubmitCommandBuffer(RHICommandBuffer* commandBuffer) override;
    void WaitIdle() override;

    RHIBufferPtr CreateBuffer(const BufferDesc& desc, const void* initialData = nullptr) override;
    RHITexturePtr CreateTexture(const TextureDesc& desc, const void* initialData = nullptr) override;
    RHISamplerPtr CreateSampler(const SamplerDesc& desc) override;

    RHIShaderPtr CreateShader(ShaderStage stage, const void* bytecode, size_t size) override;
    RHIShaderPtr CreateShaderFromSource(ShaderStage stage, const std::string& source, const std::string& entryPoint = "main") override;

    RHIPipelinePtr CreateGraphicsPipeline(
        const std::vector<RHIShader*>& shaders,
        const BlendStateDesc& blendState,
        const DepthStencilStateDesc& depthStencilState,
        const RasterizerStateDesc& rasterizerState,
        PrimitiveTopology topology
    ) override;

    RHIRenderPassPtr CreateRenderPass(
        const std::vector<TextureFormat>& colorFormats,
        TextureFormat depthFormat = TextureFormat::D24_UNORM_S8_UINT
    ) override;

    RHIFramebufferPtr CreateFramebuffer(
        RHIRenderPass* renderPass,
        const std::vector<RHITexture*>& colorAttachments,
        RHITexture* depthAttachment = nullptr
    ) override;

    RHITexture* GetBackBuffer() override;
    uint32_t GetBackBufferWidth() const override { return width_; }
    uint32_t GetBackBufferHeight() const override { return height_; }

    void ResizeSwapChain(uint32_t width, uint32_t height) override;

    bool IsDeviceLost() override { return deviceLost_; }
    bool ResetDevice() override;

    ID3D11Device* GetD3D11Device() const { return device_.Get(); }
    ID3D11DeviceContext* GetD3D11Context() const { return context_.Get(); }
    IDXGISwapChain* GetSwapChain() const { return swapChain_.Get(); }

private:
    bool CreateDeviceAndSwapChain(const SwapChainDesc& desc);
    bool CreateBackBufferResources();
    void CleanupBackBufferResources();

    ComPtr<ID3D11Device> device_;
    ComPtr<ID3D11DeviceContext> context_;
    ComPtr<IDXGISwapChain> swapChain_;
    ComPtr<ID3D11RenderTargetView> backBufferRTV_;
    ComPtr<ID3D11DepthStencilView> depthStencilView_;
    ComPtr<ID3D11Texture2D> depthStencilBuffer_;

    RHITexturePtr backBufferTexture_;

    uint32_t width_;
    uint32_t height_;
    bool deviceLost_;
    void* windowHandle_;
};

} // namespace RHI
} // namespace Nexus

#endif // NEXUS_DIRECTX11_ENABLED
