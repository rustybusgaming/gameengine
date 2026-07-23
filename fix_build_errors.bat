@echo off
REM Quick fix for compilation errors

echo Fixing build errors...

REM Fix 1: D3D11Resource.cpp bytecode_ issue
echo Fixing D3D11Resource.cpp...
powershell -Command "(Get-Content 'src\rhi\d3d11\D3D11Resource.cpp') -replace 'bytecode_', 'bytecode' | Set-Content 'src\rhi\d3d11\D3D11Resource.cpp'"

REM Fix 2: UISystem.cpp include path
echo Fixing UISystem.cpp include path...
powershell -Command "(Get-Content 'src\ui\UISystem.cpp') -replace '#include \"imgui/imgui_impl_win32.h\"', '#include \"imgui_impl_win32.h\"' | Set-Content 'src\ui\UISystem.cpp'"
powershell -Command "(Get-Content 'src\ui\UISystem.cpp') -replace '#include \"imgui/imgui_impl_dx11.h\"', '#include \"imgui_impl_dx11.h\"' | Set-Content 'src\ui\UISystem.cpp'"

REM Fix 3: EngineUI.cpp InputText issue
echo Fixing EngineUI.cpp InputText...
powershell -Command "$content = Get-Content 'src\ui\EngineUI.cpp' -Raw; $content = $content -replace 'ImGui::InputText\(\"Project\", &projectName_, ImGuiInputTextFlags_None\)', 'static char projectBuf[256]; ImGui::InputText(\"Project\", projectBuf, 256)'; Set-Content 'src\ui\EngineUI.cpp' -Value $content"

REM Fix 4: Disable UnityImporter (needs yaml-cpp)
echo Temporarily disabling UnityImporter...
if exist "src\utils\UnityImporter.cpp" (
    ren "src\utils\UnityImporter.cpp" "UnityImporter.cpp.disabled"
)

REM Fix 5: GameImporter.cpp pointer arithmetic
echo Fixing GameImporter.cpp...
powershell -Command "$content = Get-Content 'src\utils\GameImporter.cpp' -Raw; $content = $content -replace '\+ data \+ offset', '+ offset'; Set-Content 'src\utils\GameImporter.cpp' -Value $content"

echo.
echo Build errors fixed! Now try building again.
echo.
pause
