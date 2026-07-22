/**
 * @file SimpleTriangle.cpp
 * @brief Minimal example demonstrating the RHI with a colored triangle
 * @author Nexus Engine Team
 * @date 2026-07-22
 *
 * This example shows:
 * - RHI device creation with auto-API selection
 * - SDL2 window setup
 * - Basic vertex buffer creation
 * - Simple shader loading (SPIR-V for Vulkan, HLSL bytecode for D3D11)
 * - Pipeline creation
 * - Render loop
 */

#include "RHI/RHIDevice.h"
#include "Logger.h"
#include <SDL2/SDL.h>
#include <vector>
#include <fstream>

using namespace Nexus;
using namespace Nexus::RHI;

// Simple vertex structure
struct Vertex {
    float position[3];
    float color[3];
};

// Load shader bytecode from file
std::vector<uint8_t> LoadShaderFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        Logger::Error("Failed to open shader file: " + filename);
        return {};
    }

    size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<uint8_t> buffer(fileSize);

    file.seekg(0);
    file.read(reinterpret_cast<char*>(buffer.data()), fileSize);
    file.close();

    return buffer;
}

int main(int argc, char* argv[]) {
    Logger::Info("=== Nexus Engine - Simple Triangle Demo ===");

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        Logger::Error("Failed to initialize SDL: " + std::string(SDL_GetError()));
        return -1;
    }

    // Create window (SDL_WINDOW_VULKAN works for both Vulkan and D3D11 via SDL_syswm)
    SDL_Window* window = SDL_CreateWindow(
        "Nexus Engine - Triangle Demo",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        1280, 720,
        SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE
    );

    if (!window) {
        Logger::Error("Failed to create window: " + std::string(SDL_GetError()));
        SDL_Quit();
        return -1;
    }

    // Create graphics device (auto-selects best available API)
    Logger::Info("Creating graphics device...");
    auto device = RHIDevice::Create(GraphicsAPI::Auto);
    if (!device) {
        Logger::Error("Failed to create RHI device");
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    Logger::Info("Using API: " + std::string(device->GetAPIName()));

    // Setup swap chain
    SwapChainDesc swapChainDesc{};
    swapChainDesc.windowHandle = window;
    swapChainDesc.width = 1280;
    swapChainDesc.height = 720;
    swapChainDesc.bufferCount = 2;
    swapChainDesc.format = TextureFormat::RGBA8_SRGB;
    swapChainDesc.vsync = true;
    swapChainDesc.fullscreen = false;

    if (!device->Initialize(swapChainDesc)) {
        Logger::Error("Failed to initialize graphics device");
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    Logger::Info("Graphics device initialized successfully");

    // Create triangle vertices (centered, with RGB colors)
    std::vector<Vertex> vertices = {
        { { 0.0f,  0.5f, 0.0f}, {1.0f, 0.0f, 0.0f} },  // Top (Red)
        { { 0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f} },  // Bottom-right (Green)
        { {-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f} }   // Bottom-left (Blue)
    };

    // Create vertex buffer
    BufferDesc bufferDesc{};
    bufferDesc.size = sizeof(Vertex) * vertices.size();
    bufferDesc.usage = BufferUsage::Vertex;
    bufferDesc.accessMode = BufferAccessMode::Static;

    auto vertexBuffer = device->CreateBuffer(bufferDesc, vertices.data());
    if (!vertexBuffer) {
        Logger::Error("Failed to create vertex buffer");
        device->Shutdown();
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    Logger::Info("Vertex buffer created");

    // Load shaders (would need SPIR-V for Vulkan, DXBC for D3D11)
    // For now, this is a placeholder - real implementation would detect API and load appropriate format
    std::string vertexShaderPath = device->GetAPI() == GraphicsAPI::Vulkan ?
        "shaders/triangle.vert.spv" : "shaders/triangle_vs.cso";
    std::string fragmentShaderPath = device->GetAPI() == GraphicsAPI::Vulkan ?
        "shaders/triangle.frag.spv" : "shaders/triangle_ps.cso";

    auto vertexShaderCode = LoadShaderFile(vertexShaderPath);
    auto fragmentShaderCode = LoadShaderFile(fragmentShaderPath);

    RHIShaderPtr vertexShader;
    RHIShaderPtr fragmentShader;

    if (!vertexShaderCode.empty() && !fragmentShaderCode.empty()) {
        vertexShader = device->CreateShader(ShaderStage::Vertex, vertexShaderCode.data(), vertexShaderCode.size());
        fragmentShader = device->CreateShader(ShaderStage::Fragment, fragmentShaderCode.data(), fragmentShaderCode.size());

        if (vertexShader && fragmentShader) {
            Logger::Info("Shaders loaded successfully");
        }
    } else {
        Logger::Warning("Shader files not found - will render without custom shaders");
    }

    // Create pipeline state
    RHIPipelinePtr pipeline;
    if (vertexShader && fragmentShader) {
        std::vector<RHIShader*> shaders = { vertexShader.get(), fragmentShader.get() };

        BlendStateDesc blendState{};
        blendState.blendEnable = false;

        DepthStencilStateDesc depthState{};
        depthState.depthEnable = true;
        depthState.depthWriteEnable = true;
        depthState.depthFunc = CompareFunc::Less;

        RasterizerStateDesc rasterizerState{};
        rasterizerState.fillMode = FillMode::Solid;
        rasterizerState.cullMode = CullMode::Back;
        rasterizerState.frontCounterClockwise = false;
        rasterizerState.depthClipEnable = true;
        rasterizerState.scissorEnable = false;
        rasterizerState.multisampleEnable = false;

        pipeline = device->CreateGraphicsPipeline(
            shaders,
            blendState,
            depthState,
            rasterizerState,
            PrimitiveTopology::TriangleList
        );

        if (pipeline) {
            Logger::Info("Graphics pipeline created");
        }
    }

    // Main loop
    Logger::Info("Entering main loop...");
    bool running = true;
    uint32_t frameCount = 0;
    auto startTime = SDL_GetTicks();

    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    running = false;
                } else if (event.key.keysym.sym == SDLK_F11) {
                    // Toggle fullscreen
                    uint32_t flags = SDL_GetWindowFlags(window);
                    SDL_SetWindowFullscreen(window,
                        (flags & SDL_WINDOW_FULLSCREEN) ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP);
                }
            } else if (event.type == SDL_WINDOWEVENT) {
                if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                    device->ResizeSwapChain(event.window.data1, event.window.data2);
                }
            }
        }

        // Begin frame
        device->BeginFrame();

        // Create command buffer
        auto cmdBuf = device->CreateCommandBuffer();
        cmdBuf->Begin();

        // Set viewport
        Viewport viewport{};
        viewport.x = 0;
        viewport.y = 0;
        viewport.width = static_cast<float>(device->GetBackBufferWidth());
        viewport.height = static_cast<float>(device->GetBackBufferHeight());
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        cmdBuf->SetViewport(viewport);

        // Set scissor
        Scissor scissor{};
        scissor.x = 0;
        scissor.y = 0;
        scissor.width = device->GetBackBufferWidth();
        scissor.height = device->GetBackBufferHeight();
        cmdBuf->SetScissor(scissor);

        // Clear
        ClearColor clearColor{ 0.1f, 0.1f, 0.15f, 1.0f };
        cmdBuf->ClearRenderTarget(device->GetBackBuffer(), clearColor);

        // Draw triangle (if pipeline is available)
        if (pipeline) {
            cmdBuf->SetPipeline(pipeline.get());
            cmdBuf->SetVertexBuffer(vertexBuffer.get(), 0, 0);
            cmdBuf->Draw(3, 1, 0, 0);
        }

        cmdBuf->End();

        // Submit and present
        device->SubmitCommandBuffer(cmdBuf.get());
        device->EndFrame();
        device->Present();

        frameCount++;

        // Print FPS every second
        auto currentTime = SDL_GetTicks();
        if (currentTime - startTime >= 1000) {
            float fps = frameCount / ((currentTime - startTime) / 1000.0f);
            Logger::Info("FPS: " + std::to_string(static_cast<int>(fps)));
            frameCount = 0;
            startTime = currentTime;
        }
    }

    // Cleanup
    Logger::Info("Shutting down...");
    pipeline.reset();
    fragmentShader.reset();
    vertexShader.reset();
    vertexBuffer.reset();

    device->Shutdown();
    SDL_DestroyWindow(window);
    SDL_Quit();

    Logger::Info("=== Demo Complete ===");
    return 0;
}
