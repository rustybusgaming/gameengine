@echo off
REM Nexus Engine - Third-Party Dependencies Setup Script
REM This script downloads and configures SDL2 and Vulkan for the engine

setlocal enabledelayedexpansion

echo ========================================
echo Nexus Engine - Dependency Setup
echo ========================================
echo.

set SCRIPT_DIR=%~dp0
set THIRDPARTY_DIR=%SCRIPT_DIR%thirdparty
set DOWNLOADS_DIR=%THIRDPARTY_DIR%\downloads

REM Create directories
if not exist "%DOWNLOADS_DIR%" mkdir "%DOWNLOADS_DIR%"

REM ============================================
REM SDL2 Setup
REM ============================================
echo [1/3] Setting up SDL2...

set SDL2_VERSION=2.30.5
set SDL2_DIR=%THIRDPARTY_DIR%\SDL2
set SDL2_URL=https://github.com/libsdl-org/SDL/releases/download/release-%SDL2_VERSION%/SDL2-devel-%SDL2_VERSION%-VC.zip
set SDL2_ZIP=%DOWNLOADS_DIR%\SDL2-devel-%SDL2_VERSION%-VC.zip

if exist "%SDL2_DIR%\include\SDL2\SDL.h" (
    echo   SDL2 already installed at %SDL2_DIR%
) else (
    echo   Downloading SDL2 %SDL2_VERSION%...

    where curl >nul 2>&1
    if %errorlevel% equ 0 (
        curl -L -o "%SDL2_ZIP%" "%SDL2_URL%"
    ) else (
        echo   ERROR: curl not found. Please install SDL2 manually from:
        echo   %SDL2_URL%
        echo   Extract to: %SDL2_DIR%
        goto :check_vulkan
    )

    echo   Extracting SDL2...
    where tar >nul 2>&1
    if %errorlevel% equ 0 (
        tar -xf "%SDL2_ZIP%" -C "%DOWNLOADS_DIR%"
        move "%DOWNLOADS_DIR%\SDL2-%SDL2_VERSION%" "%SDL2_DIR%" >nul
        echo   SDL2 installed successfully!
    ) else (
        echo   ERROR: tar not found. Please extract manually:
        echo   From: %SDL2_ZIP%
        echo   To: %SDL2_DIR%
    )
)
echo.

REM ============================================
REM Vulkan Setup
REM ============================================
:check_vulkan
echo [2/3] Checking Vulkan SDK...

set VULKAN_DIR=%THIRDPARTY_DIR%\vulkan

REM Check if Vulkan SDK is installed system-wide
set VULKAN_SDK_FOUND=0
if defined VULKAN_SDK (
    if exist "%VULKAN_SDK%\Include\vulkan\vulkan.h" (
        echo   Vulkan SDK found at: %VULKAN_SDK%
        echo   CMake will use system Vulkan SDK
        set VULKAN_SDK_FOUND=1
    )
)

if !VULKAN_SDK_FOUND! equ 0 (
    echo   WARNING: Vulkan SDK not found in system
    echo
    echo   For full Vulkan development support, please install:
    echo   https://vulkan.lunarg.com/sdk/home#windows
    echo
    echo   Downloading Vulkan headers only (no validation layers)...

    set VULKAN_HEADERS_URL=https://github.com/KhronosGroup/Vulkan-Headers/archive/refs/tags/v1.3.290.zip
    set VULKAN_ZIP=%DOWNLOADS_DIR%\vulkan-headers.zip

    if not exist "%VULKAN_DIR%\include\vulkan\vulkan.h" (
        where curl >nul 2>&1
        if !errorlevel! equ 0 (
            curl -L -o "!VULKAN_ZIP!" "!VULKAN_HEADERS_URL!"

            where tar >nul 2>&1
            if !errorlevel! equ 0 (
                tar -xf "!VULKAN_ZIP!" -C "%DOWNLOADS_DIR%"
                if not exist "%VULKAN_DIR%" mkdir "%VULKAN_DIR%"
                move "%DOWNLOADS_DIR%\Vulkan-Headers-*\include" "%VULKAN_DIR%\include" >nul
                echo   Vulkan headers installed to thirdparty/vulkan/
            )
        ) else (
            echo   ERROR: curl not found. Cannot download Vulkan headers.
        )
    ) else (
        echo   Vulkan headers already present at %VULKAN_DIR%
    )
)
echo.

REM ============================================
REM Verify Installation
REM ============================================
echo [3/3] Verifying installation...
echo.

set ALL_OK=1

REM Check SDL2
if exist "%SDL2_DIR%\include\SDL2\SDL.h" (
    echo   [OK] SDL2 headers found
) else (
    echo   [MISSING] SDL2 headers not found
    set ALL_OK=0
)

if exist "%SDL2_DIR%\lib\x64\SDL2.lib" (
    echo   [OK] SDL2 libraries found
) else (
    echo   [MISSING] SDL2 libraries not found
    set ALL_OK=0
)

REM Check Vulkan
if defined VULKAN_SDK (
    if exist "%VULKAN_SDK%\Include\vulkan\vulkan.h" (
        echo   [OK] Vulkan SDK installed system-wide
    )
) else if exist "%VULKAN_DIR%\include\vulkan\vulkan.h" (
    echo   [OK] Vulkan headers found in thirdparty
    echo   [WARNING] For full support, install Vulkan SDK from https://vulkan.lunarg.com/
) else (
    echo   [MISSING] Vulkan not found
    echo   [ACTION REQUIRED] Install from: https://vulkan.lunarg.com/sdk/home#windows
    set ALL_OK=0
)

echo.
echo ========================================
if !ALL_OK! equ 1 (
    echo Setup complete! You can now build the engine.
    echo.
    echo Next steps:
    echo   1. mkdir build
    echo   2. cd build
    echo   3. cmake ..
    echo   4. cmake --build . --config Release
) else (
    echo Setup incomplete. Please resolve missing dependencies.
    echo See thirdparty/README.md for manual installation instructions.
)
echo ========================================

pause
