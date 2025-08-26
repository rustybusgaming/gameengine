# Nexus Game Engine

A modern, cross-platform game engine written in C++ with Python scripting support.

## Features

- **Graphics**: DirectX 11/12 rendering with advanced lighting effects
- **Physics**: Bullet Physics integration for realistic simulations
- **Audio**: Multi-channel audio system with 3D positional audio
- **Scripting**: Python integration with hot-reloading support
- **Cross-Platform**: Windows, with console support considerations
- **Modern C++**: Uses C++17/20 features and modern best practices

### Advanced Graphics Features

- Normal mapping for enhanced surface detail
- Light bloom post-processing effects
- Heat haze distortion effects
- Unified shadowing system (shadow mapping)
- Screen-space ambient occlusion (SSAO)
- Physically-based rendering (PBR)

### Game Import Support

Import projects from:
- Unity Engine
- Unreal Engine
- Godot Engine
- Source Engine (Valve)

## Building

### Prerequisites

- CMake 3.16 or higher
- Visual Studio 2022 (Windows)
- Windows SDK 10.0.26100.0 or higher

### Third-Party Dependencies

- DirectX 11/12 SDK
- Bullet Physics
- Python 3.9+
- pybind11
- ImGui (for UI)

### Build Instructions

1. Clone the repository:
```bash
git clone https://github.com/fentbuscoding/gameengine.git
cd gameengine
```

2. Create build directory:
```bash
mkdir build
cd build
```

3. Configure with CMake:
```bash
cmake .. -G "Visual Studio 17 2022"
```

4. Build the project:
```bash
cmake --build . --config Release
```

## Usage

### Basic Example

```cpp
#include "Engine.h"

int main() {
    Nexus::Engine engine;
    
    if (!engine.Initialize()) {
        return -1;
    }
    
    engine.Run();
    return 0;
}
```

### Python Scripting

```python
import nexus_engine

# Create a simple scene
scene = nexus_engine.create_scene()
cube = nexus_engine.create_box(position=(0, 5, 0))
scene.add_object(cube)

# Run the engine
nexus_engine.run()
```

## Project Structure

```
gameengine/
├── include/          # Header files
├── src/             # Source files
│   ├── core/        # Engine core
│   ├── graphics/    # Rendering system
│   ├── physics/     # Physics simulation
│   ├── audio/       # Audio system
│   ├── scripting/   # Python integration
│   └── utils/       # Utilities
├── shaders/         # HLSL shaders
├── assets/          # Game assets
├── examples/        # Example projects
├── python/          # Python bindings
└── thirdparty/      # Third-party libraries
```

## Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

### Code Style

- Use the provided `.clang-format` configuration
- Follow the C++ Core Guidelines
- Use meaningful variable and function names
- Write comprehensive comments for complex algorithms

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Bullet Physics for collision detection and physics simulation
- ImGui for immediate mode GUI
- DirectX for graphics rendering
- Python Software Foundation for Python integration
- All contributors and open-source libraries used in this project
