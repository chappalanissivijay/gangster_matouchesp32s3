# Hardware Pinout & Wiring Guide

## Makerfabs MaTouch ESP32-S3 2.8" (ST7789V + GT911)

### Display Bus (ST7789V SPI)
| Function | ESP32-S3 Pin | Notes |
| :--- | :--- | :--- |
| TFT_CS | GPIO 15 | Chip Select |
| TFT_DC | GPIO 21 | Data / Command |
| TFT_SCLK | GPIO 14 | SPI Clock (40 MHz) |
| TFT_MOSI | GPIO 13 | SPI MOSI |
| TFT_MISO | GPIO 12 | SPI MISO |
| TFT_BL | GPIO 48 | Backlight PWM |
| TFT_RST | -1 | Software Reset |

### Capacitive Touch (GT911 I2C)
| Function | ESP32-S3 Pin | Notes |
| :--- | :--- | :--- |
| TOUCH_SDA | GPIO 39 | I2C Data |
| TOUCH_SCL | GPIO 38 | I2C Clock (400 kHz) |
| TOUCH_INT | GPIO 40 | Interrupt |
| TOUCH_RST | GPIO 1 | Hardware Reset |
| I2C Address | 0x14 | GT911 Default |

### Physical DIY Buttons (Active-Low with Internal Pull-Up)
Connect one side of the momentary pushbutton to the designated GPIO pin and the other side to GND.

| Direction / Action | GPIO Pin | J2ME Key Mapping |
| :--- | :--- | :--- |
| **D-Pad UP** | GPIO 2 | `NJ_KEY_UP` (2) |
| **D-Pad DOWN** | GPIO 3 | `NJ_KEY_DOWN` (8) |
| **D-Pad LEFT** | GPIO 4 | `NJ_KEY_LEFT` (4) |
| **D-Pad RIGHT** | GPIO 5 | `NJ_KEY_RIGHT` (6) |
| **ACTION / FIRE** | GPIO 6 | `NJ_KEY_FIRE` (5) |
| **LEFT SOFT KEY (LSK)** | GPIO 7 | `NJ_KEY_SOFT_LEFT` |
| **RIGHT SOFT KEY (RSK)**| GPIO 8 | `NJ_KEY_SOFT_RIGHT` |

> **Warning:** Do NOT connect buttons to GPIO 1, 12, 13, 14, 15, 21, 38, 39, 40, or 48, as these pins are reserved for the ST7789V display and GT911 touch controller.
