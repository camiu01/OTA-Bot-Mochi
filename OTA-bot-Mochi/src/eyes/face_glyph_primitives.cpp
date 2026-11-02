/**
 * @file face_glyph_primitives.cpp
 * @brief Bounded face symbols with readable silhouettes and panel-scaled strokes.
 */
#include "face_support.h"
namespace face_primitives {

/** @brief Shared strip drawing primitive. */
void strip(Arduino_GFX *gfx, const EyeBox &b, int16_t height, bool bottom, uint16_t color) {
	height = eye_limit(height, 0, b.h - 2);
	if (!height) { return; }
	gfx->fillRect(b.x, bottom ? b.y + b.h - height : b.y, b.w, height, color);
}

/** @brief Shared slope drawing primitive. */
void slope(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b, bool angry, int16_t depth) {
	int16_t x = (b.left == angry) ? b.x + b.w - 1 : b.x;
	gfx->fillTriangle(b.x, b.y, b.x + b.w - 1, b.y, x, b.y + depth, cfg.bgColor);
}

/** @brief Shared smile drawing primitive. */
void smile(Arduino_GFX *gfx, const EyeConfig &cfg, const EyeBox &b, int16_t depth, int16_t rise) {
	for (int16_t i = 0; i < b.w; ++i) {
		int32_t t = 2 * i - b.w + 1;
		int16_t cut = depth + rise - int16_t(t * t * rise / (int32_t(b.w) * b.w));
		cut = eye_limit(cut, 0, b.h - 2);
		gfx->fillRect(b.x + i, b.y + b.h - cut, 1, cut, cfg.bgColor);
	}
}

/** @brief Shared heart drawing primitive. */
void heart(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t s, uint16_t color) {
	gfx->fillCircle(x - s / 2, y - s / 4, s / 2, color);
	gfx->fillCircle(x + s / 2, y - s / 4, s / 2, color);
	gfx->fillTriangle(x - s, y, x + s, y, x, y + s, color);
}

/** @brief Shared cross drawing primitive. */
void cross(Arduino_GFX *gfx, const EyeBox &b, uint16_t color) {
	for (int8_t offset = -3; offset <= 3; ++offset) {
		gfx->drawLine(b.x + 4 + offset, b.y + 4, b.x + b.w - 5 + offset, b.y + b.h - 5, color);
		gfx->drawLine(b.x + b.w - 5 + offset, b.y + 4, b.x + 4 + offset, b.y + b.h - 5, color);
	}
}

/** @brief Shared star drawing primitive. */
void star(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t s, uint16_t color) {
	if (s < 2) { return; }
	int16_t px[10], py[10];
	for (int8_t i = 0; i < 10; ++i) {
		float angle = -1.5707963f + i * 0.62831853f;
		float radius = (i & 1) ? 0.42f * s : float(s);
		px[i] = int16_t(x + cosf(angle) * radius);
		py[i] = int16_t(y + sinf(angle) * radius);
	}
	for (int8_t i = 0; i < 10; ++i) {
		int8_t next = int8_t((i + 1) % 10);
		gfx->fillTriangle(x, y, px[i], py[i], px[next], py[next], color);
	}
}

/** @brief Bold spiral kept inside the glyph bounds, including its stroke. */
void spiral(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t s, uint16_t color) {
	if (s < 2) { return; }
	int16_t stroke = int16_t(s / 16 + 1);
	for (int16_t i = 0; i <= 96; ++i) {
		float angle = i * 0.1f;
		float radius = i * (s - stroke) / 96.0f;
		gfx->fillCircle(int16_t(x + cosf(angle) * radius), int16_t(y + sinf(angle) * radius), stroke, color);
	}
}

/** @brief Shared tear drawing primitive. */
void tear(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t s, uint16_t color) {
	if (s < 2) { s = 2; }
	gfx->fillCircle(x, y, s, color);
	gfx->fillTriangle(x - s, y, x + s, y, x, y + 2 * s, color);
}

/** @brief Shared tear_y drawing primitive. */
int16_t tear_y(const EyeBox &b) { return b.y + b.h * 2 / 3; }

/** @brief Shared annulus drawing primitive. */
void annulus(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t s, uint16_t color) {
	if (s < 2) { return; }
	for (int8_t ring = 0; ring < 2 && s - ring >= 2; ++ring) {
		int16_t px = int16_t(x + s - ring), py = y;
		for (int8_t i = 1; i <= 16; ++i) {
			float angle = i * 0.392699082f;
			int16_t nx = int16_t(x + (s - ring) * cosf(angle));
			int16_t ny = int16_t(y + (s - ring) * sinf(angle));
			gfx->drawLine(px, py, nx, ny, color);
			px = nx; py = ny;
		}
	}
}

/** @brief Shared halo drawing primitive. */
void halo(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t s, uint16_t color) {
	if (s < 3) { return; }
	annulus(gfx, x, int16_t(y - s / 4), int16_t(s * 3 / 4), color);
}

/** @brief Shared horns drawing primitive. */
void horns(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t s, uint16_t color) {
	if (s < 4) { return; }
	int16_t base = int16_t(y - s * 3 / 10);
	int16_t apex = int16_t(y - s * 9 / 10);
	int16_t outer = int16_t(x - s * 85 / 100);
	int16_t inner = int16_t(x - s / 4);
	int16_t tip = int16_t(x - s / 3);
	gfx->fillTriangle(outer, base, inner, base, tip, apex, color);
	gfx->fillTriangle(int16_t(2 * x - outer), base, int16_t(2 * x - inner), base,
		int16_t(2 * x - tip), apex, color);
}

/** @brief Lobed flame silhouette with a leaning tip and a smaller side tongue. */
void flame(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t s, uint16_t color) {
	if (s < 3) { return; }
	gfx->fillEllipse(x, y + s / 3, int16_t(s * 2 / 3), int16_t(s * 3 / 5), color);
	gfx->fillTriangle(x - s / 2, y + s / 3, x + s / 2, y + s / 3, x + s / 4, y - s, color);
	gfx->fillTriangle(x - s * 2 / 3, y + s / 3, x, y + s / 2, x - s * 3 / 5, y - s / 3, color);
	gfx->fillTriangle(x, y + s / 3, x + s * 2 / 3, y + s / 3, x + s / 2, y - s / 6, color);
}

/** @brief Open lunar crescent; subtraction is clipped to the original disc. */
void crescent(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t s, uint16_t color, uint16_t cut) {
	if (s < 3) { return; }
	int16_t shift = int16_t(s / 2), inner = int16_t(s * 9 / 10);
	for (int16_t row = -s; row <= s; ++row) {
		int16_t outer = int16_t(sqrtf(float(int32_t(s) * s - int32_t(row) * row)));
		gfx->fillRect(x - outer, y + row, 2 * outer + 1, 1, color);
		if (row < -inner || row > inner) { continue; }
		int16_t span = int16_t(sqrtf(float(int32_t(inner) * inner - int32_t(row) * row)));
		int16_t left = eye_limit(shift - span, -outer, outer + 1);
		int16_t right = eye_limit(shift + span, -outer - 1, outer);
		if (right >= left) { gfx->fillRect(x + left, y + row, right - left + 1, 1, cut); }
	}
}

/** @brief Shared sparkle drawing primitive. */
void sparkle(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t s, uint16_t color) {
	if (s < 2) { return; }
	int16_t thin = int16_t(s / 3 ? s / 3 : 1);
	gfx->fillTriangle(x, int16_t(y - s), int16_t(x - thin), y, int16_t(x + thin), y, color);
	gfx->fillTriangle(x, int16_t(y + s), int16_t(x - thin), y, int16_t(x + thin), y, color);
	gfx->fillTriangle(int16_t(x - s), y, x, int16_t(y - thin), x, int16_t(y + thin), color);
	gfx->fillTriangle(int16_t(x + s), y, x, int16_t(y - thin), x, int16_t(y + thin), color);
}

/** @brief Connected diagonal lightning silhouette with two pointed ends. */
void bolt(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t s, uint16_t color) {
	if (s < 4) { return; }
	int16_t wing = int16_t(s * 2 / 3);
	gfx->fillTriangle(int16_t(x + s / 3), int16_t(y - s), int16_t(x - wing),
		int16_t(y + s / 6), int16_t(x + s / 6), int16_t(y + s / 6), color);
	gfx->fillTriangle(int16_t(x - s / 3), int16_t(y + s), int16_t(x + wing),
		int16_t(y - s / 6), int16_t(x - s / 6), int16_t(y - s / 6), color);
}

/** @brief Shared rhombus drawing primitive. */
void rhombus(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t s, uint16_t color) {
	if (s < 2) { return; }
	int16_t wing = int16_t(s * 2 / 3);
	gfx->fillTriangle(x, int16_t(y - s), int16_t(x - wing), y, int16_t(x + wing), y, color);
	gfx->fillTriangle(x, int16_t(y + s), int16_t(x - wing), y, int16_t(x + wing), y, color);
}

/** @brief Shared crosshair drawing primitive. */
void crosshair(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t s, uint16_t color) {
	annulus(gfx, x, y, s, color);
	if (s < 3) { return; }
	gfx->drawLine(int16_t(x - s), y, int16_t(x + s), y, color);
	gfx->drawLine(x, int16_t(y - s), x, int16_t(y + s), color);
}

/** @brief Angular dollar sign with a legible S stroke and vertical bar. */
void money(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t s, uint16_t color) {
	if (s < 3) { return; }
	int16_t w = int16_t(s * 3 / 4), h = int16_t(s * 3 / 4);
	const int px[] = {w, -w / 2, -w, -w / 2, w / 2, w, w / 2, -w};
	const int py[] = {-h, -h, -h / 2, -h / 4, h / 4, h / 2, h, h};
	int16_t stroke = int16_t(s / 7 + 1);
	for (int16_t offset = 0; offset < stroke; ++offset) {
		for (uint8_t i = 1; i < 8; ++i) {
			gfx->drawLine(x + px[i - 1] + offset - stroke / 2, y + py[i - 1], x + px[i] + offset - stroke / 2, y + py[i], color);
			gfx->drawLine(x + px[i - 1], y + py[i - 1] + offset - stroke / 2, x + px[i], y + py[i] + offset - stroke / 2, color);
		}
	}
	gfx->fillRect(x - stroke / 2, y - s, stroke, 2 * s + 1, color);
}

/** @brief Shared glitch_glyph drawing primitive. */
void glitch_glyph(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t s, uint16_t color) {
	if (s < 3) { return; }
	int16_t h = int16_t(s / 3 ? s / 3 : 1);
	gfx->fillRect(int16_t(x - s), int16_t(y - s * 2 / 3), int16_t(s * 4 / 3), h, color);
	gfx->fillRect(int16_t(x - s / 3), int16_t(y - h / 2), int16_t(s * 4 / 3), h, color);
	gfx->fillRect(int16_t(x - s * 2 / 3), int16_t(y + s / 2), int16_t(s * 4 / 3), h, color);
}

/** @brief Shared hourglass drawing primitive. */
void hourglass(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t s, uint16_t color) {
	if (s < 3) { return; }
	int16_t w = int16_t(s * 3 / 4);
	gfx->fillTriangle(int16_t(x - w), int16_t(y - s), int16_t(x + w), int16_t(y - s), x, y, color);
	gfx->fillTriangle(int16_t(x - w), int16_t(y + s), int16_t(x + w), int16_t(y + s), x, y, color);
}

/** @brief Thick horizontal battery case with a terminal and one remaining charge bar. */
void low_batt(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t s, uint16_t color) {
	if (s < 4) { return; }
	int16_t w = int16_t(s * 3 / 2), h = int16_t(s * 6 / 5);
	int16_t stroke = eye_limit(int16_t(s / 8), 1, 8);
	int16_t left = int16_t(x - w / 2), top = int16_t(y - h / 2);
	for (int16_t i = 0; i < stroke; ++i) { gfx->drawRect(left + i, top + i, w - 2 * i, h - 2 * i, color); }
	gfx->fillRect(left + w, y - h / 4, stroke + 1, h / 2, color);
	int16_t charge = int16_t(h - 2 * stroke - 2);
	if (charge > 0 && s >= 6) { gfx->fillRect(left + stroke + 1, top + stroke + 1, stroke + 1, charge, color); }
}

/** @brief Paired eighth notes with thick stems and a slanted connecting beam. */
void music_note(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t s, uint16_t color) {
	if (s < 3) { return; }
	int16_t r = int16_t(s / 3), stroke = int16_t(s / 7 + 1);
	int16_t left = int16_t(x - s / 2), right = int16_t(x + s / 2);
	gfx->fillEllipse(left, y + s / 2, r, int16_t(r * 2 / 3 + 1), color);
	gfx->fillEllipse(right, y + s / 3, r, int16_t(r * 2 / 3 + 1), color);
	gfx->fillRect(left + r / 2, y - s * 2 / 3, stroke, s * 7 / 6, color);
	gfx->fillRect(right + r / 2, y - s, stroke, s * 4 / 3, color);
	for (int16_t i = 0; i < stroke * 2; ++i) {
		gfx->drawLine(left + r / 2, y - s * 2 / 3 + i, right + r / 2 + stroke - 1, y - s + i, color);
	}
}

/** @brief Open power ring with a separated central switch stem. */
void power_sym(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t s, uint16_t color) {
	if (s < 3) { return; }
	int16_t stroke = int16_t(s / 12 + 1), radius = int16_t(s * 2 / 3);
	for (int16_t i = 0; i <= 48; ++i) {
		float angle = 0.7f + i * (6.2831853f - 1.4f) / 48;
		gfx->fillCircle(int16_t(x + sinf(angle) * radius), int16_t(y - cosf(angle) * radius), stroke, color);
	}
	gfx->fillRect(x - stroke / 2, y - s, stroke + 1, s, color);
}

/** @brief Shared exclaim drawing primitive. */
void exclaim(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t s, uint16_t color) {
	if (s < 3) { return; }
	int16_t w = int16_t(s / 3 ? s / 3 : 1);
	gfx->fillRoundRect(int16_t(x - w / 2), int16_t(y - s), w, int16_t(s * 5 / 4), int16_t(w / 2), color);
	gfx->fillCircle(x, int16_t(y + s * 3 / 4), int16_t(w / 2 + 1), color);
}

/** @brief Open question hook with a distinct stem and separated dot. */
void question(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t s, uint16_t color) {
	if (s < 4) { return; }
	int16_t radius = int16_t(s / 2), stroke = int16_t(s / 12 + 1);
	for (int16_t i = 0; i <= 32; ++i) {
		float angle = -3.1415927f + i * 4.712389f / 32;
		gfx->fillCircle(int16_t(x + cosf(angle) * radius), int16_t(y - s / 3 + sinf(angle) * radius), stroke, color);
	}
	gfx->fillRect(x - stroke, y + s / 6, 2 * stroke + 1, s / 4 + 1, color);
	gfx->fillCircle(x, y + s * 4 / 5, stroke, color);
}
}
