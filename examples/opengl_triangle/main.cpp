/**
 * @file main.cpp
 * @brief Simple OpenGL triangle demo using Nexus RHI
 * @author Nexus Engine Team
 * @date 2026-07-22
 */

#include "RHI/RHIDevice.h"
#include <SDL2/SDL.h>
#include <iostream>
#include <vector>

// Vertex structure
struct Vertex {
    float position[3];
    float color[3];
};

// GLSL Shaders
const char* vertexShaderSource = R"(
#version 450 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aColor;

out vec3 vColor;

void main() {
    gl_Position = vec4(aPosition, 1.0);
    vColor = aColor;
}
)";

const char* fragmentShaderSource = R"(
#version 450 core

in vec3 vColor;
out vec4 FragColor;

void main() {
    FragColor = vec4(vColor, 1.0);
}
)";

int main(int argc, char* argv[]) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "Failed to initialize SDL: " << SDL_GetError() << std::endl;
        return -1;
    }

    // Create window with OpenGL support
    SDL_Window* window = SDL_CreateWindow(
        "Nexus Engine - OpenGL Triangle Demo",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        1280, 720,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN
    );

    if (!window) {
        std::cerr << "Failed to create window: " << SDL_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }

    // Create OpenGL RHI device
    std::cout << "Creating OpenGL RHI Device..." << std::endl;
    auto device = Nexus::RHI::RHIDevice::Create(Nexus::RHI::GraphicsAPI::OpenGL);

    if (!device) {
        std::cerr << "Failed to create OpenGL device" << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    // Initialize device with swap chain
    Nexus::RHI::SwapChainDesc swapChainDesc{};
    swapChainDesc.windowHandle = window;
    swapChainDesc.width = 1280;
    swapChainDesc.height = 720;
    swapChainDesc.bufferCount = 2;
    swapChainDesc.format = Nexus::RHI::TextureFormat::RGBA8_SRGB;
    swapChainDesc.vsync = true;
    swapChainDesc.fullscreen = false;

    if (!device->Initialize(swapChainDesc)) {
        std::cerr << "Failed to initialize OpenGL device" << std::endl;
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    std::cout << "OpenGL Device: " << device->GetAPIName() << std::endl;

    // Create vertex buffer with triangle data
    std::vector<Vertex> vertices = {
        // Position              Color
        {{ 0.0f,  0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},  // Top (Red)
        {{-0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},  // Bottom-left (Green)
        {{ 0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}}   // Bottom-right (Blue)
    };

    Nexus::RHI::BufferDesc vertexBufferDesc{};
    vertexBufferDesc.size = sizeof(Vertex) * vertices.size();
    vertexBufferDesc.usage = Nexus::RHI::BufferUsage::Vertex;
    vertexBufferDesc.accessMode = Nexus::RHI::BufferAccessMode::Static;

    auto vertexBuffer = device->CreateBuffer(vertexBufferDesc, vertices.data());

    if (!vertexBuffer) {
        std::cerr << "Failed to create vertex buffer" << std::endl;
        device->Shutdown();
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    std::cout << "Created vertex buffer with " << vertices.size() << " vertices" << std::endl;

    // Create shaders
    auto vertexShader = device->CreateShaderFromSource(
        Nexus::RHI::ShaderStage::Vertex,
        vertexShaderSource,
        "main"
    );

    auto fragmentShader = device->CreateShaderFromSource(
        Nexus::RHI::ShaderStage::Fragment,
        fragmentShaderSource,
        "main"
    );

    if (!vertexShader || !fragmentShader) {
        std::cerr << "Failed to create shaders" << std::endl;
        device->Shutdown();
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    std::cout << "Shaders compiled successfully" << std::endl;

    // Create graphics pipeline
    Nexus::RHI::BlendStateDesc blendState{};
    blendState.blendEnable = false;

    Nexus::RHI::DepthStencilStateDesc depthStencilState{};
    depthStencilState.depthTestEnable = false;
    depthStencilState.depthWriteEnable = false;
    depthStencilState.stencilTestEnable = false;

    Nexus::RHI::RasterizerStateDesc rasterizerState{};
    rasterizerState.cullMode = Nexus::RHI::CullMode::None;
    rasterizerState.fillMode = Nexus::RHI::FillMode::Solid;
    rasterizerState.frontFace = Nexus::RHI::FrontFace::CCW;

    std::vector<Nexus::RHI::RHIShader*> shaders = {vertexShader.get(), fragmentShader.get()};

    auto pipeline = device->CreateGraphicsPipeline(
        shaders,
        blendState,
        depthStencilState,
        rasterizerState,
        Nexus::RHI::PrimitiveTopology::TriangleList
    );

    if (!pipeline) {
        std::cerr << "Failed to create graphics pipeline" << std::endl;
        device->Shutdown();
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    std::cout << "Graphics pipeline created successfully" << std::endl;
    std::cout << "\n=== OpenGL Triangle Demo Running ===" << std::endl;
    std::cout << "Press ESC or close window to exit" << std::endl;

    // Main rendering loop
    bool running = true;
    SDL_Event event;
    uint32_t frameCount = 0;
    uint32_t lastTime = SDL_GetTicks();

    while (running) {
        // Handle events
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            } else if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    running = false;
                }
            } else if (event.type == SDL_WINDOWEVENT) {
                if (event.window.event == SDL_WINDOWEVENT_RESIZED) {
                    int width = event.window.data1;
                    int height = event.window.data2;
                    device->ResizeSwapChain(width, height);
                    std::cout << "Window resized to " << width << "x" << height << std::endl;
                }
            }
        }

        // Begin frame
        device->BeginFrame();

        // Create command buffer
        auto cmdBuffer = device->CreateCommandBuffer();
        cmdBuffer->Begin();

        // Set viewport
        Nexus::RHI::Viewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(device->GetBackBufferWidth());
        viewport.height = static_cast<float>(device->GetBackBufferHeight());
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        cmdBuffer->SetViewport(viewport);

        // Clear screen
        Nexus::RHI::ClearColor clearColor{0.1f, 0.1f, 0.15f, 1.0f};
        cmdBuffer->ClearRenderTarget(device->GetBackBuffer(), clearColor);

        // Bind pipeline and vertex buffer
        cmdBuffer->SetPipeline(pipeline.get());
        cmdBuffer->SetVertexBuffer(vertexBuffer.get());

        // Draw triangle
        cmdBuffer->Draw(3, 1, 0, 0);

        // End command buffer
        cmdBuffer->End();

        // Submit and present
        device->SubmitCommandBuffer(cmdBuffer.get());
        device->EndFrame();
        device->Present();

        // FPS counter
        frameCount++;
        uint32_t currentTime = SDL_GetTicks();
        if (currentTime - lastTime >= 1000) {
            std::cout << "FPS: " << frameCount << std::endl;
            frameCount = 0;
            lastTime = currentTime;
        }
    }

    // Cleanup
    std::cout << "\nShutting down..." << std::endl;
    pipeline.reset();
    fragmentShader.reset();
    vertexShader.reset();
    vertexBuffer.reset();
    device->Shutdown();
    SDL_DestroyWindow(window);
    SDL_Quit();

    std::cout << "OpenGL Triangle Demo completed successfully!" << std::endl;
    return 0;
}
