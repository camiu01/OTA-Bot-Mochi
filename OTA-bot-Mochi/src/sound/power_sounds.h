/**
 * @file power_sounds.h
 * @brief Shared ascending charge-start and descending unplug melodies for playback and WAV export.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "sound.h"
#include "pcm_sequence.h"

/** @brief Build one of twenty power melodies. @param connected Charge start, otherwise unplug. @param variant Zero-based variant. @param out Three notes including a pause. */
inline void power_sound_notes(bool connected, uint8_t variant, SoundNote (&out)[3]) {
	variant %= SOUND_POWER_VARIANTS;
	uint8_t contour = variant / 5;
	uint16_t hz = (connected ? 1400 : 2400) + (variant % 5) * 100;
	uint16_t rise = 400 + contour * 100;
	out[0] = {hz, uint16_t(70 + contour * 10)};
	out[1] = {0, 30};
	out[2] = {uint16_t(connected ? hz + rise : hz - rise), 100};
}

/** @brief Encode a power melody for the SD worker. @param connected Charge start, otherwise unplug. @param variant Valid zero-based variant. @return Fixed SD ID. */
constexpr uint8_t power_sound_file(bool connected, uint8_t variant) {
	return SOUND_SD_POWER_BASE + (connected ? 0 : SOUND_POWER_VARIANTS) + variant;
}
static_assert(SOUND_SD_FACE_BASE + uint8_t(FaceTone::Count) * SOUND_FACE_VARIANTS <= SOUND_SD_POWER_BASE,
	"Keep face and power file IDs separate");
static_assert(SOUND_SD_POWER_BASE + 2 * SOUND_POWER_VARIANTS < 255, "Preserve no-SD sentinel");
