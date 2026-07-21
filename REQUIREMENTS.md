# Nexus Game Engine - Requirements

## System Requirements

### Minimum Requirements
- **OS**: Windows 10 (64-bit) or later
- **CPU**: Intel Core i5 or AMD equivalent
- **RAM**: 8 GB
- **GPU**: DirectX 11 compatible with 2 GB VRAM
- **Storage**: 2 GB available space

### Recommended Requirements
- **OS**: Windows 11 (64-bit)
- **CPU**: Intel Core i7 or AMD Ryzen 7
- **RAM**: 16 GB or more
- **GPU**: DirectX 12 compatible with 4 GB VRAM
- **Storage**: 5 GB available space (SSD recommended)

## Development Requirements

### Build Tools
- **CMake**: 3.16 or higher
- **Visual Studio**: 2022 (with C++ Desktop Development workload)
- **Windows SDK**: 10.0.26100.0 or higher
- **C++ Standard**: C++17 or later

### Required Dependencies
- **DirectX 11/12 SDK**: For graphics rendering
- **Bullet Physics**: Physics engine (included in thirdparty/)
- **ImGui**: UI framework (included in thirdparty/)
- **STB Libraries**: Image loading (included in thirdparty/)
- **RapidJSON**: JSON parsing (included in thirdparty/)

### Optional Dependencies
- **Python 3.9+**: For Python scripting support
  - pybind11: Python bindings
- **Lua 5.4+**: For Lua scripting support
- **NVIDIA PhysX**: Alternative physics engine
- **FMOD**: Advanced audio engine
- **Assimp**: Advanced 3D model loading
- **Steam Audio**: Spatial audio

## Third-Party Libraries

The following libraries are included in the `thirdparty/` directory:
- Bullet3 (Physics)
- ImGui (UI)
- Lua (Scripting)
- PhysX (Physics - optional)
- RapidJSON (JSON)
- STB (Image loading)
- Steam Audio (Audio - optional)

## Python Environment Setup

If enabling Python scripting:

```bash
python -m pip install --upgrade pip
pip install -r requirements.txt
```

Required Python packages:
- pybind11
- numpy (optional, for advanced scripting)

## Build Configuration Options

The following CMake options can be configured:

- `ENABLE_PYTHON` - Enable Python scripting (default: ON)
- `ENABLE_LUA` - Enable Lua scripting (default: ON)
- `ENABLE_BULLET_PHYSICS` - Enable Bullet Physics (default: ON)
- `ENABLE_PHYSX` - Enable NVIDIA PhysX (default: OFF)
- `ENABLE_FMOD` - Enable FMOD audio (default: OFF)
- `ENABLE_IMGUI` - Enable ImGui UI (default: ON)
- `ENABLE_ASSIMP` - Enable Assimp model loading (default: OFF)
- `ENABLE_ADVANCED_RENDERING` - Enable advanced rendering (default: ON)
- `ENABLE_RAY_TRACING` - Enable ray tracing (default: ON)
- `ENABLE_VR_SUPPORT` - Enable VR support (default: OFF)
- `ENABLE_NETWORKING` - Enable networking (default: ON)
- `ENABLE_C_API` - Enable C API bindings (default: ON)
- `ENABLE_GAME_IMPORTERS` - Enable game importers (default: ON)

## DirectX Setup

Ensure you have the DirectX SDK installed:
- **Windows 10/11**: DirectX 11/12 is included with the OS
- **Windows SDK**: Install via Visual Studio Installer

## Verification

To verify your system meets the requirements:

```bash
# Check DirectX version
dxdiag

# Verify CMake installation
cmake --version

# Check Python (if using scripting)
python --version
```

## Additional Notes

- GPU must support Shader Model 5.0 or higher for advanced rendering features
- Ray tracing requires a DirectX 12 compatible GPU with ray tracing support
- VR support requires compatible VR headset and runtime (SteamVR, Oculus)
