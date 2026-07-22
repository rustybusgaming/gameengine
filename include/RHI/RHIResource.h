#pragma once

#include "RHIDefinitions.h"
#include <memory>
#include <string>

namespace Nexus {
namespace RHI {

class RHIResource {
public:
    virtual ~RHIResource() = default;

    const std::string& GetName() const { return name_; }
    void SetName(const std::string& name) { name_ = name; }

protected:
    std::string name_;
};

class RHIBuffer : public RHIResource {
public:
    virtual ~RHIBuffer() = default;

    virtual void* Map() = 0;
    virtual void Unmap() = 0;
    virtual void UpdateData(const void* data, uint64_t size, uint64_t offset = 0) = 0;

    uint64_t GetSize() const { return desc_.size; }
    BufferUsage GetUsage() const { return desc_.usage; }
    const BufferDesc& GetDesc() const { return desc_; }

protected:
    BufferDesc desc_;
};

class RHITexture : public RHIResource {
public:
    virtual ~RHITexture() = default;

    virtual void UpdateData(const void* data, uint32_t mipLevel = 0, uint32_t arraySlice = 0) = 0;

    uint32_t GetWidth() const { return desc_.width; }
    uint32_t GetHeight() const { return desc_.height; }
    uint32_t GetDepth() const { return desc_.depth; }
    TextureFormat GetFormat() const { return desc_.format; }
    const TextureDesc& GetDesc() const { return desc_; }

protected:
    TextureDesc desc_;
};

class RHISampler : public RHIResource {
public:
    virtual ~RHISampler() = default;

    const SamplerDesc& GetDesc() const { return desc_; }

protected:
    SamplerDesc desc_;
};

class RHIShader : public RHIResource {
public:
    virtual ~RHIShader() = default;

    ShaderStage GetStage() const { return stage_; }

protected:
    ShaderStage stage_;
};

class RHIPipeline : public RHIResource {
public:
    virtual ~RHIPipeline() = default;
};

class RHIRenderPass : public RHIResource {
public:
    virtual ~RHIRenderPass() = default;
};

class RHIFramebuffer : public RHIResource {
public:
    virtual ~RHIFramebuffer() = default;
};

using RHIBufferPtr = std::shared_ptr<RHIBuffer>;
using RHITexturePtr = std::shared_ptr<RHITexture>;
using RHISamplerPtr = std::shared_ptr<RHISampler>;
using RHIShaderPtr = std::shared_ptr<RHIShader>;
using RHIPipelinePtr = std::shared_ptr<RHIPipeline>;
using RHIRenderPassPtr = std::shared_ptr<RHIRenderPass>;
using RHIFramebufferPtr = std::shared_ptr<RHIFramebuffer>;

} // namespace RHI
} // namespace Nexus
