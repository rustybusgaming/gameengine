#ifdef NEXUS_DIRECTX11_ENABLED

#include "RHI/D3D11/D3D11Device.h"
#include "RHI/D3D11/D3D11Resource.h"
#include "RHI/D3D11/D3D11CommandBuffer.h"
#include "Logger.h"

#ifdef NEXUS_SDL2_ENABLED
    #include <SDL2/SDL.h>
    #include <SDL2/SDL_syswm.h>
#else
    #include <Windows.h>
#endif

namespace Nexus {
namespace RHI {

static DXGI_FORMAT ToDXGIFormat(TextureFormat format) {
    switch (format) {
        case TextureFormat::R8_UNORM: return DXGI_FORMAT_R8_UNORM;
        case TextureFormat::RG8_UNORM: return DXGI_FORMAT_R8G8_UNORM;
        case TextureFormat::RGBA8_UNORM: return DXGI_FORMAT_R8G8B8A8_UNORM;
        case TextureFormat::RGBA8_SRGB: return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        case TextureFormat::R16_FLOAT: return DXGI_FORMAT_R16_FLOAT;
        case TextureFormat::RG16_FLOAT: return DXGI_FORMAT_R16G16_FLOAT;
        case TextureFormat::RGBA16_FLOAT: return DXGI_FORMAT_R16G16B16A16_FLOAT;
        case TextureFormat::R32_FLOAT: return DXGI_FORMAT_R32_FLOAT;
        case TextureFormat::RG32_FLOAT: return DXGI_FORMAT_R32G32_FLOAT;
        case TextureFormat::RGBA32_FLOAT: return DXGI_FORMAT_R32G32B32A32_FLOAT;
        case TextureFormat::D24_UNORM_S8_UINT: return DXGI_FORMAT_D24_UNORM_S8_UINT;
        case TextureFormat::D32_FLOAT: return DXGI_FORMAT_D32_FLOAT;
        case TextureFormat::BC1_UNORM: return DXGI_FORMAT_BC1_UNORM;
        case TextureFormat::BC3_UNORM: return DXGI_FORMAT_BC3_UNORM;
        case TextureFormat::BC5_UNORM: return DXGI_FORMAT_BC5_UNORM;
        case TextureFormat::BC7_UNORM: return DXGI_FORMAT_BC7_UNORM;
        default: return DXGI_FORMAT_UNKNOWN;
    }
}

D3D11Device::D3D11Device()
    : width_(0)
    , height_(0)
    , deviceLost_(false)
    , windowHandle_(nullptr) {
}

D3D11Device::~D3D11Device() {
    Shutdown();
}

bool D3D11Device::Initialize(const SwapChainDesc& swapChainDesc) {
    Logger::Info("Initializing DirectX 11 Device...");

    windowHandle_ = swapChainDesc.windowHandle;
    width_ = swapChainDesc.width;
    height_ = swapChainDesc.height;

    if (!CreateDeviceAndSwapChain(swapChainDesc)) {
        Logger::Error("Failed to create D3D11 device and swap chain");
        return false;
    }

    if (!CreateBackBufferResources()) {
        Logger::Error("Failed to create back buffer resources");
        return false;
    }

    Logger::Info("DirectX 11 Device initialized successfully");
    return true;
}

void D3D11Device::Shutdown() {
    if (context_) {
        context_->ClearState();
        context_->Flush();
    }

    backBufferTexture_.reset();
    CleanupBackBufferResources();

    swapChain_.Reset();
    context_.Reset();
    device_.Reset();
}

bool D3D11Device::CreateDeviceAndSwapChain(const SwapChainDesc& desc) {
    // Get HWND from SDL window
    SDL_Window* window = static_cast<SDL_Window*>(desc.windowHandle);
    SDL_SysWMinfo wmInfo;
    SDL_VERSION(&wmInfo.version);
    if (!SDL_GetWindowWMInfo(window, &wmInfo)) {
        Logger::Error("Failed to get window info: " + std::string(SDL_GetError()));
        return false;
    }

    HWND hwnd = wmInfo.info.win.window;

    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    swapChainDesc.BufferCount = desc.bufferCount;
    swapChainDesc.BufferDesc.Width = desc.width;
    swapChainDesc.BufferDesc.Height = desc.height;
    swapChainDesc.BufferDesc.Format = ToDXGIFormat(desc.format);
    swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.OutputWindow = hwnd;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.Windowed = !desc.fullscreen;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
    };

    UINT createDeviceFlags = 0;
#ifdef NEXUS_DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL featureLevel;
    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        createDeviceFlags,
        featureLevels,
        ARRAYSIZE(featureLevels),
        D3D11_SDK_VERSION,
        &swapChainDesc,
        &swapChain_,
        &device_,
        &featureLevel,
        &context_
    );

    if (FAILED(hr)) {
        Logger::Error("Failed to create D3D11 device and swap chain");
        return false;
    }

    Logger::Info("Created D3D11 device with feature level: " + std::to_string(featureLevel >> 12) + "." + std::to_string((featureLevel >> 8) & 0xF));
    return true;
}

bool D3D11Device::CreateBackBufferResources() {
    ComPtr<ID3D11Texture2D> backBuffer;
    HRESULT hr = swapChain_->GetBuffer(0, __uuidof(ID3D11Texture2D), &backBuffer);
    if (FAILED(hr)) {
        Logger::Error("Failed to get swap chain back buffer");
        return false;
    }

    hr = device_->CreateRenderTargetView(backBuffer.Get(), nullptr, &backBufferRTV_);
    if (FAILED(hr)) {
        Logger::Error("Failed to create back buffer RTV");
        return false;
    }

    // Create depth stencil buffer
    D3D11_TEXTURE2D_DESC depthDesc = {};
    depthDesc.Width = width_;
    depthDesc.Height = height_;
    depthDesc.MipLevels = 1;
    depthDesc.ArraySize = 1;
    depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthDesc.SampleDesc.Count = 1;
    depthDesc.SampleDesc.Quality = 0;
    depthDesc.Usage = D3D11_USAGE_DEFAULT;
    depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    hr = device_->CreateTexture2D(&depthDesc, nullptr, &depthStencilBuffer_);
    if (FAILED(hr)) {
        Logger::Error("Failed to create depth stencil buffer");
        return false;
    }

    hr = device_->CreateDepthStencilView(depthStencilBuffer_.Get(), nullptr, &depthStencilView_);
    if (FAILED(hr)) {
        Logger::Error("Failed to create depth stencil view");
        return false;
    }

    // Wrap back buffer in RHI texture
    TextureDesc texDesc{};
    texDesc.width = width_;
    texDesc.height = height_;
    texDesc.depth = 1;
    texDesc.mipLevels = 1;
    texDesc.arraySize = 1;
    texDesc.format = TextureFormat::RGBA8_UNORM;
    texDesc.usage = TextureUsage::RenderTarget;
    texDesc.sampleCount = 1;

    backBufferTexture_ = std::make_shared<D3D11Texture>(
        this, backBuffer.Get(), nullptr, backBufferRTV_.Get(), texDesc);

    return true;
}

void D3D11Device::CleanupBackBufferResources() {
    backBufferTexture_.reset();
    depthStencilView_.Reset();
    depthStencilBuffer_.Reset();
    backBufferRTV_.Reset();
}

void D3D11Device::BeginFrame() {
    float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    context_->ClearRenderTargetView(backBufferRTV_.Get(), clearColor);
    context_->ClearDepthStencilView(depthStencilView_.Get(), D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

    context_->OMSetRenderTargets(1, backBufferRTV_.GetAddressOf(), depthStencilView_.Get());
}

void D3D11Device::EndFrame() {
}

void D3D11Device::Present() {
    HRESULT hr = swapChain_->Present(1, 0);
    if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET) {
        deviceLost_ = true;
    }
}

RHICommandBufferPtr D3D11Device::CreateCommandBuffer() {
    return std::make_unique<D3D11CommandBuffer>(this);
}

void D3D11Device::SubmitCommandBuffer(RHICommandBuffer* commandBuffer) {
    // D3D11 uses immediate context, commands are already submitted
}

void D3D11Device::WaitIdle() {
    if (context_) {
        context_->Flush();
    }
}

RHIBufferPtr D3D11Device::CreateBuffer(const BufferDesc& desc, const void* initialData) {
    return std::make_shared<D3D11Buffer>(this, desc, initialData);
}

RHITexturePtr D3D11Device::CreateTexture(const TextureDesc& desc, const void* initialData) {
    return std::make_shared<D3D11Texture>(this, desc, initialData);
}

RHISamplerPtr D3D11Device::CreateSampler(const SamplerDesc& desc) {
    return std::make_shared<D3D11Sampler>(this, desc);
}

RHIShaderPtr D3D11Device::CreateShader(ShaderStage stage, const void* bytecode, size_t size) {
    return std::make_shared<D3D11Shader>(this, stage, bytecode, size);
}

RHIShaderPtr D3D11Device::CreateShaderFromSource(ShaderStage stage, const std::string& source, const std::string& entryPoint) {
    Logger::Warning("CreateShaderFromSource not implemented for D3D11 - use compiled bytecode");
    return nullptr;
}

RHIPipelinePtr D3D11Device::CreateGraphicsPipeline(
    const std::vector<RHIShader*>& shaders,
    const BlendStateDesc& blendState,
    const DepthStencilStateDesc& depthStencilState,
    const RasterizerStateDesc& rasterizerState,
    PrimitiveTopology topology) {

    D3D11Shader* vs = nullptr;
    D3D11Shader* ps = nullptr;

    for (auto* shader : shaders) {
        D3D11Shader* d3dShader = static_cast<D3D11Shader*>(shader);
        if (d3dShader->GetStage() == ShaderStage::Vertex) {
            vs = d3dShader;
        } else if (d3dShader->GetStage() == ShaderStage::Fragment) {
            ps = d3dShader;
        }
    }

    if (!vs) {
        Logger::Error("Vertex shader required for graphics pipeline");
        return nullptr;
    }

    // Create blend state
    D3D11_BLEND_DESC blendDesc = {};
    blendDesc.RenderTarget[0].BlendEnable = blendState.blendEnable;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

    ComPtr<ID3D11BlendState> d3dBlendState;
    device_->CreateBlendState(&blendDesc, &d3dBlendState);

    // Create depth stencil state
    D3D11_DEPTH_STENCIL_DESC dsDesc = {};
    dsDesc.DepthEnable = depthStencilState.depthEnable;
    dsDesc.DepthWriteMask = depthStencilState.depthWriteEnable ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
    dsDesc.DepthFunc = D3D11_COMPARISON_LESS;

    ComPtr<ID3D11DepthStencilState> d3dDepthStencilState;
    device_->CreateDepthStencilState(&dsDesc, &d3dDepthStencilState);

    // Create rasterizer state
    D3D11_RASTERIZER_DESC rastDesc = {};
    rastDesc.FillMode = rasterizerState.fillMode == FillMode::Solid ? D3D11_FILL_SOLID : D3D11_FILL_WIREFRAME;
    rastDesc.CullMode = rasterizerState.cullMode == CullMode::Back ? D3D11_CULL_BACK :
                        rasterizerState.cullMode == CullMode::Front ? D3D11_CULL_FRONT : D3D11_CULL_NONE;
    rastDesc.FrontCounterClockwise = rasterizerState.frontCounterClockwise;
    rastDesc.DepthClipEnable = rasterizerState.depthClipEnable;
    rastDesc.ScissorEnable = rasterizerState.scissorEnable;
    rastDesc.MultisampleEnable = rasterizerState.multisampleEnable;

    ComPtr<ID3D11RasterizerState> d3dRasterizerState;
    device_->CreateRasterizerState(&rastDesc, &d3dRasterizerState);

    // Create input layout (simple position-normal for now)
    D3D11_INPUT_ELEMENT_DESC inputElements[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    ComPtr<ID3D11InputLayout> inputLayout;
    device_->CreateInputLayout(inputElements, 2, vs->GetBytecode(), vs->GetBytecodeSize(), &inputLayout);

    D3D10_PRIMITIVE_TOPOLOGY d3dTopology = D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    switch (topology) {
        case PrimitiveTopology::TriangleList: d3dTopology = D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST; break;
        case PrimitiveTopology::TriangleStrip: d3dTopology = D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP; break;
        case PrimitiveTopology::LineList: d3dTopology = D3D10_PRIMITIVE_TOPOLOGY_LINELIST; break;
        case PrimitiveTopology::PointList: d3dTopology = D3D10_PRIMITIVE_TOPOLOGY_POINTLIST; break;
    }

    return std::make_shared<D3D11Pipeline>(this, d3dBlendState.Get(), d3dDepthStencilState.Get(),
        d3dRasterizerState.Get(), inputLayout.Get(), vs, ps, d3dTopology);
}

RHIRenderPassPtr D3D11Device::CreateRenderPass(
    const std::vector<TextureFormat>& colorFormats,
    TextureFormat depthFormat) {
    return std::make_shared<D3D11RenderPass>(colorFormats, depthFormat);
}

RHIFramebufferPtr D3D11Device::CreateFramebuffer(
    RHIRenderPass* renderPass,
    const std::vector<RHITexture*>& colorAttachments,
    RHITexture* depthAttachment) {

    std::vector<ID3D11RenderTargetView*> rtvs;
    uint32_t width = 0, height = 0;

    for (auto* tex : colorAttachments) {
        D3D11Texture* d3dTex = static_cast<D3D11Texture*>(tex);
        rtvs.push_back(d3dTex->GetRTV());
        if (width == 0) {
            width = tex->GetWidth();
            height = tex->GetHeight();
        }
    }

    ID3D11DepthStencilView* dsv = nullptr;
    if (depthAttachment) {
        // Would need DSV support in D3D11Texture
    }

    return std::make_shared<D3D11Framebuffer>(rtvs, dsv, width, height);
}

RHITexture* D3D11Device::GetBackBuffer() {
    return backBufferTexture_.get();
}

void D3D11Device::ResizeSwapChain(uint32_t width, uint32_t height) {
    if (width == 0 || height == 0) return;

    context_->OMSetRenderTargets(0, nullptr, nullptr);
    CleanupBackBufferResources();

    HRESULT hr = swapChain_->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
    if (FAILED(hr)) {
        Logger::Error("Failed to resize swap chain");
        deviceLost_ = true;
        return;
    }

    width_ = width;
    height_ = height;

    CreateBackBufferResources();
    deviceLost_ = false;
}

bool D3D11Device::ResetDevice() {
    // D3D11 doesn't support device reset like D3D9
    // Would need to recreate device entirely
    return !deviceLost_;
}

} // namespace RHI
} // namespace Nexus

#endif // NEXUS_DIRECTX11_ENABLED
