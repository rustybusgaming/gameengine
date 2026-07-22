# Nexus Engine - Multi-API Graphics Support

## Overview

The Nexus Engine now supports multiple graphics APIs through a Render Hardware Interface (RHI) abstraction layer:

- **Vulkan** (Primary, cross-platform)
- **OpenGL 4.5+** (Planned, wide compatibility)
- **DirectX 11** (Legacy, Windows only)

## Quick Start

### Prerequisites

1. **SDL2** - Cross-platform windowing
   - Windows: Download from https://www.libsdl.org/download-2.0.php
   - Extract to `C:\SDL2` or `thirdparty/SDL2`

2. **Vulkan SDK** (for Vulkan backend)
   - Download from https://vulkan.lunarg.com/
   - Install to default location
   - SDK includes validation layers and debugging tools

3. **CMake 3.16+** and a C++17 compiler

### Building

```bash
# Create build directory
mkdir build
cd build

# Configure with CMake (default: Vulkan enabled)
cmake ..

# Build
cmake --build . --config Release

# Or disable Vulkan if SDK not available
cmake .. -DENABLE_VULKAN=OFF
```

### CMake Options

```cmake
# Graphics API Selection
ENABLE_VULKAN=ON/OFF          # Enable Vulkan backend (default: ON)
ENABLE_OPENGL=ON/OFF          # Enable OpenGL backend (default: ON, not implemented)
ENABLE_DIRECTX11=ON/OFF       # Enable DirectX 11 backend (default: ON, needs refactor)

# Set default API to use at runtime
NEXUS_DEFAULT_GRAPHICS_API    # "Vulkan", "OpenGL", or "DirectX11"
```

### Example Configuration

```bash
# Vulkan only build
cmake .. -DENABLE_VULKAN=ON -DENABLE_OPENGL=OFF -DENABLE_DIRECTX11=OFF

# All APIs enabled (when ready)
cmake .. -DENABLE_VULKAN=ON -DENABLE_OPENGL=ON -DENABLE_DIRECTX11=ON -DNEXUS_DEFAULT_GRAPHICS_API=Vulkan
```

## Architecture

### RHI Abstraction Layer

Located in `include/RHI/` and `src/rhi/`:

```
include/RHI/
├── RHIDefinitions.h     # Enums, structs, common types
├── RHIResource.h        # Abstract resource classes (Buffer, Texture, etc.)
├── RHIDevice.h          # Main device interface
├── Vulkan/
│   ├── VulkanDevice.h
│   ├── VulkanResource.h
│   └── VulkanCommandBuffer.h
└── (OpenGL/, D3D11/ - planned)

src/rhi/
├── RHIDevice.cpp        # Device factory
└── vulkan/
    ├── VulkanDevice.cpp
    ├── VulkanResource.cpp
    └── VulkanCommandBuffer.cpp
```

### Key Classes

**RHIDevice** - Main graphics device
- Creates resources (buffers, textures, shaders)
- Manages swap chain and presentation
- Command buffer allocation

**RHICommandBuffer** - Records rendering commands
- Begin/end render passes
- Draw calls
- Resource bindings

**RHIBuffer** - GPU buffers (vertex, index, uniform)
**RHITexture** - Textures and render targets
**RHIShader** - Shader modules
**RHIPipeline** - Complete graphics pipeline state

## Usage Example

```cpp
#include "RHI/RHIDevice.h"
#include <SDL2/SDL.h>

// Initialize SDL window
SDL_Init(SDL_INIT_VIDEO);
SDL_Window* window = SDL_CreateWindow("Nexus Engine", 
    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    1280, 720, SDL_WINDOW_SHOWN | SDL_WINDOW_VULKAN);

// Create graphics device (auto-selects best API)
auto device = Nexus::RHI::RHIDevice::Create(Nexus::RHI::GraphicsAPI::Auto);

// Initialize with swap chain
Nexus::RHI::SwapChainDesc swapChainDesc{};
swapChainDesc.windowHandle = window;
swapChainDesc.width = 1280;
swapChainDesc.height = 720;
swapChainDesc.bufferCount = 2;
swapChainDesc.format = Nexus::RHI::TextureFormat::RGBA8_SRGB;
swapChainDesc.vsync = true;
swapChainDesc.fullscreen = false;

if (!device->Initialize(swapChainDesc)) {
    // Handle error
}

// Create resources
Nexus::RHI::BufferDesc bufferDesc{};
bufferDesc.size = 1024;
bufferDesc.usage = Nexus::RHI::BufferUsage::Vertex;
bufferDesc.accessMode = Nexus::RHI::BufferAccessMode::Static;

auto vertexBuffer = device->CreateBuffer(bufferDesc, vertexData);

// Render loop
bool running = true;
while (running) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) running = false;
    }

    // Begin frame
    device->BeginFrame();

    // Create and record command buffer
    auto cmdBuf = device->CreateCommandBuffer();
    cmdBuf->Begin();
    
    // Set viewport
    Nexus::RHI::Viewport viewport{0, 0, 1280, 720, 0.0f, 1.0f};
    cmdBuf->SetViewport(viewport);
    
    // Drawing commands...
    // cmdBuf->SetPipeline(pipeline.get());
    // cmdBuf->SetVertexBuffer(vertexBuffer.get());
    // cmdBuf->Draw(3);
    
    cmdBuf->End();

    // Submit and present
    device->SubmitCommandBuffer(cmdBuf.get());
    device->EndFrame();
    device->Present();
}

// Cleanup
device->Shutdown();
SDL_DestroyWindow(window);
SDL_Quit();
```

## Current Implementation Status

### ✅ Completed

- [x] RHI abstraction layer design
- [x] SDL2 windowing integration
- [x] CMake build system updates
- [x] Vulkan device initialization
- [x] Vulkan swap chain management
- [x] Vulkan command buffer recording
- [x] Vulkan buffer creation
- [x] Vulkan texture wrapping (swap chain images)
- [x] Vulkan sampler creation
- [x] Vulkan shader module creation
- [x] Configuration system (EngineConfig.h)

### 🚧 In Progress

- [ ] Vulkan pipeline creation (graphics pipelines)
- [ ] Vulkan render pass creation
- [ ] Vulkan framebuffer management
- [ ] Vulkan texture upload/creation with data
- [ ] Shader compilation (HLSL → SPIRV)
- [ ] Descriptor set management
- [ ] Resource binding system

### 📋 Planned

- [ ] OpenGL 4.5+ backend
- [ ] Refactor DirectX 11 behind RHI
- [ ] Uniform buffer management
- [ ] Multi-threaded command recording
- [ ] ImGui integration with RHI
- [ ] WebGL/WebGPU support (via Emscripten)

## Shaders

### Vulkan
Requires SPIR-V bytecode. Use `glslangValidator` or `glslc`:

```bash
# Compile GLSL to SPIR-V
glslc shader.vert -o vert.spv
glslc shader.frag -o frag.spv
```

Or use `dxc` to compile HLSL to SPIR-V:

```bash
# HLSL to SPIR-V
dxc -spirv -T vs_6_0 -E main shader.hlsl -Fo vert.spv
dxc -spirv -T ps_6_0 -E main shader.hlsl -Fo frag.spv
```

### DirectX 11
Uses HLSL compiled to D3D bytecode with `fxc` or `dxc`.

## Troubleshooting

### SDL2 Not Found
```
CMake Warning: SDL2 not found - windowing will be limited
```
**Solution:** Install SDL2 or place in `thirdparty/SDL2/` directory with `include/` and `lib/` folders.

### Vulkan SDK Not Found
```
CMake Status: Vulkan SDK not found - Vulkan backend disabled
```
**Solution:** Install Vulkan SDK from https://vulkan.lunarg.com/ or set `VULKAN_SDK` environment variable.

### Validation Layer Errors
If you see Vulkan validation errors:
1. Update graphics drivers
2. Check Vulkan SDK installation
3. Disable validation in Release builds

### Window Not Appearing
Make sure SDL2.dll is in the same directory as your executable (Windows) or properly installed system-wide (Linux).

## Migration from Old API

If you have existing DirectX 11 code:

**Before (DirectX 11):**
```cpp
#include "GraphicsDevice.h"
GraphicsDevice* device = new GraphicsDevice();
device->Initialize(hwnd, 1280, 720);
```

**After (RHI):**
```cpp
#include "RHI/RHIDevice.h"
auto device = RHI::RHIDevice::Create(RHI::GraphicsAPI::Vulkan);
// Use SDL2 window instead of HWND
device->Initialize(swapChainDesc);
```

## Performance Notes

- **Vulkan**: Best performance, explicit control, modern API
- **OpenGL**: Good compatibility, easier to debug
- **DirectX 11**: Windows only, simpler than Vulkan but older

## Contributing

When adding new graphics APIs:

1. Create backend in `include/RHI/{API}/` and `src/rhi/{api}/`
2. Implement all pure virtual methods from RHI interfaces
3. Update `RHIDevice::Create()` factory in `src/rhi/RHIDevice.cpp`
4. Add CMake detection and linking
5. Test on target platform

## References

- [Vulkan Tutorial](https://vulkan-tutorial.com/)
- [SDL2 Documentation](https://wiki.libsdl.org/)
- [Vulkan SDK](https://vulkan.lunarg.com/)
- [RenderDoc](https://renderdoc.org/) - Graphics debugging tool

## License

See main LICENSE file in repository root.
