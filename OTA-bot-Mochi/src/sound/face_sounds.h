/**
 * @file face_sounds.h
 * @brief Shared eye-only melodies for generated playback and native WAV export.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "sound.h"
#include "pcm_sequence.h"

/** @brief Build a bounded two-note melody with five pitches and four contours/rhythms. @param tone Family. @param variant Zero-based variant. @param out Three notes, including a pause. */
inline void face_sound_notes(FaceTone tone, uint8_t variant, SoundNote (&out)[3]) {
	static const uint16_t base[] = {2300, 1600, 1000, 2600, 800};
	static const uint16_t span[] = {700, 500, 300, 700, 250};
	uint8_t family = uint8_t(tone) < uint8_t(FaceTone::Count) ? uint8_t(tone) : 0;
	variant %= SOUND_FACE_VARIANTS;
	uint8_t contour = variant / 5;
	uint16_t hz = base[family] + span[family] * (variant % 5) / 5;
	out[0] = {hz, uint16_t(tone == FaceTone::Soft ? 100 : 55)};
	out[1] = {0, 25};
	out[2] = {uint16_t(hz + (tone == FaceTone::Flat ? 0 : 200 + contour * 100)),
		uint16_t((tone == FaceTone::Bounce ? 50 : 70) + contour * 10)};
}

/** @brief Encode a family and zero-based variant for the SD worker. @param tone Valid family. @param variant Valid variant. @return Fixed file ID. */
constexpr uint8_t face_sound_file(FaceTone tone, uint8_t variant) {
	return SOUND_SD_FACE_BASE + uint8_t(tone) * SOUND_FACE_VARIANTS + variant;
}
static_assert(SOUND_SD_FACE_BASE + uint8_t(FaceTone::Count) * SOUND_FACE_VARIANTS < 255,
	"Keep face file IDs below the no-SD sentinel");
