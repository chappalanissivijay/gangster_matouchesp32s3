#!/usr/bin/env bash
set -e

echo "========================================================"
echo "  Gangstar: Crime City - MaTouch ESP32-S3 Flash Utility"
echo "========================================================"
echo ""

PORT=$1
if [ -z "$PORT" ]; then
    read -rp "Enter serial port (e.g. /dev/ttyUSB0 or /dev/ttyACM0): " PORT
fi

if [ -z "$PORT" ]; then
    echo "Error: No serial port specified."
    exit 1
fi

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo ""
echo "Flashing pre-compiled firmware to $PORT..."
echo ""

python3 -m esptool --chip esp32s3 --port "$PORT" --baud 921600 --before default-reset --after hard-reset write-flash -z --flash-mode dio --flash-freq 80m --flash-size 16MB 0x0 "$SCRIPT_DIR/gangstar_matouch_full.bin"

echo ""
echo "[SUCCESS] Flashing complete! Your board should now boot Gangstar: Crime City!"
