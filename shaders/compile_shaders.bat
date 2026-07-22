@echo off
REM Nexus Engine - Shader Compilation Script
REM Compiles shaders for all supported graphics APIs

echo ========================================
echo Nexus Engine - Shader Compiler
echo ========================================
echo.

set SHADER_DIR=%~dp0
cd /d "%SHADER_DIR%"

REM ============================================
REM Compile Vulkan Shaders (GLSL -> SPIR-V)
REM ============================================
echo [1/2] Compiling Vulkan shaders...

where glslc >nul 2>&1
if %errorlevel% equ 0 (
    echo   Using glslc ^(from Vulkan SDK^)

    glslc triangle.vert -o triangle.vert.spv
    if %errorlevel% equ 0 (
        echo   [OK] triangle.vert.spv
    ) else (
        echo   [FAIL] triangle.vert compilation failed
    )

    glslc triangle.frag -o triangle.frag.spv
    if %errorlevel% equ 0 (
        echo   [OK] triangle.frag.spv
    ) else (
        echo   [FAIL] triangle.frag compilation failed
    )
) else (
    echo   [WARNING] glslc not found in PATH
    echo   Install Vulkan SDK from https://vulkan.lunarg.com/
    echo   Or run this from Vulkan SDK command prompt
)

echo.

REM ============================================
REM Compile DirectX 11 Shaders (HLSL -> DXBC)
REM ============================================
echo [2/2] Compiling DirectX 11 shaders...

where fxc >nul 2>&1
if %errorlevel% equ 0 (
    echo   Using fxc ^(from Windows SDK^)

    fxc /T vs_5_0 /E main /Fo triangle_vs.cso triangle_vs.hlsl
    if %errorlevel% equ 0 (
        echo   [OK] triangle_vs.cso
    ) else (
        echo   [FAIL] triangle_vs.hlsl compilation failed
    )

    fxc /T ps_5_0 /E main /Fo triangle_ps.cso triangle_ps.hlsl
    if %errorlevel% equ 0 (
        echo   [OK] triangle_ps.cso
    ) else (
        echo   [FAIL] triangle_ps.hlsl compilation failed
    )
) else (
    echo   [WARNING] fxc not found in PATH
    echo   fxc is part of Windows SDK
    echo   Add to PATH or run from Developer Command Prompt
)

echo.
echo ========================================
echo Shader compilation complete!
echo.
echo Compiled files:
echo   Vulkan:     triangle.vert.spv, triangle.frag.spv
echo   DirectX 11: triangle_vs.cso, triangle_ps.cso
echo ========================================

pause
