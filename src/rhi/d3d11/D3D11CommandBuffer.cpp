#ifdef NEXUS_DIRECTX11_ENABLED

#include "RHI/D3D11/D3D11CommandBuffer.h"
#include "RHI/D3D11/D3D11Device.h"
#include "RHI/D3D11/D3D11Resource.h"
#include "Logger.h"

namespace Nexus {
namespace RHI {

D3D11CommandBuffer::D3D11CommandBuffer(D3D11Device* device)
    : device_(device)
    , context_(device->GetD3D11Context()) {
}

D3D11CommandBuffer::~D3D11CommandBuffer() {
}

void D3D11CommandBuffer::Begin() {
    // D3D11 uses immediate context, no explicit begin
}

void D3D11CommandBuffer::End() {
    // D3D11 uses immediate context, no explicit end
}

void D3D11CommandBuffer::BeginRenderPass(RHIRenderPass* renderPass, RHIFramebuffer* framebuffer) {
    D3D11Framebuffer* d3dFB = static_cast<D3D11Framebuffer*>(framebuffer);
    auto& rtvs = d3dFB->GetRTVs();
    context_->OMSetRenderTargets(static_cast<UINT>(rtvs.size()), rtvs.data(), d3dFB->GetDSV());
}

void D3D11CommandBuffer::EndRenderPass() {
    // No explicit end in D3D11
}

void D3D11CommandBuffer::SetViewport(const Viewport& viewport) {
    D3D11_VIEWPORT vp;
    vp.TopLeftX = viewport.x;
    vp.TopLeftY = viewport.y;
    vp.Width = viewport.width;
    vp.Height = viewport.height;
    vp.MinDepth = viewport.minDepth;
    vp.MaxDepth = viewport.maxDepth;
    context_->RSSetViewports(1, &vp);
}

void D3D11CommandBuffer::SetScissor(const Scissor& scissor) {
    D3D11_RECT rect;
    rect.left = scissor.x;
    rect.top = scissor.y;
    rect.right = scissor.x + scissor.width;
    rect.bottom = scissor.y + scissor.height;
    context_->RSSetScissorRects(1, &rect);
}

void D3D11CommandBuffer::SetPipeline(RHIPipeline* pipeline) {
    D3D11Pipeline* d3dPipeline = static_cast<D3D11Pipeline*>(pipeline);

    context_->IASetInputLayout(d3dPipeline->GetInputLayout());
    context_->IASetPrimitiveTopology(d3dPipeline->GetTopology());

    if (d3dPipeline->GetVertexShader()) {
        context_->VSSetShader(d3dPipeline->GetVertexShader()->GetVertexShader(), nullptr, 0);
    }
    if (d3dPipeline->GetPixelShader()) {
        context_->PSSetShader(d3dPipeline->GetPixelShader()->GetPixelShader(), nullptr, 0);
    }

    context_->OMSetBlendState(d3dPipeline->GetBlendState(), nullptr, 0xffffffff);
    context_->OMSetDepthStencilState(d3dPipeline->GetDepthStencilState(), 0);
    context_->RSSetState(d3dPipeline->GetRasterizerState());
}

void D3D11CommandBuffer::SetVertexBuffer(RHIBuffer* buffer, uint32_t binding, uint64_t offset) {
    D3D11Buffer* d3dBuffer = static_cast<D3D11Buffer*>(buffer);
    ID3D11Buffer* vb = d3dBuffer->GetD3D11Buffer();
    UINT stride = 24; // Position + Normal (3 + 3 floats)
    UINT offsetU = static_cast<UINT>(offset);
    context_->IASetVertexBuffers(binding, 1, &vb, &stride, &offsetU);
}

void D3D11CommandBuffer::SetIndexBuffer(RHIBuffer* buffer, uint64_t offset) {
    D3D11Buffer* d3dBuffer = static_cast<D3D11Buffer*>(buffer);
    context_->IASetIndexBuffer(d3dBuffer->GetD3D11Buffer(), DXGI_FORMAT_R32_UINT, static_cast<UINT>(offset));
}

void D3D11CommandBuffer::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) {
    context_->DrawInstanced(vertexCount, instanceCount, firstVertex, firstInstance);
}

void D3D11CommandBuffer::DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) {
    context_->DrawIndexedInstanced(indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void D3D11CommandBuffer::ClearRenderTarget(RHITexture* target, const ClearColor& color) {
    D3D11Texture* d3dTexture = static_cast<D3D11Texture*>(target);
    float clearColor[4] = { color.r, color.g, color.b, color.a };
    context_->ClearRenderTargetView(d3dTexture->GetRTV(), clearColor);
}

void D3D11CommandBuffer::ClearDepthStencil(RHITexture* target, float depth, uint8_t stencil) {
    // Would need DSV in D3D11Texture
}

void D3D11CommandBuffer::CopyBuffer(RHIBuffer* src, RHIBuffer* dst, uint64_t size, uint64_t srcOffset, uint64_t dstOffset) {
    D3D11Buffer* srcBuf = static_cast<D3D11Buffer*>(src);
    D3D11Buffer* dstBuf = static_cast<D3D11Buffer*>(dst);

    D3D11_BOX srcBox;
    srcBox.left = static_cast<UINT>(srcOffset);
    srcBox.right = static_cast<UINT>(srcOffset + size);
    srcBox.top = 0;
    srcBox.bottom = 1;
    srcBox.front = 0;
    srcBox.back = 1;

    context_->CopySubresourceRegion(dstBuf->GetD3D11Buffer(), 0, static_cast<UINT>(dstOffset), 0, 0,
                                     srcBuf->GetD3D11Buffer(), 0, &srcBox);
}

void D3D11CommandBuffer::CopyTexture(RHITexture* src, RHITexture* dst) {
    D3D11Texture* srcTex = static_cast<D3D11Texture*>(src);
    D3D11Texture* dstTex = static_cast<D3D11Texture*>(dst);
    context_->CopyResource(dstTex->GetD3D11Texture(), srcTex->GetD3D11Texture());
}

} // namespace RHI
} // namespace Nexus

#endif // NEXUS_DIRECTX11_ENABLED
