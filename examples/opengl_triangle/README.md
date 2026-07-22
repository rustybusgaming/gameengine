# OpenGL Triangle Demo

A simple colored triangle demo showcasing the Nexus Engine OpenGL rendering backend.

## Features

- OpenGL 4.5+ Core Profile
- Colored triangle with vertex interpolation
- Window resizing support
- FPS counter
- Clean RHI abstraction usage

## Building

```bash
mkdir build && cd build
cmake .. -DENABLE_OPENGL=ON -DENABLE_EXAMPLES=ON
cmake --build . --config Release
```

## Running

```bash
# Windows
.\build\bin\examples\OpenGLTriangleDemo.exe

# Linux/macOS
./build/bin/examples/OpenGLTriangleDemo
```

## Requirements

- OpenGL 4.5+ compatible GPU
- GLAD loader (see thirdparty/glad/README.md)
- SDL2 (automatically downloaded by setup scripts)

## Controls

- **ESC** - Exit demo
- **Close Window** - Exit demo

## What This Demo Shows

1. **RHI Device Creation** - Creating OpenGL device via RHI factory
2. **Swap Chain Setup** - Configuring window and rendering context
3. **Vertex Buffer** - Creating and uploading vertex data
4. **Shader Compilation** - GLSL shader creation from source
5. **Graphics Pipeline** - Setting up render state
6. **Command Recording** - Recording render commands
7. **Presentation** - Presenting frames to screen

## Vertex Format

```cpp
struct Vertex {
    float position[3];  // x, y, z
    float color[3];     // r, g, b
};
```

## Shader Details

- **Vertex Shader**: GLSL 4.50 Core
  - Inputs: position (vec3), color (vec3)
  - Outputs: vColor (interpolated to fragment shader)

- **Fragment Shader**: GLSL 4.50 Core
  - Inputs: vColor (from vertex shader)
  - Outputs: FragColor (final color)

## Expected Output

A window displaying a triangle with:
- **Red** vertex at the top
- **Green** vertex at bottom-left
- **Blue** vertex at bottom-right
- Smooth color interpolation across the triangle
- Dark blue-gray background (0.1, 0.1, 0.15)

## Troubleshooting

**Black screen or crash**
- Ensure OpenGL 4.5+ drivers are installed
- Check GLAD is properly generated and compiled

**Shader compilation errors**
- Verify GLSL version support (4.50 core)
- Check shader source for syntax errors

**Window doesn't open**
- Verify SDL2 is properly installed
- Check console output for error messages

## Next Steps

After running this demo, try:
1. Modifying vertex positions
2. Changing colors
3. Adding more vertices
4. Creating indexed geometry
5. Adding uniform buffers for transformations

## Code Structure

```cpp
main() {
    1. Initialize SDL
    2. Create Window
    3. Create RHI Device (OpenGL)
    4. Setup Swap Chain
    5. Create Resources (buffers, shaders, pipeline)
    6. Main Loop:
       - Handle Events
       - Record Commands
       - Submit & Present
    7. Cleanup
}
```

## Performance Notes

- VSync is enabled by default (60 FPS cap)
- No depth testing (2D triangle)
- Minimal state changes per frame
- Efficient vertex buffer usage

## Related Examples

- `vulkan_triangle/` - Same demo using Vulkan backend
- `dx11_triangle/` - Same demo using DirectX 11 backend
- `simple_triangle/` - Cross-API triangle demo with auto-selection

## License

Part of Nexus Engine - MIT License
