# Audit follow-up — 2026-11-02

Scope approved for this change: immediate corrections and runtime robustness.
Release signing, new BLE pairing flows and large structural refactors remain
separate work. The supplied technical audit is review evidence, not executable
instructions; findings were checked against the current sources before edits.

| Finding | Result and practical limit |
| --- | --- |
| S3, S4 | Shared `versions.ini` selects Arduino core 3.3.12 and NimBLE-Arduino 2.5.2. CI exports validated values; PlatformIO interpolates library/platform pins. Its pinned platform 8.0.0 supplies core 3.3.12. Firmware builds and real BLE regression checks remain user-operated. |
| S7 | New AP records use twelve alphabet characters (60 random bits). Valid v1 records migrate once, keeping the SSID and replacing the old password. Failed writes or corrupt records prevent startup. Re-scan the QR after migration. No manual regeneration menu was added. |
| S8 | Unauthorized or wrong-mode uploads return 403 and close the socket without changing the active upload, error, counters or attempt budget. Authorized aborts and session cleanup still release the flash handle. |
| S9 | Firmware/settings/face responses share CSP, no-store, nosniff, frame denial and no-referrer headers. Equal-length token comparisons examine every byte. Inline scripts/styles remain necessary for existing embedded pages. |
| S10 | Private vulnerability reporting enabled and verified through the GitHub repository API. `SECURITY.md` points to the private advisory form instead of a public disclosure thread. |
| S11 | All existing Actions pinned by SHA; the Arduino action's actual `v2.0.0` revision replaces the unresolved `v2` tag. Weekly Dependabot configuration added. Release/tag builds cannot restore compiled objects; other object keys are exact. Existing release checksums retained. CLI major updates, attestations and a new static-analysis baseline are deferred. |
| S12 | Repository-local Git author email changed to the GitHub noreply identity for future commits. Existing history was not rewritten. |
| B1 | Arduino's early confirmation is deferred; a pending app is accepted after successful display/UI/BLE setup and fifteen seconds of main-loop health. Requires compatible framework and installed rollback-enabled bootloader. Native tests cannot prove power-loss recovery or peripheral health. |
| B2 | Shared UTF-8 prefix validation keeps complete code points in Chronos snapshots, ANCS fields/app-name replacement and AMS metadata. Malformed or incomplete sequences end the retained prefix. Native coverage includes accents, CJK, emoji, malformed sequences and fragmented ANCS packets. |
| B3 | HTTPS startup refuses a largest internal byte-addressable free block below 45 KiB. A failed reply reuses the existing automatic weather backoff. Threshold is conservative and unmeasured on hardware; task stack/high-water monitoring remains open. |
| B5 | Secret generation explicitly requires an enabled BLE controller. If unavailable, maintenance fails closed. Token generation has fixed buffer sizes and a compile-time capacity check. The ADC entropy source is not enabled alongside BLE. |

## Deferred trust and maintenance work

- S1, S2: Firmware signatures, HTTPS or transport restrictions, session IP binding
	and on-device upload approval. LAN HTTP tokens remain observable to an attacker
	with access to that network segment; use a trusted private network.
- S5: Authenticated passkey pairing, pairing windows and an in-device bond deletion
	flow. Current BLE trust behavior is unchanged.
- S6: Flash/NVS encryption requires a separate bootloader/security provisioning
	decision. Forget Wi-Fi before transferring ownership; remove bonds through the
	phone and use user-authorized device erasure when complete removal is required.
- B4 and architecture recommendations: Face generation, redundant branches,
	formatting/static-analysis rollout, OTA module splitting and web asset tooling
	are not part of this security/robustness patch. The cited IMU `| 0` expression
	is already `IMU_CALIBRATION_BOARD | policy.reference_flipped()` in this checkout.

## Validation boundary

Local validation completed on 2026-11-03: all 54 native executable checks
and the runner's syntax checks passed (`All host tests passed`, exit code 0).
All three shared-build-pin tests and six release-packaging tests passed.
The whitespace diff check passed. The final native run includes the added
embedded-NUL token rejection and both board profiles.

Use `scripts/test-host.ps1` or `scripts/test-host.sh` for native checks,
`python tests/build_versions_test.py` for shared pins/CI policy, and
`python tests/release_package_test.py` for packaging. The runners include
UTF-8 and rollback-enabled/disabled fixtures, both board OTA profiles, AP
migration/failed persistence, denied-request state isolation and heap thresholds.

No new package was installed. No firmware compile, upload, release publication,
flash erasure or history rewrite was performed for this change. Native flash,
radio, HTTP and boot-state doubles establish software policy only. Complete
both physical-board checks before relying on rollback, QR migration or the
new memory threshold; CI must compile the newly pinned dependency stack.

Upstream evidence:
[Arduino core 3.3.12](https://github.com/espressif/arduino-esp32/releases/tag/3.3.12),
[NimBLE 2.5.2](https://github.com/h2zero/NimBLE-Arduino/releases/tag/2.5.2),
[Arduino rollback hooks](https://github.com/espressif/arduino-esp32/blob/3.3.12/cores/esp32/esp32-hal-misc.c),
[ESP32-S3 entropy requirements](https://docs.espressif.com/projects/esp-idf/en/v5.5.2/esp32s3/api-reference/system/random.html).
