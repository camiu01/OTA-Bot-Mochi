#pragma once

#define XPOWERS_CHIP_AXP2101

/**
 * @file pins.h
 * @brief Pin definitions for Waveshare ESP32-S3-Touch-LCD-1.69.
 *
 * @details
 * Sections index (search these tags in your editor):
 *   [S1] LCD
 *   [S2] Touch Panel
 *   [S3] I²C Bus
 *   [S4] USB
 *   [S5] UART
 *   [S6] Buzzer
 *   [S7] IMU QMI8658C
 *   [S8] RTC
 *   [S9] System Control & Power
 *   [S10] Extra GPIO / Test Points
 */

/* ==================================== [S1] LCD ==================================== */
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
#define LCD_WIDTH  240 /**< LCD width in pixels */
#define LCD_HEIGHT 280 /**< LCD height in pixels */
/** @} */  // end of lcd

/* ================================= [S2] Touch Panel ================================ */
/** @defgroup touch Touch Panel configuration
 *  @brief FT6X36 capacitive touch controller pins.
 *  @{
 */
#define TP_RST     13  /**< Touch Reset */
#define TP_INT     14  /**< Touch Interrupt */
#define TP_SDA     11  /**< Touch SDA (I2C data) */
#define TP_SCL     10  /**< Touch SCL (I2C clock) */
/** @} */  // end of touch

/* ================================== [S3] I²C Bus ================================== */
/** @defgroup i2c I2C Bus
 *  @brief Shared I²C lines for sensors.
 *  @{
 */
#define IIC_SDA    11  /**< Generic I2C SDA */
#define IIC_SCL    10  /**< Generic I2C SCL */
/** @} */  // end of i2c

/* =================================== [S4] USB ===================================== */
/** @defgroup usb USB
 *  @brief Native USB interface pins.
 *  @{
 */
#define USB_DN     20  /**< USB D- */
#define USB_DP     19  /**< USB D+ */
/** @} */  // end of usb

/* ================================== [S5] UART ===================================== */
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
#define BUZZER     34  /**< Buzzer pin */
/** @} */  // end of buzzer

/* =============================== [S7] IMU QMI8658C ================================ */
/** @defgroup imu IMU
 *  @brief QMI8658C accelerometer/gyroscope I²C pins.
 *  @{
 */
#define QMI_SDA    12  /**< IMU SDA */
#define QMI_SCL    9   /**< IMU SCL */
/** @} */  // end of imu

/* ================================== [S8] RTC ====================================== */
/** @defgroup rtc RTC
 *  @brief Real-Time Clock I²C + interrupt.
 *  @{
 */
#define RTC_SDA    11  /**< RTC SDA */
#define RTC_SCL    10  /**< RTC SCL */
#define RTC_INT    41  /**< RTC Interrupt */
/** @} */  // end of rtc

/* =========================== [S9] System Control & Power =========================== */
/** @defgroup sys System & Power
 *  @brief Power management and battery monitoring.
 *  @{
 */
#define SYS_EN     35  /**< System enable */
#define SYS_OUT    36  /**< System output */
#define BAT_ADC    1   /**< Battery ADC sense */
/** @} */  // end of sys

/* ============================== [S10] Extra GPIO ================================== */
/** @defgroup gpio_extra Extra GPIO
 *  @brief Test points and exposed GPIO pins.
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
