@echo off
REM Nexus Engine - Complete Build and Install Script
REM This script sets up all dependencies and builds the engine

setlocal enabledelayedexpansion

echo ========================================
echo Nexus Engine - Complete Installation
echo ========================================
echo.

set SCRIPT_DIR=%~dp0
set THIRDPARTY_DIR=%SCRIPT_DIR%thirdparty
set BUILD_DIR=%SCRIPT_DIR%build
set DOWNLOADS_DIR=%THIRDPARTY_DIR%\downloads

REM Check for required tools
echo [Step 1/6] Checking build tools...
echo.

set MISSING_TOOLS=0

where cmake >nul 2>&1
if %errorlevel% neq 0 (
    echo   [MISSING] CMake not found
    echo   Download from: https://cmake.org/download/
    set MISSING_TOOLS=1
) else (
    for /f "tokens=3" %%i in ('cmake --version ^| findstr /C:"cmake version"') do set CMAKE_VERSION=%%i
    echo   [OK] CMake !CMAKE_VERSION! found
)

where ninja >nul 2>&1
if %errorlevel% neq 0 (
    echo   [WARNING] Ninja not found - will use default generator
    set USE_NINJA=0
) else (
    echo   [OK] Ninja build system found
    set USE_NINJA=1
)

where cl >nul 2>&1
if %errorlevel% neq 0 (
    where g++ >nul 2>&1
    if %errorlevel% neq 0 (
        echo   [MISSING] No C++ compiler found (MSVC or MinGW)
        echo   Install Visual Studio 2019+ or MinGW-w64
        set MISSING_TOOLS=1
    ) else (
        for /f "tokens=3" %%i in ('g++ --version ^| findstr /C:"g++"') do set GCC_VERSION=%%i
        echo   [OK] GCC !GCC_VERSION! found
        set COMPILER=MinGW
    )
) else (
    for /f "tokens=*" %%i in ('cl 2^>^&1 ^| findstr /C:"Version"') do set MSVC_VERSION=%%i
    echo   [OK] MSVC found: !MSVC_VERSION!
    set COMPILER=MSVC
)

if !MISSING_TOOLS! equ 1 (
    echo.
    echo ERROR: Required build tools are missing.
    echo Please install the missing tools and try again.
    pause
    exit /b 1
)

echo.

REM Create directories
if not exist "%DOWNLOADS_DIR%" mkdir "%DOWNLOADS_DIR%"
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"

REM ============================================
REM Install SDL2
REM ============================================
echo [Step 2/6] Installing SDL2...
echo.

set SDL2_VERSION=2.30.5
set SDL2_DIR=%THIRDPARTY_DIR%\SDL2
set SDL2_URL=https://github.com/libsdl-org/SDL/releases/download/release-%SDL2_VERSION%/SDL2-devel-%SDL2_VERSION%-VC.zip
set SDL2_ZIP=%DOWNLOADS_DIR%\SDL2-devel-%SDL2_VERSION%-VC.zip

if exist "%SDL2_DIR%\include\SDL.h" (
    echo   SDL2 already installed
) else (
    echo   Downloading SDL2 %SDL2_VERSION%...
    curl -L -o "%SDL2_ZIP%" "%SDL2_URL%"

    if exist "%SDL2_ZIP%" (
        echo   Extracting SDL2...
        tar -xf "%SDL2_ZIP%" -C "%DOWNLOADS_DIR%"
        move "%DOWNLOADS_DIR%\SDL2-%SDL2_VERSION%" "%SDL2_DIR%" >nul 2>&1
        echo   SDL2 installed successfully!
    ) else (
        echo   ERROR: Failed to download SDL2
    )
)

echo.

REM ============================================
REM Install Vulkan Headers
REM ============================================
echo [Step 3/6] Installing Vulkan headers...
echo.

set VULKAN_DIR=%THIRDPARTY_DIR%\vulkan

if defined VULKAN_SDK (
    if exist "%VULKAN_SDK%\Include\vulkan\vulkan.h" (
        echo   Using system Vulkan SDK: %VULKAN_SDK%
        goto skip_vulkan_headers
    )
)

if exist "%VULKAN_DIR%\include\vulkan\vulkan.h" (
    echo   Vulkan headers already installed
) else (
    echo   Downloading Vulkan headers...
    set VULKAN_HEADERS_URL=https://github.com/KhronosGroup/Vulkan-Headers/archive/refs/tags/v1.3.290.zip
    set VULKAN_ZIP=%DOWNLOADS_DIR%\vulkan-headers.zip

    curl -L -o "!VULKAN_ZIP!" "!VULKAN_HEADERS_URL!"

    if exist "!VULKAN_ZIP!" (
        echo   Extracting Vulkan headers...
        tar -xf "!VULKAN_ZIP!" -C "%DOWNLOADS_DIR%"
        if not exist "%VULKAN_DIR%" mkdir "%VULKAN_DIR%"
        for /d %%i in ("%DOWNLOADS_DIR%\Vulkan-Headers-*") do (
            move "%%i\include" "%VULKAN_DIR%\include" >nul 2>&1
        )
        echo   Vulkan headers installed successfully!
    ) else (
        echo   ERROR: Failed to download Vulkan headers
    )
)

:skip_vulkan_headers
echo.

REM ============================================
REM Install Python (optional)
REM ============================================
echo [Step 4/6] Checking Python...
echo.

where python >nul 2>&1
if %errorlevel% equ 0 (
    for /f "tokens=2" %%i in ('python --version 2^>^&1') do set PYTHON_VERSION=%%i
    echo   [OK] Python !PYTHON_VERSION! found
) else (
    echo   [WARNING] Python not found - scripting support will be disabled
    echo   Install from: https://www.python.org/downloads/
)

echo.

REM ============================================
REM Verify All Dependencies
REM ============================================
echo [Step 5/6] Verifying dependencies...
echo.

set DEPS_OK=1

if exist "%THIRDPARTY_DIR%\imgui\imgui.h" (
    echo   [OK] ImGui found
) else (
    echo   [MISSING] ImGui not found in thirdparty/imgui
    set DEPS_OK=0
)

if exist "%THIRDPARTY_DIR%\stb\stb_image.h" (
    echo   [OK] STB libraries found
) else (
    echo   [MISSING] STB libraries not found in thirdparty/stb
    set DEPS_OK=0
)

if exist "%THIRDPARTY_DIR%\rapidjson\include\rapidjson\rapidjson.h" (
    echo   [OK] RapidJSON found
) else (
    echo   [MISSING] RapidJSON not found in thirdparty/rapidjson
    set DEPS_OK=0
)

if exist "%THIRDPARTY_DIR%\joltphysics\Build\CMakeLists.txt" (
    echo   [OK] JoltPhysics found
) else (
    echo   [MISSING] JoltPhysics not found in thirdparty/joltphysics
    set DEPS_OK=0
)

if exist "%THIRDPARTY_DIR%\glad\include\glad\glad.h" (
    echo   [OK] GLAD found
) else (
    echo   [MISSING] GLAD not found in thirdparty/glad
    set DEPS_OK=0
)

if exist "%SDL2_DIR%\include\SDL.h" (
    echo   [OK] SDL2 found
) else (
    echo   [MISSING] SDL2 not found
    set DEPS_OK=0
)

if !DEPS_OK! equ 0 (
    echo.
    echo ERROR: Some dependencies are missing!
    echo Please check thirdparty/README.md for setup instructions.
    pause
    exit /b 1
)

echo.

REM ============================================
REM Configure and Build
REM ============================================
echo [Step 6/6] Building Nexus Engine...
echo.

cd "%BUILD_DIR%"

echo   Configuring CMake...
if !USE_NINJA! equ 1 (
    cmake -G "Ninja" -DCMAKE_BUILD_TYPE=Release ..
) else if "!COMPILER!"=="MSVC" (
    cmake -G "Visual Studio 17 2022" -A x64 ..
) else (
    cmake -G "MinGW Makefiles" -DCMAKE_BUILD_TYPE=Release ..
)

if %errorlevel% neq 0 (
    echo.
    echo ERROR: CMake configuration failed!
    cd "%SCRIPT_DIR%"
    pause
    exit /b 1
)

echo.
echo   Building (this may take several minutes)...
cmake --build . --config Release -j 8

if %errorlevel% neq 0 (
    echo.
    echo ERROR: Build failed!
    cd "%SCRIPT_DIR%"
    pause
    exit /b 1
)

echo.
echo   Installing to build/bin...
cmake --install . --config Release --prefix "%BUILD_DIR%\install"

cd "%SCRIPT_DIR%"

echo.
echo ========================================
echo Installation Complete!
echo ========================================
echo.
echo Build output: %BUILD_DIR%
echo.
echo To run the engine:
echo   cd build\bin
echo   NexusEngine.exe
echo.
echo ========================================

if not defined CI pause
