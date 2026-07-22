# Nexus Engine - Triangle Demo Quick Start

**Created:** July 22, 2026 at 03:24 UTC  
**Status:** Ready to build and run!

---

## 🎯 You Now Have Everything You Need!

The triangle demo is **complete** and ready to render. Here's how to build and run it:

---

## 📋 Step 1: Compile Shaders (5 minutes)

### Windows:
```batch
cd shaders
compile_shaders.bat
```

This will create:
- `triangle.vert.spv` (Vulkan vertex shader)
- `triangle.frag.spv` (Vulkan fragment shader)  
- `triangle_vs.cso` (DirectX 11 vertex shader)
- `triangle_ps.cso` (DirectX 11 pixel shader)

### Linux/macOS:
```bash
cd shaders
chmod +x compile_shaders.sh
./compile_shaders.sh
```

**Note:** You need either:
- **Vulkan SDK** installed (includes `glslc`)
- **glslang-tools** package: `sudo apt install glslang-tools` (Linux)
- **Homebrew glslang**: `brew install glslang` (macOS)

---

## 📋 Step 2: Build the Engine (10 minutes)

### First Time Setup:
```bash
# Download SDL2 and Vulkan headers
./setup_dependencies.bat  # Windows
./setup_dependencies.sh   # Linux/macOS

# Configure CMake
mkdir build
cd build
cmake .. -DENABLE_VULKAN=ON -DENABLE_DIRECTX11=ON -DENABLE_EXAMPLES=ON

# Build
cmake --build . --config Release --parallel
```

**Expected output:**
```
-- Found SDL2 at: thirdparty/SDL2
-- Found Vulkan headers at: thirdparty/vulkan
-- Graphics APIs:
--   Vulkan: ON
--   DirectX 11: ON (Windows only)
-- Configuring done
-- Generating done
-- Build files written to: build/
```

---

## 📋 Step 3: Run the Triangle Demo

### Windows:
```batch
cd build\examples
SimpleTriangle.exe
```

### Linux/macOS:
```bash
cd build/examples
./SimpleTriangle
```

**What you should see:**
- Window opens (1280x720)
- Black/dark blue background
- RGB-colored triangle in the center:
  - Top vertex: Red
  - Bottom-right: Green
  - Bottom-left: Blue
- FPS counter in console every second

**Controls:**
- `ESC` - Exit
- `F11` - Toggle fullscreen
- Window resize should work automatically

---

## 🐛 Troubleshooting

### "Shader file not found"
**Problem:** Shaders not compiled or not in the right directory

**Solution:**
```bash
cd shaders
./compile_shaders.bat  # Windows
./compile_shaders.sh   # Linux/macOS

# Copy compiled shaders to build directory
cp *.spv ../build/examples/shaders/  # Linux/macOS
copy *.spv ..\build\examples\shaders\  # Windows
```

### "SDL2.dll not found" (Windows)
**Problem:** SDL2 DLL not in executable directory

**Solution:**
```batch
copy thirdparty\SDL2\lib\x64\SDL2.dll build\examples\
```

### "Failed to create Vulkan instance"
**Problem:** Vulkan SDK not installed or outdated drivers

**Solution:**
1. Install Vulkan SDK: https://vulkan.lunarg.com/
2. Update graphics drivers
3. Check GPU supports Vulkan 1.3: `vulkaninfo` (from SDK)

### "No graphics API available"
**Problem:** No API backend was compiled

**Solution:**
```bash
# Reconfigure with at least one API enabled
cmake .. -DENABLE_VULKAN=ON
cmake --build . --config Release
```

### Build fails with missing headers
**Problem:** Dependencies not set up

**Solution:**
```bash
./setup_dependencies.bat  # Run this first!
```

---

## 🎨 What The Shaders Do

### Vulkan Shaders (GLSL)

**triangle.vert:**
- Takes position + color per vertex
- Passes position to GPU (no transformation)
- Passes color to fragment shader

**triangle.frag:**
- Receives interpolated color from vertex shader
- Outputs color to screen

### DirectX 11 Shaders (HLSL)

**triangle_vs.hlsl:**
- Same as Vulkan vertex shader
- HLSL syntax instead of GLSL

**triangle_ps.hlsl:**
- Same as Vulkan fragment shader
- HLSL syntax instead of GLSL

---

## 📊 Expected Console Output

```
=== Nexus Engine - Simple Triangle Demo ===
Initializing Graphics Device...
Creating graphics device...
Using API: Vulkan
Graphics device initialized successfully
Vertex buffer created
Shaders loaded successfully
Graphics pipeline created
Entering main loop...
FPS: 1438
FPS: 1442
FPS: 1445
...
^C (Ctrl+C to exit)
Shutting down...
=== Demo Complete ===
```

---

## 🚀 Next Steps

Once the triangle is rendering:

1. **Play with the code:**
   - Change vertex positions in `SimpleTriangle.cpp`
   - Modify colors
   - Try different primitive topologies

2. **Add camera movement:**
   - Create a view matrix
   - Add uniform buffer for camera
   - Implement FPS controls

3. **Load a 3D model:**
   - Add index buffer
   - Load OBJ/FBX file
   - Render a cube or sphere

4. **Add textures:**
   - Load image file
   - Create texture sampler
   - Update shaders to sample texture

---

## 📁 File Locations

```
gameengine/
├── shaders/
│   ├── triangle.vert          ← Vulkan vertex shader (GLSL)
│   ├── triangle.frag          ← Vulkan fragment shader (GLSL)
│   ├── triangle_vs.hlsl       ← DirectX vertex shader (HLSL)
│   ├── triangle_ps.hlsl       ← DirectX pixel shader (HLSL)
│   ├── compile_shaders.bat   ← Shader compiler (Windows)
│   └── compile_shaders.sh    ← Shader compiler (Linux/macOS)
├── examples/
│   └── simple_triangle/
│       ├── CMakeLists.txt
│       └── SimpleTriangle.cpp ← Main demo code
└── build/
    └── examples/
        ├── SimpleTriangle.exe ← Executable
        └── shaders/           ← Copy compiled shaders here
```

---

## ✅ Success Checklist

- [ ] Shaders compiled without errors
- [ ] Engine builds successfully
- [ ] SimpleTriangle executable runs
- [ ] Window opens
- [ ] Triangle renders with RGB colors
- [ ] FPS counter shows in console
- [ ] No error messages
- [ ] Can resize window
- [ ] ESC key exits cleanly

---

## 🎉 You're Done!

If you see a colored triangle, **congratulations!** Your multi-API graphics engine is working! 🎮

The engine now has:
- ✅ Working Vulkan backend
- ✅ Working DirectX 11 backend (Windows)
- ✅ Complete RHI abstraction
- ✅ Descriptor set management
- ✅ Uniform buffer support
- ✅ Pipeline state objects
- ✅ Shader loading
- ✅ Visual proof it works!

---

## 📞 Need Help?

If something doesn't work:

1. Check error messages in console
2. Review this guide's troubleshooting section
3. Verify all dependencies are installed
4. Check GPU supports Vulkan/DirectX 11
5. Try `verify_build.bat` to test all configurations

---

**Built with ❤️ by the Nexus Engine Team**  
*Happy rendering! 🚀*
