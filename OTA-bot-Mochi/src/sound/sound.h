/**
 * @file sound.h
 * @brief Main-task, non-blocking optional chirps with priority, mute, and rate limits.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <stdint.h>
#include "../ui/ui_state.h"
enum class SoundCue : uint8_t { Ambient, Mood, Tap, Notification, Rest, Done, Call, Test, Count };
enum class SoundStatus : uint8_t { Off, Night, Armed, Ready, Error };
/** @brief Reset session audio without changing preferences. @param now Time. */
void sound_init(uint32_t now);
/** @brief Request one short cue, dropping disallowed/stale/lower-priority work. @param cue Cue. @param now Time. */
void sound_request(SoundCue cue, uint32_t now);
/** @brief Service events and notes from the main loop. @param now Time. @param events Coherent BLE counters. @param eyesVisible Ambient context. @param resting Timer phase. @param done Timer finished. */
void sound_poll(uint32_t now, const UiPageEvents &events, bool eyesVisible, bool resting, bool done);
/** @brief Report configuration/backend status, not physical audibility. @return Status. */
SoundStatus sound_status();
/** @brief Stable USB diagnostic name; not a measurement of audibility. @return Status label. */
const char *sound_status_name();
