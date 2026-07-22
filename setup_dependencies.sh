#!/bin/bash
# Nexus Engine - Third-Party Dependencies Setup Script
# This script downloads and configures SDL2 and Vulkan for the engine

set -e

echo "========================================"
echo "Nexus Engine - Dependency Setup"
echo "========================================"
echo ""

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
THIRDPARTY_DIR="$SCRIPT_DIR/thirdparty"
DOWNLOADS_DIR="$THIRDPARTY_DIR/downloads"

# Create directories
mkdir -p "$DOWNLOADS_DIR"

# ============================================
# Detect OS
# ============================================
OS="unknown"
if [[ "$OSTYPE" == "linux-gnu"* ]]; then
    OS="linux"
elif [[ "$OSTYPE" == "darwin"* ]]; then
    OS="macos"
fi

echo "Detected OS: $OS"
echo ""

# ============================================
# SDL2 Setup
# ============================================
echo "[1/3] Setting up SDL2..."

SDL2_VERSION="2.30.5"
SDL2_DIR="$THIRDPARTY_DIR/SDL2"

if [ -f "$SDL2_DIR/include/SDL2/SDL.h" ]; then
    echo "  SDL2 already installed at $SDL2_DIR"
else
    if [ "$OS" == "linux" ]; then
        echo "  Checking for system SDL2..."
        if command -v pkg-config &> /dev/null; then
            if pkg-config --exists sdl2; then
                echo "  System SDL2 found, CMake will use it"
            else
                echo "  Installing SDL2 via package manager..."
                if command -v apt-get &> /dev/null; then
                    sudo apt-get update
                    sudo apt-get install -y libsdl2-dev
                elif command -v dnf &> /dev/null; then
                    sudo dnf install -y SDL2-devel
                elif command -v pacman &> /dev/null; then
                    sudo pacman -S --noconfirm sdl2
                else
                    echo "  ERROR: Unsupported package manager. Install SDL2 manually."
                fi
            fi
        fi
    elif [ "$OS" == "macos" ]; then
        echo "  Installing SDL2 via Homebrew..."
        if command -v brew &> /dev/null; then
            brew install sdl2
        else
            echo "  ERROR: Homebrew not found. Install from https://brew.sh/"
        fi
    else
        echo "  ERROR: Unsupported OS for automatic SDL2 installation"
    fi
fi
echo ""

# ============================================
# Vulkan Setup
# ============================================
echo "[2/3] Checking Vulkan SDK..."

VULKAN_DIR="$THIRDPARTY_DIR/vulkan"
VULKAN_SDK_FOUND=0

# Check for system Vulkan SDK
if [ -n "$VULKAN_SDK" ] && [ -f "$VULKAN_SDK/include/vulkan/vulkan.h" ]; then
    echo "  Vulkan SDK found at: $VULKAN_SDK"
    echo "  CMake will use system Vulkan SDK"
    VULKAN_SDK_FOUND=1
fi

if [ $VULKAN_SDK_FOUND -eq 0 ]; then
    echo "  WARNING: Vulkan SDK not found in system"
    echo ""

    if [ "$OS" == "linux" ]; then
        echo "  Installing Vulkan SDK..."
        if command -v apt-get &> /dev/null; then
            sudo apt-get update
            sudo apt-get install -y vulkan-tools libvulkan-dev vulkan-validationlayers-dev spirv-tools
        elif command -v dnf &> /dev/null; then
            sudo dnf install -y vulkan-tools vulkan-loader-devel mesa-vulkan-devel vulkan-validation-layers-devel
        elif command -v pacman &> /dev/null; then
            sudo pacman -S --noconfirm vulkan-devel vulkan-tools vulkan-validation-layers
        else
            echo "  Install Vulkan SDK from: https://vulkan.lunarg.com/"
        fi
    elif [ "$OS" == "macos" ]; then
        echo "  Installing Vulkan SDK via Homebrew..."
        if command -v brew &> /dev/null; then
            brew install vulkan-headers vulkan-loader molten-vk
        else
            echo "  Install Vulkan SDK from: https://vulkan.lunarg.com/"
        fi
    fi

    # Download headers as fallback
    if [ ! -f "$VULKAN_DIR/include/vulkan/vulkan.h" ]; then
        echo "  Downloading Vulkan headers..."
        VULKAN_HEADERS_URL="https://github.com/KhronosGroup/Vulkan-Headers/archive/refs/tags/v1.3.290.tar.gz"
        VULKAN_TAR="$DOWNLOADS_DIR/vulkan-headers.tar.gz"

        curl -L -o "$VULKAN_TAR" "$VULKAN_HEADERS_URL"
        tar -xzf "$VULKAN_TAR" -C "$DOWNLOADS_DIR"
        mkdir -p "$VULKAN_DIR"
        mv "$DOWNLOADS_DIR"/Vulkan-Headers-*/include "$VULKAN_DIR/"
        echo "  Vulkan headers installed to thirdparty/vulkan/"
    fi
fi
echo ""

# ============================================
# Verify Installation
# ============================================
echo "[3/3] Verifying installation..."
echo ""

ALL_OK=1

# Check SDL2
if [ "$OS" == "linux" ] || [ "$OS" == "macos" ]; then
    if command -v pkg-config &> /dev/null && pkg-config --exists sdl2; then
        echo "  [OK] SDL2 found via pkg-config"
    elif [ -f "$SDL2_DIR/include/SDL2/SDL.h" ]; then
        echo "  [OK] SDL2 headers found in thirdparty"
    else
        echo "  [MISSING] SDL2 not found"
        ALL_OK=0
    fi
else
    if [ -f "$SDL2_DIR/include/SDL2/SDL.h" ]; then
        echo "  [OK] SDL2 headers found"
    else
        echo "  [MISSING] SDL2 headers not found"
        ALL_OK=0
    fi
fi

# Check Vulkan
if [ -n "$VULKAN_SDK" ] && [ -f "$VULKAN_SDK/include/vulkan/vulkan.h" ]; then
    echo "  [OK] Vulkan SDK installed system-wide"
elif command -v pkg-config &> /dev/null && pkg-config --exists vulkan; then
    echo "  [OK] Vulkan found via pkg-config"
elif [ -f "$VULKAN_DIR/include/vulkan/vulkan.h" ]; then
    echo "  [OK] Vulkan headers found in thirdparty"
    echo "  [WARNING] For full support, install Vulkan SDK"
else
    echo "  [MISSING] Vulkan not found"
    echo "  [ACTION REQUIRED] Install from: https://vulkan.lunarg.com/"
    ALL_OK=0
fi

echo ""
echo "========================================"
if [ $ALL_OK -eq 1 ]; then
    echo "Setup complete! You can now build the engine."
    echo ""
    echo "Next steps:"
    echo "  1. mkdir build"
    echo "  2. cd build"
    echo "  3. cmake .."
    echo "  4. cmake --build . --config Release"
else
    echo "Setup incomplete. Please resolve missing dependencies."
    echo "See thirdparty/README.md for manual installation instructions."
fi
echo "========================================"
