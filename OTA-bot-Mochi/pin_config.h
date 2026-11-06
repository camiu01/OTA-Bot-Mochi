/** @file pin_config.h @brief Select one hardware profile; modules depend on capabilities, not board names. */
#pragma once
#if (defined(MOCHI_BOARD_AMOLED_18) + defined(MOCHI_BOARD_LCD_169) + defined(MOCHI_BOARD_PROFILE)) > 1
#error "Select only one Mochi board profile"
#endif
#if defined(MOCHI_BOARD_PROFILE)
#include MOCHI_BOARD_PROFILE
#elif defined(MOCHI_BOARD_LCD_169)
#include "src/boards/lcd-169.h"
#else
// Preserve the existing Arduino IDE default.
#ifndef MOCHI_BOARD_AMOLED_18
#define MOCHI_BOARD_AMOLED_18
#endif
#include "src/boards/amoled-18-v2.h"
#endif
#ifndef MOCHI_BOARD_LCD_169
#define MOCHI_BOARD_LCD_169 0
#endif
#include "src/boards/validate.h"
