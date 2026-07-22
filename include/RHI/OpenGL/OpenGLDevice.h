/**
 * @file OpenGLDevice.h
 * @brief OpenGL 4.5+ RHI implementation
 * @author Nexus Engine Team
 * @date 2026-07-22
 */

#pragma once

#ifdef NEXUS_OPENGL_ENABLED

#include "../RHIDevice.h"
#include <SDL2/SDL.h>
#include <vector>

// Forward declare OpenGL types to avoid including GL headers here
typedef unsigned int GLuint;
typedef unsigned int GLenum;

namespace Nexus {
namespace RHI {

class OpenGLDevice : public RHIDevice {
public:
    OpenGLDevice();
    ~OpenGLDevice() override;

    bool Initialize(const SwapChainDesc& swapChainDesc) override;
    void Shutdown() override;

    GraphicsAPI GetAPI() const override { return GraphicsAPI::OpenGL; }
    const char* GetAPIName() const override { return "OpenGL 4.5+"; }

    void BeginFrame() override;
    void EndFrame() override;
    void Present() override;

    RHICommandBufferPtr CreateCommandBuffer() override;
    void SubmitCommandBuffer(RHICommandBuffer* commandBuffer) override;
    void WaitIdle() override;

    RHIBufferPtr CreateBuffer(const BufferDesc& desc, const void* initialData = nullptr) override;
    RHITexturePtr CreateTexture(const TextureDesc& desc, const void* initialData = nullptr) override;
    RHISamplerPtr CreateSampler(const SamplerDesc& desc) override;

    RHIShaderPtr CreateShader(ShaderStage stage, const void* bytecode, size_t size) override;
    RHIShaderPtr CreateShaderFromSource(ShaderStage stage, const std::string& source, const std::string& entryPoint = "main") override;

    RHIPipelinePtr CreateGraphicsPipeline(
        const std::vector<RHIShader*>& shaders,
        const BlendStateDesc& blendState,
        const DepthStencilStateDesc& depthStencilState,
        const RasterizerStateDesc& rasterizerState,
        PrimitiveTopology topology
    ) override;

    RHIRenderPassPtr CreateRenderPass(
        const std::vector<TextureFormat>& colorFormats,
        TextureFormat depthFormat = TextureFormat::D24_UNORM_S8_UINT
    ) override;

    RHIFramebufferPtr CreateFramebuffer(
        RHIRenderPass* renderPass,
        const std::vector<RHITexture*>& colorAttachments,
        RHITexture* depthAttachment = nullptr
    ) override;

    RHITexture* GetBackBuffer() override;
    uint32_t GetBackBufferWidth() const override { return width_; }
    uint32_t GetBackBufferHeight() const override { return height_; }

    void ResizeSwapChain(uint32_t width, uint32_t height) override;

    bool IsDeviceLost() override { return deviceLost_; }
    bool ResetDevice() override;

    SDL_GLContext GetGLContext() const { return glContext_; }

private:
    bool CreateGLContext(const SwapChainDesc& desc);
    bool InitializeGLFunctions();

    SDL_Window* window_;
    SDL_GLContext glContext_;

    GLuint defaultFramebuffer_;
    GLuint defaultVAO_;

    RHITexturePtr backBufferTexture_;

    uint32_t width_;
    uint32_t height_;
    bool vsync_;
    bool deviceLost_;
};

} // namespace RHI
} // namespace Nexus

#endif // NEXUS_OPENGL_ENABLED
