# CLAUDE.md

Guidance for Claude Code when working in this repository.

## Project

PANDA is a PBL4 student project: a desk robot that helps children practice English vocabulary via voice (TTS/STT), shows expressive eyes and English words/pictures on an OLED, and reports progress via MQTT to a cloud server with a web dashboard. Full proposal context: see `README.md` and the team's PBL4 proposal PDF.

**Scope note:** the team's original proposal targeted two-way Vietnamese↔Japanese vocabulary practice. The team narrowed this to one-way English vocabulary practice for children after the AI teammate found building/customizing bidirectional Vietnamese↔Japanese speech models infeasible on the timeline. See the "Quyết định thu hẹp phạm vi" section in `PROJECT.md` for the rationale. When in doubt about target language or scope, treat English-only as current — don't reintroduce Japanese-specific work (e.g. Kana/Kanji OLED fonts) unless the user says the scope changed again.

## Author context — read this before editing embedded code

The primary author working in this repo, Khoa, owns the "Firmware nhúng" (embedded firmware) role on the team but has strong web/app background and near-zero prior embedded experience. He is self-teaching under a tight deadline.

- When touching firmware code (`firmware-demo/`, or later real ESP32 firmware), explain the **why** behind embedded-specific choices inline as short comments — e.g. why `INPUT_PULLUP` flips button logic, why a debounce delay is needed, why WiFi/MQTT reconnect loops are structured the way they are. Don't assume familiarity with microcontroller idioms the way you would for backend/web code.
- Keep firmware code simple and readable over "idiomatic embedded C" — this is a learning project on a deadline, not production firmware.
- Web/JS code in this repo (e.g. `web-dashboard/`) can be treated as Khoa's strong suit — no extra hand-holding needed there.

## Tech stack

- **Firmware**: C/C++, Arduino framework for ESP32, built with **PlatformIO** (not Arduino IDE).
- **Simulation**: **Wokwi** (no physical hardware available yet) — `wokwi.toml` + `diagram.json` per demo folder.
- **MQTT**: `PubSubClient` library on the firmware side; public broker `test.mosquitto.org` for demos/prototyping (not for the real product — the real system uses a self-hosted Mosquitto broker per the proposal).
- **OLED**: `Adafruit_SSD1306` + `Adafruit_GFX`, I2C.
- **Web dashboard demos**: plain HTML/JS + `mqtt.js` (CDN) subscribing over MQTT-over-WebSocket. No build step.

## Repo layout

- `firmware-demo/` — self-contained PlatformIO project for a given demo milestone. Each demo should stay runnable purely in Wokwi (no physical board required) until the team has real hardware.
- `firmware-demo/web-dashboard/` — companion static HTML page(s) that subscribe to the same MQTT topics the firmware publishes, for visualizing results without a full backend.
- `PROJECT.md` — Khoa's running plan/log for the firmware track (day-by-day tasks, checklists, learnings). Update it as tasks complete rather than creating new planning docs.

## Conventions

- MQTT topics for demos follow `panda/demo/<member>/<subject>` (e.g. `panda/demo/khoa/result`, `panda/demo/khoa/progress`) so they don't collide with topics other teammates use, and so the naming pattern is easy to carry over to the real product's topic scheme later.
- Prefer PlatformIO (`pio run`, `pio run --target upload`) over the Arduino IDE for anything committed to this repo.
- Don't add abstractions (state machines, config files, OTA, etc.) beyond what a given demo milestone needs — scope is intentionally minimal given the deadline-driven, learning-focused nature of this track.
