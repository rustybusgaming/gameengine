#include "RHI/D3D10/D3D10Device.h"
#include "Logger.h"

#ifdef NEXUS_DIRECTX10_ENABLED

namespace Nexus {
namespace RHI {

D3D10Device::D3D10Device()
    : device_(nullptr)
    , swapChain_(nullptr)
    , renderTargetView_(nullptr)
    , depthStencilView_(nullptr)
    , backBuffer_(nullptr)
    , depthStencilBuffer_(nullptr)
    , deviceLost_(false) {
}

D3D10Device::~D3D10Device() {
    Shutdown();
}

bool D3D10Device::Initialize(void* windowHandle, const DeviceDesc& desc) {
    Logger::Info("Initializing Direct3D 10 device...");

    // Setup swap chain description
    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    swapChainDesc.BufferCount = 2;
    swapChainDesc.BufferDesc.Width = desc.width;
    swapChainDesc.BufferDesc.Height = desc.height;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.OutputWindow = (HWND)windowHandle;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.Windowed = !desc.fullscreen;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    swapChainDesc.Flags = 0;

    // Create device and swap chain
    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D10_CREATE_DEVICE_DEBUG;
#endif

    HRESULT hr = D3D10CreateDeviceAndSwapChain(
        nullptr,
        D3D10_DRIVER_TYPE_HARDWARE,
        nullptr,
        createDeviceFlags,
        D3D10_SDK_VERSION,
        &swapChainDesc,
        &swapChain_,
        &device_
    );

    if (FAILED(hr)) {
        Logger::Error("Failed to create Direct3D 10 device and swap chain");
        return false;
    }

    // Get back buffer and create render target view
    hr = swapChain_->GetBuffer(0, __uuidof(ID3D10Texture2D), (void**)&backBuffer_);
    if (FAILED(hr)) {
        Logger::Error("Failed to get back buffer");
        ReleaseD3D10Resources();
        return false;
    }

    hr = device_->CreateRenderTargetView(backBuffer_, nullptr, &renderTargetView_);
    if (FAILED(hr)) {
        Logger::Error("Failed to create render target view");
        ReleaseD3D10Resources();
        return false;
    }

    // Create depth stencil buffer
    D3D10_TEXTURE2D_DESC depthStencilDesc = {};
    depthStencilDesc.Width = desc.width;
    depthStencilDesc.Height = desc.height;
    depthStencilDesc.MipLevels = 1;
    depthStencilDesc.ArraySize = 1;
    depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthStencilDesc.SampleDesc.Count = 1;
    depthStencilDesc.SampleDesc.Quality = 0;
    depthStencilDesc.Usage = D3D10_USAGE_DEFAULT;
    depthStencilDesc.BindFlags = D3D10_BIND_DEPTH_STENCIL;

    hr = device_->CreateTexture2D(&depthStencilDesc, nullptr, &depthStencilBuffer_);
    if (FAILED(hr)) {
        Logger::Error("Failed to create depth stencil buffer");
        ReleaseD3D10Resources();
        return false;
    }

    hr = device_->CreateDepthStencilView(depthStencilBuffer_, nullptr, &depthStencilView_);
    if (FAILED(hr)) {
        Logger::Error("Failed to create depth stencil view");
        ReleaseD3D10Resources();
        return false;
    }

    // Set render targets
    device_->OMSetRenderTargets(1, &renderTargetView_, depthStencilView_);

    // Setup viewport
    D3D10_VIEWPORT viewport = {};
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = desc.width;
    viewport.Height = desc.height;
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    device_->RSSetViewports(1, &viewport);

    Logger::Info("Direct3D 10 device created successfully");
    return true;
}

void D3D10Device::Shutdown() {
    ReleaseD3D10Resources();
}

void D3D10Device::ReleaseD3D10Resources() {
    if (depthStencilView_) {
        depthStencilView_->Release();
        depthStencilView_ = nullptr;
    }
    if (depthStencilBuffer_) {
        depthStencilBuffer_->Release();
        depthStencilBuffer_ = nullptr;
    }
    if (renderTargetView_) {
        renderTargetView_->Release();
        renderTargetView_ = nullptr;
    }
    if (backBuffer_) {
        backBuffer_->Release();
        backBuffer_ = nullptr;
    }
    if (swapChain_) {
        swapChain_->Release();
        swapChain_ = nullptr;
    }
    if (device_) {
        device_->Release();
        device_ = nullptr;
    }
}

void D3D10Device::WaitIdle() {
    if (device_) {
        device_->Flush();
    }
}

RHICommandBuffer* D3D10Device::CreateCommandBuffer() {
    // D3D10 doesn't have command buffers - immediate mode only
    return nullptr;
}

void D3D10Device::DestroyCommandBuffer(RHICommandBuffer* commandBuffer) {
    // Nothing to do
}

RHIBuffer* D3D10Device::CreateBuffer(const BufferDesc& desc, const void* initialData) {
    // TODO: Implement D3D10 buffer creation
    return nullptr;
}

RHITexture* D3D10Device::CreateTexture(const TextureDesc& desc, const void* initialData) {
    // TODO: Implement D3D10 texture creation
    return nullptr;
}

RHISampler* D3D10Device::CreateSampler(const SamplerDesc& desc) {
    // TODO: Implement D3D10 sampler creation
    return nullptr;
}

RHIShader* D3D10Device::CreateShader(ShaderStage stage, const void* bytecode, size_t size) {
    // TODO: Implement D3D10 shader creation
    return nullptr;
}

RHIPipeline* D3D10Device::CreateGraphicsPipeline(const GraphicsPipelineDesc& desc) {
    // D3D10 doesn't have pipeline objects - state is set directly
    return nullptr;
}

RHIPipeline* D3D10Device::CreateComputePipeline(const ComputePipelineDesc& desc) {
    // D3D10 doesn't support compute shaders
    return nullptr;
}

RHIRenderPass* D3D10Device::CreateRenderPass(const std::vector<TextureFormat>& colorFormats,
                                              TextureFormat depthFormat) {
    // D3D10 doesn't have render pass objects
    return nullptr;
}

RHIFramebuffer* D3D10Device::CreateFramebuffer(const std::vector<RHITexture*>& colorTargets,
                                                RHITexture* depthTarget,
                                                uint32_t width, uint32_t height) {
    // TODO: Implement D3D10 framebuffer
    return nullptr;
}

void D3D10Device::DestroyBuffer(RHIBuffer* buffer) {
    delete buffer;
}

void D3D10Device::DestroyTexture(RHITexture* texture) {
    delete texture;
}

void D3D10Device::DestroySampler(RHISampler* sampler) {
    delete sampler;
}

void D3D10Device::DestroyShader(RHIShader* shader) {
    delete shader;
}

void D3D10Device::DestroyPipeline(RHIPipeline* pipeline) {
    delete pipeline;
}

void D3D10Device::DestroyRenderPass(RHIRenderPass* renderPass) {
    delete renderPass;
}

void D3D10Device::DestroyFramebuffer(RHIFramebuffer* framebuffer) {
    delete framebuffer;
}

void D3D10Device::SubmitCommandBuffer(RHICommandBuffer* commandBuffer) {
    // D3D10 is immediate mode - nothing to submit
}

void D3D10Device::Present() {
    if (swapChain_) {
        swapChain_->Present(0, 0);
    }
}

bool D3D10Device::IsDeviceLost() const {
    // D3D10 handles device removal differently than D3D9
    return deviceLost_;
}

bool D3D10Device::ResetDevice() {
    // D3D10 doesn't have a Reset like D3D9 - need to recreate
    return false;
}

} // namespace RHI
} // namespace Nexus

#endif // NEXUS_DIRECTX10_ENABLED
