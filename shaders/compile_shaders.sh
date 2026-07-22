#!/bin/bash
# Nexus Engine - Shader Compilation Script (Linux/macOS)
# Compiles shaders for all supported graphics APIs

set -e

echo "========================================"
echo "Nexus Engine - Shader Compiler"
echo "========================================"
echo ""

SHADER_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
cd "$SHADER_DIR"

# ============================================
# Compile Vulkan Shaders (GLSL -> SPIR-V)
# ============================================
echo "[1/2] Compiling Vulkan shaders..."

if command -v glslc &> /dev/null; then
    echo "  Using glslc (from Vulkan SDK)"

    if glslc triangle.vert -o triangle.vert.spv; then
        echo "  [OK] triangle.vert.spv"
    else
        echo "  [FAIL] triangle.vert compilation failed"
    fi

    if glslc triangle.frag -o triangle.frag.spv; then
        echo "  [OK] triangle.frag.spv"
    else
        echo "  [FAIL] triangle.frag compilation failed"
    fi
elif command -v glslangValidator &> /dev/null; then
    echo "  Using glslangValidator (alternative)"

    if glslangValidator -V triangle.vert -o triangle.vert.spv; then
        echo "  [OK] triangle.vert.spv"
    else
        echo "  [FAIL] triangle.vert compilation failed"
    fi

    if glslangValidator -V triangle.frag -o triangle.frag.spv; then
        echo "  [OK] triangle.frag.spv"
    else
        echo "  [FAIL] triangle.frag compilation failed"
    fi
else
    echo "  [WARNING] glslc or glslangValidator not found"
    echo "  Install Vulkan SDK: https://vulkan.lunarg.com/"
    echo "  Or install glslang: sudo apt install glslang-tools (Linux)"
    echo "                      brew install glslang (macOS)"
fi

echo ""
echo "========================================"
echo "Shader compilation complete!"
echo ""
echo "Compiled files:"
echo "  Vulkan: triangle.vert.spv, triangle.frag.spv"
echo "========================================"
