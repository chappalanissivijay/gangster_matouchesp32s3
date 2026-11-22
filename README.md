# ESP32-S3 J2ME Retro Handheld Gaming Console (MaTouch 2.8")

A complete, high-performance J2ME (MIDP 2.0 / CLDC 1.1) mobile gaming engine running natively on the **Makerfabs MaTouch ESP32-S3 2.8"** handheld touch display.

Features real-time hardware scaling ($128 \times 160 \rightarrow 240 \times 320$), capacitive multi-touch D-pad/keypad emulation, debounced physical GPIO button inputs, and dual-core FreeRTOS task scheduling.

---

## Hardware Specifications

| Component | Specification |
| :--- | :--- |
| **MCU** | ESP32-S3-WROOM-1 (Dual-core Xtensa LX7 @ 240 MHz) |
| **Memory** | 16 MB Octal SPI Flash + 8 MB Octal OPI PSRAM (N16R8) |
| **Display** | 2.8" ST7789V SPI TFT LCD (240 x 320 resolution) |
| **Touch** | Goodix GT911 Capacitive Multi-touch (I2C) |
| **Graphics Driver** | LovyanGFX Hardware Accelerated Engine |
| **J2ME Core** | Optimized C99 CLDC 1.1 / MIDP 2.0 VM |

---

## Hardware Pinout

### ST7789V SPI Display
- **CS:** GPIO 15
- **DC:** GPIO 21
- **MOSI:** GPIO 13
- **MISO:** GPIO 12
- **SCLK:** GPIO 14
- **Backlight (PWM):** GPIO 48

### GT911 Capacitive Touch
- **SDA:** GPIO 39
- **SCL:** GPIO 38
- **INT:** GPIO 40
- **RST:** GPIO 1
- **I2C Address:** 0x14

### Physical Pushbuttons (Active-Low to GND)
Connect any momentary tactile switches between the specified GPIO and GND (internal pull-ups enabled):
- **UP:** GPIO 2
- **DOWN:** GPIO 3
- **LEFT:** GPIO 4
- **RIGHT:** GPIO 5
- **ACTION / FIRE (Key 5):** GPIO 6
- **LEFT SOFT KEY (LSK):** GPIO 7
- **RIGHT SOFT KEY (RSK):** GPIO 8

> **Note:** Avoid connecting switches to GPIOs 1, 12, 13, 14, 15, 21, 38, 39, 40, or 48.

---

## In-Game Controls

### 1. On-Screen Touch Controls
The capacitive touch screen is mapped into virtual keypad zones:
- **Top-Left Corner:** Left Soft Key (LSK / Menu / Select)
- **Top-Right Corner:** Right Soft Key (RSK / Back / Skip)
- **Upper Quadrant:** D-Pad UP
- **Lower Quadrant:** D-Pad DOWN
- **Left Quadrant:** D-Pad LEFT
- **Right Quadrant:** D-Pad RIGHT
- **Center Area:** Action / Fire / Number 5

### 2. Physical Buttons
Any buttons connected to GPIOs 2-8 will trigger standard J2ME keypad scan codes with sub-millisecond latency.

---

## How to Flash

### Option 1: One-Click Flash (Pre-compiled Binary)

1. Connect your MaTouch ESP32-S3 board to your PC via USB-C.
2. Determine your COM port (Windows Device Manager or `ls /dev/tty*`).
3. Run the flashing script:
   - **Windows:** Double-click `binaries/flash_windows.bat` and enter your COM port.
   - **Linux / macOS:** Run `chmod +x binaries/flash_linux_mac.sh && ./binaries/flash_linux_mac.sh /dev/ttyUSB0`
4. Or flash directly using `esptool`:
   ```bash
   python -m esptool --chip esp32s3 -b 921600 write_flash 0x0 binaries/gangstar_matouch_full.bin
   ```

### Option 2: Compile with Arduino IDE

1. Install **ESP32 Arduino Core** (v2.0.14 or later) in Arduino IDE.
2. Install the **LovyanGFX** library via Arduino Library Manager.
3. Copy the `libraries/nano_j2me` folder from this repository into your `Documents/Arduino/libraries/` folder.
4. Open `Gangstar_MaTouch/Gangstar_MaTouch.ino`.
5. In the Tools menu, configure:
   - **Board:** "ESP32S3 Dev Module"
   - **Flash Size:** "16MB (128Mb)"
   - **Partition Scheme:** "16M Flash (3MB APP/9.9MB FATFS)" or "app3M_fat9M_16MB"
   - **PSRAM:** "OPI PSRAM"
   - **Flash Mode:** "QIO 80MHz"
   - **USB CDC On Boot:** "Enabled"
6. Click **Upload**.

---

## Porting Custom J2ME Games (.jar)

To convert any other J2ME MIDP 2.0 game to run on the console:

```bash
python tools/jar_to_c.py your_game.jar Gangstar_MaTouch/gangstar_jar.c
```

Recompile and upload the sketch to enjoy your custom game.

---

## License

This project is released under the MIT License. See [LICENSE](LICENSE) for details.
All J2ME games and third-party software trademarks belong to their respective copyright holders.
