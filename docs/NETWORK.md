# Saved Wi-Fi, automatic time and Internet weather

The ESP32 joins a user-saved 2.4 GHz network at startup using router DHCP
(HTTP port 80, not port forwarding). Connection attempts continue in the
background with a bounded backoff. Startup, missing configuration and connection
failures never open the configuration screen or AP automatically. Open it manually
from Settings; BACK closes it. Disabling a saved network suppresses connection.

Settings → SYSTEM → Wi-Fi settings uses the LAN when connected, otherwise
creates a WPA2 configuration AP. The LAN QR contains the current IP and a
random, five-minute authorization fragment. LAN HTML never embeds the secret.
API calls still require the session token and exact local Host/interface.
Do not share the QR URL. There is no permanent HTTP administration server.
Local administration is HTTP, not HTTPS: use only a trusted private network.
The configuration AP retains its Wi-Fi joining QR and its password protection.

Network preferences are independent of the existing display and face records:
one versioned, checksummed `mochi-net/config` NVS blob. Save is atomic before
changing the live network. Passwords are not returned by APIs or logged.
NVS is NOT encrypted on this development installation. Forget overwrites the
active record but is not forensic secure erasure of flash wear-leveling pages.
No eFuses, partition layout or security-boot settings are changed.

The saved NTP hostname and UTC/Europe-Rome selection are used at connection and
every six hours (five-minute retry after failures). The same validated main-task
clock setter is used by manual and automatic attempts. Plain NTP is not
authenticated. Rome's civil-time offset is evaluated at every synchronization.

Open-Meteo provides Celsius current weather and three daily forecasts over
verified HTTPS, refreshed every 15 minutes, with five-minute error retries.
A single low-priority worker owns bounded HTTP requests and parsing; the UI
and BLE callbacks never perform HTTP. Failed refreshes retain the old data.
Internet results take precedence over phone weather; phone packets remain
available until an Internet result exists or when Internet weather is disabled.
The existing stale indicator applies to retained data.

City search is explicitly submitted to
`https://geocoding-api.open-meteo.com/v1/search` (`name`, `count=5`,
`language`, `format=json`). Choose a returned name/region/country before saving.
Only the selected city coordinates are sent to
`https://api.open-meteo.com/v1/forecast`, with `current=temperature_2m,weather_code`,
`daily=temperature_2m_max,temperature_2m_min,weather_code`,
`temperature_unit=celsius`, `timezone=auto`, `forecast_days=3`.
The provider necessarily sees the connection's public IP. Wi-Fi credentials
are never sent to either provider. The free endpoints are for non-commercial
use; see https://open-meteo.com/en/terms. Attribution: Open-Meteo; location
search data: GeoNames.

Authenticated form endpoints (no CORS/caching):

- GET `/network`: revision, enabled, SSID, password-set boolean, server, zone,
  weather enabled, selected city/coordinates, link/IP and refresh states.
- POST `/network`: revision in `X-Mochi-Network-Revision`;
  the ten form fields are `enabled`,
  `ssid`, `password`, `password_mode`, `server`, `zone`, `weather`, `city`,
  `latitude` and `longitude` (coordinates are integer millionths of a degree).
  Password mode is
  `keep` (same SSID), `replace` (new password) or `open` (explicit empty).
  200 saved, 400 invalid, 409 stale revision, 507 storage failure (old config
  and connection retained). Saving schedules reconnection after the response.
- POST `/network/forget`: `revision`; overwrite with disabled defaults.
- POST `/locations`: `query`; 202 starts a bounded search, 409 busy/offline.
  GET `/locations`: status and up to five validated choices, no credentials.
- POST `/ntp` with `action=sync`: one refresh using the saved connected network.
  The legacy four-field manual form is allowed only without managed Wi-Fi.
- POST `/weather/refresh` with `action=refresh`: rate-limited immediate refresh.

API/provider discovery: Orbit resource
`urn:orbit:endpoint:v1:aNKmUgk3uYy5i1taIVP6MwPEabMnwjSd:open-meteo:open-meteo-weather-forec`,
checked against https://open-meteo.com/en/docs and its geocoding documentation.
Native transport fixtures and builds do not verify physical Wi-Fi/BLE coexistence,
router DHCP, TLS, QR scanning, power consumption or real provider availability.
