@echo off
setlocal

REM Check if CMake is installed
cmake --version >nul 2>&1
IF ERRORLEVEL 1 (
    echo CMake is not installed.
    echo Please install CMake from: https://cmake.org/download/
    exit /b 1
)

REM Locate vswhere executable
set "VSWHERE=C:\Program Files (x86)\Microsoft Visual Studio\Installer\vswhere.exe"
IF NOT EXIST "%VSWHERE%" (
    echo vswhere.exe not found.
    echo Please install Visual Studio with the Visual Studio Installer.
    exit /b 1
)

REM Use vswhere to find the latest installed Visual Studio instance
for /f "tokens=*" %%i in ('"%VSWHERE%" -latest -requires Microsoft.Component.MSBuild -property installationPath') do set VS_INSTALL_PATH=%%i

REM Check if Visual Studio installation path was found
IF NOT DEFINED VS_INSTALL_PATH (
    echo Visual Studio installation not found.
    echo Please install Visual Studio.
    exit /b 1
)

REM Create build directory if it does not exist
IF NOT EXIST build (
    mkdir build
)

REM Navigate to the build directory
cd build

REM Generate Visual Studio solution using CMake
call "%VS_INSTALL_PATH%\Common7\Tools\VsDevCmd.bat"
cmake .. -G "Visual Studio 17 2022" -A x64

REM Check if the CMake generation was successful
IF ERRORLEVEL 1 (
    echo Failed to generate Visual Studio solution.
    exit /b 1
)

echo Visual Studio solution generated successfully.
endlocal
pause
