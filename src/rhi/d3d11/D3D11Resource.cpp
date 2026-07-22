#ifdef NEXUS_DIRECTX11_ENABLED

#include "RHI/D3D11/D3D11Resource.h"
#include "RHI/D3D11/D3D11Device.h"
#include "Logger.h"

namespace Nexus {
namespace RHI {

// D3D11Buffer
D3D11Buffer::D3D11Buffer(D3D11Device* device, const BufferDesc& desc, const void* initialData)
    : device_(device)
    , mappedData_(nullptr) {

    desc_ = desc;

    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.ByteWidth = static_cast<UINT>(desc.size);
    bufferDesc.Usage = desc.accessMode == BufferAccessMode::Dynamic ? D3D11_USAGE_DYNAMIC : D3D11_USAGE_DEFAULT;
    bufferDesc.CPUAccessFlags = desc.accessMode == BufferAccessMode::Dynamic ? D3D11_CPU_ACCESS_WRITE : 0;

    switch (desc.usage) {
        case BufferUsage::Vertex:
            bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
            break;
        case BufferUsage::Index:
            bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
            break;
        case BufferUsage::Uniform:
            bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
            break;
        case BufferUsage::Storage:
            bufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
            break;
    }

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = initialData;

    HRESULT hr = device_->GetD3D11Device()->CreateBuffer(&bufferDesc, initialData ? &initData : nullptr, &buffer_);
    if (FAILED(hr)) {
        Logger::Error("Failed to create D3D11 buffer");
    }
}

D3D11Buffer::~D3D11Buffer() {
    if (mappedData_) {
        Unmap();
    }
}

void* D3D11Buffer::Map() {
    if (!mappedData_) {
        D3D11_MAPPED_SUBRESOURCE mapped;
        HRESULT hr = device_->GetD3D11Context()->Map(buffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
        if (SUCCEEDED(hr)) {
            mappedData_ = mapped.pData;
        }
    }
    return mappedData_;
}

void D3D11Buffer::Unmap() {
    if (mappedData_) {
        device_->GetD3D11Context()->Unmap(buffer_.Get(), 0);
        mappedData_ = nullptr;
    }
}

void D3D11Buffer::UpdateData(const void* data, uint64_t size, uint64_t offset) {
    void* mapped = Map();
    if (mapped) {
        memcpy(static_cast<char*>(mapped) + offset, data, size);
        Unmap();
    }
}

// D3D11Texture
D3D11Texture::D3D11Texture(D3D11Device* device, const TextureDesc& desc, const void* initialData)
    : device_(device)
    , ownsTexture_(true) {

    desc_ = desc;
    Logger::Warning("D3D11Texture creation from data not fully implemented");
}

D3D11Texture::D3D11Texture(D3D11Device* device, ID3D11Texture2D* texture, ID3D11ShaderResourceView* srv,
                           ID3D11RenderTargetView* rtv, const TextureDesc& desc)
    : device_(device)
    , texture_(texture)
    , srv_(srv)
    , rtv_(rtv)
    , ownsTexture_(false) {

    desc_ = desc;
}

D3D11Texture::~D3D11Texture() {
    // ComPtr handles cleanup automatically
}

void D3D11Texture::UpdateData(const void* data, uint32_t mipLevel, uint32_t arraySlice) {
    device_->GetD3D11Context()->UpdateSubresource(texture_.Get(),
        D3D11CalcSubresource(mipLevel, arraySlice, desc_.mipLevels),
        nullptr, data, desc_.width * 4, 0);
}

// D3D11Sampler
D3D11Sampler::D3D11Sampler(D3D11Device* device, const SamplerDesc& desc)
    : device_(device) {

    desc_ = desc;

    D3D11_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = desc.magFilter == SamplerFilter::Point ? D3D11_FILTER_MIN_MAG_MIP_POINT : D3D11_FILTER_MIN_MAG_MIP_LINEAR;

    auto ConvertAddressMode = [](SamplerAddressMode mode) {
        switch (mode) {
            case SamplerAddressMode::Wrap: return D3D11_TEXTURE_ADDRESS_WRAP;
            case SamplerAddressMode::Clamp: return D3D11_TEXTURE_ADDRESS_CLAMP;
            case SamplerAddressMode::Mirror: return D3D11_TEXTURE_ADDRESS_MIRROR;
            case SamplerAddressMode::Border: return D3D11_TEXTURE_ADDRESS_BORDER;
            default: return D3D11_TEXTURE_ADDRESS_WRAP;
        }
    };

    samplerDesc.AddressU = ConvertAddressMode(desc.addressU);
    samplerDesc.AddressV = ConvertAddressMode(desc.addressV);
    samplerDesc.AddressW = ConvertAddressMode(desc.addressW);
    samplerDesc.MipLODBias = desc.mipLodBias;
    samplerDesc.MaxAnisotropy = desc.maxAnisotropy;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samplerDesc.MinLOD = desc.minLod;
    samplerDesc.MaxLOD = desc.maxLod;

    HRESULT hr = device_->GetD3D11Device()->CreateSamplerState(&samplerDesc, &sampler_);
    if (FAILED(hr)) {
        Logger::Error("Failed to create D3D11 sampler");
    }
}

D3D11Sampler::~D3D11Sampler() {
}

// D3D11Shader
D3D11Shader::D3D11Shader(D3D11Device* device, ShaderStage stage, const void* bytecode, size_t size)
    : device_(device) {

    stage_ = stage;
    bytecode_.assign(static_cast<const uint8_t*>(bytecode), static_cast<const uint8_t*>(bytecode) + size);

    HRESULT hr = S_OK;
    switch (stage) {
        case ShaderStage::Vertex:
            hr = device_->GetD3D11Device()->CreateVertexShader(bytecode, size, nullptr, &vertexShader_);
            break;
        case ShaderStage::Fragment:
            hr = device_->GetD3D11Device()->CreatePixelShader(bytecode, size, nullptr, &pixelShader_);
            break;
        case ShaderStage::Geometry:
            hr = device_->GetD3D11Device()->CreateGeometryShader(bytecode, size, nullptr, &geometryShader_);
            break;
        case ShaderStage::Compute:
            hr = device_->GetD3D11Device()->CreateComputeShader(bytecode, size, nullptr, &computeShader_);
            break;
    }

    if (FAILED(hr)) {
        Logger::Error("Failed to create D3D11 shader");
    }
}

D3D11Shader::~D3D11Shader() {
}

// D3D11Pipeline
D3D11Pipeline::D3D11Pipeline(D3D11Device* device,
                             ID3D11BlendState* blendState,
                             ID3D11DepthStencilState* depthStencilState,
                             ID3D11RasterizerState* rasterizerState,
                             ID3D11InputLayout* inputLayout,
                             D3D11Shader* vertexShader,
                             D3D11Shader* pixelShader,
                             D3D10_PRIMITIVE_TOPOLOGY topology)
    : device_(device)
    , blendState_(blendState)
    , depthStencilState_(depthStencilState)
    , rasterizerState_(rasterizerState)
    , inputLayout_(inputLayout)
    , vertexShader_(vertexShader)
    , pixelShader_(pixelShader)
    , topology_(topology) {
}

D3D11Pipeline::~D3D11Pipeline() {
}

// D3D11RenderPass
D3D11RenderPass::D3D11RenderPass(const std::vector<TextureFormat>& colorFormats, TextureFormat depthFormat)
    : colorFormats_(colorFormats)
    , depthFormat_(depthFormat) {
}

D3D11RenderPass::~D3D11RenderPass() {
}

// D3D11Framebuffer
D3D11Framebuffer::D3D11Framebuffer(const std::vector<ID3D11RenderTargetView*>& rtvs,
                                   ID3D11DepthStencilView* dsv,
                                   uint32_t width, uint32_t height)
    : rtvs_(rtvs)
    , dsv_(dsv)
    , width_(width)
    , height_(height) {
}

D3D11Framebuffer::~D3D11Framebuffer() {
}

} // namespace RHI
} // namespace Nexus

#endif // NEXUS_DIRECTX11_ENABLED
