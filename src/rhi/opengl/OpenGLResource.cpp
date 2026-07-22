#ifdef NEXUS_OPENGL_ENABLED

#include "RHI/OpenGL/OpenGLResource.h"
#include "RHI/OpenGL/OpenGLDevice.h"
#include "Logger.h"
#include <glad/glad.h>
#include <cstring>

namespace Nexus {
namespace RHI {

// OpenGLBuffer
OpenGLBuffer::OpenGLBuffer(OpenGLDevice* device, const BufferDesc& desc, const void* initialData)
    : device_(device)
    , buffer_(0)
    , target_(GL_ARRAY_BUFFER)
    , mappedData_(nullptr) {

    desc_ = desc;

    switch (desc.usage) {
        case BufferUsage::Vertex: target_ = GL_ARRAY_BUFFER; break;
        case BufferUsage::Index: target_ = GL_ELEMENT_ARRAY_BUFFER; break;
        case BufferUsage::Uniform: target_ = GL_UNIFORM_BUFFER; break;
        case BufferUsage::Storage: target_ = GL_SHADER_STORAGE_BUFFER; break;
    }

    GLenum usage = desc.accessMode == BufferAccessMode::Dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW;

    glGenBuffers(1, &buffer_);
    glBindBuffer(target_, buffer_);
    glBufferData(target_, desc.size, initialData, usage);
    glBindBuffer(target_, 0);
}

OpenGLBuffer::~OpenGLBuffer() {
    if (buffer_) {
        glDeleteBuffers(1, &buffer_);
    }
}

void* OpenGLBuffer::Map() {
    if (!mappedData_) {
        glBindBuffer(target_, buffer_);
        mappedData_ = glMapBuffer(target_, GL_WRITE_ONLY);
        glBindBuffer(target_, 0);
    }
    return mappedData_;
}

void OpenGLBuffer::Unmap() {
    if (mappedData_) {
        glBindBuffer(target_, buffer_);
        glUnmapBuffer(target_);
        glBindBuffer(target_, 0);
        mappedData_ = nullptr;
    }
}

void OpenGLBuffer::UpdateData(const void* data, uint64_t size, uint64_t offset) {
    glBindBuffer(target_, buffer_);
    glBufferSubData(target_, offset, size, data);
    glBindBuffer(target_, 0);
}

// OpenGLTexture
OpenGLTexture::OpenGLTexture(OpenGLDevice* device, const TextureDesc& desc, const void* initialData)
    : device_(device)
    , texture_(0)
    , target_(GL_TEXTURE_2D)
    , ownsTexture_(true) {

    desc_ = desc;

    if (desc.depth > 1) {
        target_ = GL_TEXTURE_3D;
    } else if (desc.arraySize > 1) {
        target_ = GL_TEXTURE_2D_ARRAY;
    }

    glGenTextures(1, &texture_);
    glBindTexture(target_, texture_);

    GLenum internalFormat = GL_RGBA8;
    GLenum format = GL_RGBA;
    GLenum type = GL_UNSIGNED_BYTE;

    switch (desc.format) {
        case TextureFormat::RGBA8_UNORM:
            internalFormat = GL_RGBA8; format = GL_RGBA; type = GL_UNSIGNED_BYTE; break;
        case TextureFormat::RGBA8_SRGB:
            internalFormat = GL_SRGB8_ALPHA8; format = GL_RGBA; type = GL_UNSIGNED_BYTE; break;
        case TextureFormat::RGBA16_FLOAT:
            internalFormat = GL_RGBA16F; format = GL_RGBA; type = GL_FLOAT; break;
        case TextureFormat::RGBA32_FLOAT:
            internalFormat = GL_RGBA32F; format = GL_RGBA; type = GL_FLOAT; break;
        case TextureFormat::D24_UNORM_S8_UINT:
            internalFormat = GL_DEPTH24_STENCIL8; format = GL_DEPTH_STENCIL; type = GL_UNSIGNED_INT_24_8; break;
        case TextureFormat::D32_FLOAT:
            internalFormat = GL_DEPTH_COMPONENT32F; format = GL_DEPTH_COMPONENT; type = GL_FLOAT; break;
        default: break;
    }

    if (target_ == GL_TEXTURE_2D) {
        glTexStorage2D(target_, desc.mipLevels, internalFormat, desc.width, desc.height);
        if (initialData) {
            glTexSubImage2D(target_, 0, 0, 0, desc.width, desc.height, format, type, initialData);
        }
    }

    // Set default filtering
    glTexParameteri(target_, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(target_, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(target_, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(target_, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glBindTexture(target_, 0);
}

OpenGLTexture::OpenGLTexture(OpenGLDevice* device, GLuint texture, const TextureDesc& desc)
    : device_(device)
    , texture_(texture)
    , target_(GL_TEXTURE_2D)
    , ownsTexture_(false) {
    desc_ = desc;
}

OpenGLTexture::~OpenGLTexture() {
    if (ownsTexture_ && texture_) {
        glDeleteTextures(1, &texture_);
    }
}

void OpenGLTexture::UpdateData(const void* data, uint32_t mipLevel, uint32_t arraySlice) {
    glBindTexture(target_, texture_);

    GLenum format = GL_RGBA;
    GLenum type = GL_UNSIGNED_BYTE;

    glTexSubImage2D(target_, mipLevel, 0, 0, desc_.width >> mipLevel, desc_.height >> mipLevel,
                    format, type, data);

    glBindTexture(target_, 0);
}

// OpenGLSampler
OpenGLSampler::OpenGLSampler(OpenGLDevice* device, const SamplerDesc& desc)
    : device_(device)
    , sampler_(0) {

    desc_ = desc;

    glGenSamplers(1, &sampler_);

    auto ToGLFilter = [](SamplerFilter filter) {
        return filter == SamplerFilter::Point ? GL_NEAREST : GL_LINEAR;
    };

    auto ToGLWrap = [](SamplerAddressMode mode) {
        switch (mode) {
            case SamplerAddressMode::Wrap: return GL_REPEAT;
            case SamplerAddressMode::Clamp: return GL_CLAMP_TO_EDGE;
            case SamplerAddressMode::Mirror: return GL_MIRRORED_REPEAT;
            case SamplerAddressMode::Border: return GL_CLAMP_TO_BORDER;
            default: return GL_REPEAT;
        }
    };

    glSamplerParameteri(sampler_, GL_TEXTURE_MIN_FILTER, ToGLFilter(desc.minFilter));
    glSamplerParameteri(sampler_, GL_TEXTURE_MAG_FILTER, ToGLFilter(desc.magFilter));
    glSamplerParameteri(sampler_, GL_TEXTURE_WRAP_S, ToGLWrap(desc.addressU));
    glSamplerParameteri(sampler_, GL_TEXTURE_WRAP_R, ToGLWrap(desc.addressV));
    glSamplerParameteri(sampler_, GL_TEXTURE_WRAP_T, ToGLWrap(desc.addressW));

    if (desc.maxAnisotropy > 1) {
        glSamplerParameterf(sampler_, GL_TEXTURE_MAX_ANISOTROPY, static_cast<float>(desc.maxAnisotropy));
    }
}

OpenGLSampler::~OpenGLSampler() {
    if (sampler_) {
        glDeleteSamplers(1, &sampler_);
    }
}

// OpenGLShader
OpenGLShader::OpenGLShader(OpenGLDevice* device, ShaderStage stage, const std::string& source)
    : device_(device)
    , shader_(0) {

    stage_ = stage;

    GLenum glStage = GL_VERTEX_SHADER;
    switch (stage) {
        case ShaderStage::Vertex: glStage = GL_VERTEX_SHADER; break;
        case ShaderStage::Fragment: glStage = GL_FRAGMENT_SHADER; break;
        case ShaderStage::Geometry: glStage = GL_GEOMETRY_SHADER; break;
        case ShaderStage::Compute: glStage = GL_COMPUTE_SHADER; break;
        default: break;
    }

    shader_ = glCreateShader(glStage);

    const char* src = source.c_str();
    glShaderSource(shader_, 1, &src, nullptr);
    glCompileShader(shader_);

    GLint success;
    glGetShaderiv(shader_, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader_, 512, nullptr, infoLog);
        Logger::Error("OpenGL shader compilation failed: " + std::string(infoLog));
        glDeleteShader(shader_);
        shader_ = 0;
    }
}

OpenGLShader::~OpenGLShader() {
    if (shader_) {
        glDeleteShader(shader_);
    }
}

// OpenGLPipeline
OpenGLPipeline::OpenGLPipeline(OpenGLDevice* device,
                               GLuint program,
                               const BlendStateDesc& blendState,
                               const DepthStencilStateDesc& depthStencilState,
                               const RasterizerStateDesc& rasterizerState,
                               GLenum topology)
    : device_(device)
    , program_(program)
    , topology_(topology)
    , blendState_(blendState)
    , depthStencilState_(depthStencilState)
    , rasterizerState_(rasterizerState) {
}

OpenGLPipeline::~OpenGLPipeline() {
    if (program_) {
        glDeleteProgram(program_);
    }
}

// OpenGLRenderPass
OpenGLRenderPass::OpenGLRenderPass(const std::vector<TextureFormat>& colorFormats, TextureFormat depthFormat)
    : colorFormats_(colorFormats)
    , depthFormat_(depthFormat) {
}

OpenGLRenderPass::~OpenGLRenderPass() {
}

// OpenGLFramebuffer
OpenGLFramebuffer::OpenGLFramebuffer(OpenGLDevice* device, const std::vector<GLuint>& colorAttachments,
                                     GLuint depthAttachment, uint32_t width, uint32_t height)
    : device_(device)
    , framebuffer_(0)
    , width_(width)
    , height_(height) {

    glGenFramebuffers(1, &framebuffer_);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);

    for (size_t i = 0; i < colorAttachments.size(); ++i) {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D,
                               colorAttachments[i], 0);
    }

    if (depthAttachment) {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthAttachment, 0);
    }

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        Logger::Error("OpenGL framebuffer incomplete");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

OpenGLFramebuffer::~OpenGLFramebuffer() {
    if (framebuffer_) {
        glDeleteFramebuffers(1, &framebuffer_);
    }
}

} // namespace RHI
} // namespace Nexus

#endif // NEXUS_OPENGL_ENABLED
