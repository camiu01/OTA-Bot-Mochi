/** @file phone_view.cpp @brief Bounded phone controls and read-only alarms, forecasts, contacts, reminders and QR pages. */
#include "phone_view.h"
#include "companion_state.h"
#include "companion_service.h"
#include "companion_language.h"
#include "phone_qr.h"
#include "../settings/settings.h"
#include "../ui/ui_theme.h"
#include "../ui/i18n.h"
#include "../ui/screens.h"

namespace {
enum class Panel : uint8_t { Home, Alarms, Hours, Contacts, Qr, Reminders, Language };
Panel panel = Panel::Home;
uint8_t record_index = 0, bind_result = 0;
bool camera_hint = false;
/** @brief Identify subviews with independent records. @return Paging available. */
bool paged() { return panel == Panel::Alarms || panel == Panel::Hours || panel == Panel::Contacts || panel == Panel::Qr; }
/** @brief Draw a localized button in design coordinates. @param gfx Canvas. @param x Left. @param y Top. @param width Width. @param label Text. @param height Height. */
void button(Arduino_GFX *gfx, int16_t x, int16_t y, int16_t width, Text label, int16_t height = 34) {
	gfx->fillRoundRect(ui_s(x), ui_s(y), ui_s(width), ui_s(height), ui_s(6), UI_CARD);
	ui_label(gfx, tr(label), ui_s(x + 6), ui_s(y + 12), ui_s(width - 12), 1, UI_TEXT);
}
/** @brief Format minute-accurate schedules with explicit disabled state. @param window Schedule. @return Label. */
String window_text(const CompanionWindow &window) {
	if (!window.enabled) { return tr(Text::Off); }
	char text[24];
	snprintf(text, sizeof(text), "%02u:%02u-%02u:%02u", unsigned(window.start / 60), unsigned(window.start % 60),
		unsigned(window.end / 60), unsigned(window.end % 60));
	return String(text);
}
/** @brief Render connected actions and phone battery without inventing missing measurements. @param gfx Canvas. @param data Snapshot. */
void home(Arduino_GFX *gfx, const CompanionSnapshot &data) {
	String battery = String(tr(Text::Phone)) + ": " + (data.battery > 100 ? String("--%") : String(data.battery) + "%");
	ui_label(gfx, battery, ui_s(16), ui_s(58), ui_s(188), 1, UI_ACCENT);
	if (data.phone_charging) { ui_charge_badge(gfx, ui_s(214), ui_s(58)); }
	const Text labels[] = {companion_finding() ? Text::CompanionStopFind : Text::CompanionFindPhone,
		Text::CompanionCamera, Text::CompanionAlarms, Text::CompanionHoursShort,
		Text::CompanionContacts, Text::CompanionQr, Text::CompanionReminders, Text::CompanionLanguageShort};
	for (uint8_t i = 0; i < 8; ++i) { button(gfx, i % 2 ? 126 : 8, 76 + (i / 2) * 41, 106, labels[i]); }
	Text status = !data.connected ? Text::Offline : (camera_hint ? Text::CompanionCameraHint :
		(companion_camera_result() == 1 ? Text::CompanionSent : Text::CompanionUp));
	ui_label(gfx, tr(status), ui_s(16), ui_s(245), ui_s(208), 1, UI_MUTED);
}
/** @brief Render the selected alarm and its weekday mask. @param gfx Canvas. @param data Snapshot. */
void alarms(Arduino_GFX *gfx, const CompanionSnapshot &data) {
	if (!(data.alarm_mask & (1U << record_index))) { ui_empty(gfx, tr(Text::CompanionNoData), tr(Text::CompanionSync)); return; }
	const Alarm &alarm = data.alarms[record_index];
	char time[8]; snprintf(time, sizeof(time), "%02u:%02u", unsigned(alarm.hour), unsigned(alarm.minute));
	ui_card(gfx, 82, 135);
	ui_label(gfx, time, ui_s(34), ui_s(100), ui_s(172), 4, UI_TEXT);
	ui_label(gfx, tr(alarm.enabled ? Text::On : Text::Off), ui_s(28), ui_s(145), ui_s(184), 1, UI_ACCENT);
	String repeat = tr(alarm.repeat == 128 ? Text::CompanionOnce : Text::CompanionDaily);
	if (alarm.repeat < 127) {
		repeat = "";
		for (uint8_t i = 0; i < 7; ++i) { repeat += (alarm.repeat & (1U << i)) ? String(i + 1) : String("-"); }
	}
	ui_label(gfx, tr(Text::CompanionDays), ui_s(28), ui_s(173), ui_s(184), 1, UI_MUTED);
	ui_label(gfx, repeat, ui_s(28), ui_s(195), ui_s(184), 1, UI_TEXT);
}
/** @brief Render only a received forecast slot, with an explicit stale-day label. @param gfx Canvas. @param data Snapshot. @param day Local day. */
void hours(Arduino_GFX *gfx, const CompanionSnapshot &data, int day) {
	if (!(data.forecast_mask & (uint32_t(1) << record_index))) { ui_empty(gfx, tr(Text::CompanionNoData), tr(Text::CompanionSync)); return; }
	CompanionHour value = companion_hour(record_index);
	char hour[8]; snprintf(hour, sizeof(hour), "%02u:00", unsigned(value.hour));
	ui_label(gfx, hour, ui_s(28), ui_s(88), ui_s(120), 3, UI_TEXT);
	ui_weather_icon(gfx, value.icon, ui_s(187), ui_s(109));
	ui_label(gfx, String(value.temp) + " C", ui_s(28), ui_s(126), ui_s(184), 3, UI_ACCENT);
	ui_label(gfx, String("RH ") + String(value.humidity) + "% / UV " + String(value.uv), ui_s(28), ui_s(169), ui_s(184), 1, UI_TEXT);
	ui_label(gfx, String(value.wind) + " km/h", ui_s(28), ui_s(192), ui_s(184), 1, UI_TEXT);
	if (value.day != day) { ui_label(gfx, tr(Text::Stale), ui_s(16), ui_s(218), ui_s(208), 1, UI_MUTED); }
}
/** @brief Render one bounded cached contact with the received/cache counts visible. @param gfx Canvas. @param data Snapshot. */
void contacts(Arduino_GFX *gfx, const CompanionSnapshot &data) {
	if (!data.contacts) { ui_empty(gfx, tr(Text::CompanionNoData), tr(Text::CompanionSync)); return; }
	CompanionContact value = companion_contact(record_index % data.contacts);
	ui_card(gfx, 90, 120);
	ui_label(gfx, value.name, ui_s(28), ui_s(112), ui_s(184), 2, UI_ACCENT);
	ui_label(gfx, value.number, ui_s(28), ui_s(154), ui_s(184), 1, UI_TEXT);
	ui_label(gfx, String(data.contacts) + "/" + String(data.total_contacts), ui_s(28), ui_s(184), ui_s(184), 1, UI_MUTED);
}
/** @brief Show the two reminders and both minute-accurate quiet/sleep windows. @param gfx Canvas. @param data Snapshot. */
void reminders(Arduino_GFX *gfx, const CompanionSnapshot &data) {
	const Text labels[] = {Text::CompanionWater, Text::CompanionMove, Text::CompanionQuiet, Text::CompanionSleep};
	const CompanionWindow windows[] = {data.reminders[0], data.reminders[1], data.quiet, data.sleep};
	for (uint8_t i = 0; i < 4; ++i) {
		ui_label(gfx, tr(labels[i]), ui_s(20), ui_s(82 + i * 36), ui_s(200), 1, UI_MUTED);
		String text = window_text(windows[i]);
		if (i < 2 && windows[i].enabled) { text += String(" / ") + String(windows[i].interval) + "m"; }
		ui_label(gfx, text, ui_s(20), ui_s(96 + i * 36), ui_s(200), 1, UI_TEXT);
	}
}
/** @brief Make opaque app-language IDs explicitly learnable as EN or IT. @param gfx Canvas. @param data Snapshot. */
void language(Arduino_GFX *gfx, const CompanionSnapshot &data) {
	ui_label(gfx, tr(Text::CompanionBindHint), ui_s(20), ui_s(88), ui_s(200), 1, UI_MUTED);
	ui_label(gfx, data.language == 255 ? String("--") : String("ID ") + String(data.language), ui_s(36), ui_s(119), ui_s(168), 3, UI_TEXT);
	button(gfx, 12, 164, 100, Text::CompanionBindEn);
	button(gfx, 126, 164, 100, Text::CompanionBindIt);
	if (bind_result) { ui_label(gfx, tr(bind_result == 1 ? Text::CompanionBindOk : Text::CompanionBindError), ui_s(16), ui_s(211), ui_s(208), 1, UI_MUTED); }
}
/** @brief Cycle only valid slots while all arithmetic stays within the relevant cache. @param step Direction. */
void move(int step) {
	if (!paged()) { return; }
	CompanionSnapshot data = companion_snapshot();
	uint8_t count = panel == Panel::Alarms ? 8 : (panel == Panel::Hours ? 24 : (panel == Panel::Contacts ? data.contacts : 9));
	if (!count) { record_index = 0; return; }
	for (uint8_t i = 0; i < count; ++i) {
		record_index = uint8_t((record_index + count + step) % count);
		if (panel == Panel::Contacts) { return; }
		uint32_t mask = panel == Panel::Hours ? data.forecast_mask : (panel == Panel::Qr ? data.qr_mask : data.alarm_mask);
		if (mask & (uint32_t(1) << record_index)) { return; }
	}
}
}

void phone_view_draw(Arduino_GFX *gfx, int day) {
	if (settings_get().phone_ios) { ui_empty(gfx, tr(Text::Offline), tr(Text::Phone)); return; }
	CompanionSnapshot data = companion_snapshot();
	if (panel == Panel::Home) { home(gfx, data); return; }
	static const Text titles[] = {Text::CompanionTitle, Text::CompanionAlarms, Text::CompanionHours,
		Text::CompanionContacts, Text::CompanionQr, Text::CompanionReminders, Text::CompanionLanguage};
	String title = tr(titles[uint8_t(panel)]);
	if (paged()) { title += String(" / ") + String(record_index + 1); }
	ui_label(gfx, title, ui_s(16), ui_s(58), ui_s(208), 1, UI_ACCENT);
	if (panel == Panel::Alarms) { alarms(gfx, data); }
	if (panel == Panel::Hours) { hours(gfx, data, day); }
	if (panel == Panel::Contacts) { contacts(gfx, data); }
	if (panel == Panel::Reminders) { reminders(gfx, data); }
	if (panel == Panel::Language) { language(gfx, data); }
	if (panel == Panel::Qr && !phone_qr_draw(gfx, companion_qr(record_index))) {
		ui_empty(gfx, tr(Text::CompanionNoData), "QR: max 96 bytes");
	}
	button(gfx, 8, 232, 94, Text::Back, 26);
	if (paged()) {
		ui_label(gfx, "<", ui_s(141), ui_s(239), ui_s(24), 2, UI_TEXT);
		ui_label(gfx, ">", ui_s(198), ui_s(239), ui_s(24), 2, UI_TEXT);
	}
}
void phone_view_tap(int16_t x, int16_t y) {
	if (settings_get().phone_ios) { return; }
	if (panel == Panel::Home) {
		for (uint8_t i = 0; i < 8; ++i) {
			int16_t left = ui_s(i % 2 ? 126 : 8), top = ui_s(76 + (i / 2) * 41);
			if (x < left || x >= left + ui_s(106) || y < top || y >= top + ui_s(34)) { continue; }
			if (i == 0) { companion_request(companion_finding() ? CompanionAction::StopFind : CompanionAction::FindPhone); }
			else if (i == 1) { camera_hint = !companion_request(CompanionAction::Camera); }
			else {
				panel = static_cast<Panel>(i - 1); record_index = 0;
				CompanionSnapshot data = companion_snapshot();
				uint32_t mask = panel == Panel::Hours ? data.forecast_mask : (panel == Panel::Qr ? data.qr_mask : data.alarm_mask);
				if (panel != Panel::Contacts && !(mask & 1)) { move(1); }
			}
		}
	} else if (panel == Panel::Language && y >= ui_s(164) && y < ui_s(198)) {
		uint8_t locale = x >= ui_s(126) && x < ui_s(226) ? 1 : 0;
		if (x >= ui_s(12) && x < ui_s(226) && (x < ui_s(112) || x >= ui_s(126))) {
			bind_result = companion_language_bind(companion_snapshot().language, locale) ? 1 : 2;
		}
	} else if (y >= ui_s(232) && y < ui_s(258)) {
		if (x < ui_s(110)) { panel = Panel::Home; }
		else { move(x < ui_s(181) ? -1 : 1); }
	}
	screens_invalidate();
}
bool phone_view_scroll(int8_t step) {
	if (panel == Panel::Home || !step) { return false; }
	if (step > 0) { panel = Panel::Home; }
	else { move(1); }
	screens_invalidate(); return true;
}
