/**
 * @file sd_update_screen_test.cpp
 * @brief SD maintenance text bounds, confirmation, cancellation and bilingual progress.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "../OTA-bot-Mochi/src/update/sd_update_screen.h"
#include "../OTA-bot-Mochi/src/update/sd_update.h"
#include "../OTA-bot-Mochi/src/settings/settings.h"
#include "../OTA-bot-Mochi/src/eyes/eyes_api.h"
#include "../OTA-bot-Mochi/src/ui/i18n.h"
#include "../OTA-bot-Mochi/src/ui/ui_theme.h"
#include <algorithm>
#include <cassert>
#include <iostream>

static SdUpdateStatus session;
static bool tapped = false;
static int16_t tap_x = 0, tap_y = 0;
bool sd_update_begin(uint32_t) {
	session = SdUpdateStatus{}; session.active = true; session.phase = SdUpdatePhase::Checking;
	return true;
}
void sd_update_poll(uint32_t) {}
void sd_update_stop() { if (session.phase != SdUpdatePhase::Success) { session = SdUpdateStatus{}; } }
const SdUpdateStatus &sd_update_status() { return session; }
bool sd_update_confirm() {
	if (session.phase != SdUpdatePhase::Ready) { return false; }
	session.phase = SdUpdatePhase::Writing; return true;
}
void backlight_activity(uint32_t) {}
void backlight_update(uint32_t, bool touching, bool keep_awake) { assert(!touching && keep_awake); }
int8_t touch_poll(uint32_t) { return 0; }
bool touch_take_tap(int16_t &x, int16_t &y) {
	x = tap_x; y = tap_y;
	bool result = tapped; tapped = false; return result;
}
/** @brief Simulate one released tap and a scheduled frame. */
static void refresh(Arduino_GFX &gfx, int16_t x = -1, int16_t y = 248) {
	gfx.text.clear(); test_now += 201;
	if (x >= 0) { tapped = true; tap_x = ui_s(x); tap_y = ui_s(y); }
	sd_update_screen_poll(test_now);
}
/** @brief Search for a complete bounded label. */
static bool contains(const Arduino_GFX &gfx, Text text) {
	return std::find(gfx.text.begin(), gfx.text.end(), tr(text)) != gfx.text.end();
}
/** @brief Render EN/IT at the compiled panel size without writing any flash. @return Success. */
int main() {
	assert(settings_init()); eyes_init(nullptr);
	for (uint8_t language = 0; language < 2; ++language) {
		auto p = settings_get(); p.language = language; assert(settings_update(p));
		Arduino_GFX gfx;
		sd_update_screen_begin(&gfx, ++test_now);
		assert(contains(gfx, Text::SdChecking) && !contains(gfx, Text::Start));
		refresh(gfx, 180); assert(session.phase == SdUpdatePhase::Checking);
		session.phase = SdUpdatePhase::Ready; session.total = session.checked = 100;
		refresh(gfx);
		assert(contains(gfx, Text::SdConfirm) && contains(gfx, Text::Start));
		refresh(gfx, 180, 280); assert(session.phase == SdUpdatePhase::Ready);
		refresh(gfx, 180);
		assert(session.phase == SdUpdatePhase::Writing && contains(gfx, Text::OtaWriting));
		session.written = 25; refresh(gfx);
		assert(std::find(gfx.text.begin(), gfx.text.end(), "25%") != gfx.text.end());
		refresh(gfx, 20); assert(!session.active);
		sd_update_screen_begin(&gfx, ++test_now);
		session.error = "Missing /mochi/update/ release app.bin.";
		session.phase = SdUpdatePhase::Failed;
		refresh(gfx); assert(contains(gfx, Text::Failed) && !contains(gfx, Text::Start));
		refresh(gfx, 20); assert(!session.active);
		sd_update_screen_begin(&gfx, ++test_now);
		session.phase = SdUpdatePhase::Success;
		refresh(gfx); assert(contains(gfx, Text::OtaRestart) && !contains(gfx, Text::Back));
		refresh(gfx, 20); assert(session.active && session.phase == SdUpdatePhase::Success);
	}
	std::cout << "SD confirmation, cancellation, progress and EN/IT panel bounds passed\n";
}
