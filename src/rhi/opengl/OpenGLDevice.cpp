#ifdef NEXUS_OPENGL_ENABLED

#include "RHI/OpenGL/OpenGLDevice.h"
#include "RHI/OpenGL/OpenGLResource.h"
#include "RHI/OpenGL/OpenGLCommandBuffer.h"
#include "Logger.h"

#include <SDL2/SDL.h>
#include <glad/glad.h> // Using GLAD for OpenGL function loading

namespace Nexus {
namespace RHI {

static GLenum ToGLFormat(TextureFormat format) {
    switch (format) {
        case TextureFormat::R8_UNORM: return GL_R8;
        case TextureFormat::RG8_UNORM: return GL_RG8;
        case TextureFormat::RGBA8_UNORM: return GL_RGBA8;
        case TextureFormat::RGBA8_SRGB: return GL_SRGB8_ALPHA8;
        case TextureFormat::R16_FLOAT: return GL_R16F;
        case TextureFormat::RG16_FLOAT: return GL_RG16F;
        case TextureFormat::RGBA16_FLOAT: return GL_RGBA16F;
        case TextureFormat::R32_FLOAT: return GL_R32F;
        case TextureFormat::RG32_FLOAT: return GL_RG32F;
        case TextureFormat::RGBA32_FLOAT: return GL_RGBA32F;
        case TextureFormat::D24_UNORM_S8_UINT: return GL_DEPTH24_STENCIL8;
        case TextureFormat::D32_FLOAT: return GL_DEPTH_COMPONENT32F;
        default: return GL_RGBA8;
    }
}

OpenGLDevice::OpenGLDevice()
    : window_(nullptr)
    , glContext_(nullptr)
    , defaultFramebuffer_(0)
    , defaultVAO_(0)
    , width_(0)
    , height_(0)
    , vsync_(true)
    , deviceLost_(false) {
}

OpenGLDevice::~OpenGLDevice() {
    Shutdown();
}

bool OpenGLDevice::Initialize(const SwapChainDesc& swapChainDesc) {
    Logger::Info("Initializing OpenGL Device...");

    window_ = static_cast<SDL_Window*>(swapChainDesc.windowHandle);
    width_ = swapChainDesc.width;
    height_ = swapChainDesc.height;
    vsync_ = swapChainDesc.vsync;

    if (!CreateGLContext(swapChainDesc)) {
        Logger::Error("Failed to create OpenGL context");
        return false;
    }

    if (!InitializeGLFunctions()) {
        Logger::Error("Failed to initialize OpenGL functions");
        return false;
    }

    // Get OpenGL info
    const char* vendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
    const char* renderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
    const char* version = reinterpret_cast<const char*>(glGetString(GL_VERSION));

    Logger::Info("OpenGL Vendor: " + std::string(vendor));
    Logger::Info("OpenGL Renderer: " + std::string(renderer));
    Logger::Info("OpenGL Version: " + std::string(version));

    // Create default VAO (required for core profile)
    glGenVertexArrays(1, &defaultVAO_);
    glBindVertexArray(defaultVAO_);

    // Set default state
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    // Create back buffer texture wrapper
    TextureDesc texDesc{};
    texDesc.width = width_;
    texDesc.height = height_;
    texDesc.depth = 1;
    texDesc.mipLevels = 1;
    texDesc.arraySize = 1;
    texDesc.format = TextureFormat::RGBA8_SRGB;
    texDesc.usage = TextureUsage::RenderTarget;
    texDesc.sampleCount = 1;

    backBufferTexture_ = std::make_shared<OpenGLTexture>(this, 0, texDesc);

    Logger::Info("OpenGL Device initialized successfully");
    return true;
}

void OpenGLDevice::Shutdown() {
    backBufferTexture_.reset();

    if (defaultVAO_) {
        glDeleteVertexArrays(1, &defaultVAO_);
        defaultVAO_ = 0;
    }

    if (glContext_) {
        SDL_GL_DeleteContext(glContext_);
        glContext_ = nullptr;
    }
}

bool OpenGLDevice::CreateGLContext(const SwapChainDesc& desc) {
    // Set OpenGL attributes before creating context
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);

    // Set framebuffer attributes
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

#ifdef NEXUS_DEBUG
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
#endif

    glContext_ = SDL_GL_CreateContext(window_);
    if (!glContext_) {
        Logger::Error("Failed to create OpenGL context: " + std::string(SDL_GetError()));
        return false;
    }

    SDL_GL_MakeCurrent(window_, glContext_);

    // Set VSync
    if (SDL_GL_SetSwapInterval(desc.vsync ? 1 : 0) < 0) {
        Logger::Warning("VSync not supported: " + std::string(SDL_GetError()));
    }

    return true;
}

bool OpenGLDevice::InitializeGLFunctions() {
    // Load OpenGL functions using GLAD
    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
        Logger::Error("Failed to load OpenGL functions with GLAD");
        return false;
    }

    // Check version
    GLint major, minor;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);

    if (major < 4 || (major == 4 && minor < 5)) {
        Logger::Error("OpenGL 4.5+ required, but got " + std::to_string(major) + "." + std::to_string(minor));
        return false;
    }

    return true;
}

void OpenGLDevice::BeginFrame() {
    glBindFramebuffer(GL_FRAMEBUFFER, 0); // Bind default framebuffer
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void OpenGLDevice::EndFrame() {
    // Flush any pending commands
    glFlush();
}

void OpenGLDevice::Present() {
    SDL_GL_SwapWindow(window_);
}

RHICommandBufferPtr OpenGLDevice::CreateCommandBuffer() {
    return std::make_unique<OpenGLCommandBuffer>(this);
}

void OpenGLDevice::SubmitCommandBuffer(RHICommandBuffer* commandBuffer) {
    // OpenGL uses immediate mode, commands are already executed
}

void OpenGLDevice::WaitIdle() {
    glFinish();
}

RHIBufferPtr OpenGLDevice::CreateBuffer(const BufferDesc& desc, const void* initialData) {
    return std::make_shared<OpenGLBuffer>(this, desc, initialData);
}

RHITexturePtr OpenGLDevice::CreateTexture(const TextureDesc& desc, const void* initialData) {
    return std::make_shared<OpenGLTexture>(this, desc, initialData);
}

RHISamplerPtr OpenGLDevice::CreateSampler(const SamplerDesc& desc) {
    return std::make_shared<OpenGLSampler>(this, desc);
}

RHIShaderPtr OpenGLDevice::CreateShader(ShaderStage stage, const void* bytecode, size_t size) {
    // OpenGL doesn't use bytecode, needs source
    Logger::Error("OpenGL CreateShader requires source code, use CreateShaderFromSource");
    return nullptr;
}

RHIShaderPtr OpenGLDevice::CreateShaderFromSource(ShaderStage stage, const std::string& source, const std::string& entryPoint) {
    return std::make_shared<OpenGLShader>(this, stage, source);
}

RHIPipelinePtr OpenGLDevice::CreateGraphicsPipeline(
    const std::vector<RHIShader*>& shaders,
    const BlendStateDesc& blendState,
    const DepthStencilStateDesc& depthStencilState,
    const RasterizerStateDesc& rasterizerState,
    PrimitiveTopology topology) {

    // Create shader program
    GLuint program = glCreateProgram();

    for (auto* shader : shaders) {
        OpenGLShader* glShader = static_cast<OpenGLShader*>(shader);
        glAttachShader(program, glShader->GetGLShader());
    }

    glLinkProgram(program);

    // Check link status
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        Logger::Error("OpenGL program linking failed: " + std::string(infoLog));
        glDeleteProgram(program);
        return nullptr;
    }

    // Detach shaders (program has them compiled in now)
    for (auto* shader : shaders) {
        OpenGLShader* glShader = static_cast<OpenGLShader*>(shader);
        glDetachShader(program, glShader->GetGLShader());
    }

    GLenum glTopology = GL_TRIANGLES;
    switch (topology) {
        case PrimitiveTopology::TriangleList: glTopology = GL_TRIANGLES; break;
        case PrimitiveTopology::TriangleStrip: glTopology = GL_TRIANGLE_STRIP; break;
        case PrimitiveTopology::LineList: glTopology = GL_LINES; break;
        case PrimitiveTopology::LineStrip: glTopology = GL_LINE_STRIP; break;
        case PrimitiveTopology::PointList: glTopology = GL_POINTS; break;
    }

    return std::make_shared<OpenGLPipeline>(this, program, blendState, depthStencilState,
                                            rasterizerState, glTopology);
}

RHIRenderPassPtr OpenGLDevice::CreateRenderPass(
    const std::vector<TextureFormat>& colorFormats,
    TextureFormat depthFormat) {
    return std::make_shared<OpenGLRenderPass>(colorFormats, depthFormat);
}

RHIFramebufferPtr OpenGLDevice::CreateFramebuffer(
    RHIRenderPass* renderPass,
    const std::vector<RHITexture*>& colorAttachments,
    RHITexture* depthAttachment) {

    std::vector<GLuint> colorTextures;
    uint32_t width = 0, height = 0;

    for (auto* tex : colorAttachments) {
        OpenGLTexture* glTex = static_cast<OpenGLTexture*>(tex);
        colorTextures.push_back(glTex->GetGLTexture());
        if (width == 0) {
            width = tex->GetWidth();
            height = tex->GetHeight();
        }
    }

    GLuint depthTexture = 0;
    if (depthAttachment) {
        OpenGLTexture* glDepth = static_cast<OpenGLTexture*>(depthAttachment);
        depthTexture = glDepth->GetGLTexture();
    }

    return std::make_shared<OpenGLFramebuffer>(this, colorTextures, depthTexture, width, height);
}

RHITexture* OpenGLDevice::GetBackBuffer() {
    return backBufferTexture_.get();
}

void OpenGLDevice::ResizeSwapChain(uint32_t width, uint32_t height) {
    width_ = width;
    height_ = height;
    glViewport(0, 0, width, height);

    // Update back buffer texture descriptor
    TextureDesc texDesc{};
    texDesc.width = width_;
    texDesc.height = height_;
    texDesc.depth = 1;
    texDesc.mipLevels = 1;
    texDesc.arraySize = 1;
    texDesc.format = TextureFormat::RGBA8_SRGB;
    texDesc.usage = TextureUsage::RenderTarget;
    texDesc.sampleCount = 1;

    backBufferTexture_ = std::make_shared<OpenGLTexture>(this, 0, texDesc);
}

bool OpenGLDevice::ResetDevice() {
    // OpenGL doesn't really have device loss like D3D
    return true;
}

} // namespace RHI
} // namespace Nexus

#endif // NEXUS_OPENGL_ENABLED
