#include "RHI/D3D9/D3D9Device.h"
#include "Logger.h"
#include <d3d9.h>

#ifdef NEXUS_DIRECTX9_ENABLED

namespace Nexus {
namespace RHI {

D3D9Device::D3D9Device()
    : d3d9_(nullptr)
    , device_(nullptr)
    , deviceLost_(false) {
}

D3D9Device::~D3D9Device() {
    Shutdown();
}

bool D3D9Device::Initialize(void* windowHandle, const DeviceDesc& desc) {
    Logger::Info("Initializing Direct3D 9 device...");

    // Create Direct3D 9
    d3d9_ = Direct3DCreate9(D3D_SDK_VERSION);
    if (!d3d9_) {
        Logger::Error("Failed to create Direct3D 9");
        return false;
    }

    // Setup present parameters
    ZeroMemory(&presentParams_, sizeof(presentParams_));
    presentParams_.BackBufferWidth = desc.width;
    presentParams_.BackBufferHeight = desc.height;
    presentParams_.BackBufferFormat = D3DFMT_A8R8G8B8;
    presentParams_.BackBufferCount = 1;
    presentParams_.MultiSampleType = D3DMULTISAMPLE_NONE;
    presentParams_.MultiSampleQuality = 0;
    presentParams_.SwapEffect = D3DSWAPEFFECT_DISCARD;
    presentParams_.hDeviceWindow = (HWND)windowHandle;
    presentParams_.Windowed = !desc.fullscreen;
    presentParams_.EnableAutoDepthStencil = TRUE;
    presentParams_.AutoDepthStencilFormat = D3DFMT_D24S8;
    presentParams_.Flags = 0;
    presentParams_.FullScreen_RefreshRateInHz = desc.fullscreen ? D3DPRESENT_RATE_DEFAULT : 0;
    presentParams_.PresentationInterval = desc.vsync ? D3DPRESENT_INTERVAL_ONE : D3DPRESENT_INTERVAL_IMMEDIATE;

    // Create device
    HRESULT hr = d3d9_->CreateDevice(
        D3DADAPTER_DEFAULT,
        D3DDEVTYPE_HAL,
        (HWND)windowHandle,
        D3DCREATE_HARDWARE_VERTEXPROCESSING,
        &presentParams_,
        &device_
    );

    if (FAILED(hr)) {
        Logger::Error("Failed to create Direct3D 9 device");
        ReleaseD3D9Resources();
        return false;
    }

    Logger::Info("Direct3D 9 device created successfully");
    return true;
}

void D3D9Device::Shutdown() {
    ReleaseD3D9Resources();
}

void D3D9Device::ReleaseD3D9Resources() {
    if (device_) {
        device_->Release();
        device_ = nullptr;
    }
    if (d3d9_) {
        d3d9_->Release();
        d3d9_ = nullptr;
    }
}

void D3D9Device::WaitIdle() {
    // D3D9 doesn't have an explicit wait idle, but we can query the device
    if (device_) {
        device_->EndScene();
    }
}

RHICommandBuffer* D3D9Device::CreateCommandBuffer() {
    // D3D9 doesn't have command buffers - immediate mode only
    return nullptr;
}

void D3D9Device::DestroyCommandBuffer(RHICommandBuffer* commandBuffer) {
    // Nothing to do
}

RHIBuffer* D3D9Device::CreateBuffer(const BufferDesc& desc, const void* initialData) {
    // TODO: Implement D3D9 buffer creation
    return nullptr;
}

RHITexture* D3D9Device::CreateTexture(const TextureDesc& desc, const void* initialData) {
    // TODO: Implement D3D9 texture creation
    return nullptr;
}

RHISampler* D3D9Device::CreateSampler(const SamplerDesc& desc) {
    // D3D9 doesn't have sampler objects - state is set directly
    return nullptr;
}

RHIShader* D3D9Device::CreateShader(ShaderStage stage, const void* bytecode, size_t size) {
    // TODO: Implement D3D9 shader creation
    return nullptr;
}

RHIPipeline* D3D9Device::CreateGraphicsPipeline(const GraphicsPipelineDesc& desc) {
    // D3D9 doesn't have pipeline objects - state is set directly
    return nullptr;
}

RHIPipeline* D3D9Device::CreateComputePipeline(const ComputePipelineDesc& desc) {
    // D3D9 doesn't support compute shaders
    return nullptr;
}

RHIRenderPass* D3D9Device::CreateRenderPass(const std::vector<TextureFormat>& colorFormats,
                                             TextureFormat depthFormat) {
    // D3D9 doesn't have render pass objects
    return nullptr;
}

RHIFramebuffer* D3D9Device::CreateFramebuffer(const std::vector<RHITexture*>& colorTargets,
                                               RHITexture* depthTarget,
                                               uint32_t width, uint32_t height) {
    // TODO: Implement D3D9 framebuffer
    return nullptr;
}

void D3D9Device::DestroyBuffer(RHIBuffer* buffer) {
    delete buffer;
}

void D3D9Device::DestroyTexture(RHITexture* texture) {
    delete texture;
}

void D3D9Device::DestroySampler(RHISampler* sampler) {
    delete sampler;
}

void D3D9Device::DestroyShader(RHIShader* shader) {
    delete shader;
}

void D3D9Device::DestroyPipeline(RHIPipeline* pipeline) {
    delete pipeline;
}

void D3D9Device::DestroyRenderPass(RHIRenderPass* renderPass) {
    delete renderPass;
}

void D3D9Device::DestroyFramebuffer(RHIFramebuffer* framebuffer) {
    delete framebuffer;
}

void D3D9Device::SubmitCommandBuffer(RHICommandBuffer* commandBuffer) {
    // D3D9 is immediate mode - nothing to submit
}

void D3D9Device::Present() {
    if (!device_) return;

    HRESULT hr = device_->Present(NULL, NULL, NULL, NULL);

    if (hr == D3DERR_DEVICELOST) {
        deviceLost_ = true;
    }
}

bool D3D9Device::IsDeviceLost() const {
    if (!device_) return true;

    HRESULT hr = device_->TestCooperativeLevel();
    return (hr == D3DERR_DEVICELOST || hr == D3DERR_DEVICENOTRESET);
}

bool D3D9Device::ResetDevice() {
    if (!device_ || !deviceLost_) return false;

    // Release all default pool resources here
    // ...

    HRESULT hr = device_->Reset(&presentParams_);

    if (SUCCEEDED(hr)) {
        deviceLost_ = false;
        Logger::Info("Direct3D 9 device reset successfully");

        // Recreate all default pool resources here
        // ...

        return true;
    }

    return false;
}

} // namespace RHI
} // namespace Nexus

#endif // NEXUS_DIRECTX9_ENABLED
