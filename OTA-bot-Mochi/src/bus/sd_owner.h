/**
 * @file sd_owner.h
 * @brief Exclusive SDMMC ownership across the audio worker and firmware maintenance.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <atomic>
#include <stdint.h>

enum class SdOwner : uint8_t { None, Audio, Update };
inline std::atomic<SdOwner> sd_owner{SdOwner::None};
/** @brief Claim an unowned card without blocking another task. @param owner Intended user. @return Acquired. */
inline bool sd_acquire(SdOwner owner) {
	SdOwner expected = SdOwner::None;
	return owner != SdOwner::None && sd_owner.compare_exchange_strong(expected, owner);
}
/** @brief Release only the caller's lease, after closing files and unmounting. @param owner Current user. */
inline void sd_release(SdOwner owner) {
	if (owner != SdOwner::None) { sd_owner.compare_exchange_strong(owner, SdOwner::None); }
}
