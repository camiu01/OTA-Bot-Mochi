# ⌚ OTA Bot Mochi

User interface for a smartwatch based on **ESP32 + ST7789 display**. The default target board is **Waveshare ESP32‑S3‑Touch‑LCD‑1.69**, but you can adapt the project to other ESP32‑S3 boards. **All pins are defined in `pin_config.h` and can be changed there.**

---

## 📂 Project Structure

* `OTA-Bot-Mochi.ino` → main firmware with setup/loop, Chronos callbacks, and UI handling
* `images.h` → icons and bitmaps for the graphical interface
* `FreeSansBold12pt7b.h` → main font used in the UI
* `gfx_helper.h` / `gfx_helper.cpp` → text/layout helpers used by the UI (required)
* `pin_config.h` → **pin map and board configuration (required; edit here to change pins)**
* `README.md` → this file

---

## 🚀 Features

* Custom splash screen at startup
* Header with title, icon, and Bluetooth status
* Dedicated screens:

  * 📎 Base (`BASE`)
  * 🕒 Clock (`CLOCK`)
  * 🌦️ Weather (`WEATHER`)
  * 🗺️ Navigation (`NAVIGATION`)
  * 🔔 Notifications (`NOTIFICATION`)
* Chronos callbacks for:

  * Push notifications (title, message, app, icon)
  * Incoming calls
  * Connection status
  * Weather and navigation updates

---

## 🛠️ Requirements

* **Board**: ESP32‑S3 (tested on Waveshare ESP32‑S3‑Touch‑LCD‑1.69)
* **Display**: ST7789 (SPI, 240×280)
* **Peripherals**: FT6X36 touch (I²C), QMI8658C IMU (I²C), external RTC with INT, active buzzer
* **Arduino Libraries**:

  * [Arduino\_GFX\_Library](https://github.com/moononournation/Arduino_GFX)
  * [ChronosESP32](https://github.com/fbiego/chronos-esp32)
  * Standard ESP32 drivers (`SPI`, `Wire`, etc.)

---

## 🔌 Hardware & Pins (default)

Target: **Waveshare ESP32‑S3‑Touch‑LCD‑1.69**. You can change any pin in `pin_config.h`.

**LCD – ST7789 (240×280, SPI)**

* `LCD_DC=4`, `LCD_CS=5`, `LCD_SCK=6`, `LCD_MOSI=7`, `LCD_RST=8`, `LCD_BL=15`
* `LCD_WIDTH=240`, `LCD_HEIGHT=280`

**Touch – FT6X36 (I²C)**

* `TP_RST=13`, `TP_INT=14`, `TP_SDA=11`, `TP_SCL=10`

**Shared I²C bus**

* `IIC_SDA=11`, `IIC_SCL=10`

**USB (native)**

* `USB_DN=20`, `USB_DP=19`

**UART0**

* `UART_TX=43`, `UART_RX=44`

**Buzzer**

* `BUZZER=34`

**IMU – QMI8658C (I²C)**

* `QMI_SDA=12`, `QMI_SCL=9`

**RTC (I²C + interrupt)**

* `RTC_SDA=11`, `RTC_SCL=10`, `RTC_INT=41`

**System / Power**

* `SYS_EN=35`, `SYS_OUT=36`, `BAT_ADC=1`

**Extra GPIO / Test pads**

* `TP_GPIO2=2`, `TP_GPIO3=3`, `TP_GPIO17=17`, `TP_GPIO18=18`
* Extra pads for I²C (`TP_SCL=10`, `TP_SDA=11`) and UART (`TP_UART_TX=43`, `TP_UART_RX=44`), plus 3V3/5V/GND test pads

---

## ▶️ Usage

**At startup:**

* The custom splash screen is displayed.

**During runtime:**

* The UI cycles through the configured screens (`BASE`, `WEATHER`, `NAVIGATION`, `NOTIFICATION`).
* Notifications, weather data, and navigation info are received via **ChronosESP32** over BLE.

---

## 📝 Configuration & Notes

* **Screens/UI**: the layout uses helper functions for horizontal/vertical centering, top/bottom alignment, and margins.
* **Font**: the UI uses `FreeSansBold12pt7b`.
* **Splash/Icons**: all bitmaps live in `images.h` (you can swap the splash with your own **“Ota Bot”** logo).
* **BLE/Chronos**: after flashing, pair the device with a **ChronosESP32** compatible app/host to receive notifications, weather and navigation data.

---

## 🧰 Customize

* **Colors & margins**: see `gfx_helper.h` (`BACKGROUND`, `TEXT_COLOR`, `ELEMENT_COLOR`, `MARGIN`, ...).
* **Resolution**: `LCD_WIDTH=240`, `LCD_HEIGHT=280` (already set).
* **Pins**: edit `pin_config.h` if you use different hardware.

---

## 🙌 Credits

* Font: **FreeSans Bold** (header generated for Arduino GFX).
* Libraries: **Arduino\_GFX\_Library**, **ChronosESP32**, and the **Arduino core for ESP32 (espressif/arduino-esp32)**.
