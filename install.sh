#!/bin/bash
# Nexus Engine - Complete Build and Install Script
# This script sets up all dependencies and builds the engine

set -e

echo "========================================"
echo "Nexus Engine - Complete Installation"
echo "========================================"
echo ""

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
THIRDPARTY_DIR="${SCRIPT_DIR}/thirdparty"
BUILD_DIR="${SCRIPT_DIR}/build"
DOWNLOADS_DIR="${THIRDPARTY_DIR}/downloads"

# Detect OS
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    OS="linux"
elif [[ "$OSTYPE" == "darwin"* ]]; then
    OS="macos"
else
    echo "Unsupported OS: $OSTYPE"
    exit 1
fi

# ============================================
# Check for required tools
# ============================================
echo "[Step 1/6] Checking build tools..."
echo ""

MISSING_TOOLS=0

if ! command -v cmake &> /dev/null; then
    echo "  [MISSING] CMake not found"
    echo "  Install: sudo apt install cmake (Ubuntu) or brew install cmake (macOS)"
    MISSING_TOOLS=1
else
    CMAKE_VERSION=$(cmake --version | head -n1 | awk '{print $3}')
    echo "  [OK] CMake $CMAKE_VERSION found"
fi

if ! command -v ninja &> /dev/null; then
    echo "  [WARNING] Ninja not found - will use Make"
    USE_NINJA=0
else
    echo "  [OK] Ninja build system found"
    USE_NINJA=1
fi

if ! command -v g++ &> /dev/null && ! command -v clang++ &> /dev/null; then
    echo "  [MISSING] No C++ compiler found (g++ or clang++)"
    echo "  Install: sudo apt install build-essential (Ubuntu) or xcode-select --install (macOS)"
    MISSING_TOOLS=1
else
    if command -v g++ &> /dev/null; then
        GCC_VERSION=$(g++ --version | head -n1 | awk '{print $3}')
        echo "  [OK] GCC $GCC_VERSION found"
    else
        CLANG_VERSION=$(clang++ --version | head -n1 | awk '{print $3}')
        echo "  [OK] Clang $CLANG_VERSION found"
    fi
fi

if [ $MISSING_TOOLS -eq 1 ]; then
    echo ""
    echo "ERROR: Required build tools are missing."
    echo "Please install the missing tools and try again."
    exit 1
fi

echo ""

# Create directories
mkdir -p "$DOWNLOADS_DIR"
mkdir -p "$BUILD_DIR"

# ============================================
# Install SDL2
# ============================================
echo "[Step 2/6] Installing SDL2..."
echo ""

if [ "$OS" == "linux" ]; then
    if dpkg -l | grep -q libsdl2-dev; then
        echo "  SDL2 development package already installed"
    else
        echo "  Installing SDL2 via apt..."
        sudo apt-get update
        sudo apt-get install -y libsdl2-dev
    fi
elif [ "$OS" == "macos" ]; then
    if brew list sdl2 &> /dev/null; then
        echo "  SDL2 already installed via Homebrew"
    else
        echo "  Installing SDL2 via Homebrew..."
        brew install sdl2
    fi
fi

echo ""

# ============================================
# Install Vulkan
# ============================================
echo "[Step 3/6] Installing Vulkan..."
echo ""

if [ "$OS" == "linux" ]; then
    if dpkg -l | grep -q libvulkan-dev; then
        echo "  Vulkan development package already installed"
    else
        echo "  Installing Vulkan via apt..."
        sudo apt-get install -y libvulkan-dev vulkan-tools
    fi
elif [ "$OS" == "macos" ]; then
    if [ -d "$HOME/VulkanSDK" ]; then
        echo "  Vulkan SDK found in $HOME/VulkanSDK"
    else
        echo "  [WARNING] Vulkan SDK not found"
        echo "  Download from: https://vulkan.lunarg.com/sdk/home"
    fi
fi

echo ""

# ============================================
# Install Python (optional)
# ============================================
echo "[Step 4/6] Checking Python..."
echo ""

if command -v python3 &> /dev/null; then
    PYTHON_VERSION=$(python3 --version | awk '{print $2}')
    echo "  [OK] Python $PYTHON_VERSION found"
else
    echo "  [WARNING] Python3 not found - scripting support will be disabled"
    if [ "$OS" == "linux" ]; then
        echo "  Install: sudo apt install python3 python3-dev"
    elif [ "$OS" == "macos" ]; then
        echo "  Install: brew install python3"
    fi
fi

echo ""

# ============================================
# Verify All Dependencies
# ============================================
echo "[Step 5/6] Verifying dependencies..."
echo ""

DEPS_OK=1

[ -f "$THIRDPARTY_DIR/imgui/imgui.h" ] && echo "  [OK] ImGui found" || { echo "  [MISSING] ImGui"; DEPS_OK=0; }
[ -f "$THIRDPARTY_DIR/stb/stb_image.h" ] && echo "  [OK] STB libraries found" || { echo "  [MISSING] STB libraries"; DEPS_OK=0; }
[ -f "$THIRDPARTY_DIR/rapidjson/include/rapidjson/rapidjson.h" ] && echo "  [OK] RapidJSON found" || { echo "  [MISSING] RapidJSON"; DEPS_OK=0; }
[ -f "$THIRDPARTY_DIR/joltphysics/Build/CMakeLists.txt" ] && echo "  [OK] JoltPhysics found" || { echo "  [MISSING] JoltPhysics"; DEPS_OK=0; }
[ -f "$THIRDPARTY_DIR/glad/include/glad/glad.h" ] && echo "  [OK] GLAD found" || { echo "  [MISSING] GLAD"; DEPS_OK=0; }

if [ $DEPS_OK -eq 0 ]; then
    echo ""
    echo "ERROR: Some dependencies are missing!"
    echo "Please check thirdparty/README.md for setup instructions."
    exit 1
fi

echo ""

# ============================================
# Configure and Build
# ============================================
echo "[Step 6/6] Building Nexus Engine..."
echo ""

cd "$BUILD_DIR"

echo "  Configuring CMake..."
if [ $USE_NINJA -eq 1 ]; then
    cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Release ..
else
    cmake -DCMAKE_BUILD_TYPE=Release ..
fi

echo ""
echo "  Building (this may take several minutes)..."
cmake --build . --config Release -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

echo ""
echo "  Installing to build/bin..."
cmake --install . --config Release --prefix "$BUILD_DIR/install"

cd "$SCRIPT_DIR"

echo ""
echo "========================================"
echo "Installation Complete!"
echo "========================================"
echo ""
echo "Build output: $BUILD_DIR"
echo ""
echo "To run the engine:"
echo "  cd build/bin"
echo "  ./NexusEngine"
echo ""
echo "========================================"
