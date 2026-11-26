/** @file alert_layout.h @brief Shared geometry for the three notification actions. */
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "ui_theme.h"

namespace alert_layout {
inline int16_t button_top() { return ui_s(222); }
inline int16_t button_height() { return ui_s(38); }
inline int16_t button_bottom() { return button_top() + button_height(); }
inline int16_t button_left(uint8_t index) { return ui_s(12 + index * 74); }
inline int16_t button_width() { return ui_s(68); }
}
