@echo off
echo ========================================
echo NSys Project Setup Script
echo ========================================
echo.

echo Checking Git installation...
git --version >nul 2>&1
if errorlevel 1 (
    echo ERROR: Git is not installed or not in PATH
    echo Please install Git from https://git-scm.com/
    pause
    exit /b 1
)
echo Git is installed.
echo.

echo Checking if this is a Git repository...
if not exist ".git" (
    echo ERROR: This directory is not a Git repository
    echo Please run this script from the root of the NSys project
    pause
    exit /b 1
)
echo This is a Git repository.
echo.

echo Checking submodule status...
git submodule status >nul 2>&1
if errorlevel 1 (
    echo No submodules found. Initializing submodules...
    git submodule init
    git submodule update --recursive
) else (
    echo Submodules found. Checking if they are properly initialized...
    git submodule status | findstr "^-" >nul
    if not errorlevel 1 (
        echo Some submodules are not initialized. Initializing...
        git submodule update --init --recursive
    ) else (
        echo All submodules are properly initialized.
    )
)
echo.

echo Checking if ImGui files are present...
if not exist "NSys\external\imgui\imgui.h" (
    echo WARNING: ImGui files not found in NSys\external\imgui\
    echo Attempting to fix submodule issues...
    
    echo Removing empty imgui directories...
    if exist "NSys\external\imgui" rmdir /s /q "NSys\external\imgui"
    if exist "PluginTest\external\imgui" rmdir /s /q "PluginTest\external\imgui"
    
    echo Re-adding submodules...
    git submodule add -b docking https://github.com/ocornut/imgui.git NSys/external/imgui
    git submodule add -b docking https://github.com/ocornut/imgui.git PluginTest/external/imgui
    
    echo Submodules re-added successfully.
) else (
    echo ImGui files found in NSys\external\imgui\
)

if not exist "PluginTest\external\imgui\imgui.h" (
    echo WARNING: ImGui files not found in PluginTest\external\imgui\
    echo This should be fixed by the above steps.
) else (
    echo ImGui files found in PluginTest\external\imgui\
)
echo.

echo Final submodule status:
git submodule status
echo.

echo ========================================
echo Setup completed successfully!
echo ========================================
echo.
echo Next steps:
echo 1. Open NSys.sln in Visual Studio
echo 2. Build the solution
echo 3. Run the application
echo.
echo If you encounter any issues, please check the README.md file
echo for troubleshooting information.
echo.
pause 