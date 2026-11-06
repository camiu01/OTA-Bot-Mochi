/** @file lcd-169.h @brief Hardware profile; preserve verified wiring and persistent identities. */
#pragma once
#define MOCHI_BOARD_NAME "LCD 1.69 OLD"
#define MOCHI_BOARD_ID "lcd-169"
#define MOCHI_BATTERY_EMPTY_PERCENT 0
#define MOCHI_BATTERY_FULL_PERCENT 100
#define MOCHI_IMU_CALIBRATION_ID 0
#define MOCHI_DISPLAY_ST7789 1
#define MOCHI_DISPLAY_CO5300 0
#define MOCHI_HAS_AXP2101 0
#define MOCHI_HAS_ES8311 0
#define MOCHI_HAS_SD_AUDIO 0
#define MOCHI_POWER_LATCH 1
#define MOCHI_PANEL_EXPANDER 0
#define MOCHI_CHRONOS_SCREEN CS_240x283_172_RTF
#define MOCHI_PANEL_OFFSET_X 0
#define MOCHI_PANEL_OFFSET_Y 20
#define MOCHI_OTA_MARKER "MOCHI-OTA-v1:lcd-169:16m-v1:END"
#define MOCHI_OTA_FILENAME "mochi-lcd-169-app.bin"
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
#define TP_RST     13  /**< Touch Reset */
#define TP_INT     14  /**< Touch Interrupt */
#define TP_SDA     11  /**< Touch SDA (I2C data) */
#define TP_SCL     10  /**< Touch SCL (I2C clock) */
#define IIC_SDA    11  /**< Generic I2C SDA */
#define IIC_SCL    10  /**< Generic I2C SCL */
#define USB_DN     20  /**< USB D- */
#define USB_DP     19  /**< USB D+ */
#define UART_TX    43  /**< UART0 TX */
#define UART_RX    44  /**< UART0 RX */
#define BUZZER     33  /**< Buzzer pin (OLD board net "Buzz"; the new revision uses GPIO42) */
#define QMI_SDA    11  /**< IMU SDA, shared with touch and RTC */
#define QMI_SCL    10  /**< IMU SCL, shared with touch and RTC */
#define RTC_SDA    11  /**< RTC SDA */
#define RTC_SCL    10  /**< RTC SCL */
#define RTC_INT    41  /**< RTC Interrupt */
#define SYS_EN     35  /**< Original board: active-high battery-power hold (SS8050/AO3401). */
#define SYS_OUT    36  /**< Original board: active-low PWR/Key2 input. */
#define BAT_ADC    1   /**< Unverified ADC candidate; do not assume a battery divider ratio. */
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
#define BOOT_BUTTON -1
