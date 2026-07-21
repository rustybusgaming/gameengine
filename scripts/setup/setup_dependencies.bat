
@echo off
echo Setting up Nexus Engine third-party dependencies...

mkdir thirdparty 2>nul
cd thirdparty

echo Downloading core dependencies...

:: Download ImGui (small, direct download)
echo - Dear ImGui...
curl -L -o imgui.zip https://github.com/ocornut/imgui/archive/refs/heads/master.zip
if exist imgui.zip (
    powershell -command "Expand-Archive -Path imgui.zip -DestinationPath . -Force"
    ren imgui-master imgui
    del imgui.zip
)

:: Download STB (header-only libraries)
echo - STB Libraries...
git clone https://github.com/nothings/stb.git 2>nul || echo STB already exists

:: Download RapidJSON
echo - RapidJSON...
curl -L -o rapidjson.zip https://github.com/Tencent/rapidjson/archive/refs/heads/master.zip
if exist rapidjson.zip (
    powershell -command "Expand-Archive -Path rapidjson.zip -DestinationPath . -Force"
    ren rapidjson-master rapidjson
    del rapidjson.zip
)

echo.
echo ✅ Basic dependencies downloaded!
echo.
echo 📝 MANUAL DOWNLOADS REQUIRED:
echo    1. Bullet Physics: https://github.com/bulletphysics/bullet3/releases
echo    2. Lua: https://www.lua.org/download.html
echo    3. FMOD: https://www.fmod.com/download
echo    4. NVIDIA PhysX: https://github.com/NVIDIA-Omniverse/PhysX
echo.
echo 📁 Extract them to the corresponding folders in thirdparty/
echo.
pause