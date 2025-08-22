# ⌚ OTA Bot Mochi

User interface for a smartwatch based on **ESP32 + ST7789 display**, with **ChronosESP32** support for notifications, weather, navigation, and calls.

---

## 📂 Project Structure

- `main.ino` → main firmware with setup/loop, Chronos callbacks, and UI handling  
- `images.h` → icons and bitmaps for the graphical interface  
- `FreeSansBold12pt7b.h` → main font used in the UI  
- `README.md` → this file  

---

## 🚀 Features

- Custom splash screen at startup  
- Header with title, icon, and Bluetooth status  
- Dedicated screens:
  - 📎 Base (`BASE`)
  - 🕒 Clock (`CLOCK`)
  - 🌦️ Weather (`WEATHER`)
  - 🗺️ Navigation (`NAVIGATION`)
  - 🔔 Notifications (`NOTIFICATION`)
- Chronos callbacks for:
  - Push notifications (title, message, app, icon)  
  - Incoming calls  
  - Connection status  
  - Weather and navigation updates  

---

## 🛠️ Requirements

- **Board**: ESP32  
- **Display**: ST7789 (SPI)  
- **Arduino Libraries**:
  - [Arduino_GFX_Library](https://github.com/moononournation/Arduino_GFX)
  - [ChronosESP32](https://github.com/OpenChronos/ChronosESP32)
  - Standard ESP32 drivers (`SPI`, `Wire`, etc.)  

---

## ▶️ Usage

At startup:  
The custom splash screen is displayed.  

During runtime:  
The UI cycles through the configured screens (`BASE`, `WEATHER`, `NAVIGATION`, `NOTIFICATION`).  
Notifications, weather data, and navigation info are received via **ChronosESP32** over BLE.  

---
