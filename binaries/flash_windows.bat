@echo off
setlocal enabledelayedexpansion

echo ========================================================
echo   Gangstar: Crime City - MaTouch ESP32-S3 Flash Utility
echo ========================================================
echo.

set PORT=%1
if "%PORT%"=="" (
    set /p PORT="Enter ESP32-S3 Serial Port (e.g. COM15): "
)

if "%PORT%"=="" (
    echo Error: No COM port specified.
    pause
    exit /b 1
)

echo.
echo Flashing pre-compiled firmware to %PORT%...
echo.

python -m esptool --chip esp32s3 --port %PORT% --baud 921600 --before default-reset --after hard-reset write-flash -z --flash-mode dio --flash-freq 80m --flash-size 16MB 0x0 "%~dp0gangstar_matouch_full.bin"

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [ERROR] Flashing failed! Ensure the ESP32-S3 is connected and not opened in another serial monitor.
    echo Hold BOOT button while plugging in if your board is in download mode.
) else (
    echo.
    echo [SUCCESS] Flashing complete! Your board should now boot Gangstar: Crime City!
)

echo.
pause
