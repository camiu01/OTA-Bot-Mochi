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
	X(FirmwareUpdate, "Firmware update", "Aggiorna firmware") \
	X(OtaWifi, "Connect to Wi-Fi:", "Connetti al Wi-Fi:") \
	X(OtaPassword, "Wi-Fi password:", "Password Wi-Fi:") \
	X(OtaBrowser, "Open browser:", "Apri nel browser:") \
	X(OtaReady, "Ready / 5 min timeout", "Pronto / scade in 5 min") \
	X(OtaWriting, "Writing / keep power on", "Scrittura / non spegnere") \
	X(OtaRestart, "Verified / restarting", "Verificato / riavvio") \
	X(OtaScan, "Scan to join Wi-Fi", "Scansiona per Wi-Fi") \
	X(OtaDetails, "DETAILS", "DETTAGLI") \
	X(OtaQr, "WI-FI QR", "QR WI-FI") \
	X(Desk, "DESK TOOLS", "SCRIVANIA") \
	X(BigClock, "BIG CLOCK", "OROLOGIO GRANDE") \
	X(Overview, "DESK OVERVIEW", "RIEPILOGO") \
	X(Timer, "FOCUS TIMER", "TIMER") \
	X(Device, "DEVICE STATUS", "STATO DISPOSITIVO") \
	X(Pong, "PONG", "PONG") \
	X(AnalogClock, "ANALOG CLOCK", "OROLOGIO ANALOGICO") \
	X(PongHint, "TAP TO MOVE LEFT PADDLE", "TOCCA: MUOVI RACCHETTA") \
	X(CallTitle, "PHONE CALL", "CHIAMATA") \
	X(CallTag, "CALL", "TEL") \
	X(Quiet, "All quiet for now", "Nessuna notifica") \
	X(AlertHint, "Alerts appear here", "Le notifiche appaiono qui") \
	X(Dismiss, "CLEAR", "TOGLI") \
	X(CalibrateImu, "IMU zero", "Zero IMU") \
	X(Calibrating, "keep still", "fermo") \
	X(Calibrate, "calibrate", "calibra") \
	X(Calibrated, "ready", "pronto") \
	X(AutoRotate, "Auto rotate", "Auto ruota") \
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
	X(BatteryPrefix, "BATTERY: ", "BATTERIA: ") \
	X(BatteryMissing, "NOT MEASURED", "NON MISURATA") \
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
	X(ScreenFlip, "Screen flip", "Ruota schermo") \
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
	X(Flipped, "flipped", "girato") \
	X(Mochi, "mochi", "mochi") \
	X(Bloub, "bloub", "bloub") \
	X(Trapez, "trapez", "trapez") \
	X(White, "white", "bianco") \
	X(Mint, "mint", "menta") \
	X(Cyan, "cyan", "ciano") \
	X(Amber, "amber", "ambra") \
	X(Pink, "pink", "rosa") \
	X(Blue, "blue", "blu") \
	X(ExprNeutral, "neutral", "neutro") \
	X(ExprHappy, "happy", "felice") \
	X(ExprSad, "sad", "triste") \
	X(ExprAngry, "angry", "arrabbiato") \
	X(ExprSurprised, "surprised", "sorpreso") \
	X(ExprSleepy, "sleepy", "assonnato") \
	X(ExprLove, "love", "amore") \
	X(ExprDizzy, "dizzy", "girato") \
	X(ExprGlee, "glee", "gioia") \
	X(ExprWorried, "worried", "inquieto") \
	X(ExprFocused, "focused", "attento") \
	X(ExprAnnoyed, "annoyed", "seccato") \
	X(ExprSkeptic, "skeptic", "scettico") \
	X(ExprFrustrated, "frustrated", "frustrato") \
	X(ExprUnimpressed, "unimpressed", "noioso") \
	X(ExprSuspicious, "suspicious", "sospettoso") \
	X(ExprSquint, "squint", "strizzata") \
	X(ExprFurious, "furious", "furioso") \
	X(ExprScared, "scared", "spaventato") \
	X(ExprAwe, "awe", "stupore") \
	X(ExprSleepyEyes, "sleepy eyes", "assonnati") \
	X(ExprSadDown, "sad down", "triste giu") \
	X(ExprSadUp, "sad up", "triste su") \
	X(ExprExcited, "excited", "esaltato") \
	X(ExprTerrified, "terrified", "terrore") \
	X(ExprDisgusted, "disgusted", "disgustato") \
	X(ExprShy, "shy", "timido") \
	X(ExprProud, "proud", "orgoglioso") \
	X(ExprTired, "tired", "stanco") \
	X(ExprGrin, "grin", "ghigno") \
	X(ExprPanicked, "panicked", "in panico") \
	X(ExprGlum, "glum", "mesto") \
	X(ExprSmug, "smug", "sdegno") \
	X(ExprDeadpan, "deadpan", "impassibile") \
	X(ExprZombie, "zombie", "zombie") \
	X(ExprConfused, "confused", "confuso") \
	X(ExprCurious, "curious", "curioso") \
	X(ExprWink, "wink", "occhiolino") \
	X(ExprWondering, "wondering", "perplesso") \
	X(ExprStar, "star", "stella") \
	X(ExprCrying, "crying", "piange") \
	X(ExprSwirly, "swirly", "vortice") \
	X(ExprCat, "cat", "gatto") \
	X(ExprHalo, "halo", "alone") \
	X(ExprDevil, "devil", "diavolo") \
	X(ExprFire, "fire", "fuoco") \
	X(ExprMoon, "moon", "luna") \
	X(ExprSparkle, "sparkle", "scintilla") \
	X(ExprBolt, "bolt", "fulmine") \
	X(ExprDiamond, "diamond", "diamante") \
	X(ExprRing, "ring", "anello") \
	X(ExprTarget, "target", "bersaglio") \
	X(ExprMoney, "money", "soldi") \
	X(ExprMildHappy, "mild happy", "soddisfatto") \
	X(ExprSmirk, "smirk", "smorfia") \
	X(ExprPout, "pout", "muso") \
	X(ExprKiss, "kiss", "bacio") \
	X(ExprNervous, "nervous", "nervoso") \
	X(ExprShocked, "shocked", "sorpreso") \
	X(ExprMelting, "melting", "scioglie") \
	X(ExprLazy, "lazy", "pigro") \
	X(ExprDoze, "doze", "pisolino") \
	X(ExprGroggy, "groggy", "stordito") \
	X(ExprDaydream, "daydream", "sognante") \
	X(ExprGrumpy, "grumpy", "brontolo") \
	X(ExprSassy, "sassy", "sfacciato") \
	X(ExprCheeky, "cheeky", "birichino") \
	X(ExprFlirt, "flirt", "lusinga") \
	X(ExprZen, "zen", "zen") \
	X(ExprMerciful, "merciful", "pietoso") \
	X(ExprManic, "manic", "frenetico") \
	X(ExprSideEye, "side eye", "di traverso") \
	X(ExprBeaming, "beaming", "raggiante") \
	X(ExprMusic, "music", "musica") \
	X(ExprLowBatt, "low battery", "scarica") \
	X(ExprPower, "power", "stand-by") \
	X(ExprLoading, "loading", "attendi") \
	X(ExprIdea, "idea", "idea") \
	X(ExprHyper, "hyper", "iper") \
	X(ExprSquish, "squish", "schiacciato") \
	X(ExprDerp, "derp", "derp") \
	X(ExprGlitch, "glitch", "glitch") \
	X(ExprTwitch, "twitch", "tic") \
	X(ExprSneeze, "sneeze", "starnuto") \
	X(ExprJudge, "judge", "giudice") \
	X(ExprQuestion, "question", "domanda") \
	X(ExprCalculating, "calculating", "calcola") \
	X(ExprCyborg, "cyborg", "cyborg") \
	X(ExprArcHappy, "happy arcs", "archi felici") \
	X(ExprArcSad, "sad arcs", "archi tristi") \
	X(ExprSqueezed, "squeezed", "strizzati") \
	X(ExprTilted, "tilted", "inclinati") \
	X(ExprPong, "pong demo", "pong demo") \
	X(ExprClockEyes, "clock eyes", "occhi orologio") \
	X(ExprGauge, "gauge demo", "tachimetro demo") \
	X(ExprGlow, "color glow", "bagliore colori") \
	X(ExprHeadlights, "headlights", "occhi faro") \
	X(ExprDancing, "dancing", "equalizzatore") \
	X(ExprPolice, "red / blue", "rosso / blu") \
	X(ExprRain, "rain", "pioggia") \
	X(ExprGrowing, "growing", "crescita") \
	X(ExprShrink, "shrink", "restringi") \
	X(ExprSwinging, "swinging", "dondolio") \
	X(ExprBlinding, "amber rays", "raggi ambra") \
	X(ExprBuzzing, "buzzing", "ronzio") \
	X(ExprSick, "sick", "malessere") \
	X(ExprSplash, "splash", "onda") \
	X(ExprSpraying, "spraying", "spruzzi") \
	X(ExprSushi, "sushi", "sushi") \
	X(ExprEnergetic, "turbines", "turbine") \
	X(ExprSmoke, "smoke", "nuvole") \
	X(ExprMenacing, "orbital rings", "anelli orbitali") \
	X(ExprHello, "hello", "saluto") \
	X(ExprDistracted, "distracted", "distratto") \
	X(ExprLookLeft, "look left", "guarda sinistra") \
	X(ExprLookRight, "look right", "guarda destra") \
	X(ExprLookDown, "look down", "guarda giu") \
	X(ExprStargazing, "stargazing", "cielo stellato") \
	X(ExprMistake, "mistake", "errore") \
	X(ExprEncouragement, "encourage", "incoraggia") \
	X(ExprAdore, "adore", "adorare") \
	X(ExprLaughing, "laughing", "ridere") \
	X(ExprEnraged, "enraged", "infuriato") \
	X(ExprTraffic, "traffic", "semaforo") \
	X(ExprNeon, "neon", "neon") \
	X(ExprWheels, "wheels", "ruote") \
	X(ExprSakura, "sakura", "petali") \
	X(ExprSamurai, "samurai", "samurai") \
	X(ExprMecha, "mecha", "mecha") \
	X(ExprBamboo, "bamboo", "bambu") \
	X(ExprTitan, "titan", "titano") \
	X(ExprUpsideDown, "upside down", "capovolto") \
	X(ExprUwu, "uwu", "uwu") \
	X(ExprYokai, "yokai", "yokai") \
	X(Calm, "calm", "calmo") \
	X(Dasai, "normal", "normale") \
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
		X(SoundError, "SOUND: PWM FAILED", "SUONI: ERRORE PWM") \
		X(FaceFlow, "Face flow", "Ciclo facce")

enum class Text : uint16_t {
#define UI_ENUM(key, en, it) key,
	UI_TRANSLATIONS(UI_ENUM)
#undef UI_ENUM
	Count
};
/** @brief Get a label using active settings. @param key Key. @return Static text. */
const char *tr(Text key);
/** @brief Localized settings section title. @param index Section index. @return Title or "?". */
const char *tr_settings_tab(uint8_t index);
/** @brief Inspect a translation safely. @param key Key. @param language 0 English/1 Italian. @return Text. */
const char *tr_language(Text key, uint8_t language);
/** @brief Localize preset groups without changing USB names. @param group 0 style/1 color/2 mood/3 animation. @param index Choice. @return Label. */
const char *tr_choice(uint8_t group, uint8_t index);
/** @brief Localize an eye expression by its stable Expression value. @param index Expression value. @return Label, or "?" when out of range. */
const char *tr_expression(uint8_t index);
