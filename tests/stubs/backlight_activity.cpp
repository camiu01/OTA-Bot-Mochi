/** @file backlight_activity.cpp @brief Main-task wake substitute for companion tests without the PWM driver. */
#include "../../OTA-bot-Mochi/src/power/backlight.h"
/** @brief Accept a wake event without simulating physical illumination. @param now Timestamp. */
void backlight_activity(uint32_t now) { (void)now; }
