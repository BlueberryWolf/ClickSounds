@echo off
premake5 gmake
if %errorlevel% neq 0 (
    echo Failed to generate makefiles
    exit /b 1
)

taskkill /f /im ClickSounds.exe

echo Building with Clang...
make config=release_x64 CC=clang CXX="clang++ -D_ALLOW_COMPILER_AND_STL_VERSION_MISMATCH -static"
if %errorlevel% neq 0 (
    echo Build failed
    exit /b 1
)

echo Build successful!