@echo off
taskkill /f /im ClickSounds.exe 2>nul
if %errorlevel% equ 0 (
    echo ClickSounds stopped successfully
) else (
    echo ClickSounds was not running
)
pause
