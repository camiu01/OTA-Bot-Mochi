# Changelog

## 1.3.0 — planned, not yet published

Changes since v1.2.1. This entry does not create a Git tag, GitHub release or
downloadable release binaries.

### Added

- iPhone ANCS notifications without Chronos, with a saved Phone mode selector.
  SAVE and restart to switch between Android/Chronos and iPhone/ANCS.
- Saved 2.4 GHz Wi-Fi, background reconnection, automatic NTP and Open-Meteo
  current weather/three-day forecasts for a city selected in web settings.
- Temporary local browser settings and face-library editors, with a protected
  LAN QR when connected or a password-protected configuration AP otherwise.
- Global saved face exclusions across manual selection, automatic playback,
  touch, petting, startup, IMU and shake reactions. At least one face stays enabled.
- Playable Tris desk tool and a reversible tic-tac-toe eye animation.

### Changed

- Wi-Fi configuration opens only when requested from Settings or the face
  library. Startup and connection failures never interrupt the normal screen.
- Automatically opened Alerts returns to Eyes after 60 seconds without a new
  alert or touch; manual page selection cancels the return.
- Display/face settings and network settings have separate explicit saves.
- Closing a web session stops its server/AP but retains enabled saved Wi-Fi.
- Expanded regression coverage for networking, web authorization, face filters,
  NTP, ANCS parsing, settings migration and game behavior.

### Compatibility and limits

- The settings profile is still 13 bytes; version seven adds Phone mode and
  migrates versions one through six. Network and face records are independent.
- Existing 16 MB OTA layout v1 is unchanged. Use the matching board application
  for updates; a full image or flash erase can remove saved data.
- ANCS provides notifications, not phone navigation or a call overlay. Clock
  and weather can use Wi-Fi independently of the selected phone mode.
- Local administration uses HTTP on a trusted private LAN; development NVS
  credentials are not encrypted. No automatic screen/server exposure at boot.
- No automatic deployment, signed firmware or boot-health rollback is added.

### Validation

- User-confirmed on AMOLED V2: Wi-Fi, settings changes and ANCS notifications.
- Local LCD/AMOLED PlatformIO builds and targeted ANCS/settings/web/panel checks
  passed for source checkpoint `8b752ba`. A complete native suite passed before
  ANCS integration; this is not a new complete post-ANCS or remote CI result.
- Separate LCD hardware checks, reconnect/failure matrices, browser OTA recovery,
  endurance, power and battery measurements remain open in [TODO](TODO.md).
