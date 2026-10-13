/**
 * @file i18n.cpp
 * @brief Bounded lookup with an English fallback; ASCII labels fit the bitmap font.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#include "i18n.h"
#include "../settings/settings.h"
static const char *const labels[][2] = {
#define UI_ROW(key, en, it) {en, it},
	UI_TRANSLATIONS(UI_ROW)
#undef UI_ROW
};
static_assert(sizeof(labels) / sizeof(labels[0]) == uint8_t(Text::Count), "Every UI key requires both translations");
/** @brief Read a translation without indexing outside its table. @param key Key. @param language Language. @return Text. */
const char *tr_language(Text key, uint8_t language) {
	return key < Text::Count ? labels[uint8_t(key)][language < 2 ? language : 0] : "?";
}
/** @brief Translate using current RAM preferences. @param key Key. @return Text. */
const char *tr(Text key) { return tr_language(key, settings_get().language); }
/** @brief Preset display text. @param group Preset group. @param index Choice. @return Label. */
const char *tr_choice(uint8_t group, uint8_t index) {
	static const Text styles[] = {Text::Normal, Text::Mochi, Text::Bloub, Text::Trapez};
	static const Text colors[] = {Text::White, Text::Mint, Text::Cyan, Text::Amber, Text::Pink, Text::Blue};
	static const Text moods[] = {Text::Neutral, Text::Happy, Text::Focused, Text::Sleepy, Text::Love};
	static const Text animations[] = {Text::Calm, Text::Dasai, Text::Playful};
	if (group == 0 && index < 4) { return tr(styles[index]); }
	if (group == 1 && index < 6) { return tr(colors[index]); }
	if (group == 2 && index < 5) { return tr(moods[index]); }
	if (group == 3 && index < 3) { return tr(animations[index]); }
	return "?";
}
