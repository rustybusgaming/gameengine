# Third-Party Dependencies for Nexus Engine

This directory contains vendored third-party libraries used by the Nexus Engine.

## Directory Structure

```
thirdparty/
├── bullet3/          # Bullet Physics engine
├── imgui/            # Dear ImGui UI library
├── lua/              # Lua scripting language
├── rapidjson/        # RapidJSON parser
├── stb/              # STB single-file libraries
├── SDL2/             # SDL2 windowing library (auto-downloaded)
├── vulkan/           # Vulkan SDK headers (system SDK preferred)
└── downloads/        # Temporary download location (gitignored)
```

## Automatic Setup

Run the setup script to download required dependencies:

**Windows:**
```bash
setup_dependencies.bat
```

**Linux/macOS:**
```bash
./setup_dependencies.sh
```

## Manual Setup

### SDL2

**Option 1: Auto-download (Recommended)**
The setup script will download SDL2 development libraries automatically.

**Option 2: Manual download**
1. Download SDL2 development libraries from https://www.libsdl.org/download-2.0.php
2. Extract to `thirdparty/SDL2/` so you have:
   ```
   thirdparty/SDL2/
   ├── include/
   │   └── SDL2/
   │       └── SDL.h
   └── lib/
       ├── x64/
       │   ├── SDL2.lib
       │   ├── SDL2main.lib
       │   └── SDL2.dll
       └── x86/
   ```

### Vulkan SDK

**Option 1: System install (Recommended)**
Install the full Vulkan SDK from https://vulkan.lunarg.com/
- Includes validation layers, debugging tools, and shader compiler
- CMake will automatically detect system installation

**Option 2: Headers only**
If you only need headers (not recommended for development):
1. Download Vulkan-Headers from https://github.com/KhronosGroup/Vulkan-Headers
2. Extract to `thirdparty/vulkan/include/`

## Dependencies

### Required for Core Engine
- **SDL2** - Cross-platform windowing and input
- **Vulkan SDK** or **DirectX 11** - Graphics API

### Optional Libraries
- **Bullet3** - Physics simulation (can be disabled with `-DENABLE_BULLET_PHYSICS=OFF`)
- **Lua** - Scripting support (can be disabled with `-DENABLE_LUA=OFF`)
- **ImGui** - Debug UI (can be disabled with `-DENABLE_IMGUI=OFF`)
- **Assimp** - Advanced model loading (can be disabled with `-DENABLE_ASSIMP=OFF`)

## Building Without Dependencies

You can disable optional features in CMake:

```bash
cmake .. -DENABLE_BULLET_PHYSICS=OFF -DENABLE_LUA=OFF -DENABLE_IMGUI=OFF
```

## License Notes

Each third-party library has its own license. See individual `LICENSE` files in subdirectories.

- **SDL2**: zlib license
- **Vulkan**: Apache 2.0
- **Bullet3**: zlib license
- **Dear ImGui**: MIT license
- **Lua**: MIT license
- **RapidJSON**: MIT license
- **STB**: Public domain or MIT

## Updating Dependencies

To update to newer versions, download the updated library and replace the contents of the corresponding directory. Check the engine's compatibility before upgrading major versions.
