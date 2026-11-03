/** @file companion_language.h @brief Explicit EN/IT binding for opaque, undocumented Chronos language identifiers. */
#pragma once
#include <stdint.h>
/** @brief Load independently stored language bindings. @return Storage readable. */
bool companion_language_init();
/** @brief Explicitly save the current app identifier as EN or IT. @param id App identifier. @param language Local EN=0/IT=1. @return Saved. */
bool companion_language_bind(uint8_t id, uint8_t language);
/** @brief Resolve a known app identifier. @param id App identifier. @return EN/IT or 255 if unbound. */
uint8_t companion_language_get(uint8_t id);
