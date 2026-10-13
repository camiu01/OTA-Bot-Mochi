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

/** @brief Expression labels kept in the same order as the Expression enum. */
static const Text expressions[EXPRESSION_COUNT] = {
	Text::ExprNeutral, Text::ExprHappy, Text::ExprSad, Text::ExprAngry, Text::ExprSurprised,
	Text::ExprSleepy, Text::ExprLove, Text::ExprDizzy, Text::ExprGlee, Text::ExprWorried,
	Text::ExprFocused, Text::ExprAnnoyed, Text::ExprSkeptic, Text::ExprFrustrated, Text::ExprUnimpressed,
	Text::ExprSuspicious, Text::ExprSquint, Text::ExprFurious, Text::ExprScared, Text::ExprAwe,
	Text::ExprSleepyEyes, Text::ExprSadDown, Text::ExprSadUp, Text::ExprExcited, Text::ExprTerrified,
	Text::ExprDisgusted, Text::ExprShy, Text::ExprProud, Text::ExprTired, Text::ExprGrin,
	Text::ExprPanicked, Text::ExprGlum, Text::ExprSmug, Text::ExprDeadpan, Text::ExprZombie,
	Text::ExprConfused, Text::ExprCurious, Text::ExprWink, Text::ExprWondering, Text::ExprStar,
	Text::ExprCrying, Text::ExprSwirly, Text::ExprCat, Text::ExprHalo, Text::ExprDevil,
	Text::ExprFire, Text::ExprMoon, Text::ExprSparkle, Text::ExprBolt, Text::ExprDiamond,
	Text::ExprRing, Text::ExprTarget, Text::ExprMoney, Text::ExprMildHappy, Text::ExprSmirk,
	Text::ExprPout, Text::ExprKiss, Text::ExprNervous, Text::ExprShocked, Text::ExprMelting,
	Text::ExprLazy, Text::ExprDoze, Text::ExprGroggy, Text::ExprDaydream, Text::ExprGrumpy,
	Text::ExprSassy, Text::ExprCheeky, Text::ExprFlirt, Text::ExprZen, Text::ExprMerciful,
	Text::ExprManic, Text::ExprSideEye, Text::ExprBeaming
};

static_assert(uint8_t(Text::ExprNeutral) + EXPRESSION_COUNT - 1 == uint8_t(Text::ExprBeaming),
	"Expression labels must stay contiguous to match Expression values");
static_assert(uint8_t(Text::Count) < 250, "Text is stored in a uint8_t; the table is nearly full");

/** @brief Localize an eye expression by its stable Expression value. @param index Expression value. @return Label, or "?" when out of range. */
const char *tr_expression(uint8_t index) {
	return index < EXPRESSION_COUNT ? tr(expressions[index]) : "?";
}

/** @brief Preset display text. @param group Preset group. @param index Choice. @return Label. */
const char *tr_choice(uint8_t group, uint8_t index) {
	static const Text styles[] = {Text::Normal, Text::Mochi, Text::Bloub, Text::Trapez};
	static const Text colors[] = {Text::White, Text::Mint, Text::Cyan, Text::Amber, Text::Pink, Text::Blue};
	static const Text animations[] = {Text::Calm, Text::Dasai, Text::Playful};
	if (group == 0 && index < 4) { return tr(styles[index]); }
	if (group == 1 && index < 6) { return tr(colors[index]); }
	if (group == 2) { return tr_expression(index); }
	if (group == 3 && index < 3) { return tr(animations[index]); }
	return "?";
}
