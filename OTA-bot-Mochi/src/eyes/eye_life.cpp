/**
 * @file eye_life.cpp
 * @brief Dashboard/night-safe animation intensity around a portable motion policy.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "eye_life.h"
#include "eyes_api.h"
#include "../settings/settings.h"
#include <Arduino.h>
static EyeLife life;
/** @brief Reset visual state for direct/native drawing and new sessions. */
void eyes_life_reset() { life.reset(); }
/** @brief Advance expressive pose at bounded dashboard intensity. @param now Time. @param dt Delta. */
void eyes_life_step(uint32_t now, uint32_t dt) {
	const auto &p = settings_get();
	float amount = p.dashboard ? 0.55f : (p.animation == 2 ? 1.2f : 1.0f);
	if (settings_night()) { amount *= 0.7f; }
	life.step(eyes_config(), eyes_state(), now, dt, p.animation, amount, uint32_t(random(1000000L)));
}
/** @brief Get the immutable renderer pose. @return Pose. */
const EyeLifeFrame &eyes_life_frame() { return life.get(); }
