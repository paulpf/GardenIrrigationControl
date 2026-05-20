# ESP32 PlatformIO Template

Minimal ESP32 (Arduino) template for PlatformIO with modular WiFi, OTA, and GPIO management.

## Project tree

```text
esp32-platformio-template/
├── include/
│   └── config.h
├── lib/
│   ├── gpio_manager/
│   │   ├── gpio_manager.cpp
│   │   └── gpio_manager.h
│   ├── ota_update/
│   │   ├── ota_update.cpp
│   │   └── ota_update.h
│   └── wifi_manager/
│       ├── wifi_manager.cpp
│       └── wifi_manager.h
├── src/
│   └── main.cpp
└── platformio.ini
```

## Usage

1. Set WiFi and OTA values in:
   - `include/config.h`
   - `platformio.ini` (`upload_port`, `upload_flags`)
2. Flash once via USB.
3. Upload subsequent firmware updates via OTA (`upload_protocol = espota`).
