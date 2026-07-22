/**
 * @file OpenGLResource.h
 * @brief OpenGL resource implementations
 * @author Nexus Engine Team
 * @date 2026-07-22
 */

#pragma once

#ifdef NEXUS_OPENGL_ENABLED

#include "../RHIResource.h"
#include <vector>

typedef unsigned int GLuint;
typedef unsigned int GLenum;

namespace Nexus {
namespace RHI {

class OpenGLDevice;

class OpenGLBuffer : public RHIBuffer {
public:
    OpenGLBuffer(OpenGLDevice* device, const BufferDesc& desc, const void* initialData);
    ~OpenGLBuffer() override;

    void* Map() override;
    void Unmap() override;
    void UpdateData(const void* data, uint64_t size, uint64_t offset = 0) override;

    GLuint GetGLBuffer() const { return buffer_; }
    GLenum GetGLTarget() const { return target_; }

private:
    OpenGLDevice* device_;
    GLuint buffer_;
    GLenum target_;
    void* mappedData_;
};

class OpenGLTexture : public RHITexture {
public:
    OpenGLTexture(OpenGLDevice* device, const TextureDesc& desc, const void* initialData);
    OpenGLTexture(OpenGLDevice* device, GLuint texture, const TextureDesc& desc); // For back buffer
    ~OpenGLTexture() override;

    void UpdateData(const void* data, uint32_t mipLevel = 0, uint32_t arraySlice = 0) override;

    GLuint GetGLTexture() const { return texture_; }
    GLenum GetGLTarget() const { return target_; }

private:
    OpenGLDevice* device_;
    GLuint texture_;
    GLenum target_;
    bool ownsTexture_;
};

class OpenGLSampler : public RHISampler {
public:
    OpenGLSampler(OpenGLDevice* device, const SamplerDesc& desc);
    ~OpenGLSampler() override;

    GLuint GetGLSampler() const { return sampler_; }

private:
    OpenGLDevice* device_;
    GLuint sampler_;
};

class OpenGLShader : public RHIShader {
public:
    OpenGLShader(OpenGLDevice* device, ShaderStage stage, const std::string& source);
    ~OpenGLShader() override;

    GLuint GetGLShader() const { return shader_; }

private:
    OpenGLDevice* device_;
    GLuint shader_;
};

class OpenGLPipeline : public RHIPipeline {
public:
    OpenGLPipeline(OpenGLDevice* device,
                   GLuint program,
                   const BlendStateDesc& blendState,
                   const DepthStencilStateDesc& depthStencilState,
                   const RasterizerStateDesc& rasterizerState,
                   GLenum topology);
    ~OpenGLPipeline() override;

    GLuint GetGLProgram() const { return program_; }
    GLenum GetGLTopology() const { return topology_; }

    const BlendStateDesc& GetBlendState() const { return blendState_; }
    const DepthStencilStateDesc& GetDepthStencilState() const { return depthStencilState_; }
    const RasterizerStateDesc& GetRasterizerState() const { return rasterizerState_; }

private:
    OpenGLDevice* device_;
    GLuint program_;
    GLenum topology_;
    BlendStateDesc blendState_;
    DepthStencilStateDesc depthStencilState_;
    RasterizerStateDesc rasterizerState_;
};

class OpenGLRenderPass : public RHIRenderPass {
public:
    OpenGLRenderPass(const std::vector<TextureFormat>& colorFormats, TextureFormat depthFormat);
    ~OpenGLRenderPass() override;

    const std::vector<TextureFormat>& GetColorFormats() const { return colorFormats_; }
    TextureFormat GetDepthFormat() const { return depthFormat_; }

private:
    std::vector<TextureFormat> colorFormats_;
    TextureFormat depthFormat_;
};

class OpenGLFramebuffer : public RHIFramebuffer {
public:
    OpenGLFramebuffer(OpenGLDevice* device, const std::vector<GLuint>& colorAttachments,
                      GLuint depthAttachment, uint32_t width, uint32_t height);
    ~OpenGLFramebuffer() override;

    GLuint GetGLFramebuffer() const { return framebuffer_; }
    uint32_t GetWidth() const { return width_; }
    uint32_t GetHeight() const { return height_; }

private:
    OpenGLDevice* device_;
    GLuint framebuffer_;
    uint32_t width_;
    uint32_t height_;
};

} // namespace RHI
} // namespace Nexus

#endif // NEXUS_OPENGL_ENABLED
