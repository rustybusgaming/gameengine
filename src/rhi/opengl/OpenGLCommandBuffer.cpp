#ifdef NEXUS_OPENGL_ENABLED

#include "RHI/OpenGL/OpenGLCommandBuffer.h"
#include "RHI/OpenGL/OpenGLDevice.h"
#include "RHI/OpenGL/OpenGLResource.h"
#include "Logger.h"
#include <glad/glad.h>

namespace Nexus {
namespace RHI {

OpenGLCommandBuffer::OpenGLCommandBuffer(OpenGLDevice* device)
    : device_(device)
    , currentPipeline_(nullptr) {
}

OpenGLCommandBuffer::~OpenGLCommandBuffer() {
}

void OpenGLCommandBuffer::Begin() {
    // OpenGL is immediate mode, no explicit command buffer recording needed
    currentPipeline_ = nullptr;
}

void OpenGLCommandBuffer::End() {
    // Flush commands to ensure they're executed
    glFlush();
}

void OpenGLCommandBuffer::BeginRenderPass(RHIRenderPass* renderPass, RHIFramebuffer* framebuffer) {
    if (!framebuffer) {
        // Bind default framebuffer (back buffer)
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    } else {
        OpenGLFramebuffer* glFB = static_cast<OpenGLFramebuffer*>(framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, glFB->GetGLFramebuffer());
    }
}

void OpenGLCommandBuffer::EndRenderPass() {
    // Unbind framebuffer (optional, usually next pass will bind its own)
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void OpenGLCommandBuffer::SetViewport(const Viewport& viewport) {
    glViewport(
        static_cast<GLint>(viewport.x),
        static_cast<GLint>(viewport.y),
        static_cast<GLsizei>(viewport.width),
        static_cast<GLsizei>(viewport.height)
    );
    glDepthRangef(viewport.minDepth, viewport.maxDepth);
}

void OpenGLCommandBuffer::SetScissor(const Scissor& scissor) {
    glEnable(GL_SCISSOR_TEST);
    glScissor(
        static_cast<GLint>(scissor.x),
        static_cast<GLint>(scissor.y),
        static_cast<GLsizei>(scissor.width),
        static_cast<GLsizei>(scissor.height)
    );
}

void OpenGLCommandBuffer::SetPipeline(RHIPipeline* pipeline) {
    currentPipeline_ = pipeline;

    if (!pipeline) return;

    OpenGLPipeline* glPipeline = static_cast<OpenGLPipeline*>(pipeline);

    // Bind shader program
    glUseProgram(glPipeline->GetGLProgram());

    // Apply blend state
    const auto& blendState = glPipeline->GetBlendState();
    if (blendState.blendEnable) {
        glEnable(GL_BLEND);

        auto ToGLBlend = [](BlendFactor factor) -> GLenum {
            switch (factor) {
                case BlendFactor::Zero: return GL_ZERO;
                case BlendFactor::One: return GL_ONE;
                case BlendFactor::SrcColor: return GL_SRC_COLOR;
                case BlendFactor::InvSrcColor: return GL_ONE_MINUS_SRC_COLOR;
                case BlendFactor::SrcAlpha: return GL_SRC_ALPHA;
                case BlendFactor::InvSrcAlpha: return GL_ONE_MINUS_SRC_ALPHA;
                case BlendFactor::DstColor: return GL_DST_COLOR;
                case BlendFactor::InvDstColor: return GL_ONE_MINUS_DST_COLOR;
                case BlendFactor::DstAlpha: return GL_DST_ALPHA;
                case BlendFactor::InvDstAlpha: return GL_ONE_MINUS_DST_ALPHA;
                default: return GL_ONE;
            }
        };

        auto ToGLBlendOp = [](BlendOp op) -> GLenum {
            switch (op) {
                case BlendOp::Add: return GL_FUNC_ADD;
                case BlendOp::Subtract: return GL_FUNC_SUBTRACT;
                case BlendOp::ReverseSubtract: return GL_FUNC_REVERSE_SUBTRACT;
                case BlendOp::Min: return GL_MIN;
                case BlendOp::Max: return GL_MAX;
                default: return GL_FUNC_ADD;
            }
        };

        glBlendFuncSeparate(
            ToGLBlend(blendState.srcBlend),
            ToGLBlend(blendState.dstBlend),
            ToGLBlend(blendState.srcBlendAlpha),
            ToGLBlend(blendState.dstBlendAlpha)
        );

        glBlendEquationSeparate(
            ToGLBlendOp(blendState.blendOp),
            ToGLBlendOp(blendState.blendOpAlpha)
        );
    } else {
        glDisable(GL_BLEND);
    }

    // Apply depth/stencil state
    const auto& depthState = glPipeline->GetDepthStencilState();
    if (depthState.depthTestEnable) {
        glEnable(GL_DEPTH_TEST);

        auto ToGLCompareOp = [](CompareOp op) -> GLenum {
            switch (op) {
                case CompareOp::Never: return GL_NEVER;
                case CompareOp::Less: return GL_LESS;
                case CompareOp::Equal: return GL_EQUAL;
                case CompareOp::LessOrEqual: return GL_LEQUAL;
                case CompareOp::Greater: return GL_GREATER;
                case CompareOp::NotEqual: return GL_NOTEQUAL;
                case CompareOp::GreaterOrEqual: return GL_GEQUAL;
                case CompareOp::Always: return GL_ALWAYS;
                default: return GL_LESS;
            }
        };

        glDepthFunc(ToGLCompareOp(depthState.depthCompareOp));
        glDepthMask(depthState.depthWriteEnable ? GL_TRUE : GL_FALSE);
    } else {
        glDisable(GL_DEPTH_TEST);
    }

    if (depthState.stencilTestEnable) {
        glEnable(GL_STENCIL_TEST);
    } else {
        glDisable(GL_STENCIL_TEST);
    }

    // Apply rasterizer state
    const auto& rasterizerState = glPipeline->GetRasterizerState();

    if (rasterizerState.cullMode != CullMode::None) {
        glEnable(GL_CULL_FACE);
        glCullFace(rasterizerState.cullMode == CullMode::Back ? GL_BACK : GL_FRONT);
    } else {
        glDisable(GL_CULL_FACE);
    }

    glFrontFace(rasterizerState.frontFace == FrontFace::CCW ? GL_CCW : GL_CW);

    if (rasterizerState.fillMode == FillMode::Wireframe) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } else {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    if (rasterizerState.depthBiasEnable) {
        glEnable(GL_POLYGON_OFFSET_FILL);
        glPolygonOffset(rasterizerState.depthBiasSlope, rasterizerState.depthBias);
    } else {
        glDisable(GL_POLYGON_OFFSET_FILL);
    }

    if (rasterizerState.scissorEnable) {
        glEnable(GL_SCISSOR_TEST);
    } else {
        glDisable(GL_SCISSOR_TEST);
    }
}

void OpenGLCommandBuffer::SetVertexBuffer(RHIBuffer* buffer, uint32_t binding, uint64_t offset) {
    if (!buffer) return;

    OpenGLBuffer* glBuffer = static_cast<OpenGLBuffer*>(buffer);
    glBindBuffer(GL_ARRAY_BUFFER, glBuffer->GetGLBuffer());

    // Note: Vertex attribute setup should be done separately with vertex input layout
    // This is a simplified binding - full implementation would track vertex attributes
}

void OpenGLCommandBuffer::SetIndexBuffer(RHIBuffer* buffer, uint64_t offset) {
    if (!buffer) return;

    OpenGLBuffer* glBuffer = static_cast<OpenGLBuffer*>(buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, glBuffer->GetGLBuffer());
}

void OpenGLCommandBuffer::Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) {
    if (!currentPipeline_) {
        Logger::Warning("Draw called without pipeline set");
        return;
    }

    OpenGLPipeline* glPipeline = static_cast<OpenGLPipeline*>(currentPipeline_);
    GLenum topology = glPipeline->GetGLTopology();

    if (instanceCount > 1) {
        glDrawArraysInstancedBaseInstance(topology, firstVertex, vertexCount, instanceCount, firstInstance);
    } else {
        glDrawArrays(topology, firstVertex, vertexCount);
    }
}

void OpenGLCommandBuffer::DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance) {
    if (!currentPipeline_) {
        Logger::Warning("DrawIndexed called without pipeline set");
        return;
    }

    OpenGLPipeline* glPipeline = static_cast<OpenGLPipeline*>(currentPipeline_);
    GLenum topology = glPipeline->GetGLTopology();

    // Assuming 32-bit indices
    const void* offset = reinterpret_cast<const void*>(static_cast<uintptr_t>(firstIndex * sizeof(uint32_t)));

    if (instanceCount > 1) {
        if (vertexOffset != 0 || firstInstance != 0) {
            glDrawElementsInstancedBaseVertexBaseInstance(topology, indexCount, GL_UNSIGNED_INT, offset,
                                                          instanceCount, vertexOffset, firstInstance);
        } else {
            glDrawElementsInstanced(topology, indexCount, GL_UNSIGNED_INT, offset, instanceCount);
        }
    } else {
        if (vertexOffset != 0) {
            glDrawElementsBaseVertex(topology, indexCount, GL_UNSIGNED_INT, offset, vertexOffset);
        } else {
            glDrawElements(topology, indexCount, GL_UNSIGNED_INT, offset);
        }
    }
}

void OpenGLCommandBuffer::ClearRenderTarget(RHITexture* target, const ClearColor& color) {
    glClearColor(color.r, color.g, color.b, color.a);
    glClear(GL_COLOR_BUFFER_BIT);
}

void OpenGLCommandBuffer::ClearDepthStencil(RHITexture* target, float depth, uint8_t stencil) {
    glClearDepth(depth);
    glClearStencil(stencil);
    glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void OpenGLCommandBuffer::CopyBuffer(RHIBuffer* src, RHIBuffer* dst, uint64_t size, uint64_t srcOffset, uint64_t dstOffset) {
    if (!src || !dst) return;

    OpenGLBuffer* glSrc = static_cast<OpenGLBuffer*>(src);
    OpenGLBuffer* glDst = static_cast<OpenGLBuffer*>(dst);

    glBindBuffer(GL_COPY_READ_BUFFER, glSrc->GetGLBuffer());
    glBindBuffer(GL_COPY_WRITE_BUFFER, glDst->GetGLBuffer());
    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, srcOffset, dstOffset, size);
    glBindBuffer(GL_COPY_READ_BUFFER, 0);
    glBindBuffer(GL_COPY_WRITE_BUFFER, 0);
}

void OpenGLCommandBuffer::CopyTexture(RHITexture* src, RHITexture* dst) {
    if (!src || !dst) return;

    OpenGLTexture* glSrc = static_cast<OpenGLTexture*>(src);
    OpenGLTexture* glDst = static_cast<OpenGLTexture*>(dst);

    // Simple blit for 2D textures
    GLuint srcFBO, dstFBO;
    glGenFramebuffers(1, &srcFBO);
    glGenFramebuffers(1, &dstFBO);

    glBindFramebuffer(GL_READ_FRAMEBUFFER, srcFBO);
    glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, glSrc->GetGLTexture(), 0);

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dstFBO);
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, glDst->GetGLTexture(), 0);

    glBlitFramebuffer(
        0, 0, src->GetWidth(), src->GetHeight(),
        0, 0, dst->GetWidth(), dst->GetHeight(),
        GL_COLOR_BUFFER_BIT, GL_NEAREST
    );

    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

    glDeleteFramebuffers(1, &srcFBO);
    glDeleteFramebuffers(1, &dstFBO);
}

} // namespace RHI
} // namespace Nexus

#endif // NEXUS_OPENGL_ENABLED
