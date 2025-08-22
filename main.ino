/**
 * @file main.ino
 * @brief OTA bot watch UI with ChronosESP32 and ST7789.
 *
 * @details
 * Sections index (search these tags in your editor):
 *   [S1] Includes & compile-time config
 *   [S2] Globals & objects
 *   [S3] Chronos callbacks
 *   [S4] Initialization
 *   [S5] Text alignment helpers
 *   [S6] UI header & screens
 *   [S7] Arduino setup/loop
 */

/* =============================== [S1] Includes & compile-time config =============================== */
/** @defgroup config Compile-time configuration
 *  @brief Constants, includes, and compile-time parameters.
 *  @{
 */
#include <Arduino_GFX_Library.h>
#include "FreeSansBold12pt7b.h"
#include <ChronosESP32.h>
#include "pin_config.h"
#include "images.h"

#define BACKGROUND BLACK
#define TEXT_COLOR WHITE
#define ELEMENT_COLOR WHITE
#define MARGIN 10
#define HEADER_MARGIN_TOP 15
#define TOP_SPACER_Y 55
/** @} */  // end of config

/* =================================== [S2] Globals & objects ======================================= */
/** @defgroup globals Globals & objects
 *  @brief Global state and display/watch instances.
 *  @{
 */
ChronosESP32 watch("OTA bot");

bool has_change = false;
bool is_connected = false;
uint32_t nav_crc = 0xFFFFFFFF;

Arduino_DataBus *bus = new Arduino_ESP32SPI(LCD_DC, LCD_CS, LCD_SCK, LCD_MOSI);

Arduino_GFX *gfx = new Arduino_ST7789(
  bus, LCD_RST,
  0 /* rotation */, true /* IPS */, LCD_WIDTH, LCD_HEIGHT, 0, 20, 0, 0);
/** @} */  // end of globals

/* =================================== [S3] Chronos callbacks ======================================= */
/** @defgroup callbacks Chronos callbacks
 *  @brief Bluetooth, notification, and navigation callbacks.
 *  @{
 */

/**
 * @brief Callback for notification events.
 * @param notification Notification object containing time, app, title, and message.
 */
void notification_callback(Notification notification) {
  // Log full notification payload for debugging/telemetry
  Serial.print("Notification received at ");
  Serial.println(notification.time);
  Serial.print("From: ");
  Serial.print(notification.app);
  Serial.print("\tIcon: ");
  Serial.println(notification.icon);
  Serial.println(notification.title);
  Serial.println(notification.message);
}

/**
 * @brief Callback for incoming call (ringer) events.
 * @param caller Caller ID as string.
 * @param state  True if incoming call, False if dismissed.
 */
void ringer_callback(String caller, bool state) {
  if (state) {
    Serial.print("Ringer: Incoming call from ");
    Serial.println(caller);
  } else {
    Serial.println("Ringer dismissed");
  }
}

/**
 * @brief Callback for connection state changes.
 * @param state True if connected, False otherwise.
 */
void connection_callback(bool state) {
  Serial.print("Connection state: ");
  Serial.println(state ? "Connected" : "Disconnected");
  is_connected = state;  // cache for UI header
}

/**
 * @brief Callback for receiving configuration data.
 * @param config Type of configuration update (time/weather/navigation/icon).
 * @param a First parameter (semantic depends on config).
 * @param b Second parameter (semantic depends on config).
 */
void config_callback(Config config, uint32_t a, uint32_t b) {
  switch (config) {
    case CF_TIME:
      // Time sync received (nothing to draw here yet)
      break;

    case CF_WEATHER:
      Serial.println("Weather received");
      if (a) {
        // a==2 typically means forecast values finalized
        if (a == 2) {
          int count = watch.getWeatherCount();
          String update_time = watch.getWeatherTime();
          Serial.print("Weather Count: ");
          Serial.print(count);
          Serial.print("\tUpdated at: ");
          Serial.println(update_time);

          for (int i = 0; i < count; i++) {
            Weather w = watch.getWeatherAt(i);
            Serial.print("Day:");
            Serial.print(w.day);
            Serial.print("\tIcon:");
            Serial.print(w.icon);
            Serial.print("\t");
            Serial.print(w.temp);
            Serial.print("°C");
            Serial.print("\tHigh:");
            Serial.print(w.high);
            Serial.print("°C");
            Serial.print("\tLow:");
            Serial.print(w.low);
            Serial.println("°C");
            if (i == 0) {
              Serial.print("Pressure: ");
              Serial.print(w.pressure);
              Serial.print("\tUV: ");
              Serial.println(w.uv);
            }
          }
        }
      }
      if (b) {
        Serial.print("City name: ");
        String city = watch.getWeatherCity();
        Serial.print(city);
      }
      Serial.println();
      break;

    case CF_NAV_DATA:
      Serial.print("Navigation state: ");
      Serial.println(a ? "Active" : "Inactive");
      has_change = true;  // flag possible UI refresh
      if (a) {
        Navigation nav = watch.getNavigation();
        // Dump navigation useful fields
        Serial.println(nav.directions);
        Serial.println(nav.eta);
        Serial.println(nav.duration);
        Serial.println(nav.distance);
        Serial.println(nav.title);
        Serial.println(nav.speed);
      }
      break;

    case CF_NAV_ICON:
      Serial.print("Navigation Icon data, position: ");
      Serial.println(a);
      Serial.print("Icon CRC: ");
      Serial.printf("0x%04X\n", b);
      if (a == 2) {
        Navigation nav = watch.getNavigation();
        if (nav_crc != nav.iconCRC) {
          nav_crc = nav.iconCRC;  // update icon digest
        }
      }
      break;
  }
}
/** @} */  // end of callbacks

/* ===================================== [S4] Initialization ======================================== */
/** @defgroup init Initialization
 *  @brief Boot-time setup helpers (Chronos and display).
 *  @{
 */

/**
 * @brief Initialize Chronos callbacks and start watch service.
 */
void init_chronos() {
  Serial.println("init chronos");
  watch.setConnectionCallback(connection_callback);
  watch.setNotificationCallback(notification_callback);
  watch.setConfigurationCallback(config_callback);
  watch.setRingerCallback(ringer_callback);
  watch.begin();
}

/**
 * @brief Initialize the screen and backlight.
 */
void init_screen() {
  if (!gfx->begin()) {
    Serial.println("gfx->begin() failed!");
  }
  gfx->fillScreen(BACKGROUND);

  pinMode(LCD_BL, OUTPUT);
  digitalWrite(LCD_BL, HIGH);  // turn on backlight
}
/** @} */  // end of init

/* ============================== [S5] Text alignment helpers ====================================== */
/** @defgroup text_helpers Text alignment helpers
 *  @brief Utilities for text placement on screen.
 *  @{
 */

/**
 * @brief Calculate X position to horizontally center text.
 * @param str Text string.
 * @return X coordinate for centered placement.
 */
int16_t get_align_horizontal(String str) {
  int16_t x1, y1;
  uint16_t w, h;
  gfx->getTextBounds(str, 0, 0, &x1, &y1, &w, &h);
  return (LCD_WIDTH - w) / 2;
}

/**
 * @brief Calculate Y position to vertically center text.
 * @param str Text string.
 * @return Y coordinate (baseline) for centered placement.
 */
int16_t get_align_vertical(String str) {
  int16_t x1, y1;
  uint16_t w, h;
  gfx->getTextBounds(str, 0, 0, &x1, &y1, &w, &h);
  return (LCD_HEIGHT + h) / 2;
}

/**
 * @brief Draw text horizontally centered.
 * @param str Text string.
 * @param y   Baseline Y coordinate.
 */
void draw_align_horizontal(String str, int16_t y) {
  int16_t x = get_align_horizontal(str);
  gfx->setCursor(x, y);
  gfx->print(str);
}

/**
 * @brief Draw text vertically centered.
 * @param str Text string.
 * @param x   X coordinate.
 */
void draw_align_vertical(String str, int16_t x) {
  int16_t y = get_align_vertical(str);
  gfx->setCursor(x, y);
  gfx->print(str);
}

/**
 * @brief Draw text centered in X and Y.
 * @param str Text string.
 */
void draw_align_center(String str) {
  int16_t x = get_align_horizontal(str);
  int16_t y = get_align_vertical(str);
  gfx->setCursor(x, y);
  gfx->print(str);
}

/** @brief Draw text aligned left at given Y (with margin). */
void draw_align_left(String str, int16_t y) {
  gfx->setCursor(MARGIN, y);
  gfx->print(str);
}

/** @brief Draw text aligned right at given Y (with margin). */
void draw_align_right(String str, int16_t y) {
  int16_t x1, y1;
  uint16_t w, h;
  gfx->getTextBounds(str, 0, 0, &x1, &y1, &w, &h);
  gfx->setCursor(LCD_WIDTH - w - MARGIN, y);
  gfx->print(str);
}

/** @brief Draw text aligned at top (baseline adjusted). */
void draw_align_top(String str, int16_t x) {
  int16_t x1, y1;
  uint16_t w, h;
  gfx->getTextBounds(str, 0, 0, &x1, &y1, &w, &h);
  gfx->setCursor(x, h + MARGIN);
  gfx->print(str);
}

/** @brief Draw text aligned at bottom with margin. */
void draw_align_bottom(String str, int16_t x) {
  gfx->setCursor(x, LCD_HEIGHT - MARGIN);
  gfx->print(str);
}

/** @brief Draw text top-centered with a top margin. */
void draw_align_top_centered(String str, int16_t margin) {
  int16_t x1, y1;
  uint16_t w, h;
  gfx->getTextBounds(str, 0, 0, &x1, &y1, &w, &h);

  int16_t x = (LCD_WIDTH - w) / 2;
  int16_t y = margin - y1;  // align top of bounding box

  gfx->setCursor(x, y);
  gfx->print(str);
}

/** @brief Draw text bottom-centered with a bottom margin. */
void draw_align_bottom_centered(String str, int16_t margin) {
  int16_t x1, y1;
  uint16_t w, h;
  gfx->getTextBounds(str, 0, 0, &x1, &y1, &w, &h);

  int16_t x = (LCD_WIDTH - w) / 2;
  int16_t y = (LCD_HEIGHT - margin) - (y1 + h);  // align bottom

  gfx->setCursor(x, y);
  gfx->print(str);
}
/** @} */  // end of text_helpers

/* ============================== [S6] UI header & screens ========================================= */
/** @defgroup ui UI drawing
 *  @brief Header and screen renderers.
 *  @{
 */

/**
 * @brief Draw header with icon, centered title, and Bluetooth status.
 * @param text   Header title.
 * @param bitmap Icon image (unused in current draw, reserved for future).
 */
void draw_header(String text, const uint8_t bitmap[]) {
  // Left static info icon
  gfx->drawBitmap(MARGIN, HEADER_MARGIN_TOP, bitmap, 28, 32, TEXT_COLOR);

  // Separator line + decorative bitmap
  gfx->drawRect(0, TOP_SPACER_Y, LCD_WIDTH, 2, ELEMENT_COLOR);
  gfx->drawBitmap(0, TOP_SPACER_Y, image_Unplug_bg_bottom_bits, 128, 10, ELEMENT_COLOR);

  // Centered title
  draw_align_top_centered(text, HEADER_MARGIN_TOP);

  
  // Right BT status
  if (is_connected)
    gfx->drawBitmap(212 - MARGIN, HEADER_MARGIN_TOP, image_bluetooth_bits, 28, 32, TEXT_COLOR);
  else
    gfx->drawBitmap(212 - MARGIN, HEADER_MARGIN_TOP, image_bluetooth_not_connected_bits, 28, 32, TEXT_COLOR);
}

/** @brief Available screens. */
enum screen_type {
  BASE,
  CLOCK,
  NAVIGATION,
  NOTIFICATION,
  WEATHER
};

screen_type current_screen = BASE;

/** @brief Show splash screen at boot. */
void start_screen() {
  gfx->fillScreen(BACKGROUND);
  gfx->drawBitmap(8, 0, image_DolphinReadingSuccess_bits, 224, 192, TEXT_COLOR);

  gfx->setTextColor(TEXT_COLOR);
  gfx->setFont(&FreeSansBold12pt7b);
  gfx->setTextSize(2);

  // Anchor title above spacer line
  draw_align_bottom_centered("OTA BOT", 10 + HEADER_MARGIN_TOP);

  delay(3000);
  gfx->fillScreen(BACKGROUND);
}
/** @brief Base screen. */
void base_screen(){
  //TODO read gif and visualize them
}

/** @brief Clock screen. */
void clock_screen() {
  draw_header("clock", image_clock_bits);
}

/** @brief Weather screen. */
void weather_screen() {
  draw_header("weather", image_display_brightness_bits);
}

/** @brief Navigation screen. */
void navigation_screen() {
  draw_header("navigation", image_location_map_bits);
}

/** @brief Notification screen. */
void notification_screen() {
  draw_header("notification", image_menu_information_sign_white_bits);
}
/** @} */  // end of ui

/* ================================ [S7] Arduino setup/loop ========================================= */
/** @defgroup arduino Arduino runtime
 *  @brief setup() and loop() entry points.
 *  @{
 */

/** @brief Arduino setup: init subsystems and show splash. */
void setup() {
  Serial.begin(115200);
  init_chronos();
  init_screen();
  start_screen();
}

/** @brief Arduino loop: draw current screen and service Chronos. */
void loop() {
  switch (current_screen) {
    case BASE: base_screen(); break;
    case CLOCK: clock_screen(); break;
    case WEATHER: weather_screen(); break;
    case NAVIGATION: navigation_screen(); break;
    case NOTIFICATION: notification_screen(); break;
    default: base_screen(); break;
  }
  watch.loop();  // must be called regularly to handle events & BLE
}
/** @} */  // end of arduino
