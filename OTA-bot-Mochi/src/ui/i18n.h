/**
 * @file i18n.h
 * @brief Compile-time English/Italian labels; user payloads and USB command names are unchanged.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <stdint.h>
#define UI_TRANSLATIONS(X) \
	X(Eyes, "MOCHI", "MOCHI") \
	X(Clock, "CLOCK", "OROLOGIO") \
	X(Route, "ROUTE", "NAVIGAZIONE") \
	X(Alerts, "ALERTS", "NOTIFICHE") \
	X(Weather, "WEATHER", "METEO") \
	X(Settings, "SETTINGS", "IMPOSTAZIONI") \
	X(Desk, "DESK TOOLS", "SCRIVANIA") \
	X(BigClock, "BIG CLOCK", "OROLOGIO GRANDE") \
	X(Overview, "DESK OVERVIEW", "RIEPILOGO") \
	X(Timer, "FOCUS TIMER", "TIMER") \
	X(Device, "DEVICE STATUS", "STATO DISPOSITIVO") \
	X(CallTitle, "PHONE CALL", "CHIAMATA") \
	X(CallTag, "CALL", "TEL") \
	X(Quiet, "All quiet for now", "Nessuna notifica") \
	X(AlertHint, "Alerts appear here", "Le notifiche appaiono qui") \
	X(Phone, "PHONE", "TELEFONO") \
	X(Incoming, "Incoming call", "Chiamata in arrivo") \
	X(UnknownCaller, "Unknown caller", "Numero sconosciuto") \
	X(AnswerPhone, "Answer on your phone", "Rispondi dal telefono") \
	X(NoRoute, "No active route", "Nessun percorso") \
	X(RouteHint, "Start a route on phone", "Avvia il percorso sul telefono") \
	X(WeatherMissing, "Weather not synced", "Meteo non sincronizzato") \
	X(WeatherHint, "Enable weather in Chronos", "Attiva meteo in Chronos") \
	X(ForecastPending, "Forecast pending", "Previsioni in attesa") \
	X(Stale, "STALE / OPEN CHRONOS", "DATI VECCHI / APRI CHRONOS") \
	X(Updated, "UPDATED ", "AGGIORNATO ") \
	X(SyncWeather, "SYNC / REFRESH", "SINCRONIZZA / AGGIORNA") \
	X(LocalTime, "LOCAL TIME", "ORA LOCALE") \
	X(PhoneSync, "PHONE SYNC", "SYNC TELEFONO") \
	X(Waiting, "WAITING", "IN ATTESA") \
	X(OpenChronos, "Open Chronos", "Apri Chronos") \
	X(OpenSync, "OPEN CHRONOS TO SYNC", "APRI CHRONOS PER SYNC") \
	X(TimeUnavailable, "Time unavailable", "Ora non disponibile") \
	X(Connected, "PHONE CONNECTED", "TELEFONO CONNESSO") \
	X(Offline, "PHONE OFFLINE", "TELEFONO SCOLLEGATO") \
	X(BackClock, "BACK TO CLOCK", "TORNA ALL'OROLOGIO") \
	X(BackDesk, "BACK TO DESK", "TORNA ALLA SCRIVANIA") \
	X(TimerMenu, "TIMER / POMODORO", "TIMER / POMODORO") \
	X(DoneQuiet, "DONE", "FINE") \
	X(Rest, "REST", "PAUSA") \
	X(Focus, "FOCUS", "CONCENTRAZIONE") \
	X(Pause, "PAUSE", "PAUSA") \
	X(Start, "START", "AVVIA") \
	X(Reset, "RESET", "AZZERA") \
	X(Timer5, "MODE: 5 MIN", "MODO: 5 MIN") \
	X(Timer15, "MODE: 15 MIN", "MODO: 15 MIN") \
	X(Pomodoro, "MODE: POMODORO 25 / 5", "MODO: POMODORO 25 / 5") \
	X(BleConnected, "BLE: CONNECTED", "BLE: CONNESSO") \
	X(BleOffline, "BLE: OFFLINE", "BLE: SCOLLEGATO") \
	X(LightPrefix, "LIGHT: ", "LUCE: ") \
	X(ImuReady, "IMU: READY", "IMU: PRONTO") \
	X(ImuMissing, "IMU: UNAVAILABLE", "IMU: NON DISPONIBILE") \
	X(ClockSynced, "CLOCK: SYNCED", "OROLOGIO: SINCRONIZZATO") \
	X(ClockWaiting, "CLOCK: WAITING", "OROLOGIO: IN ATTESA") \
	X(PrefsUnsaved, "PREFERENCES: UNSAVED", "PREFERENZE: NON SALVATE") \
	X(PrefsSaved, "PREFERENCES: SAVED", "PREFERENZE: SALVATE") \
	X(ViewLarge, "VIEW: LARGE / TAP DETAILS", "GRANDE / TOCCA: DETTAGLI") \
	X(ViewDetails, "VIEW: DETAILS / TAP LARGE", "DETTAGLI / TOCCA: GRANDE") \
	X(LookTab, "LOOK", "OCCHI") \
	X(LightTab, "LIGHT", "LUCE") \
	X(DriveTab, "DRIVE", "AUTO") \
	X(MoveTab, "MOVE", "ANIM") \
	X(AudioTab, "AUDIO", "AUDIO") \
	X(EyesLabel, "Eyes", "Occhi") \
	X(ColorLabel, "Color", "Colore") \
	X(MoodLabel, "Mood", "Espressione") \
	X(RouteDefault, "Route default", "Nav predef.") \
	X(Large, "large", "grande") \
	X(Details, "details", "dettagli") \
	X(DayLevel, "Day level", "Luce giorno") \
	X(Night, "Night", "Notte") \
	X(NightLevel, "Night level", "Luce notte") \
	X(IdleDim, "Idle dim", "Luce inattiva") \
	X(Dashboard, "Dashboard", "Dashboard") \
	X(Shake, "Shake dizzy", "Scossa") \
	X(RandomMood, "Random mood", "Facce casuali") \
	X(ImuMood, "IMU mood", "Facce IMU") \
	X(Animation, "Animation", "Animazione") \
	X(Language, "Language", "Lingua") \
	X(MotionHint, "Dashboard/night: calmer", "Auto/notte: movimenti ridotti") \
	X(Sound, "Sound", "Suoni") \
	X(Level, "Level", "Livello") \
	X(NightMute, "Night mute", "Muto notte") \
	X(SoundOptions, "Sound options", "Opzioni suoni") \
	X(TestSound, "Test sound", "Prova suoni") \
	X(Test, "test", "prova") \
	X(Open, "open", "apri") \
	X(Faces, "Faces", "Facce") \
	X(TimerLabel, "Timer", "Timer") \
	X(CallsAlerts, "Alerts / calls", "Avvisi / tel.") \
	X(Ambient, "Ambient", "Spontanei") \
	X(On, "on", "si") \
	X(Off, "off", "no") \
	X(Save, "SAVE", "SALVA") \
	X(Saved, "SAVED", "SALVATO") \
	X(Failed, "FAILED", "ERRORE") \
	X(EyesButton, "EYES", "OCCHI") \
	X(Back, "BACK", "INDIETRO") \
	X(Normal, "normal", "normale") \
	X(Mochi, "mochi", "mochi") \
	X(Bloub, "bloub", "bloub") \
	X(Trapez, "trapez", "trapez") \
	X(White, "white", "bianco") \
	X(Mint, "mint", "menta") \
	X(Cyan, "cyan", "ciano") \
	X(Amber, "amber", "ambra") \
	X(Pink, "pink", "rosa") \
	X(Blue, "blue", "blu") \
	X(Neutral, "neutral", "neutro") \
	X(Happy, "happy", "felice") \
	X(Focused, "focused", "attento") \
	X(Sleepy, "sleepy", "assonnato") \
	X(Love, "love", "amore") \
	X(Calm, "calm", "calmo") \
	X(Dasai, "dasai", "dasai") \
	X(Playful, "playful", "vivace") \
	X(Sunday, "SUN", "DOM") \
	X(Monday, "MON", "LUN") \
	X(Tuesday, "TUE", "MAR") \
	X(Wednesday, "WED", "MER") \
	X(Thursday, "THU", "GIO") \
	X(Friday, "FRI", "VEN") \
	X(Saturday, "SAT", "SAB") \
	X(SoundOff, "SOUND: OFF", "SUONI: SPENTI") \
	X(SoundNight, "SOUND: NIGHT MUTE", "SUONI: MUTO NOTTE") \
	X(SoundArmed, "SOUND: ARMED", "SUONI: ABILITATI") \
	X(SoundReady, "SOUND: PWM READY", "SUONI: PWM PRONTO") \
	X(SoundError, "SOUND: PWM FAILED", "SUONI: ERRORE PWM")

enum class Text : uint8_t {
#define UI_ENUM(key, en, it) key,
	UI_TRANSLATIONS(UI_ENUM)
#undef UI_ENUM
	Count
};
/** @brief Get a label using active settings. @param key Key. @return Static text. */
const char *tr(Text key);
/** @brief Inspect a translation safely. @param key Key. @param language 0 English/1 Italian. @return Text. */
const char *tr_language(Text key, uint8_t language);
/** @brief Localize preset groups without changing USB names. @param group 0 style/1 color/2 mood/3 animation. @param index Choice. @return Label. */
const char *tr_choice(uint8_t group, uint8_t index);
