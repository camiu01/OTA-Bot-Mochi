/** @file validate.h @brief Reject unsupported profile combinations before hardware initialization. */
#pragma once
#if (MOCHI_DISPLAY_ST7789 + MOCHI_DISPLAY_CO5300) != 1
#error "Select exactly one supported display driver"
#endif
#if TP_SDA != IIC_SDA || TP_SCL != IIC_SCL || QMI_SDA != IIC_SDA || QMI_SCL != IIC_SCL || RTC_SDA != IIC_SDA || RTC_SCL != IIC_SCL
#error "Touch, IMU and RTC must use the shared I2C bus"
#endif
#if MOCHI_HAS_SD_AUDIO && !MOCHI_HAS_ES8311
#error "SD audio requires the ES8311 backend"
#endif
#if MOCHI_HAS_ES8311 && BUZZER >= 0
#error "Select either ES8311 or a PWM buzzer"
#endif
#if LCD_WIDTH <= 0 || LCD_HEIGHT <= 0
#error "Panel geometry must be positive"
#endif
#if MOCHI_IMU_CALIBRATION_ID < 0 || MOCHI_IMU_CALIBRATION_ID > 254 || (MOCHI_IMU_CALIBRATION_ID & 1)
#error "Calibration identity must be a unique even byte"
#endif
#if !MOCHI_HAS_AXP2101 && !MOCHI_POWER_LATCH
#error "Select an implemented power-button backend"
#endif
#if MOCHI_HAS_AXP2101 && MOCHI_POWER_LATCH
#error "Select only one power-button backend"
#endif

#if MOCHI_BATTERY_EMPTY_PERCENT < 0 || MOCHI_BATTERY_FULL_PERCENT > 100 || MOCHI_BATTERY_EMPTY_PERCENT >= MOCHI_BATTERY_FULL_PERCENT
#error "Battery curve endpoints must be increasing percentages"
#endif
#ifndef MOCHI_BOARD_NAME
#error "Profile must define a board name"
#endif
#ifndef MOCHI_OTA_MARKER
#error "Profile must define a unique board/layout OTA marker"
#endif
#ifndef MOCHI_OTA_FILENAME
#error "Profile must define its release filename"
#endif
