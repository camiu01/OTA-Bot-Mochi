# Security Policy

---

## Supported Versions

| Version | Supported |
| :--- | :--- |
| 1.x | Yes |
| < 1.0 | No |

Only the latest 1.x release receives security updates. Older revisions, forks without upstream patches, and experimental branches are not supported.

---

## Reporting a Vulnerability

Report vulnerabilities via GitHub Issues on the OTA-Bot-Mochi repository.

Include:

- Affected version, board target, and commit hash
- Steps to reproduce
- Impact assessment
- Suggested mitigation if known

Do not include secrets, tokens, credentials, private keys, personal data, or physical access details in reports. Redact logs before posting.

We aim to acknowledge valid reports within 7 days and to provide a remediation plan or timeline once triage is complete.

---

## Scope

In scope:

- ESP32-S3 firmware in this repository
- BLE Chronos integration and callbacks
- IMU handling and expression logic
- Display rendering and UI screens

Out of scope:

- Physical hardware attacks requiring device possession
- Third-party libraries, Arduino core, toolchains, and companion mobile apps
- Social engineering and physical tampering

---

## Safe Disclosure

Do not publicly disclose an unpatched vulnerability. Give maintainers reasonable time to fix and release before publishing details. Coordinate disclosure through the original GitHub Issue thread. Credit is given on request once the fix is released.
