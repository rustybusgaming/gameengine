#!/bin/bash
# Nexus Engine - Build Verification Script
# Tests all build configurations to ensure everything works

set -e

echo "=================================================="
echo "Nexus Engine - Build Verification"
echo "=================================================="
echo ""

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
ROOT_DIR="$SCRIPT_DIR/.."

cd "$ROOT_DIR"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Test counter
TESTS_PASSED=0
TESTS_FAILED=0

run_test() {
    local test_name="$1"
    local build_dir="$2"
    shift 2
    local cmake_args="$@"

    echo ""
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    echo "Test: $test_name"
    echo "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"

    if [ -d "$build_dir" ]; then
        rm -rf "$build_dir"
    fi
    mkdir -p "$build_dir"

    echo "Configuring..."
    if cmake -B "$build_dir" $cmake_args > "$build_dir/cmake_output.log" 2>&1; then
        echo -e "${GREEN}✓ Configuration successful${NC}"
    else
        echo -e "${RED}✗ Configuration failed${NC}"
        echo "See $build_dir/cmake_output.log for details"
        ((TESTS_FAILED++))
        return 1
    fi

    echo "Building..."
    if cmake --build "$build_dir" --config Release -j 4 > "$build_dir/build_output.log" 2>&1; then
        echo -e "${GREEN}✓ Build successful${NC}"
        ((TESTS_PASSED++))
        return 0
    else
        echo -e "${RED}✗ Build failed${NC}"
        echo "See $build_dir/build_output.log for details"
        ((TESTS_FAILED++))
        return 1
    fi
}

# Test 1: Vulkan only
run_test "Vulkan Backend Only" \
    "build_test_vulkan" \
    -DENABLE_VULKAN=ON -DENABLE_DIRECTX11=OFF -DENABLE_OPENGL=OFF

# Test 2: DirectX 11 only (Windows)
if [[ "$OSTYPE" == "msys" || "$OSTYPE" == "win32" ]]; then
    run_test "DirectX 11 Backend Only" \
        "build_test_dx11" \
        -DENABLE_VULKAN=OFF -DENABLE_DIRECTX11=ON -DENABLE_OPENGL=OFF
fi

# Test 3: All APIs enabled
run_test "All APIs Enabled" \
    "build_test_all" \
    -DENABLE_VULKAN=ON -DENABLE_DIRECTX11=ON -DENABLE_OPENGL=OFF

# Test 4: Minimal build (no optional features)
run_test "Minimal Build" \
    "build_test_minimal" \
    -DENABLE_VULKAN=ON \
    -DENABLE_PYTHON=OFF \
    -DENABLE_LUA=OFF \
    -DENABLE_BULLET_PHYSICS=OFF \
    -DENABLE_IMGUI=OFF

# Test 5: Debug build
run_test "Debug Build" \
    "build_test_debug" \
    -DCMAKE_BUILD_TYPE=Debug \
    -DENABLE_VULKAN=ON

# Test 6: Release with optimizations
run_test "Release Build" \
    "build_test_release" \
    -DCMAKE_BUILD_TYPE=Release \
    -DENABLE_VULKAN=ON \
    -DENABLE_ADVANCED_RENDERING=ON

echo ""
echo "=================================================="
echo "Build Verification Summary"
echo "=================================================="
echo ""
echo -e "Tests Passed: ${GREEN}$TESTS_PASSED${NC}"
echo -e "Tests Failed: ${RED}$TESTS_FAILED${NC}"
echo ""

if [ $TESTS_FAILED -eq 0 ]; then
    echo -e "${GREEN}✓ All tests passed!${NC}"
    echo ""
    echo "The engine is ready for development."
    echo ""
    echo "Recommended configuration:"
    echo "  mkdir build && cd build"
    echo "  cmake .. -DENABLE_VULKAN=ON -DENABLE_DIRECTX11=ON"
    echo "  cmake --build . --config Release"
    exit 0
else
    echo -e "${RED}✗ Some tests failed.${NC}"
    echo ""
    echo "Check the build logs in build_test_* directories"
    echo "for detailed error messages."
    exit 1
fi
