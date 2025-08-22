#pragma once

#define XPOWERS_CHIP_AXP2101

/**
 * @file pins.h
 * @brief Pin definitions for Waveshare ESP32-S3-Touch-LCD-1.69
 *
 * Questa mappatura è basata sulla documentazione Waveshare
 * e collega le periferiche principali: LCD, Touch, I2C, USB, UART,
 * Buzzer, QMI8658C e alimentazione.
 */

/* -----------------------------
 * LCD (ST7789 240x280)
 * ----------------------------- */
/** @brief LCD Data/Command pin */
#define LCD_DC     4
/** @brief LCD Chip Select pin */
#define LCD_CS     5
/** @brief LCD SPI Clock pin */
#define LCD_SCK    6
/** @brief LCD SPI MOSI pin */
#define LCD_MOSI   7
/** @brief LCD Reset pin */
#define LCD_RST    8
/** @brief LCD Backlight control pin */
#define LCD_BL     15
/** @brief LCD width in pixels */
#define LCD_WIDTH  240
/** @brief LCD height in pixels */
#define LCD_HEIGHT 280

/* -----------------------------
 * Touch Panel (FT6X36)
 * ----------------------------- */
/** @brief Touch Panel Reset pin */
#define TP_RST     13
/** @brief Touch Panel Interrupt pin */
#define TP_INT     14
/** @brief Touch Panel SDA (I2C data) */
#define TP_SDA     11
/** @brief Touch Panel SCL (I2C clock) */
#define TP_SCL     10

/* -----------------------------
 * I²C Bus (shared with sensors)
 * ----------------------------- */
/** @brief Generic I2C SDA */
#define IIC_SDA    11
/** @brief Generic I2C SCL */
#define IIC_SCL    10

/* -----------------------------
 * USB
 * ----------------------------- */
/** @brief USB D- pin */
#define USB_DN     20
/** @brief USB D+ pin */
#define USB_DP     19

/* -----------------------------
 * UART (Serial0)
 * ----------------------------- */
/** @brief UART0 TX */
#define UART_TX    43
/** @brief UART0 RX */
#define UART_RX    44

/* -----------------------------
 * Buzzer
 * ----------------------------- */
/** @brief Buzzer pin */
#define BUZZER     34

/* -----------------------------
 * QMI8658C (IMU)
 * ----------------------------- */
/** @brief IMU SDA pin */
#define QMI_SDA    12
/** @brief IMU SCL pin */
#define QMI_SCL    9

/* -----------------------------
 * RTC
 * ----------------------------- */
/** @brief RTC SDA pin */
#define RTC_SDA    11
/** @brief RTC SCL pin */
#define RTC_SCL    10
/** @brief RTC Interrupt pin */
#define RTC_INT    41

/* -----------------------------
 * System Control
 * ----------------------------- */
/** @brief System Enable pin */
#define SYS_EN     35
/** @brief System Output pin */
#define SYS_OUT    36
/** @brief Battery ADC sense pin */
#define BAT_ADC    1

/* -----------------------------
 * General GPIOs (Test points)
 * ----------------------------- */
/** @brief Test Point GPIO2 */
#define TP_GPIO2   2
/** @brief Test Point GPIO3 */
#define TP_GPIO3   3
/** @brief Test Point GPIO17 */
#define TP_GPIO17  17
/** @brief Test Point GPIO18 */
#define TP_GPIO18  18
/** @brief Test Point GPIO38 */
#define TP_GPIO38  38
