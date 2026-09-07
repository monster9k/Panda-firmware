# CLAUDE.md

Guidance for Claude Code when working in this repository.

## Project

PANDA is a PBL4 student project: a desk robot that helps children practice English vocabulary via voice (TTS/STT), shows expressive eyes and English words/pictures on an OLED, and reports progress via MQTT to a cloud server with a web dashboard. Full proposal context: see `README.md` and the team's PBL4 proposal PDF.

**Scope note:** the team's original proposal targeted two-way Vietnamese↔Japanese vocabulary practice. The team narrowed this to one-way English vocabulary practice for children after the AI teammate found building/customizing bidirectional Vietnamese↔Japanese speech models infeasible on the timeline. See the "Quyết định thu hẹp phạm vi" section in `PROJECT.md` for the rationale. When in doubt about target language or scope, treat English-only as current — don't reintroduce Japanese-specific work (e.g. Kana/Kanji OLED fonts) unless the user says the scope changed again.

**Known discrepancy (2026-09-03):** the AI teammate's actual code in `Panda-Robotics-Client-/` (summarized in `ai.md`) implements a much broader system than the narrowed scope above — a free-form Vietnamese conversational assistant (wake-word, LLM chat across 24 topics, face/emotion recognition), not just one-way English vocabulary drilling. This is a factual observation from reading that code, not a re-decision of scope — don't silently treat either document as more authoritative than the other; if scope comes up, surface this mismatch rather than picking a side.

**Cross-project reference:** `Panda-Robotics-Client-/` (the AI teammate's Python/Node backend + dashboard, plus a reference `panda_firmware.ino` Arduino sketch) lives alongside `firmware-demo/` in this repo. Read `ai.md` before touching MQTT topics, OLED expression names/eye coordinates, or GPIO pin choices in `firmware-demo/` — it documents the MQTT contract (`panda/cmd/*`, `panda/status`, `panda/ai/*`) and the shared OLED expression set (`neutral/happy/sad/angry/surprised/sleepy/wink/love/cool/cute/dizzy/questioning/thinking/speaking`) that `firmware-demo/src/display.*` was deliberately ported from, to stay compatible for future integration.

## Author context — read this before editing embedded code

The primary author working in this repo, Khoa, owns the "Firmware nhúng" (embedded firmware) role on the team but has strong web/app background and near-zero prior embedded experience. He is self-teaching under a tight deadline.

- When touching firmware code (`firmware-demo/`, or later real ESP32 firmware), explain the **why** behind embedded-specific choices inline as short comments — e.g. why `INPUT_PULLUP` flips button logic, why a debounce delay is needed, why WiFi/MQTT reconnect loops are structured the way they are. Don't assume familiarity with microcontroller idioms the way you would for backend/web code.
- Keep firmware code simple and readable over "idiomatic embedded C" — this is a learning project on a deadline, not production firmware.
- Web/JS code in this repo (e.g. `web-dashboard/`) can be treated as Khoa's strong suit — no extra hand-holding needed there.

## Tech stack

- **Firmware**: C/C++, Arduino framework for ESP32, built with **PlatformIO** (not Arduino IDE).
- **Simulation**: **Wokwi** (no physical hardware available yet) — `wokwi.toml` + `diagram.json` per demo folder.
- **MQTT**: `PubSubClient` library on the firmware side; public broker `broker.hivemq.com` for demos/prototyping (switched from `test.mosquitto.org` in Week 2 after `rc=-2` TCP-connect failures — see `WEEKLY_LOGIC.md`, not pushed to git). Not for the real product — the real system uses a self-hosted Mosquitto broker per the proposal, and a different topic namespace (see `ai.md`).
- **OLED**: `Adafruit_SSD1306` + `Adafruit_GFX`, I2C.
- **Web dashboard demos**: plain HTML/JS + `mqtt.js` (CDN) subscribing over MQTT-over-WebSocket. No build step.

## Repo layout

- `firmware-demo/` — self-contained PlatformIO project for a given demo milestone. Each demo should stay runnable purely in Wokwi (no physical board required) until the team has real hardware. `src/` is split into modules (`main.cpp`, `pins.h`, `display.*`, `input.*`, `network.*`, `audio_i2s.*`) rather than one file — see `WEEKLY_LOGIC.md` for why.
- `firmware-demo/web-dashboard/` — companion static HTML page(s) that subscribe to the same MQTT topics the firmware publishes, for visualizing results without a full backend.
- `PROJECT.md` — Khoa's running plan/log for the firmware track (day-by-day tasks, checklists, learnings). Update it as tasks complete rather than creating new planning docs.
- `ai.md` — summary of the AI teammate's `Panda-Robotics-Client-/` project (MQTT contract, OLED expression reference, integration caveats). Tracked in git (unlike `PROJECT.md`'s companion personal-log files below) since it documents another teammate's work for the whole team.
- `Panda-Robotics-Client-/` — the AI teammate's own project (Python voice/vision/LLM backend + Node dashboard + a reference `panda_firmware.ino`). Not Khoa's to restructure; treat as read-only reference unless the user explicitly asks to change it.
- `learningprocess.md`, `WEEKLY_LOGIC.md` — Khoa's personal learning tracker and technical notes-by-week. Both gitignored (not for the team). Never suggest committing them.

## Conventions

- MQTT topics for demos follow `panda/demo/<member>/<subject>` (e.g. `panda/demo/khoa/result`, `panda/demo/khoa/progress`) so they don't collide with topics other teammates use, and so the naming pattern is easy to carry over to the real product's topic scheme later.
- Prefer PlatformIO (`pio run`, `pio run --target upload`) over the Arduino IDE for anything committed to this repo.
- Don't add abstractions (state machines, config files, OTA, etc.) beyond what a given demo milestone needs — scope is intentionally minimal given the deadline-driven, learning-focused nature of this track.

## Git — two separate repos, don't conflate them (2026-09-08)

This working directory (`MOON`) and `Panda-Robotics-Client-/` are **two independent git repositories** (the latter has its own nested `.git` and is gitignored by the former — see `.gitignore` line ~29). Full explanation for humans lives in `gitrule.md` (gitignored, local-only, not pushed anywhere) — read that file for the complete picture before doing any git operation that spans both repos.

- **`MOON` (`origin` = `monster9k/Panda-firmware`)** — Khoa's personal repo. Commit/push here as usual for day-to-day work (`firmware-demo/`, planning docs, etc.) — no special rule, this is his own sandbox/backup.
- **`Panda-Robotics-Client-/` (`origin` = `Thainguyen2103/Panda-Robotics-Client-`)** — the team's shared repo. Khoa has push access here now. Team git rule (from Thái, 2026-09-08):
  1. `git checkout develop && git pull origin develop`
  2. `git checkout -b feature/<ten>-<task>` (e.g. `feature/khoa-w3-i2s-module`)
  3. Commit normally while working.
  4. `git push origin feature/<ten>-<task>` — **never push directly to `main` or `develop`**; `main` is branch-protected (push blocked).
  5. Open a Pull Request into `develop` on GitHub (not `main`).
- Khoa's firmware code lives at `Panda-Robotics-Client-/firmware/kt_firmware/` (named for the "Kỹ thuật/embedded" pair, not just Khoa — a second embedded teammate shares this same folder; parallel to the AI teammate's reference `firmware/panda_firmware/`) — copied over from `MOON/firmware-demo/` (git-tracked files only, `.pio/` build artifacts excluded) each time there's a finished chunk of work to share, not on every small commit. It carries its own scoped `README.md`; **no other `.md` file from `MOON`** (`PROJECT.md`, `ai.md`, `CLAUDE.md`, `MOON/README.md`, or the gitignored `WEEKLY_LOGIC.md`/`learningprocess.md`) gets copied into the team repo.
- **Commits pushed to `Panda-Robotics-Client-` must NOT carry a `Co-Authored-By: Claude` trailer** (explicit user instruction — this repo is visible to teammates/instructor, unlike personal projects). This overrides the default attribution instruction for this specific remote only; commits to `MOON`'s own `origin` keep the default attribution unless told otherwise.
- Second embedded teammate: **one person, one branch** (decided 2026-09-08, kept simple since this is just a school project) — they clone `Panda-Robotics-Client-`, branch off `develop` with their own `feature/<ten>-...`, work inside the same `firmware/kt_firmware/` folder, and open their own PR into `develop`. No shared/pair-programming branch. See `gitrule.md` for the fuller onboarding explanation.
