/**
 * @file es8311_output.h
 * @brief Lazy, playback-only ES8311 backend for the AMOLED V2 board.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <stdint.h>
#include "pcm_sequence.h"
/** @brief Configure the shared-bus codec and start the PCM worker, initially silent. @return Success. */
bool es8311_begin();
/** @brief Publish one bounded tone without waiting for DMA. @param hz Hertz, or zero. @param percent Amplitude 0..30. @return Success. */
bool es8311_write(uint16_t hz, uint8_t percent);
/** @brief Copy one complete cue for sample-clocked playback, without main-loop refresh. @param notes Notes. @param count Count, at most five. @param percent Amplitude. @return Accepted. */
bool es8311_play(const SoundNote *notes, uint8_t count, uint8_t percent);
/** @brief Include queued notes and the bounded DMA drain in playback state. @return Busy. */
bool es8311_playing();
/** @brief Mute the amplifier immediately; retain the lazy worker for reuse. */
void es8311_end();
/** @brief Read the PCM worker's latched transport failure. @return Healthy. */
bool es8311_healthy();
