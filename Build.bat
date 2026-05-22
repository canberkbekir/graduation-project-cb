@echo off
setlocal enabledelayedexpansion

:: === CONFIGURATION ===
set PROJECT_NAME=RizzGame
set PROJECT_PATH=%~dp0%PROJECT_NAME%.uproject
set UE_PATH=C:\Program Files\Epic Games\UE_5.6
set SOURCE_PATH=%~dp0Source\%PROJECT_NAME%
set SLN_PATH=%~dp0%PROJECT_NAME%.sln

:: === CHECK IF REGENERATION IS NEEDED ===
set REGEN_REQUIRED=false

if not exist "%~dp0\Binaries" (
    echo Missing Binaries folder - regeneration needed.
    set REGEN_REQUIRED=true
)


:: === REGENERATE PROJECT FILES IF NEEDED ===
if "!REGEN_REQUIRED!"=="true" (
    echo.
    echo Regenerating project files...
    "%UE_PATH%\Engine\Binaries\DotNET\UnrealBuildTool\UnrealBuildTool.exe" -projectfiles -project="%PROJECT_PATH%" -game -engine
    if errorlevel 1 (
        echo Failed to regenerate project files.
        pause
        exit /b 1
    )
)

:: === BUILD THE PROJECT ===
echo.
echo Building project...
"%UE_PATH%\Engine\Build\BatchFiles\Build.bat" %PROJECT_NAME%Editor Win64 Development "%PROJECT_PATH%"

pause
