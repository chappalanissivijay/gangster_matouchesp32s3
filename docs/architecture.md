# System Architecture

## Overview
This project ports a pure C-based J2ME CLDC 1.1 / MIDP 2.0 runtime environment onto the ESP32-S3 microcontroller, specifically optimized for the Makerfabs MaTouch 2.8" display platform.

```
+--------------------------------------------------------------------+
|                         ESP32-S3 Dual Core                         |
|                                                                    |
|   +--------------------------+       +-------------------------+   |
|   |         CORE 0           |       |         CORE 1          |   |
|   |    (Async I/O Task)      |       |    (J2ME Engine Task)   |   |
|   |                          |       |                         |   |
|   | - GT911 Capacitive Touch |       | - J2ME Bytecode VM      |   |
|   | - Debounced GPIO Buttons |       | - Canvas & Display API  |   |
|   | - Input Event Dispatcher |       | - High-Speed DMA Render |   |
|   +------------+-------------+       +------------^------------+   |
|                |                                  |                |
|                +-------------> [ FreeRTOS ] <-----+                |
|                                [  Queue   ]                        |
+--------------------------------------------------------------------+
                                   |
         +-------------------------+-------------------------+
         |                                                   |
         v                                                   v
+------------------+                               +------------------+
|   8MB OPI PSRAM  |                               |   ST7789V SPI    |
| (Dynamic Memory) |                               | (Display Output) |
+------------------+                               +------------------+
```

## Key Architectural Components

### 1. Dual-Core Concurrency Model
- **Core 0 (`input_task`)**: Runs an asynchronous input polling loop every 15ms. It evaluates GT911 touch coordinates and physical button states, debounces contacts, and translates them into normalized J2ME event records placed into a FreeRTOS event queue.
- **Core 1 (`game_task`)**: Dedicated exclusively to bytecode interpretation, game logic, and graphic rendering. This prevents input latency from dropping frame rates.

### 2. Memory Architecture & PSRAM
- The ESP32-S3 N16R8 module features 8 MB of Octal (OPI) PSRAM.
- The J2ME runtime allocates its primary memory pool (4 MB) in external PSRAM via `heap_caps_calloc(..., MALLOC_CAP_SPIRAM)`.
- Internal SRAM (512 KB) is preserved for FreeRTOS task stacks, DMA buffers, and LovyanGFX display transfers.

### 3. Display Pipeline & Hardware Scaling
- **Native Resolution**: Most retro mobile games render at $128 \times 160$ pixels (Nokia standard).
- **Target Panel**: The MaTouch 2.8" panel has a resolution of $240 \times 320$ pixels.
- **Scaling Algorithm**:
  - Vertical: Exact $2\times$ integer scaling via line duplication ($dy \gg 1$).
  - Horizontal: High-performance precomputed lookup table mapping 128 source columns across 240 target columns ($dx \times 128 / 240$).
  - Hardware Acceleration: Output lines are transferred to the ST7789V controller via SPI2 DMA at 40 MHz.
