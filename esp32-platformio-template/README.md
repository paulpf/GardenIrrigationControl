# ESP32 PlatformIO Template

Minimal ESP32 (Arduino) template for PlatformIO with modular WiFi, OTA, and GPIO management.

## Project tree

```text
esp32-platformio-template/
├── include/
│   └── config.h          ← WiFi credentials and OTA settings
├── lib/
│   ├── gpio_manager/     ← central pin registry (enum → GPIO number)
│   ├── ota_update/       ← ArduinoOTA wrapper
│   └── wifi_manager/     ← WiFi STA init + auto-reconnect
├── src/
│   └── main.cpp          ← setup / loop entry point
└── platformio.ini
```

## Quick start

1. Edit `include/config.h` — set your WiFi SSID, WiFi password, OTA hostname, and OTA password.
2. Edit `lib/gpio_manager/gpio_manager.cpp` — map `OUTPUT_0` / `INPUT_0` to your actual pin numbers.
3. Add or rename `PinFunction` enum entries in `gpio_manager.h` as your project grows.
4. Flash **once via USB** (`pio run -t upload`).
5. All subsequent uploads work **via OTA** — update `upload_port` in `platformio.ini` with the device IP.
