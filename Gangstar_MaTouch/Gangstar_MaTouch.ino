#include <Arduino.h>
#define LGFX_USE_V1
#include <LovyanGFX.hpp>

#include <nano_j2me.h>

extern "C" {
  extern const uint8_t gangstar_jar[];
  extern const uint32_t gangstar_jar_len;
}

// ============================================================================
// 1. MATOUCH ESP32-S3 (ST7789V SPI + GT911 TOUCH) EXACT PINOUT
// ============================================================================
#define TFT_CS_PIN     15
#define TFT_DC_PIN     21
#define TFT_RST_PIN    -1
#define TFT_MOSI_PIN   13
#define TFT_MISO_PIN   12
#define TFT_SCLK_PIN   14
#define TFT_BL_PIN     48

#define TOUCH_SDA_PIN  39
#define TOUCH_SCL_PIN  38
#define TOUCH_INT_PIN  40
#define TOUCH_RST_PIN  1

// Physical Button Pins (Wired to GND with INPUT_PULLUP - Safe non-conflicting GPIOs)
#define PIN_UP     2
#define PIN_DOWN   3
#define PIN_LEFT   4
#define PIN_RIGHT  5
#define PIN_ACTION 6
#define PIN_LSK    7
#define PIN_RSK    8

// Native game canvas resolution for this Gameloft JAR build (128x160 Nokia Series 40)
static const int GAME_W = 128;
static const int GAME_H = 160;

// Panel resolution (MaTouch 2.8" ST7789V)
static const int PANEL_W = 240;
static const int PANEL_H = 320;

// ============================================================================
// 2. LOVYANGFX DISPLAY & GT911 TOUCH DRIVER
// ============================================================================
class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_ST7789 _panel_instance;
  lgfx::Bus_SPI      _bus_instance;
  lgfx::Light_PWM    _light_instance;
  lgfx::Touch_GT911  _touch_instance;

public:
  LGFX(void) {
    {
      auto cfg = _bus_instance.config();
      cfg.spi_host    = SPI2_HOST;
      cfg.spi_mode    = 0;
      cfg.freq_write  = 40000000;
      cfg.freq_read   = 16000000;
      cfg.spi_3wire   = false;
      cfg.use_lock    = true;
      cfg.dma_channel = SPI_DMA_CH_AUTO;
      cfg.pin_sclk    = TFT_SCLK_PIN;
      cfg.pin_mosi    = TFT_MOSI_PIN;
      cfg.pin_miso    = TFT_MISO_PIN;
      cfg.pin_dc      = TFT_DC_PIN;
      _bus_instance.config(cfg);
      _panel_instance.setBus(&_bus_instance);
    }
    {
      auto cfg = _panel_instance.config();
      cfg.pin_cs           = TFT_CS_PIN;
      cfg.pin_rst          = TFT_RST_PIN;
      cfg.pin_busy         = -1;
      cfg.panel_width      = 240;
      cfg.panel_height     = 320;
      cfg.offset_x         = 0;
      cfg.offset_y         = 0;
      cfg.offset_rotation  = 0;
      cfg.dummy_read_pixel = 8;
      cfg.dummy_read_bits  = 1;
      cfg.readable         = false;
      cfg.invert           = true;
      cfg.rgb_order        = false;
      cfg.dlen_16bit       = false;
      cfg.bus_shared       = false;
      _panel_instance.config(cfg);
    }
    {
      auto cfg = _light_instance.config();
      cfg.pin_bl      = TFT_BL_PIN;
      cfg.invert      = false;
      cfg.freq        = 12000;
      cfg.pwm_channel = 7;
      _light_instance.config(cfg);
      _panel_instance.setLight(&_light_instance);
    }
    {
      auto cfg = _touch_instance.config();
      cfg.x_min           = 0;
      cfg.x_max           = 239;
      cfg.y_min           = 0;
      cfg.y_max           = 319;
      cfg.pin_int         = TOUCH_INT_PIN;
      cfg.pin_rst         = TOUCH_RST_PIN;
      cfg.bus_shared      = false;
      cfg.offset_rotation = 0;
      cfg.i2c_port        = 0;
      cfg.i2c_addr        = 0x14;
      cfg.pin_sda         = TOUCH_SDA_PIN;
      cfg.pin_scl         = TOUCH_SCL_PIN;
      cfg.freq            = 400000;
      _touch_instance.config(cfg);
      _panel_instance.setTouch(&_touch_instance);
    }
    setPanel(&_panel_instance);
  }
};

LGFX lcd;

// ============================================================================
// 3. J2ME PLATFORM INTEGRATION (TRUE FULL-SCREEN 128x160 -> 240x320)
// ============================================================================
static uint16_t *frame_buffer = NULL;
static QueueHandle_t event_queue = NULL;
static int map_x[PANEL_W];

static void *matouch_alloc(void *ud, nj_size n) {
  (void)ud;
  size_t sz = ((size_t)n + 15u) & ~15u;
  void *p = heap_caps_malloc(sz, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
  if (!p) {
    p = heap_caps_malloc(sz, MALLOC_CAP_8BIT);
  }
  return p;
}

static void matouch_free(void *ud, void *p) {
  (void)ud;
  heap_caps_free(p);
}

static nj_u4 matouch_millis(void *ud) {
  (void)ud;
  return (nj_u4)millis();
}

static void matouch_idle(void *ud) {
  (void)ud;
  vTaskDelay(1);
}

static void matouch_sleep_ms(void *ud, nj_u4 ms) {
  (void)ud;
  vTaskDelay(pdMS_TO_TICKS(ms ? ms : 1));
}

static void matouch_log(void *ud, int level, const char *msg) {
  (void)ud;
  Serial.printf("[J2ME %d] %s\n", level, msg);
}

static int matouch_display_init(void *ud, int *width, int *height, int *stride, void **pixels) {
  (void)ud;
  *width = GAME_W;
  *height = GAME_H;
  *stride = GAME_W * 2;
  if (!frame_buffer) {
    frame_buffer = (uint16_t *)heap_caps_calloc(GAME_W * GAME_H, sizeof(uint16_t), MALLOC_CAP_SPIRAM);
    if (!frame_buffer) {
      frame_buffer = (uint16_t *)calloc(GAME_W * GAME_H, sizeof(uint16_t));
    }
    // Precompute horizontal scale mapping (128 -> 240)
    for (int x = 0; x < PANEL_W; x++) {
      map_x[x] = (x * GAME_W) / PANEL_W;
    }
  }
  *pixels = frame_buffer;
  return NJ_PIXFMT_RGB565;
}

// Scale native 128x160 to full 240x320 edge-to-edge
static void matouch_display_present(void *ud, int x, int y, int w, int h) {
  (void)ud; (void)x; (void)y; (void)w; (void)h;
  if (!frame_buffer) return;

  static uint16_t line[PANEL_W];
  lcd.startWrite();
  lcd.setAddrWindow(0, 0, PANEL_W, PANEL_H);
  for (int dy = 0; dy < PANEL_H; dy++) {
    // dy >> 1 gives exact 2x integer scaling (160 * 2 = 320)
    int sy = dy >> 1;
    const uint16_t *src_row = &frame_buffer[sy * GAME_W];
    for (int dx = 0; dx < PANEL_W; dx++) {
      line[dx] = src_row[map_x[dx]];
    }
    lcd.writePixels((lgfx::rgb565_t *)line, PANEL_W);
  }
  lcd.endWrite();
}

static int matouch_input_poll(void *ud, nj_event *ev) {
  (void)ud;
  if (!event_queue) return 0;
  return (xQueueReceive(event_queue, ev, 0) == pdTRUE) ? 1 : 0;
}

static const void *matouch_app_blob(void *ud, nj_u4 *size) {
  (void)ud;
  if (size) *size = (nj_u4)gangstar_jar_len;
  return gangstar_jar;
}

// ============================================================================
// 4. DUAL INPUT CONTROLLER (GPIO BUTTONS + CAPACITIVE TOUCH OVERLAY)
// ============================================================================
struct ButtonDef {
  uint8_t pin;
  uint8_t key;
  bool lastState;
};

ButtonDef buttons[] = {
  {PIN_UP,     NJ_KEY_UP,         HIGH},
  {PIN_DOWN,   NJ_KEY_DOWN,       HIGH},
  {PIN_LEFT,   NJ_KEY_LEFT,       HIGH},
  {PIN_RIGHT,  NJ_KEY_RIGHT,      HIGH},
  {PIN_ACTION, NJ_KEY_FIRE,       HIGH},
  {PIN_LSK,    NJ_KEY_SOFT_LEFT,  HIGH},
  {PIN_RSK,    NJ_KEY_SOFT_RIGHT, HIGH}
};

static uint8_t last_touch_key = NJ_KEY_NONE;
static bool touch_was_down = false;

void poll_inputs() {
  // 1. Physical Buttons
  for (int i = 0; i < 7; i++) {
    bool state = digitalRead(buttons[i].pin);
    if (state != buttons[i].lastState) {
      buttons[i].lastState = state;
      nj_event ev;
      memset(&ev, 0, sizeof(ev));
      ev.type = (state == LOW) ? NJ_EV_KEY_DOWN : NJ_EV_KEY_UP;
      ev.key = buttons[i].key;
      xQueueSend(event_queue, &ev, 0);
    }
  }

  // 2. Capacitive Touch Screen
  uint16_t tx, ty;
  bool touched = lcd.getTouch(&tx, &ty);
  if (touched) {
    uint8_t current_key = NJ_KEY_NONE;
    // Top corners: Soft keys (Menu / Skip / OK)
    if (ty < 50) {
      if (tx < 80) current_key = NJ_KEY_SOFT_LEFT;
      else if (tx > 160) current_key = NJ_KEY_SOFT_RIGHT;
    } else {
      // D-Pad zones and center action
      if (ty < 120) current_key = NJ_KEY_UP;
      else if (ty > 220) current_key = NJ_KEY_DOWN;
      else if (tx < 70) current_key = NJ_KEY_LEFT;
      else if (tx > 170) current_key = NJ_KEY_RIGHT;
      else current_key = NJ_KEY_FIRE; // Key 5: Shoot / Enter Car / Next
    }

    if (current_key != last_touch_key) {
      if (last_touch_key != NJ_KEY_NONE) {
        nj_event ev; memset(&ev, 0, sizeof(ev));
        ev.type = NJ_EV_KEY_UP;
        ev.key = last_touch_key;
        xQueueSend(event_queue, &ev, 0);
      }
      if (current_key != NJ_KEY_NONE) {
        nj_event ev; memset(&ev, 0, sizeof(ev));
        ev.type = NJ_EV_KEY_DOWN;
        ev.key = current_key;
        xQueueSend(event_queue, &ev, 0);
      }
      last_touch_key = current_key;
    }

    // Normalized pointer event for Canvas listeners
    nj_event pev; memset(&pev, 0, sizeof(pev));
    pev.type = touch_was_down ? NJ_EV_POINTER_MOVE : NJ_EV_POINTER_DOWN;
    pev.x = (tx * GAME_W) / PANEL_W;
    pev.y = (ty * GAME_H) / PANEL_H;
    xQueueSend(event_queue, &pev, 0);
    touch_was_down = true;

  } else {
    if (touch_was_down) {
      if (last_touch_key != NJ_KEY_NONE) {
        nj_event ev; memset(&ev, 0, sizeof(ev));
        ev.type = NJ_EV_KEY_UP;
        ev.key = last_touch_key;
        xQueueSend(event_queue, &ev, 0);
        last_touch_key = NJ_KEY_NONE;
      }
      nj_event pev; memset(&pev, 0, sizeof(pev));
      pev.type = NJ_EV_POINTER_UP;
      xQueueSend(event_queue, &pev, 0);
      touch_was_down = false;
    }
  }
}

void input_task(void *pv) {
  while (true) {
    poll_inputs();
    vTaskDelay(pdMS_TO_TICKS(15));
  }
}

void game_task(void *pv) {
  nj_port port;
  memset(&port, 0, sizeof(port));
  port.name = "matouch_s3";
  port.alloc = matouch_alloc;
  port.free = matouch_free;
  port.millis = matouch_millis;
  port.idle = matouch_idle;
  port.sleep_ms = matouch_sleep_ms;
  port.log = matouch_log;
  port.display_init = matouch_display_init;
  port.display_present = matouch_display_present;
  port.input_poll = matouch_input_poll;
  port.app_blob = matouch_app_blob;

  nj_run_opts opts;
  memset(&opts, 0, sizeof(opts));
  opts.heap_bytes = 4 * 1024 * 1024; // 4MB PSRAM heap
  opts.midlet_class = NULL;

  Serial.println("Booting Gangstar: Crime City...");
  int rc = nj_run(&port, &opts);
  Serial.printf("MIDlet exited with %d\n", rc);
  vTaskDelete(NULL);
}

// ============================================================================
// 5. MAIN SETUP & LOOP
// ============================================================================
void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("MaTouch ESP32-S3 Gangstar (True 128x160 -> 240x320 Full Screen)");

  lcd.init();
  lcd.setRotation(0);
  lcd.fillScreen(TFT_BLACK);

  for (int i = 0; i < 7; i++) {
    pinMode(buttons[i].pin, INPUT_PULLUP);
  }

  event_queue = xQueueCreate(64, sizeof(nj_event));

  xTaskCreatePinnedToCore(input_task, "input_task", 4096, NULL, 4, NULL, 0);
  xTaskCreatePinnedToCore(game_task, "game_task", 32768, NULL, 5, NULL, 1);
}

void loop() {
  delay(1000);
}
