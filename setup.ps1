# NSys Project Setup Script (PowerShell)
# This script sets up the NSys project with proper submodule initialization

param(
    [switch]$Verbose,
    [switch]$Force
)

function Write-Status {
    param([string]$Message, [string]$Type = "Info")
    
    $timestamp = Get-Date -Format "HH:mm:ss"
    switch ($Type) {
        "Info"    { Write-Host "[$timestamp] INFO: $Message" -ForegroundColor Cyan }
        "Success" { Write-Host "[$timestamp] SUCCESS: $Message" -ForegroundColor Green }
        "Warning" { Write-Host "[$timestamp] WARNING: $Message" -ForegroundColor Yellow }
        "Error"   { Write-Host "[$timestamp] ERROR: $Message" -ForegroundColor Red }
    }
}

function Test-Command {
    param([string]$Command)
    try {
        Get-Command $Command -ErrorAction Stop | Out-Null
        return $true
    }
    catch {
        return $false
    }
}

function Test-GitRepository {
    if (-not (Test-Path ".git")) {
        Write-Status "This directory is not a Git repository" "Error"
        Write-Status "Please run this script from the root of the NSys project" "Error"
        return $false
    }
    return $true
}

function Initialize-Submodules {
    Write-Status "Checking submodule status..."
    
    try {
        $submoduleStatus = git submodule status 2>$null
        if ($LASTEXITCODE -ne 0) {
            Write-Status "No submodules found. Initializing submodules..." "Info"
            git submodule init
            git submodule update --recursive
        }
        else {
            Write-Status "Submodules found. Checking if they are properly initialized..." "Info"
            
            # Check if any submodules are not initialized (start with -)
            $uninitialized = $submoduleStatus | Where-Object { $_ -match "^-" }
            if ($uninitialized) {
                Write-Status "Some submodules are not initialized. Initializing..." "Warning"
                git submodule update --init --recursive
            }
            else {
                Write-Status "All submodules are properly initialized." "Success"
            }
        }
        return $true
    }
    catch {
        Write-Status "Failed to initialize submodules: $($_.Exception.Message)" "Error"
        return $false
    }
}

function Test-ImGuiFiles {
    $nsysImguiPath = "NSys\external\imgui\imgui.h"
    $pluginTestImguiPath = "PluginTest\external\imgui\imgui.h"
    
    $nsysExists = Test-Path $nsysImguiPath
    $pluginTestExists = Test-Path $pluginTestImguiPath
    
    if (-not $nsysExists -or -not $pluginTestExists) {
        Write-Status "ImGui files not found in one or more locations" "Warning"
        Write-Status "Attempting to fix submodule issues..." "Info"
        
        # Remove empty directories
        if (Test-Path "NSys\external\imgui") {
            Write-Status "Removing empty NSys\external\imgui directory..." "Info"
            Remove-Item -Recurse -Force "NSys\external\imgui" -ErrorAction SilentlyContinue
        }
        
        if (Test-Path "PluginTest\external\imgui") {
            Write-Status "Removing empty PluginTest\external\imgui directory..." "Info"
            Remove-Item -Recurse -Force "PluginTest\external\imgui" -ErrorAction SilentlyContinue
        }
        
        # Re-add submodules
        Write-Status "Re-adding submodules..." "Info"
        git submodule add -b docking https://github.com/ocornut/imgui.git NSys/external/imgui
        git submodule add -b docking https://github.com/ocornut/imgui.git PluginTest/external/imgui
        
        # Verify files are now present
        $nsysExists = Test-Path $nsysImguiPath
        $pluginTestExists = Test-Path $pluginTestImguiPath
        
        if ($nsysExists -and $pluginTestExists) {
            Write-Status "Submodules re-added successfully." "Success"
            return $true
        }
        else {
            Write-Status "Failed to re-add submodules properly" "Error"
            return $false
        }
    }
    else {
        Write-Status "ImGui files found in all locations." "Success"
        return $true
    }
}

# Main execution
Write-Host "========================================" -ForegroundColor White
Write-Host "NSys Project Setup Script (PowerShell)" -ForegroundColor White
Write-Host "========================================" -ForegroundColor White
Write-Host ""

# Check Git installation
Write-Status "Checking Git installation..."
if (-not (Test-Command "git")) {
    Write-Status "Git is not installed or not in PATH" "Error"
    Write-Status "Please install Git from https://git-scm.com/" "Error"
    exit 1
}
Write-Status "Git is installed." "Success"

# Check if this is a Git repository
if (-not (Test-GitRepository)) {
    exit 1
}
Write-Status "This is a Git repository." "Success"

# Initialize submodules
if (-not (Initialize-Submodules)) {
    exit 1
}

# Check ImGui files
if (-not (Test-ImGuiFiles)) {
    exit 1
}

# Final status
Write-Status "Final submodule status:" "Info"
git submodule status

Write-Host ""
Write-Host "========================================" -ForegroundColor White
Write-Host "Setup completed successfully!" -ForegroundColor Green
Write-Host "========================================" -ForegroundColor White
Write-Host ""
Write-Host "Next steps:" -ForegroundColor Yellow
Write-Host "1. Open NSys.sln in Visual Studio" -ForegroundColor White
Write-Host "2. Build the solution" -ForegroundColor White
Write-Host "3. Run the application" -ForegroundColor White
Write-Host ""
Write-Host "If you encounter any issues, please check the README.md file" -ForegroundColor White
Write-Host "for troubleshooting information." -ForegroundColor White
Write-Host ""

if (-not $Force) {
    Read-Host "Press Enter to continue"
} 