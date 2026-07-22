# Nexus Engine - Quick Start Guide

Last updated: July 22, 2026

## Prerequisites

- **Windows 10/11** (primary support), Linux, or macOS
- **CMake 3.16+**
- **C++17 Compiler** (Visual Studio 2019+, GCC 9+, or Clang 10+)
- **Git** (for cloning)

## Quick Setup (Windows)

### 1. Clone the Repository

```bash
git clone <repository-url> nexus-engine
cd nexus-engine
```

### 2. Run Dependency Setup

```batch
setup_dependencies.bat
```

This automatically downloads:
- **SDL2 2.30.5** → `thirdparty/SDL2/`
- **Vulkan Headers 1.3.290** → `thirdparty/vulkan/`

### 3. Build the Engine

```bash
mkdir build
cd build
cmake ..
cmake --build . --config Release
```

### 4. Run

```bash
cd bin
.\NexusEngine.exe
```

## Quick Setup (Linux/macOS)

### 1. Clone and Setup

```bash
git clone <repository-url> nexus-engine
cd nexus-engine
chmod +x setup_dependencies.sh
./setup_dependencies.sh
```

### 2. Build

```bash
mkdir build && cd build
cmake ..
cmake --build . -j$(nproc)
```

### 3. Run

```bash
./bin/NexusEngine
```

## Graphics API Support

The engine now supports multiple graphics APIs:

| API | Platform | Status | Notes |
|-----|----------|--------|-------|
| **Vulkan** | Windows, Linux, macOS | ✅ Implemented | Default, best performance |
| **OpenGL 4.5+** | Windows, Linux, macOS | 🚧 Planned | Wide compatibility |
| **DirectX 11** | Windows only | 🔄 Legacy | Being refactored to RHI |

### Choosing Graphics API

**At CMake configure time:**
```bash
cmake .. -DENABLE_VULKAN=ON -DENABLE_OPENGL=OFF -DNEXUS_DEFAULT_GRAPHICS_API=Vulkan
```

**At runtime (in code):**
```cpp
auto device = RHI::RHIDevice::Create(RHI::GraphicsAPI::Vulkan);
// or RHI::GraphicsAPI::Auto to auto-select
```

## Project Structure

```
nexus-engine/
├── include/              # Public headers
│   ├── RHI/             # Graphics API abstraction
│   │   ├── Vulkan/      # Vulkan backend
│   │   └── ...
│   └── Engine.h, Camera.h, etc.
├── src/                 # Implementation
│   ├── rhi/             # RHI implementations
│   │   └── vulkan/      # Vulkan backend code
│   ├── graphics/        # Graphics systems
│   ├── physics/         # Physics engine
│   └── ...
├── thirdparty/          # Third-party libraries
│   ├── SDL2/            # SDL2 (auto-downloaded)
│   ├── vulkan/          # Vulkan headers (auto-downloaded)
│   ├── bullet3/         # Bullet Physics
│   ├── imgui/           # Dear ImGui
│   ├── lua/             # Lua scripting
│   └── ...
├── shaders/             # HLSL/GLSL shaders
├── examples/            # Example projects
└── GRAPHICS_API_SETUP.md  # Detailed graphics guide
```

## CMake Build Options

### Graphics APIs
```cmake
ENABLE_VULKAN=ON/OFF           # Vulkan backend (default: ON)
ENABLE_OPENGL=ON/OFF           # OpenGL backend (default: ON, not implemented)
ENABLE_DIRECTX11=ON/OFF        # DirectX 11 backend (default: ON)
NEXUS_DEFAULT_GRAPHICS_API     # "Vulkan", "OpenGL", or "DirectX11"
```

### Features
```cmake
ENABLE_PYTHON=ON/OFF           # Python scripting (default: ON)
ENABLE_LUA=ON/OFF              # Lua scripting (default: ON)
ENABLE_BULLET_PHYSICS=ON/OFF   # Bullet Physics (default: ON)
ENABLE_PHYSX=OFF               # NVIDIA PhysX (default: OFF)
ENABLE_FMOD=OFF                # FMOD audio (default: OFF)
ENABLE_IMGUI=ON                # Dear ImGui debug UI (default: ON)
ENABLE_ASSIMP=OFF              # Advanced model loading (default: OFF)
ENABLE_ADVANCED_RENDERING=ON   # Advanced rendering features (default: ON)
ENABLE_RAY_TRACING=ON          # Ray tracing support (default: ON)
ENABLE_VR_SUPPORT=OFF          # VR support (default: OFF)
```

### Example Configurations

**Minimal build (Vulkan only, no scripting):**
```bash
cmake .. -DENABLE_PYTHON=OFF -DENABLE_LUA=OFF -DENABLE_BULLET_PHYSICS=OFF -DENABLE_IMGUI=OFF
```

**Full featured build:**
```bash
cmake .. -DENABLE_VULKAN=ON -DENABLE_OPENGL=ON -DENABLE_PYTHON=ON -DENABLE_LUA=ON
```

## Troubleshooting

### SDL2 Not Found
```
CMake Warning: SDL2 not found
```
**Solution:** Run `setup_dependencies.bat` or manually place SDL2 in `thirdparty/SDL2/`

### Vulkan Headers Not Found
```
CMake Status: Vulkan not found - Vulkan backend disabled
```
**Solution:** 
1. Run `setup_dependencies.bat` to get headers
2. For full support (validation, debugging), install [Vulkan SDK](https://vulkan.lunarg.com/)

### Missing vulkan-1.lib
```
CMake Warning: Vulkan headers found but vulkan-1.lib not found
```
**Solution:** Install full [Vulkan SDK](https://vulkan.lunarg.com/) which includes the loader library

### Build Fails with Link Errors
- Make sure you're using the correct build configuration (Debug/Release)
- Check that all dependencies are in `thirdparty/` folder
- Try cleaning and rebuilding: `rm -rf build && mkdir build && cd build && cmake ..`

### SDL2.dll Missing at Runtime (Windows)
Copy `thirdparty/SDL2/lib/x64/SDL2.dll` to the same folder as your executable (`build/bin/`)

## Development

### Using the RHI

```cpp
#include "RHI/RHIDevice.h"
#include <SDL2/SDL.h>

// Initialize SDL
SDL_Init(SDL_INIT_VIDEO);
SDL_Window* window = SDL_CreateWindow("My Game", 
    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
    1280, 720, SDL_WINDOW_VULKAN);

// Create graphics device
auto device = Nexus::RHI::RHIDevice::Create(Nexus::RHI::GraphicsAPI::Vulkan);

// Setup swap chain
Nexus::RHI::SwapChainDesc desc{};
desc.windowHandle = window;
desc.width = 1280;
desc.height = 720;
desc.bufferCount = 2;
desc.vsync = true;

device->Initialize(desc);

// Main loop
bool running = true;
while (running) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) running = false;
    }

    device->BeginFrame();
    
    auto cmdBuf = device->CreateCommandBuffer();
    cmdBuf->Begin();
    // ... rendering commands ...
    cmdBuf->End();
    
    device->SubmitCommandBuffer(cmdBuf.get());
    device->EndFrame();
    device->Present();
}

device->Shutdown();
```

### Creating Buffers and Textures

```cpp
// Vertex buffer
Nexus::RHI::BufferDesc bufferDesc{};
bufferDesc.size = sizeof(vertices);
bufferDesc.usage = Nexus::RHI::BufferUsage::Vertex;
bufferDesc.accessMode = Nexus::RHI::BufferAccessMode::Static;

auto vertexBuffer = device->CreateBuffer(bufferDesc, vertices.data());

// Texture
Nexus::RHI::TextureDesc texDesc{};
texDesc.width = 1024;
texDesc.height = 1024;
texDesc.format = Nexus::RHI::TextureFormat::RGBA8_UNORM;
texDesc.usage = Nexus::RHI::TextureUsage::ShaderResource;

auto texture = device->CreateTexture(texDesc, pixelData);
```

## Additional Documentation

- **GRAPHICS_API_SETUP.md** - Detailed graphics API setup and architecture
- **thirdparty/README.md** - Third-party dependencies overview
- **examples/** - Sample projects and demos

## Next Steps

1. ✅ **Core Vulkan backend implemented**
2. 🚧 **Complete pipeline/render pass creation**
3. 📋 **OpenGL 4.5+ backend**
4. 📋 **Refactor DirectX 11 to RHI**
5. 📋 **Shader compilation pipeline (HLSL→SPIRV)**
6. 📋 **Example projects with new RHI**

## Getting Help

- Check **GRAPHICS_API_SETUP.md** for graphics-specific issues
- Review example code in `examples/` folder
- Check CMake output for detailed configuration info

## License

See LICENSE file in repository root.

---

**Built with:** C++17, SDL2, Vulkan, Bullet Physics, Dear ImGui, Lua  
**Supported Platforms:** Windows 10/11 (primary), Linux, macOS
