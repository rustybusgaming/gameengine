@echo off
echo === Building Nexus Asset Converter with Source Engine Support ===

if not exist "build" (
    mkdir build
    cd build
    cmake .. -G "Visual Studio 17 2022" -A x64
    cd ..
)

cmake --build build --config Release --target NexusAssetConverter

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo Build failed with error code %ERRORLEVEL%
    exit /b %ERRORLEVEL%
)

echo.
echo === Build Successful! ===
echo.
echo The asset converter with Source engine support is available at:
echo build\bin\Release\NexusAssetConverter.exe
echo.
echo Example usage:
echo NexusAssetConverter path\to\source.vmf converted\map.scene
echo NexusAssetConverter path\to\source.mdl converted\model.fbx
echo NexusAssetConverter --source-game "C:\Program Files (x86)\Steam\steamapps\common\Half-Life 2" converted_hl2\
echo.