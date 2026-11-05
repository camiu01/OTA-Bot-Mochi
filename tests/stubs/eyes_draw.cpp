/**
 * @file eyes_draw.cpp
 * @brief Native animation-state tests do not rasterize the eye geometry.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "../../OTA-bot-Mochi/src/eyes/eyes_api.h"
#include "../../OTA-bot-Mochi/src/eyes/eye_life.h"
#include "../../OTA-bot-Mochi/src/eyes/face_runtime.h"
/** @brief Leave eye config/state available for assertions without a physical display. */
void eyes_draw() {
	const auto &pose = eyes_life_frame();
	eye_effect_elapsed(eyes_state(), pose.active ? pose.expression : eyes_config().expression);
}
