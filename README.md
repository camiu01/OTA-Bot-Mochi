# OTA Bot Watch UI ⌚

Interfaccia utente per smartwatch basato su **ESP32 + display ST7789**, con supporto a **ChronosESP32** per notifiche, meteo, navigazione e chiamate.

---

## 📂 Struttura del progetto

- `main.ino` → codice principale con setup/loop, callback Chronos e gestione UI  
- `images.h` → icone e bitmap per interfaccia grafica  
- `FreeSansBold12pt7b.h` → font principale usato nella UI  
- `README.md` → questo file  

---

## 🚀 Funzionalità

- Splash screen personalizzato all’avvio  
- Header con titolo, icona e stato Bluetooth  
- Schermate dedicate a:
  - 🕒 Orologio (`BASE`)
  - 🌦️ Meteo (`WEATHER`)
  - 🗺️ Navigazione (`NAVIGATION`)
  - 🔔 Notifiche (`NOTIFICATION`)
- Callbacks per:
  - Notifiche push (titolo, messaggio, app, icona)  
  - Chiamate in arrivo  
  - Stato connessione  
  - Aggiornamenti meteo e navigazione  

---

## 🛠️ Requisiti

- **Scheda**: ESP32  
- **Display**: ST7789 (SPI)  
- **Librerie Arduino necessarie**:
  - [Arduino_GFX_Library](https://github.com/moononournation/Arduino_GFX)
  - [ChronosESP32](https://github.com/OpenChronos/ChronosESP32)
  - Driver standard ESP32 (`SPI`, `Wire`, ecc.)

---
▶️ Utilizzo

Dopo l’avvio:
Lo splash screen mostra l’animazione/logo.
La UI cicla tra le schermate configurate (BASE, WEATHER, NAVIGATION, NOTIFICATION).
Le notifiche, meteo e navigazione arrivano tramite ChronosESP32 via BLE.
