@echo off
REM Nexus Engine - Build Verification Script (Windows)
REM Tests all build configurations to ensure everything works

setlocal enabledelayedexpansion

echo ==================================================
echo Nexus Engine - Build Verification
echo ==================================================
echo.

set SCRIPT_DIR=%~dp0
set ROOT_DIR=%SCRIPT_DIR%..
cd /d "%ROOT_DIR%"

set TESTS_PASSED=0
set TESTS_FAILED=0

goto :main

:run_test
    set TEST_NAME=%~1
    set BUILD_DIR=%~2
    set CMAKE_ARGS=%~3

    echo.
    echo ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
    echo Test: !TEST_NAME!
    echo ━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━

    if exist "%BUILD_DIR%" rmdir /s /q "%BUILD_DIR%"
    mkdir "%BUILD_DIR%"

    echo Configuring...
    cmake -B "%BUILD_DIR%" %CMAKE_ARGS% > "%BUILD_DIR%\cmake_output.log" 2>&1
    if !errorlevel! equ 0 (
        echo [92m✓ Configuration successful[0m
    ) else (
        echo [91m✗ Configuration failed[0m
        echo See %BUILD_DIR%\cmake_output.log for details
        set /a TESTS_FAILED+=1
        goto :eof
    )

    echo Building...
    cmake --build "%BUILD_DIR%" --config Release --parallel 4 > "%BUILD_DIR%\build_output.log" 2>&1
    if !errorlevel! equ 0 (
        echo [92m✓ Build successful[0m
        set /a TESTS_PASSED+=1
    ) else (
        echo [91m✗ Build failed[0m
        echo See %BUILD_DIR%\build_output.log for details
        set /a TESTS_FAILED+=1
    )

    goto :eof

:main

REM Test 1: Vulkan only
call :run_test "Vulkan Backend Only" "build_test_vulkan" "-DENABLE_VULKAN=ON -DENABLE_DIRECTX11=OFF -DENABLE_OPENGL=OFF"

REM Test 2: DirectX 11 only
call :run_test "DirectX 11 Backend Only" "build_test_dx11" "-DENABLE_VULKAN=OFF -DENABLE_DIRECTX11=ON -DENABLE_OPENGL=OFF"

REM Test 3: All APIs enabled
call :run_test "All APIs Enabled" "build_test_all" "-DENABLE_VULKAN=ON -DENABLE_DIRECTX11=ON -DENABLE_OPENGL=OFF"

REM Test 4: Minimal build
call :run_test "Minimal Build" "build_test_minimal" "-DENABLE_VULKAN=ON -DENABLE_PYTHON=OFF -DENABLE_LUA=OFF -DENABLE_BULLET_PHYSICS=OFF -DENABLE_IMGUI=OFF"

REM Test 5: Debug build
call :run_test "Debug Build" "build_test_debug" "-DCMAKE_BUILD_TYPE=Debug -DENABLE_VULKAN=ON"

REM Test 6: Release build
call :run_test "Release Build" "build_test_release" "-DCMAKE_BUILD_TYPE=Release -DENABLE_VULKAN=ON -DENABLE_ADVANCED_RENDERING=ON"

echo.
echo ==================================================
echo Build Verification Summary
echo ==================================================
echo.
echo Tests Passed: %TESTS_PASSED%
echo Tests Failed: %TESTS_FAILED%
echo.

if %TESTS_FAILED% equ 0 (
    echo [92m✓ All tests passed![0m
    echo.
    echo The engine is ready for development.
    echo.
    echo Recommended configuration:
    echo   mkdir build ^&^& cd build
    echo   cmake .. -DENABLE_VULKAN=ON -DENABLE_DIRECTX11=ON
    echo   cmake --build . --config Release
    exit /b 0
) else (
    echo [91m✗ Some tests failed.[0m
    echo.
    echo Check the build logs in build_test_* directories
    echo for detailed error messages.
    exit /b 1
)
