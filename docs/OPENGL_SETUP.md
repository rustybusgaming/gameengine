# OpenGL Backend Setup Guide

This guide explains how to set up and use the OpenGL 4.5+ rendering backend for Nexus Engine.

## Prerequisites

- **OpenGL 4.5+** compatible graphics driver
- **GLAD** OpenGL loader library
- **SDL2** for window management (already included)

## Platform Support

| Platform | Support | Notes |
|----------|---------|-------|
| Windows  | ✅ Full  | OpenGL 4.5+ via GPU drivers |
| Linux    | ✅ Full  | Mesa 20+ or proprietary drivers |
| macOS    | ⚠️ Limited | OpenGL 4.1 max (deprecated) |

## Setting Up GLAD

GLAD is required for loading OpenGL functions. You need to generate it once:

### Method 1: Web Generator (Easiest)

1. Visit https://glad.dav1d.de/
2. Configure:
   - **Language**: C/C++
   - **Specification**: OpenGL
   - **API**: gl Version 4.5 (or 4.6 for latest)
   - **Profile**: Core
   - ✅ Check "Generate a loader"
3. Click **GENERATE**
4. Download the zip file
5. Extract and copy:
   ```
   include/glad/ → thirdparty/glad/include/glad/
   include/KHR/  → thirdparty/glad/include/KHR/
   src/glad.c    → thirdparty/glad/src/glad.c
   ```

### Method 2: Python Generator

```bash
pip install glad
cd thirdparty/glad
glad --profile=core --api=gl=4.5 --generator=c --spec=gl --out-path=.
```

### Method 3: Use Provided Script

```bash
cd scripts/setup
python download_glad.py  # Shows manual setup instructions
```

## Building with OpenGL

### CMake Configuration

```bash
mkdir build && cd build
cmake .. -DENABLE_OPENGL=ON -DENABLE_VULKAN=OFF -DENABLE_DIRECTX11=OFF
cmake --build . --config Release
```

### Enable All APIs

```bash
cmake .. -DENABLE_OPENGL=ON -DENABLE_VULKAN=ON -DENABLE_DIRECTX11=ON
cmake --build . --config Release
```

## Using OpenGL Backend

### Explicit Selection

```cpp
#include "RHI/RHIDevice.h"
#include <SDL2/SDL.h>

int main() {
    SDL_Init(SDL_INIT_VIDEO);
    
    // Create OpenGL-compatible window
    SDL_Window* window = SDL_CreateWindow("OpenGL Demo",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        1280, 720, 
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    
    // Create OpenGL device explicitly
    auto device = Nexus::RHI::RHIDevice::Create(Nexus::RHI::GraphicsAPI::OpenGL);
    
    Nexus::RHI::SwapChainDesc swapChainDesc{};
    swapChainDesc.windowHandle = window;
    swapChainDesc.width = 1280;
    swapChainDesc.height = 720;
    swapChainDesc.bufferCount = 2;
    swapChainDesc.vsync = true;
    
    if (!device->Initialize(swapChainDesc)) {
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error",
            "Failed to initialize OpenGL device", nullptr);
        return -1;
    }
    
    // Rendering loop
    bool running = true;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
        }
        
        device->BeginFrame();
        
        auto cmdBuf = device->CreateCommandBuffer();
        cmdBuf->Begin();
        
        // Clear screen
        Nexus::RHI::ClearColor clearColor{0.1f, 0.1f, 0.15f, 1.0f};
        cmdBuf->ClearRenderTarget(device->GetBackBuffer(), clearColor);
        
        // Your rendering commands here
        
        cmdBuf->End();
        device->SubmitCommandBuffer(cmdBuf.get());
        
        device->EndFrame();
        device->Present();
    }
    
    device->Shutdown();
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
```

### Auto-Selection

If `ENABLE_OPENGL=ON` and OpenGL has priority in CMake, it will be auto-selected:

```cpp
// Will choose OpenGL if it's the highest priority enabled API
auto device = Nexus::RHI::RHIDevice::Create(Nexus::RHI::GraphicsAPI::Auto);
```

## API Priority Order

When using `GraphicsAPI::Auto`, the engine selects in this order:

1. **Vulkan** (if enabled) - Best performance, most features
2. **DirectX 11** (if enabled, Windows only) - Good compatibility
3. **OpenGL** (if enabled) - Maximum portability

Change the default in `CMakeLists.txt`:

```cmake
set(NEXUS_DEFAULT_GRAPHICS_API "OpenGL" CACHE STRING "Default graphics API")
```

## Features Support

| Feature | OpenGL Support | Notes |
|---------|---------------|-------|
| Vertex/Index Buffers | ✅ | Full support |
| Uniform Buffers | ✅ | UBO support |
| Textures (2D/3D/Array) | ✅ | All formats |
| Framebuffers | ✅ | Multiple attachments |
| Depth/Stencil | ✅ | All formats |
| Blending | ✅ | All blend modes |
| Rasterizer State | ✅ | Wireframe, culling, etc. |
| Instanced Drawing | ✅ | Full support |
| Compute Shaders | ✅ | OpenGL 4.3+ |
| Tessellation | ⏳ | Planned |
| Geometry Shaders | ✅ | Full support |
| MSAA | ⏳ | Planned |

## Shader Compilation

OpenGL uses GLSL shaders (not SPIR-V or HLSL):

```cpp
const char* vertexShader = R"(
#version 450 core
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aColor;

out vec3 vColor;

void main() {
    gl_Position = vec4(aPosition, 1.0);
    vColor = aColor;
}
)";

const char* fragmentShader = R"(
#version 450 core
in vec3 vColor;
out vec4 FragColor;

void main() {
    FragColor = vec4(vColor, 1.0);
}
)";

// Create shaders from source
auto vs = device->CreateShaderFromSource(
    Nexus::RHI::ShaderStage::Vertex, vertexShader);
auto fs = device->CreateShaderFromSource(
    Nexus::RHI::ShaderStage::Fragment, fragmentShader);
```

## Debugging

### Enable OpenGL Debug Output

In debug builds, OpenGL debug context is automatically enabled:

```cmake
cmake .. -DCMAKE_BUILD_TYPE=Debug -DENABLE_OPENGL=ON
```

### Check OpenGL Version

The engine logs OpenGL information on startup:

```
[INFO] Initializing OpenGL Device...
[INFO] OpenGL Vendor: NVIDIA Corporation
[INFO] OpenGL Renderer: NVIDIA GeForce RTX 3080
[INFO] OpenGL Version: 4.6.0 NVIDIA 511.79
[INFO] OpenGL Device initialized successfully
```

### Common Issues

**Issue**: "Failed to load OpenGL functions with GLAD"
- **Solution**: Ensure GLAD files are properly generated and in `thirdparty/glad/`

**Issue**: "OpenGL 4.5+ required, but got 4.1"
- **Solution**: Update graphics drivers or use compatibility profile (macOS limitation)

**Issue**: "Shader compilation failed"
- **Solution**: Check shader syntax - OpenGL uses GLSL, not HLSL

**Issue**: Linking errors with glad symbols
- **Solution**: Ensure `glad.c` is compiled and linked (CMakeLists.txt should handle this)

## Performance Tips

1. **Minimize State Changes**: OpenGL is state-machine based, group draw calls with same pipeline
2. **Use Instancing**: For repeated objects, use `DrawInstanced`
3. **Persistent Mapping**: Use `GL_MAP_PERSISTENT_BIT` for frequently updated buffers
4. **DSA (Direct State Access)**: OpenGL 4.5+ DSA functions are more efficient
5. **Reduce glGet* Calls**: Cache state on CPU side when possible

## Limitations

- **No SPIR-V Support**: Must use GLSL source shaders (SPIR-V in OpenGL requires 4.6+)
- **No Ray Tracing**: Ray tracing requires Vulkan or DX12
- **macOS Deprecated**: Apple deprecated OpenGL in favor of Metal
- **Vertex Layout**: Current implementation uses simplified vertex attribute setup

## Next Steps

1. ✅ Complete GLAD setup (see above)
2. ✅ Build with OpenGL enabled
3. ⏳ Run example demos (see `examples/` folder)
4. ⏳ Write GLSL shaders
5. ⏳ Test on your hardware

## Resources

- [OpenGL Wiki](https://www.khronos.org/opengl/wiki/)
- [GLAD Generator](https://glad.dav1d.de/)
- [OpenGL 4.5 Reference](https://registry.khronos.org/OpenGL-Refpages/gl4/)
- [Learn OpenGL](https://learnopengl.com/)

## Support

For issues specific to Nexus Engine's OpenGL backend:
- Check `src/rhi/opengl/` for implementation
- Review RHI abstraction in `include/RHI/`
- Open an issue on GitHub with system info and logs
