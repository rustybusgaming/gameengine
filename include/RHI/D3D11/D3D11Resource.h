#pragma once

#ifdef NEXUS_DIRECTX11_ENABLED

#include "../RHIResource.h"
#include <d3d11.h>
#include <wrl/client.h>
#include <vector>

namespace Nexus {
namespace RHI {

using Microsoft::WRL::ComPtr;

class D3D11Device;

class D3D11Buffer : public RHIBuffer {
public:
    D3D11Buffer(D3D11Device* device, const BufferDesc& desc, const void* initialData);
    ~D3D11Buffer() override;

    void* Map() override;
    void Unmap() override;
    void UpdateData(const void* data, uint64_t size, uint64_t offset = 0) override;

    ID3D11Buffer* GetD3D11Buffer() const { return buffer_.Get(); }

private:
    D3D11Device* device_;
    ComPtr<ID3D11Buffer> buffer_;
    void* mappedData_;
};

class D3D11Texture : public RHITexture {
public:
    D3D11Texture(D3D11Device* device, const TextureDesc& desc, const void* initialData);
    D3D11Texture(D3D11Device* device, ID3D11Texture2D* texture, ID3D11ShaderResourceView* srv,
                 ID3D11RenderTargetView* rtv, const TextureDesc& desc);
    ~D3D11Texture() override;

    void UpdateData(const void* data, uint32_t mipLevel = 0, uint32_t arraySlice = 0) override;

    ID3D11Texture2D* GetD3D11Texture() const { return texture_.Get(); }
    ID3D11ShaderResourceView* GetSRV() const { return srv_.Get(); }
    ID3D11RenderTargetView* GetRTV() const { return rtv_.Get(); }

private:
    D3D11Device* device_;
    ComPtr<ID3D11Texture2D> texture_;
    ComPtr<ID3D11ShaderResourceView> srv_;
    ComPtr<ID3D11RenderTargetView> rtv_;
    bool ownsTexture_;
};

class D3D11Sampler : public RHISampler {
public:
    D3D11Sampler(D3D11Device* device, const SamplerDesc& desc);
    ~D3D11Sampler() override;

    ID3D11SamplerState* GetD3D11Sampler() const { return sampler_.Get(); }

private:
    D3D11Device* device_;
    ComPtr<ID3D11SamplerState> sampler_;
};

class D3D11Shader : public RHIShader {
public:
    D3D11Shader(D3D11Device* device, ShaderStage stage, const void* bytecode, size_t size);
    ~D3D11Shader() override;

    ID3D11VertexShader* GetVertexShader() const { return vertexShader_.Get(); }
    ID3D11PixelShader* GetPixelShader() const { return pixelShader_.Get(); }
    ID3D11GeometryShader* GetGeometryShader() const { return geometryShader_.Get(); }
    ID3D11ComputeShader* GetComputeShader() const { return computeShader_.Get(); }
    const void* GetBytecode() const { return bytecode_.data(); }
    size_t GetBytecodeSize() const { return bytecode_.size(); }

private:
    D3D11Device* device_;
    ComPtr<ID3D11VertexShader> vertexShader_;
    ComPtr<ID3D11PixelShader> pixelShader_;
    ComPtr<ID3D11GeometryShader> geometryShader_;
    ComPtr<ID3D11ComputeShader> computeShader_;
    std::vector<uint8_t> bytecode_;
};

class D3D11Pipeline : public RHIPipeline {
public:
    D3D11Pipeline(D3D11Device* device,
                  ID3D11BlendState* blendState,
                  ID3D11DepthStencilState* depthStencilState,
                  ID3D11RasterizerState* rasterizerState,
                  ID3D11InputLayout* inputLayout,
                  D3D11Shader* vertexShader,
                  D3D11Shader* pixelShader,
                  D3D10_PRIMITIVE_TOPOLOGY topology);
    ~D3D11Pipeline() override;

    ID3D11BlendState* GetBlendState() const { return blendState_.Get(); }
    ID3D11DepthStencilState* GetDepthStencilState() const { return depthStencilState_.Get(); }
    ID3D11RasterizerState* GetRasterizerState() const { return rasterizerState_.Get(); }
    ID3D11InputLayout* GetInputLayout() const { return inputLayout_.Get(); }
    D3D11Shader* GetVertexShader() const { return vertexShader_; }
    D3D11Shader* GetPixelShader() const { return pixelShader_; }
    D3D10_PRIMITIVE_TOPOLOGY GetTopology() const { return topology_; }

private:
    D3D11Device* device_;
    ComPtr<ID3D11BlendState> blendState_;
    ComPtr<ID3D11DepthStencilState> depthStencilState_;
    ComPtr<ID3D11RasterizerState> rasterizerState_;
    ComPtr<ID3D11InputLayout> inputLayout_;
    D3D11Shader* vertexShader_;
    D3D11Shader* pixelShader_;
    D3D10_PRIMITIVE_TOPOLOGY topology_;
};

class D3D11RenderPass : public RHIRenderPass {
public:
    D3D11RenderPass(const std::vector<TextureFormat>& colorFormats, TextureFormat depthFormat);
    ~D3D11RenderPass() override;

    const std::vector<TextureFormat>& GetColorFormats() const { return colorFormats_; }
    TextureFormat GetDepthFormat() const { return depthFormat_; }

private:
    std::vector<TextureFormat> colorFormats_;
    TextureFormat depthFormat_;
};

class D3D11Framebuffer : public RHIFramebuffer {
public:
    D3D11Framebuffer(const std::vector<ID3D11RenderTargetView*>& rtvs,
                     ID3D11DepthStencilView* dsv,
                     uint32_t width, uint32_t height);
    ~D3D11Framebuffer() override;

    const std::vector<ID3D11RenderTargetView*>& GetRTVs() const { return rtvs_; }
    ID3D11DepthStencilView* GetDSV() const { return dsv_; }
    uint32_t GetWidth() const { return width_; }
    uint32_t GetHeight() const { return height_; }

private:
    std::vector<ID3D11RenderTargetView*> rtvs_;
    ID3D11DepthStencilView* dsv_;
    uint32_t width_;
    uint32_t height_;
};

} // namespace RHI
} // namespace Nexus

#endif // NEXUS_DIRECTX11_ENABLED
