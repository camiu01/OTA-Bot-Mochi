/** @file save_glow.h @brief RGB565 edge lighting for a successful settings save. */
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "panel_edge.h"

namespace save_glow {
constexpr bool Supported = panel_edge::Supported;
constexpr uint32_t SweepMs = 500, HoldMs = 350, FadeMs = 300;
constexpr uint32_t Duration = SweepMs + HoldMs + FadeMs;

/** @brief Shared fade envelope, from fully opaque (256) to invisible. */
inline uint16_t fade(uint32_t elapsed) {
	if (elapsed >= Duration) { return 0; }
	return elapsed < SweepMs + HoldMs ? 256 : uint16_t((Duration - elapsed) * 256 / FadeMs);
}

#if MOCHI_DISPLAY_CO5300 && LCD_WIDTH == 368 && LCD_HEIGHT == 448
namespace detail {
using Path = panel_edge::Path<8, 32, 12>;

/** @brief Bright core with a short outer halo and a softer glow inward. */
inline uint8_t opacity(float inward) {
	const float distance = std::fabs(inward);
	const float halo = inward < 0 ? 6.0f : 11.0f;
	if (distance >= halo) { return 0; }
	const float falloff = 1.0f - distance / halo;
	const float core = distance < 2.5f ? 170.0f * (1.0f - distance / 2.5f) : 0.0f;
	return uint8_t(core + 85.0f * falloff * falloff);
}

/** @brief Blend mint-green light over the existing RGB565 pixel. */
inline void blend(uint16_t &pixel, uint16_t alpha, uint16_t mint) {
	const uint16_t keep = 256 - alpha;
	const uint16_t r = (((pixel >> 11) * keep + (5 + mint) * alpha + 128) >> 8) << 11;
	const uint16_t g = ((((pixel >> 5) & 63) * keep + 63 * alpha + 128) >> 8) << 5;
	const uint16_t b = (((pixel & 31) * keep + (11 + mint) * alpha + 128) >> 8);
	pixel = r | g | b;
}

struct Frame {
	uint16_t *pixels;
	bool flipped;
	int head;
	uint16_t fade;

	/** @brief Paint one sample behind the advancing tip, with a brighter leading edge. */
	void operator()(int x, int y, uint8_t opacity, int along) const {
		if (!opacity || along > head) { return; }
		const int trail = head - along;
		const int tip = trail < 72 ? 72 - trail : 0;
		const int cap = trail < 12 ? 96 + 13 * trail : 256;
		const uint16_t alpha = uint32_t(opacity) * fade * cap / 65536;
		if (!alpha) { return; }
		int pos = y * LCD_WIDTH + x;
		if (flipped) { pos = LCD_WIDTH * LCD_HEIGHT - 1 - pos; }
		blend(pixels[pos], alpha, 3 + tip / 6);
	}
};
}

/** @brief Grow the rounded ring clockwise from the bottom-left, hold and fade. */
inline void draw(uint16_t *pixels, bool flipped, uint32_t elapsed) {
	if (!pixels || elapsed >= Duration) { return; }
	static const detail::Path path(detail::opacity);
	constexpr int perimeter = detail::Path::Perimeter;
	const int head = elapsed < SweepMs ? int(elapsed * perimeter / SweepMs) + 12 : perimeter + 72;
	path.visit(detail::Frame{pixels, flipped, head, fade(elapsed)});
}
#else
inline void draw(uint16_t *, bool, uint32_t) {}
#endif
}
