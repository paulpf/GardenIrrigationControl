# CLAUDE.md — GardenIrrigationControl

Project rules and coding style are in [.github/copilot-instructions.md](.github/copilot-instructions.md).
Read that file first. This file adds Claude-specific working notes.

## Communication

- Respond in **German** unless the user writes in English.
- Code, identifiers, and comments stay in **English**.

## Workflow Rules

- **Never commit** without explicit user approval after manual testing.
- Before changing code, read the affected files. Do not rely on memory alone.
- For questions like "what does X do?" — answer from the current file state, not from recall.

## Key Architecture Facts

**Loop structure** — three tiers in `GardenControllerApp::loop()`:
- Short (50 ms): MQTT loop, zone button/relay polling, water level
- Middle (1 s): MQTT publish of system status, wall clock, auto irrigation, schedules, diagnostics
- Long (60 s): WiFi health check, connectivity coordinator

**Publisher pattern** — `WallClockPublisher`, `AutomaticIrrigationPublisher`,
`ScheduledIrrigationPublisher`, `DiagnosticsReporter` all follow the same pattern:
`_wasConnected` guard forces re-publish after MQTT reconnect; change detection + periodic
interval controls publish frequency.

**MQTT callback** — `PubSubClient` requires a static callback. `MqttManager` uses a static
`_instance` pointer to forward to `instanceMqttCallback`. Only one `MqttManager` may exist.

**Cold start vs. warm restart** — `resetZoneStateAfterColdStartIfNeeded()` detects power-on,
external reset, and brownout via `esp_reset_reason()` and forces all relays off.
On OTA/watchdog restart (warm), relay state is left as-is in hardware (relays stay off because
they're not loaded from storage).

**Relay wiring** — LOW = relay active, HIGH = relay off (low-level triggered).

## NVS Storage — Quick Reference

Namespace: `garden_ctrl`. See [.github/copilot-instructions.md](.github/copilot-instructions.md)
for the full key table.

Save/load points:
- Duration: saved in `IrrigationZone::setDurationTime()`, loaded in `loadSettingsFromStorage()`
- Auto irrigation enabled: saved in `AutomaticIrrigationCommandHandler::handleCommand()`,
  loaded in `GardenControllerApp::initAutomaticIrrigation()`
- Low-water lockout: saved in `MqttManager::instanceMqttCallback()`,
  loaded in `GardenControllerApp::initializeSubsystems()`
- Schedules: saved in `ScheduledIrrigationCommandHandler::saveAndPublish()`,
  loaded in `GardenControllerApp::initScheduledIrrigation()`

## Build & Test

```bash
# Native unit tests (fast, no hardware needed)
pio test -e native

# USB firmware build
pio run -e az-delivery-devkit-v4-usb

# OTA firmware build (always validate this too)
pio run -e az-delivery-devkit-v4-ota
```

## Known Design Gaps (open, not yet fixed)

- `GardenControllerApp` constructor takes 19 parameters — God Object, no interface for `StorageManager`.
- FNV fingerprint field order differs between `GardenControllerApp` and `ScheduledIrrigationPublisher`
  (see code review 2026-05-19).
- Publisher boilerplate (`_wasConnected`, `_hasPublished`, interval logic) is repeated in four classes
  — candidate for a `PublishThrottle` helper.
- `MqttManager` topic helpers (`GetXxxTopic()`) allocate `String` on every call instead of
  caching at setup time.
