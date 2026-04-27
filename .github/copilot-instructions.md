# Copilot Instructions

These instructions apply to the `GardenIrrigationControl` repository.

## Project Context

This project is an ESP32/PlatformIO firmware for garden irrigation control.
It uses Arduino framework, MQTT, OTA updates, local button inputs, relay outputs,
persistent storage, and water-level monitoring.

The codebase is intentionally split by responsibility:

- `src/app`: composition root, bootstrap, and runtime orchestration
- `src/config`: default values and structured configuration objects
- `src/contracts`: interfaces used to keep domain logic testable
- `src/domain`: irrigation and water-level business logic
- `src/infrastructure`: ESP32, WiFi, MQTT, OTA, storage, and hardware details
- `src/services`: small coordination and loop helper services
- `test/support`: native test fakes/stubs for Arduino and ESP32 APIs

## Coding Style

- Prefer small, focused changes that match the surrounding style.
- Keep domain logic independent from ESP32-specific APIs where practical.
- Use existing configuration structs and constants instead of scattering magic values.
- Prefer dependency injection and existing interfaces over direct global coupling.
- Avoid unrelated refactors while fixing a bug or adding a narrow feature.
- Keep comments short and useful. Do not restate obvious code.
- Use ASCII unless the file already clearly uses another encoding.

## Safety-Critical Behavior

This firmware controls physical irrigation hardware. Be conservative:

- Relays are low-level triggered: `LOW` means active, `HIGH` means inactive.
- Startup behavior must avoid accidental relay activation.
- OTA updates must pause normal irrigation tasks.
- Water-level lockout behavior must stay fail-safe.
- Timer and duration math must avoid overflow, underflow, and invalid zero durations.

## MQTT Conventions

- State topics publish current state.
- Command topics should use `/set` when changing state.
- Do not subscribe to state topics unless there is a clear reason.
- Treat retained MQTT messages carefully; they may arrive immediately after reconnect.
- Validate payloads before applying state changes.
- Empty or invalid command payloads should be ignored or logged without changing stored settings.

For irrigation duration:

- `.../durationTime` is the published state topic.
- `.../durationTime/set` is the command topic.
- Duration payloads are seconds.
- Persisted duration values are milliseconds.

## Persistence

- Zone duration is persisted through `StorageManager`.
- Do not reset stored values on invalid external input.
- Factory reset behavior should explicitly restore defaults.

## Testing

Use native tests for fast feedback when changing domain logic, services, or testable infrastructure:

```bash
pio test -e native
```

Use an ESP32 build to validate firmware integration:

```bash
pio run -e az-delivery-devkit-v4-usb
```

When modifying test support files under `test/support`, keep them minimal and aligned with
the subset of Arduino/ESP32 APIs actually needed by native tests.

## Secrets

- Do not commit real WiFi, MQTT, or OTA secrets.
- Secret headers under `src/config/_secrets` are local/private configuration.
- Prefer placeholders or setup scripts for examples.

## Documentation

- Keep user-facing documentation in German where the surrounding document is German.
- Keep architecture and developer notes consistent with `docs/Entwicklerdokumentation.md`.
- Update docs only when behavior or usage changes materially.
