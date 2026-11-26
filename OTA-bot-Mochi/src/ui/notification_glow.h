/** @file notification_glow.h @brief Animated, multicolor perimeter light for Mochi alerts. */
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "panel_edge.h"
#include "ui_theme.h"

namespace notification_glow {
constexpr bool Supported = panel_edge::Supported;
constexpr uint32_t PeriodMs = 4400;
constexpr uint32_t FadeInMs = 100, FadeOutMs = 650, FadeStartMs = 10000;
constexpr uint32_t DurationMs = FadeStartMs + FadeOutMs;

struct Frame {
	uint32_t elapsed = 0;
	uint8_t opacity = 0;
	bool refresh = false;
};

/** @brief Follow only arrivals, with one last clean frame after the fade. */
class Pulse {
	uint32_t seen_ = 0, start_ = 0;
	bool active_ = false, clear_pending_ = false;

public:
	/** @brief Clear a visible ring on tap, while letting future notifications start a new one. */
	void dismiss(uint32_t notifications) {
		if (!Supported) { return; }
		seen_ = notifications;
		clear_pending_ = clear_pending_ || active_;
		active_ = false;
	}

	Frame update(uint32_t notifications, uint32_t now) {
		if (!Supported) { return {}; }
		const bool clear = clear_pending_;
		clear_pending_ = false;
		if (notifications != seen_) {
			seen_ = notifications;
			start_ = now;
			active_ = true;
		}
		if (!active_) { return {0, 0, clear}; }
		const uint32_t elapsed = now - start_;
		if (elapsed >= DurationMs) { active_ = false; return {elapsed, 0, true}; }
		const uint32_t remaining = DurationMs - elapsed;
		const uint32_t opacity = elapsed < FadeInMs ? elapsed * 255 / FadeInMs :
			(remaining < FadeOutMs ? remaining * 255 / FadeOutMs : 255);
		return {elapsed, uint8_t(opacity), true};
	}
};

#if MOCHI_DISPLAY_CO5300 && LCD_WIDTH == 368 && LCD_HEIGHT == 448
namespace detail {
using Path = panel_edge::Path<4, 36, 15>;

/** @brief A narrow bright core surrounded by a softer, asymmetric halo. */
inline uint8_t opacity(float inward) {
	const float distance = std::fabs(inward);
	const float reach = inward < 0 ? 4.0f : 14.0f;
	if (distance >= reach) { return 0; }
	const float falloff = 1.0f - distance / reach;
	const float core = distance < 2.2f ? 159.0f * (1.0f - distance / 2.2f) : 0.0f;
	return uint8_t(core + 96.0f * falloff * falloff * falloff);
}

constexpr uint16_t rgb(uint8_t r, uint8_t g, uint8_t b) {
	return uint16_t(((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3));
}

/** @brief Interpolate a seamless palette from cyan through indigo, violet and coral. */
inline uint16_t color(uint8_t phase) {
	constexpr uint16_t colors[] = {rgb(35, 225, 255), rgb(77, 104, 255),
		rgb(151, 77, 248), rgb(255, 125, 110), rgb(28, 195, 220)};
	constexpr unsigned stops = sizeof(colors) / sizeof(colors[0]);
	const unsigned step = unsigned(phase) * stops;
	const unsigned index = step >> 8;
	return ui_mix(colors[index], colors[(index + 1) % stops], step & 255);
}

/** @brief Measure a highlight on a circular timeline with a smooth trailing edge. */
inline int highlight(uint8_t offset, int reach, int strength) {
	const int distance = offset > 128 ? 256 - offset : offset;
	if (distance >= reach) { return 0; }
	const int falloff = reach - distance;
	return falloff * falloff * strength / (reach * reach);
}

struct Light { uint16_t color; uint8_t intensity; };

inline const Path &path() {
	static const Path value(opacity);
	return value;
}

inline uint16_t &pixel(uint16_t *pixels, int x, int y, bool flipped) {
	const int pos = y * LCD_WIDTH + x;
	return pixels[flipped ? LCD_WIDTH * LCD_HEIGHT - 1 - pos : pos];
}
}

constexpr size_t BackupPixels = detail::Path::MaxSamples;

/** @brief Composite onto a fresh frame, optionally saving affected pixels for restoration. */
inline void draw(uint16_t *pixels, bool flipped, uint32_t elapsed, uint8_t alpha = 255,
	uint16_t *backup = nullptr) {
	if (!pixels || !alpha) { return; }
	const uint8_t offset = uint8_t((elapsed % PeriodMs) * 256 / PeriodMs);
	detail::Light light[256];
	for (unsigned i = 0; i < 256; ++i) {
		const uint8_t phase = uint8_t(i - offset);
		const int peak = detail::highlight(phase, 46, 74) + detail::highlight(uint8_t(phase - 131), 31, 46);
		light[i] = {ui_mix(detail::color(phase), detail::rgb(230, 244, 255), peak),
			uint8_t((168 + peak) * alpha / 255)};
	}
	detail::path().visit([&](int x, int y, uint8_t weight, int along) {
		const auto &sample = light[(along * 256 / detail::Path::Perimeter) & 255];
		auto &pixel = detail::pixel(pixels, x, y, flipped);
		if (backup) { *backup++ = pixel; }
		pixel = ui_mix(pixel, sample.color, (uint16_t(weight) * sample.intensity) >> 8);
	});
}

/** @brief Restore the unmodified canvas after the display transfer completes. */
inline void restore(uint16_t *pixels, bool flipped, const uint16_t *backup) {
	detail::path().visit([&](int x, int y, uint8_t, int) {
		detail::pixel(pixels, x, y, flipped) = *backup++;
	});
}
#else
constexpr size_t BackupPixels = 0;
inline void draw(uint16_t *, bool, uint32_t, uint8_t = 255, uint16_t * = nullptr) {}
inline void restore(uint16_t *, bool, const uint16_t *) {}
#endif
}
