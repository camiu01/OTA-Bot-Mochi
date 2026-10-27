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

/** @brief Shared spiral drawing primitive. */
void spiral(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t s, uint16_t color) {
	if (s < 2) { return; }
	int16_t prev_x = x, prev_y = y;
	for (int8_t i = 1; i <= 24; ++i) {
		float t = i * 0.4f;
		float radius = t * s / 9.6f;
		int16_t px = int16_t(x + cosf(t) * radius);
		int16_t py = int16_t(y + sinf(t) * radius);
		gfx->drawLine(prev_x, prev_y, px, py, color);
		prev_x = px; prev_y = py;
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

/** @brief Shared flame drawing primitive. */
void flame(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t s, uint16_t color) {
	if (s < 3) { return; }
	int16_t wing = int16_t(s * 3 / 4);
	gfx->fillTriangle(x, int16_t(y - s), int16_t(x - wing), int16_t(y + s / 2),
		int16_t(x + wing), int16_t(y + s / 2), color);
	gfx->fillCircle(x, int16_t(y + s / 3), int16_t(s / 2), color);
}

/** @brief Shared crescent drawing primitive. */
void crescent(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t s, uint16_t color, uint16_t cut) {
	if (s < 3) { return; }
	gfx->fillCircle(x, y, s, color);
	gfx->fillEllipse(int16_t(x + s / 4), y, int16_t(s * 3 / 4), int16_t(s * 3 / 4), cut);
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

/** @brief Shared bolt drawing primitive. */
void bolt(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t s, uint16_t color) {
	if (s < 4) { return; }
	int16_t wing = int16_t(s * 2 / 3);
	gfx->fillTriangle(int16_t(x - wing / 2), int16_t(y - s), int16_t(x + wing),
		int16_t(y - s / 5), int16_t(x - wing / 2), int16_t(y + s / 5), color);
	gfx->fillTriangle(int16_t(x + wing / 2), int16_t(y + s), int16_t(x - wing),
		int16_t(y + s / 5), int16_t(x + wing / 2), int16_t(y - s / 5), color);
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

/** @brief Shared money drawing primitive. */
void money(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t s, uint16_t color) {
	annulus(gfx, x, y, int16_t(s * 3 / 4), color);
	if (s < 3) { return; }
	gfx->drawLine(x, int16_t(y - s), x, int16_t(y + s), color);
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

/** @brief Shared low_batt drawing primitive. */
void low_batt(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t s, uint16_t color) {
	if (s < 4) { return; }
	int16_t w = int16_t(s * 6 / 5), h = int16_t(s * 3 / 4);
	gfx->drawRect(int16_t(x - w / 2), int16_t(y - h / 2), w, h, color);
	gfx->fillRect(int16_t(x + w / 2), int16_t(y - h / 4), 2, int16_t(h / 2), color);
	gfx->fillRect(int16_t(x - w / 2 + 2), int16_t(y - h / 2 + 2), int16_t(w / 4), int16_t(h - 4), color);
}

/** @brief Shared music_note drawing primitive. */
void music_note(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t s, uint16_t color) {
	if (s < 3) { return; }
	int16_t r = int16_t(s / 3 ? s / 3 : 1);
	gfx->fillCircle(int16_t(x - r / 2), int16_t(y + s / 2), r, color);
	gfx->fillRect(int16_t(x + r / 2), int16_t(y - s * 2 / 3), 2, int16_t(s + r / 2), color);
	gfx->fillRect(int16_t(x + r / 2), int16_t(y - s * 2 / 3), int16_t(s / 2), int16_t(r / 2 + 1), color);
}

/** @brief Shared power_sym drawing primitive. */
void power_sym(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t s, uint16_t color) {
	if (s < 3) { return; }
	annulus(gfx, x, int16_t(y + s / 6), int16_t(s * 3 / 4), color);
	gfx->fillRect(int16_t(x - 1), int16_t(y - s), 3, int16_t(s), color);
}

/** @brief Shared exclaim drawing primitive. */
void exclaim(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t s, uint16_t color) {
	if (s < 3) { return; }
	int16_t w = int16_t(s / 3 ? s / 3 : 1);
	gfx->fillRoundRect(int16_t(x - w / 2), int16_t(y - s), w, int16_t(s * 5 / 4), int16_t(w / 2), color);
	gfx->fillCircle(x, int16_t(y + s * 3 / 4), int16_t(w / 2 + 1), color);
}

/** @brief Shared question drawing primitive. */
void question(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t s, uint16_t color) {
	if (s < 4) { return; }
	int16_t r = int16_t(s / 2);
	annulus(gfx, x, int16_t(y - s / 3), r, color);
	gfx->fillRect(int16_t(x - 1), int16_t(y + r / 2), 2, int16_t(s / 3), color);
	gfx->fillCircle(x, int16_t(y + s * 4 / 5), int16_t(s / 5 ? s / 5 : 1), color);
}
}
