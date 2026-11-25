/**
 * @file i18n.h
 * @brief Compile-time English/Italian labels; user payloads and USB command names are unchanged.
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
#pragma once
#include <stdint.h>
#include "i18n_companion.h"
#define UI_TRANSLATIONS(X) \
	UI_COMPANION_TRANSLATIONS(X) \
	X(Eyes, "MOCHI", "MOCHI") \
	X(Clock, "CLOCK", "OROLOGIO") \
	X(MusicVolume, "VOLUME", "VOLUME") \
	X(Music, "MUSIC", "MUSICA") \
	X(MusicConnect, "CONNECT YOUR PHONE", "CONNETTI IL TELEFONO") \
	X(MusicWaiting, "MUSIC UNAVAILABLE", "MUSICA NON DISPONIBILE") \
	X(MusicOpen, "OPEN A PLAYER ON PHONE", "APRI UN LETTORE SUL TELEFONO") \
	X(MusicEmpty, "NO TRACK", "NESSUN BRANO") \
	X(MusicPlaying, "PLAYING", "IN RIPRODUZIONE") \
	X(Route, "ROUTE", "NAVIGAZIONE") \
	X(Alerts, "ALERTS", "NOTIFICHE") \
	X(Weather, "WEATHER", "METEO") \
	X(Settings, "SETTINGS", "IMPOSTAZIONI") \
	X(FirmwareUpdate, "FIRMWARE UPDATE", "AGGIORNA FIRMWARE") \
	X(SdFirmware, "FIRMWARE FROM SD", "FIRMWARE DA SD") \
	X(SdChecking, "CHECKING SD FIRMWARE", "CONTROLLO FIRMWARE SD") \
	X(SdConfirm, "INSTALL THIS FIRMWARE?", "INSTALLARE FIRMWARE?") \
	X(SdKeepPower, "KEEP POWER AND SD IN", "NON SPEGNERE / TOGLIERE SD") \
	X(OtaWifi, "CONNECT TO WI-FI:", "CONNETTI AL WI-FI:") \
	X(OtaPassword, "WI-FI PASSWORD:", "PASSWORD WI-FI:") \
	X(OtaBrowser, "OPEN BROWSER:", "APRI NEL BROWSER:") \
	X(OtaReady, "READY / 5 MIN TIMEOUT", "PRONTO / SCADE IN 5 MIN") \
	X(OtaWriting, "WRITING / KEEP POWER ON", "SCRITTURA / NON SPEGNERE") \
	X(OtaRestart, "VERIFIED / RESTARTING", "VERIFICATO / RIAVVIO") \
	X(OtaScan, "SCAN TO JOIN WI-FI", "SCANSIONA PER WI-FI") \
	X(OtaDetails, "DETAILS", "DETTAGLI") \
	X(OtaQr, "WI-FI QR", "QR WI-FI") \
	X(Desk, "DESK TOOLS", "SCRIVANIA") \
	X(Games, "GAMES", "GIOCHI") \
	X(GamesHint, "SWIPE DOWN: GAMES", "SCENDI: GIOCHI") \
	X(Dice, "POLYHEDRAL DICE", "DADI POLIEDRICI") \
	X(DiceShakeRoll, "SHAKE OR TAP: ROLL", "SCUOTI O TOCCA: LANCIA") \
	X(FlappyBird, "FLAPPY BIRD", "FLAPPY BIRD") \
	X(FlappyStart, "TAP TO START", "TOCCA PER INIZIARE") \
	X(FlappyTap, "TAP TO FLAP", "TOCCA PER VOLARE") \
	X(FlappyRetry, "GAME OVER / TAP TO RETRY", "HAI PERSO / TOCCA: RIPROVA") \
	X(Drive, "DRIVE", "CRUSCOTTO") \
	X(DriveLean, "LEAN", "PIEGA") \
	X(DriveTrip, "TRIP", "VIAGGIO") \
	X(DriveHubTitle, "DRIVE TOOLS", "STRUMENTI GUIDA") \
	X(DriveBigLean, "BIG LEAN", "PIEGA GRANDE") \
	X(DriveGMeter, "G-METER", "G-METER") \
	X(DriveGraphTitle, "LEAN GRAPH", "GRAFICO PIEGA") \
	X(DriveMore, "SWIPE DOWN: DRIVE TOOLS", "SCENDI: STRUMENTI GUIDA") \
	X(DriveBack2, "SWIPE UP: DRIVE", "SALI: CRUSCOTTO") \
	X(DriveForward, "FWD", "AVANTI") \
	X(DriveBack, "BACK", "INDIETRO") \
	X(DriveObd, "OBD", "OBD") \
	X(ObdIdle, "OBD: IDLE", "OBD: INATTIVO") \
	X(ObdScanning, "SEARCHING BLE OBD DONGLE", "CERCO DONGLE OBD BLE") \
	X(ObdConnecting, "CONNECTING TO DONGLE", "CONNESSIONE AL DONGLE") \
	X(ObdStarting, "STARTING ELM327", "AVVIO ELM327") \
	X(ObdLive, "OBD LIVE", "OBD ATTIVO") \
	X(ObdNoEcu, "IGNITION OFF / NO ECU", "QUADRO SPENTO / NO ECU") \
	X(ObdWater, "H2O ", "ACQUA ") \
	X(ObdThrottle, "GAS ", "GAS ") \
	X(ObdFuel, "FUEL ", "BENZ. ") \
	X(ObdBattery, "BAT ", "BAT ") \
	X(ObdMil, "CHECK ENGINE ", "SPIA MOTORE ") \
	X(ObdNoFaults, "NO STORED FAULTS", "NESSUN GUASTO") \
	X(ObdOff, "OBD SCAN OFF (SETTINGS)", "OBD SPENTO (IMPOSTAZ.)") \
	X(ObdScan, "OBD DONGLE SCAN", "RICERCA DONGLE OBD") \
	X(DriveLegend, "LEAN / LATERAL G", "PIEGA / G LATERALE") \
	X(DriveHint, "HOLD STILL TO ZERO", "TIENI FERMO PER AZZERARE") \
	X(DriveZero, "ZERO", "AZZERA") \
	X(DiceRoll, "ROLL", "LANCIA") \
	X(DiceTotal, "TOTAL", "TOTALE") \
	X(DiceFirst, "DIE 1", "DADO 1") \
	X(DiceCritical, "CRITICAL", "CRITICO") \
	X(DiceFumble, "FUMBLE", "FALLITO") \
	X(BigClock, "BIG CLOCK", "OROLOGIO GRANDE") \
	X(Overview, "DESK OVERVIEW", "RIEPILOGO") \
	X(Timer, "FOCUS TIMER", "TIMER") \
	X(Device, "DEVICE STATUS", "STATO DISPOSITIVO") \
	X(Pong, "PONG", "PONG") \
	X(AnalogClock, "ANALOG CLOCK", "OROLOGIO ANALOGICO") \
	X(Tris, "TIC TAC TOE", "TRIS") \
	X(TrisTurn, "YOUR TURN / X", "TOCCA A TE / X") \
	X(TrisThinking, "MOCHI THINKS / O", "MOCHI PENSA / O") \
	X(TrisWin, "YOU WIN!", "HAI VINTO!") \
	X(TrisLose, "MOCHI WINS!", "VINCE MOCHI!") \
	X(TrisDraw, "DRAW", "PAREGGIO") \
	X(TrisNew, "NEW GAME", "NUOVA PARTITA") \
	X(PongHint, "TAP TO MOVE LEFT PADDLE", "TOCCA: MUOVI RACCHETTA") \
	X(CallTitle, "PHONE CALL", "CHIAMATA") \
	X(CallTag, "CALL", "TEL") \
	X(Quiet, "ALL QUIET FOR NOW", "NESSUNA NOTIFICA") \
	X(AlertHint, "ALERTS APPEAR HERE", "LE NOTIFICHE APPAIONO QUI") \
	X(Dismiss, "CLEAR", "TOGLI") \
	X(CalibrateImu, "IMU ZERO", "ZERO IMU") \
	X(Calibrating, "KEEP STILL", "FERMO") \
	X(Calibrate, "CALIBRATE", "CALIBRA") \
	X(Calibrated, "READY", "PRONTO") \
	X(AutoRotate, "AUTO ROTATE", "AUTO RUOTA") \
	X(Phone, "PHONE", "TELEFONO") \
	X(Incoming, "INCOMING CALL", "CHIAMATA IN ARRIVO") \
	X(UnknownCaller, "UNKNOWN CALLER", "NUMERO SCONOSCIUTO") \
	X(AnswerPhone, "ANSWER ON YOUR PHONE", "RISPONDI DAL TELEFONO") \
	X(NoRoute, "NO ACTIVE ROUTE", "NESSUN PERCORSO") \
	X(RouteHint, "START A ROUTE ON PHONE", "AVVIA IL PERCORSO SUL TELEFONO") \
	X(WeatherMissing, "WEATHER NOT SYNCED", "METEO NON SINCRONIZZATO") \
	X(WeatherHint, "ENABLE WEATHER IN CHRONOS", "ATTIVA METEO IN CHRONOS") \
	X(ForecastPending, "FORECAST PENDING", "PREVISIONI IN ATTESA") \
	X(Stale, "STALE / OPEN CHRONOS", "DATI VECCHI / APRI CHRONOS") \
	X(Updated, "UPDATED ", "AGGIORNATO ") \
	X(SyncWeather, "SYNC / REFRESH", "SINCRONIZZA / AGGIORNA") \
	X(LocalTime, "LOCAL TIME", "ORA LOCALE") \
	X(PhoneSync, "PHONE SYNC", "SYNC TELEFONO") \
	X(Waiting, "WAITING", "IN ATTESA") \
	X(OpenChronos, "OPEN CHRONOS", "APRI CHRONOS") \
	X(OpenSync, "OPEN CHRONOS TO SYNC", "APRI CHRONOS PER SYNC") \
	X(TimeUnavailable, "TIME UNAVAILABLE", "ORA NON DISPONIBILE") \
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
	X(Timer5, "5 MIN", "5 MIN") \
	X(Timer15, "15 MIN", "15 MIN") \
	X(Pomodoro, "POMODORO 25/5", "POMODORO 25/5") \
	X(Stopwatch, "STOPWATCH", "CRONOMETRO") \
	X(Elapsed, "ELAPSED", "TRASCORSO") \
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
	X(EyesLabel, "EYES", "OCCHI") \
	X(ColorLabel, "COLOR", "COLORE") \
	X(MoodLabel, "MOOD", "ESPRESSIONE") \
	X(RouteDefault, "ROUTE DEFAULT", "NAV PREDEF.") \
	X(Large, "LARGE", "GRANDE") \
	X(Details, "DETAILS", "DETTAGLI") \
	X(DayLevel, "DAY LEVEL", "LUCE GIORNO") \
	X(NightAuto, "AUTO NIGHT", "NOTTE AUTOMATICA") \
	X(NightStart, "NIGHT STARTS", "INIZIO NOTTE") \
	X(NightEnd, "NIGHT ENDS", "FINE NOTTE") \
	X(NightManual, "MANUAL NIGHT", "NOTTE MANUALE") \
	X(Night, "NIGHT", "NOTTE") \
	X(NightLevel, "NIGHT LEVEL", "LUCE NOTTE") \
	X(IdleDim, "IDLE DIM", "LUCE INATTIVA") \
	X(ScreenFlip, "SCREEN FLIP", "RUOTA SCHERMO") \
	X(Dashboard, "DASHBOARD", "DASHBOARD") \
	X(Shake, "SHAKE DIZZY", "SCOSSA") \
	X(RandomMood, "RANDOM MOOD", "FACCE CASUALI") \
	X(ImuMood, "IMU MOOD", "FACCE IMU") \
	X(Animation, "ANIMATION", "ANIMAZIONE") \
	X(Language, "LANGUAGE", "LINGUA") \
	X(MotionHint, "DASHBOARD/NIGHT: CALMER", "AUTO/NOTTE: MOVIMENTI RIDOTTI") \
	X(SoundSd, "SD SOUNDS", "SUONI DA SD") \
	X(Sound, "SOUND", "SUONI") \
	X(Level, "LEVEL", "LIVELLO") \
	X(NightMute, "NIGHT MUTE", "MUTO NOTTE") \
	X(SoundOptions, "SOUND OPTIONS", "OPZIONI SUONI") \
	X(TestSound, "TEST SOUND", "PROVA SUONI") \
	X(Test, "TEST", "PROVA") \
	X(Open, "OPEN", "APRI") \
	X(Faces, "FACES", "FACCE") \
	X(FaceLibrary, "FACE LIBRARY", "LIBRERIA FACCE") \
	X(Favorites, "FAVORITES", "PREFERITE") \
	X(EditFavorites, "EDIT FAVORITES", "MODIFICA PREFERITE") \
	X(NoFavorites, "NO FAVORITES ENABLED", "NESSUNA PREFERITA ATTIVA") \
	X(KeepOneFace, "KEEP ONE FACE", "TIENI UNA FACCIA") \
	X(Enabled, "ENABLED", "ATTIVE") \
	X(FacesWifi, "WI-FI SETTINGS", "IMPOSTAZIONI WI-FI") \
	X(FacesReady, "SETTINGS IN BROWSER", "MODIFICA DAL BROWSER") \
	X(WebSettings, "WI-FI SETTINGS", "IMPOSTAZIONI WI-FI") \
	X(WebScan, "SCAN TO OPEN SETTINGS", "SCANSIONA PER IMPOSTAZIONI") \
	X(WebSameWifi, "USE THE SAME WI-FI", "USA LO STESSO WI-FI") \
	X(WebAuthorize, "SCAN QR TO AUTHORIZE", "SCANSIONA QR PER ACCEDERE") \
	X(WebQr, "WEB QR", "QR WEB") \
	X(WeatherStaleWifi, "STALE / CHECK WI-FI", "DATI VECCHI / WI-FI") \
	X(TimerLabel, "TIMER", "TIMER") \
	X(CallsAlerts, "ALERTS / CALLS", "AVVISI / TEL.") \
	X(Ambient, "AMBIENT", "SPONTANEI") \
	X(Charging, "CHARGING", "RICARICA") \
	X(On, "ON", "SI") \
	X(Off, "OFF", "NO") \
	X(Save, "SAVE", "SALVA") \
	X(Saved, "SAVED", "SALVATO") \
	X(ChangesSaved, "Changes saved", "Modifiche salvate") \
	X(Font, "Font", "Font") \
	X(Failed, "FAILED", "ERRORE") \
	X(EyesButton, "EYES", "OCCHI") \
	X(Back, "BACK", "INDIETRO") \
	X(Normal, "NORMAL", "NORMALE") \
	X(Flipped, "FLIPPED", "GIRATO") \
	X(Mochi, "MOCHI", "MOCHI") \
	X(Bloub, "BLOUB", "BLOUB") \
	X(Trapez, "TRAPEZ", "TRAPEZ") \
	X(Anime, "ANIME", "ANIME") \
	X(Kitsune, "KITSUNE", "KITSUNE") \
	X(Drl, "HEADLIGHT DRL", "FARI DRL") \
	X(White, "WHITE", "BIANCO") \
	X(Mint, "MINT", "MENTA") \
	X(Cyan, "CYAN", "CIANO") \
	X(Amber, "AMBER", "AMBRA") \
	X(Pink, "PINK", "ROSA") \
	X(Blue, "BLUE", "BLU") \
	X(ExprNeutral, "NEUTRAL", "NEUTRO") \
	X(ExprHappy, "HAPPY", "FELICE") \
	X(ExprSad, "SAD", "TRISTE") \
	X(ExprAngry, "ANGRY", "ARRABBIATO") \
	X(ExprSurprised, "SURPRISED", "SORPRESO") \
	X(ExprSleepy, "SLEEPY", "ASSONNATO") \
	X(ExprLove, "LOVE", "AMORE") \
	X(ExprDizzy, "DIZZY", "GIRATO") \
	X(ExprGlee, "GLEE", "GIOIA") \
	X(ExprWorried, "WORRIED", "INQUIETO") \
	X(ExprFocused, "FOCUSED", "ATTENTO") \
	X(ExprAnnoyed, "ANNOYED", "SECCATO") \
	X(ExprSkeptic, "SKEPTIC", "SCETTICO") \
	X(ExprFrustrated, "FRUSTRATED", "FRUSTRATO") \
	X(ExprUnimpressed, "UNIMPRESSED", "NOIOSO") \
	X(ExprSuspicious, "SUSPICIOUS", "SOSPETTOSO") \
	X(ExprSquint, "SQUINT", "STRIZZATA") \
	X(ExprFurious, "FURIOUS", "FURIOSO") \
	X(ExprScared, "SCARED", "SPAVENTATO") \
	X(ExprAwe, "AWE", "STUPORE") \
	X(ExprSleepyEyes, "SLEEPY EYES", "ASSONNATI") \
	X(ExprSadDown, "SAD DOWN", "TRISTE GIU") \
	X(ExprSadUp, "SAD UP", "TRISTE SU") \
	X(ExprExcited, "EXCITED", "ESALTATO") \
	X(ExprTerrified, "TERRIFIED", "TERRORE") \
	X(ExprDisgusted, "DISGUSTED", "DISGUSTATO") \
	X(ExprShy, "SHY", "TIMIDO") \
	X(ExprProud, "PROUD", "ORGOGLIOSO") \
	X(ExprTired, "TIRED", "STANCO") \
	X(ExprGrin, "GRIN", "GHIGNO") \
	X(ExprPanicked, "PANICKED", "IN PANICO") \
	X(ExprGlum, "GLUM", "MESTO") \
	X(ExprSmug, "SMUG", "SDEGNO") \
	X(ExprDeadpan, "DEADPAN", "IMPASSIBILE") \
	X(ExprZombie, "ZOMBIE", "ZOMBIE") \
	X(ExprConfused, "CONFUSED", "CONFUSO") \
	X(ExprCurious, "CURIOUS", "CURIOSO") \
	X(ExprWink, "WINK", "OCCHIOLINO") \
	X(ExprWondering, "WONDERING", "PERPLESSO") \
	X(ExprStar, "STAR", "STELLA") \
	X(ExprCrying, "CRYING", "PIANGE") \
	X(ExprSwirly, "SWIRLY", "VORTICE") \
	X(ExprCat, "CAT", "GATTO") \
	X(ExprHalo, "HALO", "AUREOLA") \
	X(ExprDevil, "DEVIL", "DIAVOLO") \
	X(ExprFire, "FIRE", "FUOCO") \
	X(ExprMoon, "MOON", "LUNA") \
	X(ExprSparkle, "SPARKLE", "SCINTILLA") \
	X(ExprBolt, "BOLT", "FULMINE") \
	X(ExprDiamond, "DIAMOND", "DIAMANTE") \
	X(ExprRing, "RING", "ANELLO") \
	X(ExprTarget, "TARGET", "BERSAGLIO") \
	X(ExprMoney, "MONEY", "SOLDI") \
	X(ExprMildHappy, "MILD HAPPY", "SODDISFATTO") \
	X(ExprSmirk, "SMIRK", "SMORFIA") \
	X(ExprPout, "POUT", "MUSO") \
	X(ExprKiss, "KISS", "BACIO") \
	X(ExprNervous, "NERVOUS", "NERVOSO") \
	X(ExprShocked, "SHOCKED", "SORPRESO") \
	X(ExprMelting, "MELTING", "SCIOGLIE") \
	X(ExprLazy, "LAZY", "PIGRO") \
	X(ExprDoze, "DOZE", "PISOLINO") \
	X(ExprGroggy, "GROGGY", "STORDITO") \
	X(ExprDaydream, "DAYDREAM", "SOGNANTE") \
	X(ExprGrumpy, "GRUMPY", "BRONTOLO") \
	X(ExprSassy, "SASSY", "SFACCIATO") \
	X(ExprCheeky, "CHEEKY", "BIRICHINO") \
	X(ExprFlirt, "FLIRT", "LUSINGA") \
	X(ExprZen, "ZEN", "ZEN") \
	X(ExprMerciful, "MERCIFUL", "PIETOSO") \
	X(ExprManic, "MANIC", "FRENETICO") \
	X(ExprSideEye, "SIDE EYE", "DI TRAVERSO") \
	X(ExprBeaming, "BEAMING", "RAGGIANTE") \
	X(ExprMusic, "MUSIC", "MUSICA") \
	X(ExprLowBatt, "LOW BATTERY", "SCARICA") \
	X(ExprPower, "POWER", "STAND-BY") \
	X(ExprLoading, "LOADING", "ATTENDI") \
	X(ExprIdea, "IDEA", "IDEA") \
	X(ExprHyper, "HYPER", "IPER") \
	X(ExprSquish, "SQUISH", "SCHIACCIATO") \
	X(ExprDerp, "DERP", "DERP") \
	X(ExprGlitch, "GLITCH", "GLITCH") \
	X(ExprTwitch, "TWITCH", "TIC") \
	X(ExprSneeze, "SNEEZE", "STARNUTO") \
	X(ExprJudge, "JUDGE", "GIUDICE") \
	X(ExprQuestion, "QUESTION", "DOMANDA") \
	X(ExprCalculating, "CALCULATING", "CALCOLA") \
	X(ExprCyborg, "CYBORG", "CYBORG") \
	X(ExprArcHappy, "HAPPY ARCS", "ARCHI FELICI") \
	X(ExprArcSad, "SAD ARCS", "ARCHI TRISTI") \
	X(ExprSqueezed, "SQUEEZED", "STRIZZATI") \
	X(ExprTilted, "TILTED", "INCLINATI") \
	X(ExprPong, "PONG DEMO", "PONG DEMO") \
	X(ExprClockEyes, "CLOCK EYES", "OCCHI OROLOGIO") \
	X(ExprGauge, "GAUGE DEMO", "TACHIMETRO DEMO") \
	X(ExprGlow, "COLOR GLOW", "BAGLIORE COLORI") \
	X(ExprHeadlights, "HEADLIGHTS", "OCCHI FARO") \
	X(ExprDancing, "DANCING", "EQUALIZZATORE") \
	X(ExprPolice, "RED / BLUE", "ROSSO / BLU") \
	X(ExprRain, "RAIN", "PIOGGIA") \
	X(ExprGrowing, "GROWING", "CRESCITA") \
	X(ExprShrink, "SHRINK", "RESTRINGI") \
	X(ExprSwinging, "SWINGING", "DONDOLIO") \
	X(ExprBlinding, "AMBER RAYS", "RAGGI AMBRA") \
	X(ExprBuzzing, "BUZZING", "RONZIO") \
	X(ExprSick, "SICK", "MALESSERE") \
	X(ExprSplash, "SPLASH", "ONDA") \
	X(ExprSpraying, "SPRAYING", "SPRUZZI") \
	X(ExprSushi, "SUSHI", "SUSHI") \
	X(ExprEnergetic, "TURBINES", "TURBINE") \
	X(ExprSmoke, "SMOKE", "NUVOLE") \
	X(ExprMenacing, "ORBITAL RINGS", "ANELLI ORBITALI") \
	X(ExprHello, "HELLO", "SALUTO") \
	X(ExprDistracted, "DISTRACTED", "DISTRATTO") \
	X(ExprLookLeft, "LOOK LEFT", "GUARDA SINISTRA") \
	X(ExprLookRight, "LOOK RIGHT", "GUARDA DESTRA") \
	X(ExprLookDown, "LOOK DOWN", "GUARDA GIU") \
	X(ExprStargazing, "STARGAZING", "CIELO STELLATO") \
	X(ExprMistake, "MISTAKE", "ERRORE") \
	X(ExprEncouragement, "ENCOURAGE", "INCORAGGIA") \
	X(ExprAdore, "ADORE", "ADORARE") \
	X(ExprLaughing, "LAUGHING", "RIDERE") \
	X(ExprEnraged, "ENRAGED", "INFURIATO") \
	X(ExprTraffic, "TRAFFIC", "SEMAFORO") \
	X(ExprNeon, "NEON", "NEON") \
	X(ExprWheels, "WHEELS", "RUOTE") \
	X(ExprSakura, "SAKURA", "PETALI") \
	X(ExprSamurai, "SAMURAI", "SAMURAI") \
	X(ExprMecha, "MECHA", "MECHA") \
	X(ExprBamboo, "BAMBOO", "BAMBU") \
	X(ExprTitan, "TITAN", "TITANO") \
	X(ExprUpsideDown, "UPSIDE DOWN", "CAPOVOLTO") \
	X(ExprUwu, "UWU", "UWU") \
	X(ExprYokai, "YOKAI", "YOKAI") \
	X(ExprTicTacToe, "TIC TAC TOE", "TRIS") \
	X(ExprMechaMask, "MECHA MASK", "MECHA MASCHERA") \
	X(ExprMechaBreak, "MECHA BREAK", "MECHA PEZZI") \
	X(ExprHollowAnger, "HOLLOW ANGER", "RABBIA CAVA") \
	X(ExprSnickering, "SNICKERING", "RISATINA") \
	X(ExprHiccup, "HICCUP", "SINGHIOZZO") \
	X(ExprRetroTv, "RETRO TV", "TV RETRO") \
	X(ExprMechaPilot, "MECHA PILOT", "MECHA PILOTA") \
	X(ExprComet, "COMET", "COMETA") \
	X(ExprYakura, "YAKURA", "YAKURA") \
	X(ExprChineseDragon, "RED DRAGON", "DRAGO ROSSO") \
	X(ExprAngel, "ANGEL", "ANGELO") \
	X(ExprHacker, "HACKER", "HACKER") \
	X(ExprBerserk, "BERSERK", "FURIA") \
	X(ExprPortal, "PORTAL", "PORTALE") \
	X(ExprOrigami, "ORIGAMI", "ORIGAMI") \
	X(ExprGhost, "GHOST", "FANTASMA") \
	X(ExprEclipse, "ECLIPSE", "ECLISSI") \
	X(ExprDetective, "DETECTIVE", "DETECTIVE") \
	X(ExprPixelCrash, "PIXEL CRASH", "PIXEL CRASH") \
	X(ExprPowerUp, "POWER UP", "POTENZIAMENTO") \
	X(ExprNinja, "NINJA", "NINJA") \
	X(ExprAlchemist, "ALCHEMIST", "ALCHIMISTA") \
	X(ExprMagicalGirl, "MAGICAL GIRL", "MAGHETTA") \
	X(ExprMechaLock, "MECHA LOCK", "MECHA AGGANCIO") \
	X(ExprSpiritSeal, "SPIRIT SEAL", "SIGILLO") \
	X(ExprSweatDrop, "SWEAT DROP", "GOCCIA SUDORE") \
	X(ExprAnimeVein, "ANIME VEIN", "VENA RABBIA") \
	X(ExprChibiPanic, "CHIBI PANIC", "PANICO CHIBI") \
	X(ExprTsundere, "TSUNDERE", "TSUNDERE") \
	X(ExprMangaShock, "MANGA SHOCK", "SHOCK MANGA") \
	X(ExprChibiJoy, "CHIBI JOY", "GIOIA CHIBI") \
	X(ExprRubberBand, "RUBBER BAND", "ELASTICO") \
	X(ExprPeekaboo, "PEEKABOO", "CUCU") \
	X(ExprPopcorn, "POPCORN", "POPCORN") \
	X(ExprSpringEyes, "SPRING EYES", "OCCHI A MOLLA") \
	X(ExprComicGlasses, "COMIC GLASSES", "OCCHIALI BUFFI") \
	X(ExprBalloonEyes, "BALLOON EYES", "PALLONCINI") \
	X(ExprWipers, "WIPERS", "TERGICRISTALLI") \
	X(ExprTurnSignal, "TURN SIGNAL", "FRECCIA") \
	X(ExprFuelLow, "FUEL LOW", "RISERVA") \
	X(ExprDrift, "DRIFT", "DERAPATA") \
	X(ExprCarWash, "CAR WASH", "AUTOLAVAGGIO") \
	X(ExprParkingSensor, "PARKING", "PARCHEGGIO") \
	X(ExprTunnel, "TUNNEL", "GALLERIA") \
	X(ExprGearShift, "GEAR SHIFT", "CAMBIO MARCIA") \
	X(ExprCheckEngine, "CHECK ENGINE", "SPIA MOTORE") \
	X(ExprYandere, "YANDERE", "YANDERE") \
	X(ExprNapBubble, "NAP BUBBLE", "BOLLA PISOLINO") \
	X(ExprGloom, "GLOOM", "NUVOLETTA") \
	X(ExprDotEyes, "DOT EYES", "OCCHI A PUNTO") \
	X(ExprSteamBlush, "STEAM BLUSH", "VAPORE ROSSO") \
	X(ExprSnowfall, "SNOWFALL", "NEVICATA") \
	X(ExprHeatwave, "HEATWAVE", "AFA") \
	X(ExprRainbow, "RAINBOW", "ARCOBALENO") \
	X(ExprThunderstorm, "THUNDERSTORM", "TEMPORALE") \
	X(ExprFoggy, "FOGGY", "NEBBIA") \
	X(ExprWindy, "WINDY", "VENTO") \
	X(ExprHail, "HAIL", "GRANDINE") \
	X(ExprSunrise, "SUNRISE", "ALBA") \
	X(ExprTornado, "TORNADO", "TORNADO") \
	X(ExprFrosty, "FROSTY", "GELO") \
	X(ExprRamen, "RAMEN", "RAMEN") \
	X(ExprIceCream, "ICE CREAM", "GELATO") \
	X(ExprCoffee, "COFFEE", "CAFFE") \
	X(ExprPizza, "PIZZA", "PIZZA") \
	X(ExprBubbleTea, "BUBBLE TEA", "BUBBLE TEA") \
	X(ExprDonut, "DONUT", "CIAMBELLA") \
	X(ExprOnigiri, "ONIGIRI", "ONIGIRI") \
	X(ExprCookie, "COOKIE", "BISCOTTO") \
	X(ExprBunny, "BUNNY", "CONIGLIO") \
	X(ExprFrog, "FROG", "RANA") \
	X(ExprOwl, "OWL", "GUFO") \
	X(ExprPenguin, "PENGUIN", "PINGUINO") \
	X(ExprBee, "BEE", "APE") \
	X(ExprFishBowl, "FISH BOWL", "ACQUARIO") \
	X(ExprPuppy, "PUPPY", "CUCCIOLO") \
	X(ExprButterfly, "BUTTERFLY", "FARFALLA") \
	X(ExprShoujoStars, "SHOUJO STARS", "STELLINE") \
	X(ExprChibiCry, "CHIBI CRY", "PIANTO CHIBI") \
	X(ExprDetermined, "DETERMINED", "DETERMINATO") \
	X(ExprFaint, "FAINT", "SVENUTO") \
	X(ExprKawaiiWink, "KAWAII WINK", "OCCHIOLINO") \
	X(ExprSneaky, "SNEAKY", "FURTIVO") \
	X(ExprHypnotized, "HYPNOTIZED", "IPNOTIZZATO") \
	X(ExprBrainFreeze, "BRAIN FREEZE", "GELO TESTA") \
	X(ExprDj, "DJ", "DJ") \
	X(ExprKaraoke, "KARAOKE", "KARAOKE") \
	X(ExprDisco, "DISCO", "DISCOTECA") \
	X(ExprDrumRoll, "DRUM ROLL", "RULLO TAMBURI") \
	X(ExprPartyPopper, "PARTY POPPER", "CORIANDOLI") \
	X(ExprBirthday, "BIRTHDAY", "COMPLEANNO") \
	X(ExprFireworks, "FIREWORKS", "FUOCHI") \
	X(ExprMaracas, "MARACAS", "MARACAS") \
	X(ExprWifiSearch, "WIFI SEARCH", "CERCA WIFI") \
	X(ExprDownload, "DOWNLOAD", "DOWNLOAD") \
	X(ExprPairing, "PAIRING", "ABBINAMENTO") \
	X(ExprCursor, "CURSOR", "CURSORE") \
	X(ExprErrorSign, "ERROR SIGN", "ERRORE") \
	X(ExprSyncSpin, "SYNC SPIN", "SINCRONIA") \
	X(ExprCharging, "CHARGING", "RICARICA") \
	X(ExprTyping, "TYPING", "DIGITAZIONE") \
	X(ExprSoccer, "SOCCER", "CALCIO") \
	X(ExprBasketball, "BASKETBALL", "BASKET") \
	X(ExprBowling, "BOWLING", "BOWLING") \
	X(ExprDiceRoll, "DICE ROLL", "DADI") \
	X(ExprFishing, "FISHING", "PESCA") \
	X(ExprYoyo, "YOYO", "YO-YO") \
	X(ExprJuggling, "JUGGLING", "GIOCOLIERE") \
	X(ExprTennis, "TENNIS", "TENNIS") \
	X(ExprUfo, "UFO", "UFO") \
	X(ExprRocket, "ROCKET", "RAZZO") \
	X(ExprPlanetRing, "PLANET RING", "PIANETA") \
	X(ExprBlackHole, "BLACK HOLE", "BUCO NERO") \
	X(ExprCrystalBall, "CRYSTAL BALL", "SFERA") \
	X(ExprWizard, "WIZARD", "MAGO") \
	X(ExprDragonEgg, "DRAGON EGG", "UOVO DRAGO") \
	X(ExprVampire, "VAMPIRE", "VAMPIRO") \
	X(ExprPumpkin, "PUMPKIN", "ZUCCA") \
	X(ExprAstronaut, "ASTRONAUT", "ASTRONAUTA") \
	X(Calm, "CALM", "CALMO") \
	X(Dasai, "NORMAL", "NORMALE") \
	X(Playful, "PLAYFUL", "VIVACE") \
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
	X(FaceFlow, "FACE FLOW", "CICLO FACCE") \
	X(PhoneMode, "PHONE MODE (RESTART)", "TELEFONO (RIAVVIO)") \
	X(AndroidMode, "ANDROID / CHRONOS", "ANDROID / CHRONOS") \
	X(IphoneMode, "IPHONE / ANCS", "IPHONE / ANCS") \
	X(Diagnostics, "DIAGNOSTICS", "DIAGNOSTICA") \
	X(DiagBoard, "BOARD", "SCHEDA") \
	X(DiagPhone, "PHONE / BLE", "TELEFONO / BLE") \
	X(DiagWifi, "WI-FI STATION", "CONNESSIONE WI-FI") \
	X(DiagImu, "MOTION SENSOR", "SENSORE MOVIMENTO") \
	X(DiagSd, "SD: LAST AUDIO ATTEMPT", "SD: ULTIMO USO AUDIO") \
	X(DiagHeap, "FREE INTERNAL RAM", "RAM INTERNA LIBERA") \
	X(DiagMinimum, "MINIMUM FREE RAM", "MINIMO RAM LIBERA") \
	X(DiagPsram, "FREE PSRAM", "PSRAM LIBERA") \
	X(DiagUptime, "UPTIME", "TEMPO ACCESO") \
	X(DiagReset, "LAST RESET REASON", "ULTIMO RIAVVIO") \
	X(DiagUnsupported, "NOT SUPPORTED", "NON SUPPORTATA") \
	X(DiagReady, "READY", "PRONTA") \
	X(DiagArmed, "ENABLED", "ABILITATO") \
	X(DiagMissing, "UNAVAILABLE", "NON DISPONIBILE") \
	X(DiagUnchecked, "NOT CHECKED", "NON VERIFICATA")
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
