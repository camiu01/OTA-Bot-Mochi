/*
 * OTA-Bot-Mochi
 * Copyright (c) 2026 Camiu (https://github.com/camiu01)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
// Keep the Arduino IDE selection; explicit LCD builds can override it.
#if !defined(MOCHI_BOARD_AMOLED_18) && !defined(MOCHI_BOARD_LCD_169)
#define MOCHI_BOARD_AMOLED_18
#endif
#if defined(MOCHI_BOARD_AMOLED_18) && defined(MOCHI_BOARD_LCD_169)
#error "Select only one Mochi board profile"
#endif
/**
 * @file pin_config.h
 * @brief Pin definitions for two Waveshare boards selected at build time.
 *
 * @details
 * Board selection: the editable definition above currently selects AMOLED for
 * Arduino IDE. CLI/native builds explicitly select `-DMOCHI_BOARD_LCD_169`
 * or `-DMOCHI_BOARD_AMOLED_18`; do not define both.
 *   - Waveshare ESP32-S3-Touch-LCD-1.69 OLD: ST7789 240x280 over SPI, PWM
 *     backlight on GPIO15, CST816/FT6X36 touch, one shared I2C bus on
 *     SDA11/SCL10, active buzzer on GPIO33 (new revision: GPIO42).
 *   - Waveshare ESP32-S3-Touch-AMOLED-1.8 V2: CO5300 368x448 over QSPI, panel
 *     reset released through the TCA9554 expander, brightness as a controller
 *     command (no backlight pad), CST820 touch, one shared I2C bus on
 *     SDA15/SCL14, AXP2101 PMU, ES8311 codec instead of a buzzer.
 *
 * Host builds may override the panel geometry with `-DLCD_WIDTH` and
 * `-DLCD_HEIGHT` (for example `-DLCD_HEIGHT=320`) to preview another screen
 * without editing this file.
 *
 * Sections index (search these tags in your editor):
 *   [S0] Board selection
 *   [S1] LCD / AMOLED panel
 *   [S2] Touch Panel
 *   [S3] I²C Bus
 *   [S4] USB
 *   [S5] UART
 *   [S6] Buzzer
 *   [S7] IMU QMI8658C
 *   [S8] RTC
 *   [S9] System Control & Power
 *   [S10] Extra GPIO / Test Points (LCD board)
 *   [S11] AMOLED audio and expander
 *
 * @note Pin reference:
 *       https://www.waveshare.com/wiki/ESP32-S3-Touch-LCD-1.69
 *       https://docs.waveshare.com/ESP32-S3-Touch-AMOLED-1.8
 */

/* ============================ [S0] Board selection =========================== */
/** @defgroup board Board selection
 *  @brief Compile-time profile; default is the 1.69 inch LCD board.
 *  @{
 */
#if defined(MOCHI_BOARD_AMOLED_18)
#define MOCHI_BOARD_LCD_169 0 /**< LCD board profile inactive */
#else
#define MOCHI_BOARD_LCD_169 1 /**< LCD board profile active (default) */
#endif
/** @} */  // end of board

/* ==================================== [S1] LCD ==================================== */
#if defined(MOCHI_BOARD_AMOLED_18)
/** @defgroup lcd LCD configuration
 *  @brief CO5300 368x448 QSPI AMOLED pins; the visible area starts at RAM column 16.
 *  @{
 */
#define LCD_SDIO0 4   /**< QSPI bidirectional data 0 */
#define LCD_SDIO1 5   /**< QSPI data 1 */
#define LCD_SDIO2 6   /**< QSPI data 2 */
#define LCD_SDIO3 7   /**< QSPI data 3 */
#define LCD_SCLK  11  /**< QSPI serial clock */
#define LCD_CS    12  /**< LCD chip select */
#define LCD_RST   -1  /**< No reset pad: the TCA9554 expander releases the panel */
#define LCD_BL    -1  /**< No backlight pad: brightness is a controller command */
#ifndef LCD_WIDTH
#define LCD_WIDTH  368 /**< LCD width in pixels */
#endif
#ifndef LCD_HEIGHT
#define LCD_HEIGHT 448 /**< LCD height in pixels */
#endif
/** @} */  // end of lcd
#else
/** @defgroup lcd LCD configuration
 *  @brief ST7789 240x280 display pins.
 *  @{
 */
#define LCD_DC     4   /**< LCD Data/Command pin */
#define LCD_CS     5   /**< LCD Chip Select pin */
#define LCD_SCK    6   /**< LCD SPI Clock pin */
#define LCD_MOSI   7   /**< LCD SPI MOSI pin */
#define LCD_RST    8   /**< LCD Reset pin */
#define LCD_BL     15  /**< LCD Backlight pin */
#ifndef LCD_WIDTH
#define LCD_WIDTH  240 /**< LCD width in pixels */
#endif
#ifndef LCD_HEIGHT
#define LCD_HEIGHT 280 /**< LCD height in pixels */
#endif
/** @} */  // end of lcd
#endif

/* ================================= [S2] Touch Panel ================================ */
/** @defgroup touch Touch Panel configuration
 *  @brief Auto-probed CST816/FT6X36 (LCD board) or CST820 (AMOLED V2) touch pins.
 *  @{
 */
#if defined(MOCHI_BOARD_AMOLED_18)
#define TP_RST     -1  /**< No touch reset pad on the V2 board */
#define TP_INT     21  /**< Touch Interrupt */
#define TP_SDA     15  /**< Touch SDA (I2C data), shared bus */
#define TP_SCL     14  /**< Touch SCL (I2C clock), shared bus */
#else
#define TP_RST     13  /**< Touch Reset */
#define TP_INT     14  /**< Touch Interrupt */
#define TP_SDA     11  /**< Touch SDA (I2C data) */
#define TP_SCL     10  /**< Touch SCL (I2C clock) */
#endif
/** @} */  // end of touch

/* ================================== [S3] I²C Bus ================================== */
/** @defgroup i2c I2C Bus
 *  @brief Shared I²C lines for sensors.
 *  @{
 */
#if defined(MOCHI_BOARD_AMOLED_18)
#define IIC_SDA    15  /**< Generic I2C SDA */
#define IIC_SCL    14  /**< Generic I2C clock */
#else
#define IIC_SDA    11  /**< Generic I2C SDA */
#define IIC_SCL    10  /**< Generic I2C SCL */
#endif
/** @} */  // end of i2c

/* =================================== [S4] USB ===================================== */
/** @defgroup usb USB
 *  @brief Native USB interface pins.
 *  @{
 */
#define USB_DN     20  /**< USB D- */
#define USB_DP     19  /**< USB D+ */
/** @} */  // end of usb

/* ================================= [S5] UART ====================================== */
/** @defgroup uart UART
 *  @brief Default UART0 pins.
 *  @{
 */
#define UART_TX    43  /**< UART0 TX */
#define UART_RX    44  /**< UART0 RX */
/** @} */  // end of uart

/* ================================= [S6] Buzzer ==================================== */
/** @defgroup buzzer Buzzer
 *  @brief Active buzzer control pin.
 *  @{
 */
#if defined(MOCHI_BOARD_AMOLED_18)
#define BUZZER     -1  /**< No PWM buzzer: audio goes through the ES8311 codec */
#else
#define BUZZER     33  /**< Buzzer pin (OLD board net "Buzz"; the new revision uses GPIO42) */
#endif
/** @} */  // end of buzzer

/* =============================== [S7] IMU QMI8658C ================================ */
/** @defgroup imu IMU
 *  @brief QMI8658C accelerometer/gyroscope I²C pins.
 *  @details The sensor sits on the board I²C bus shared with the touch controller
 *           and the RTC, so it is driven through the same bus instance.
 *  @{
 */
#if defined(MOCHI_BOARD_AMOLED_18)
#define QMI_SDA    15  /**< IMU SDA, shared with touch and RTC */
#define QMI_SCL    14  /**< IMU SCL, shared with touch and RTC */
#else
#define QMI_SDA    11  /**< IMU SDA, shared with touch and RTC */
#define QMI_SCL    10  /**< IMU SCL, shared with touch and RTC */
#endif
/** @} */  // end of imu

/* ================================= [S8] RTC ====================================== */
/** @defgroup rtc RTC
 *  @brief Real-time clock I²C + interrupt.
 *  @{
 */
#if defined(MOCHI_BOARD_AMOLED_18)
#define RTC_SDA    15  /**< RTC SDA, shared bus */
#define RTC_SCL    14  /**< RTC SCL, shared bus */
#define RTC_INT    -1  /**< Not wired to a known pad on the V2 board; unused by firmware */
#else
#define RTC_SDA    11  /**< RTC SDA */
#define RTC_SCL    10  /**< RTC SCL */
#define RTC_INT    41  /**< RTC Interrupt */
#endif
/** @} */  // end of rtc

/* =========================== [S9] System Control & Power =========================== */
/** @defgroup sys System & Power
 *  @brief Board-specific power control; battery measurement remains unverified.
 *  @{
 */
#if defined(MOCHI_BOARD_AMOLED_18)
#define SYS_EN     -1  /**< Unused: power rails are managed by the AXP2101 PMU */
#define SYS_OUT    -1  /**< Unused: power rails are managed by the AXP2101 PMU */
#define BAT_ADC    -1  /**< No ADC divider: battery data would come from the AXP2101 */
#else
#define SYS_EN     35  /**< Original board: active-high battery-power hold (SS8050/AO3401). */
#define SYS_OUT    36  /**< Original board: active-low PWR/Key2 input. */
#define BAT_ADC    1   /**< Unverified ADC candidate; do not assume a battery divider ratio. */
#endif
/** @} */  // end of sys

/* ============================== [S10] Extra GPIO ================================== */
#if !defined(MOCHI_BOARD_AMOLED_18)
/** @defgroup gpio_extra Extra GPIO
 *  @brief Test points and exposed GPIO pins of the 1.69 inch LCD board.
 *  @{
 */
#define TP_GPIO2   2   /**< Extra GPIO2 (TP9) */
#define TP_GPIO3   3   /**< Extra GPIO3 (TP8) */
#define TP_GPIO17  17  /**< Extra GPIO17 (TP10) */
#define TP_GPIO18  18  /**< Extra GPIO18 (TP11) */
#define TP_SCL     10  /**< Extra I2C SCL (TP6) */
#define TP_SDA     11  /**< Extra I2C SDA (TP7) */
#define TP_UART_TX 43  /**< UART0 TX (TP4) */
#define TP_UART_RX 44  /**< UART0 RX (TP5) */
#define TP_3V3     -1  /**< 3.3V supply (TP3) */
#define TP_GND     -1  /**< Ground (TP2) */
#define TP_5V      -1  /**< 5V supply (TP1) */
/** @} */  // end of gpio_extra
#endif

/* =========================== [S11] AMOLED audio/expand ============================ */
#if defined(MOCHI_BOARD_AMOLED_18)
/** @defgroup amoled AMOLED board extras
 *  @brief TCA9554 panel-reset expander and the ES8311 codec wiring.
 *  @details The expander must release the panel reset before display init or the
 *           screen stays silently black. The optional ES8311 backend uses
 *           the shared I2C bus and TX-only I2S; GPIO46 enables its amplifier.
 *  @{
 */
#define IO_EXPANDER_ADDR 0x20 /**< TCA9554 I2C address (A2/A1/A0 low) */
#define IO_EXPANDER_RESET_PINS 0x07 /**< Expander bits 0..2 gate the panel reset */
#define I2S_MCK_IO   16  /**< ES8311 master clock */
#define I2S_BCK_IO   9   /**< ES8311 bit clock */
#define I2S_WS_IO    45  /**< ES8311 word select */
#define I2S_DI_IO    10  /**< ES8311 data in (microphone) */
#define I2S_DO_IO    8   /**< ES8311 data out (speaker) */
#define AUDIO_PA     46  /**< Speaker amplifier enable */
/** @} */  // end of amoled
#endif
