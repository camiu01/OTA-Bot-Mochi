/** @file amoled-18-v2.h @brief Hardware profile; preserve verified wiring and persistent identities. */
#pragma once
#define MOCHI_BOARD_NAME "AMOLED 1.8 V2"
#define MOCHI_BOARD_ID "amoled-18-v2"
#define MOCHI_BATTERY_EMPTY_PERCENT 3
#define MOCHI_BATTERY_FULL_PERCENT 92
#define MOCHI_IMU_CALIBRATION_ID 2
#define MOCHI_DISPLAY_ST7789 0
#define MOCHI_DISPLAY_CO5300 1
#define MOCHI_HAS_AXP2101 1
#define MOCHI_HAS_ES8311 1
#define MOCHI_HAS_SD_AUDIO 1
#define MOCHI_POWER_LATCH 0
#define MOCHI_PANEL_EXPANDER 1
#define MOCHI_CHRONOS_SCREEN CS_410x494_200_RTF
#define MOCHI_PANEL_OFFSET_X 16
#define MOCHI_PANEL_OFFSET_Y 0
#define MOCHI_OTA_MARKER "MOCHI-OTA-v1:amoled-18-v2:16m-v1:END"
#define MOCHI_OTA_FILENAME "mochi-amoled-18-v2-app.bin"
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
#define TP_RST     -1  /**< No touch reset pad on the V2 board */
#define TP_INT     21  /**< Touch Interrupt */
#define TP_SDA     15  /**< Touch SDA (I2C data), shared bus */
#define TP_SCL     14  /**< Touch SCL (I2C clock), shared bus */
#define IIC_SDA    15  /**< Generic I2C SDA */
#define IIC_SCL    14  /**< Generic I2C clock */
#define USB_DN     20  /**< USB D- */
#define USB_DP     19  /**< USB D+ */
#define UART_TX    43  /**< UART0 TX */
#define UART_RX    44  /**< UART0 RX */
#define SDMMC_CLK  2
#define SDMMC_CMD  1
#define SDMMC_DATA 3
#define BUZZER     -1  /**< No PWM buzzer: audio goes through the ES8311 codec */
#define QMI_SDA    15  /**< IMU SDA, shared with touch and RTC */
#define QMI_SCL    14  /**< IMU SCL, shared with touch and RTC */
#define RTC_SDA    15  /**< RTC SDA, shared bus */
#define RTC_SCL    14  /**< RTC SCL, shared bus */
#define RTC_INT    -1  /**< Not wired to a known pad on the V2 board; unused by firmware */
#define SYS_EN     -1  /**< Unused: power rails are managed by the AXP2101 PMU */
#define SYS_OUT    -1  /**< Unused: power rails are managed by the AXP2101 PMU */
#define BAT_ADC    -1  /**< No ADC divider: battery data would come from the AXP2101 */
#define BOOT_BUTTON 0 /**< Active-low BOOT key; runtime page shortcut only. */
#define IO_EXPANDER_ADDR 0x20 /**< TCA9554 I2C address (A2/A1/A0 low) */
#define IO_EXPANDER_RESET_PINS 0x07 /**< Expander bits 0..2 gate the panel reset */
#define I2S_MCK_IO   16  /**< ES8311 master clock */
#define I2S_BCK_IO   9   /**< ES8311 bit clock */
#define I2S_WS_IO    45  /**< ES8311 word select */
#define I2S_DI_IO    10  /**< ES8311 data in (microphone) */
#define I2S_DO_IO    8   /**< ES8311 data out (speaker) */
#define AUDIO_PA     46  /**< Speaker amplifier enable */
