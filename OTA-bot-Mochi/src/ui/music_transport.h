/**
 * @file music_transport.h
 * @brief Main-task Chronos media command dispatch.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include "ui_state.h"

/** @brief Forward a supported queued command using Chronos's actual wire vocabulary. @param watch Android transport. */
inline void music_android_poll(ChronosESP32 &watch) {
	MusicAction action = ui_music_take();
	if (action == MusicAction::Toggle) { watch.musicControl(MUSIC_TOGGLE); }
	else if (action == MusicAction::Next) { watch.musicControl(MUSIC_NEXT); }
	else if (action == MusicAction::Previous) { watch.musicControl(MUSIC_PREVIOUS); }
	else if (action == MusicAction::VolumeUp) { watch.musicControl(VOLUME_UP); }
	else if (action == MusicAction::VolumeDown) { watch.musicControl(VOLUME_DOWN); }
}
