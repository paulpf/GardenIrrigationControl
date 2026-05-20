#pragma once

namespace Config {
// Keep real values out of git. For real projects prefer a local secret header.
constexpr const char* WIFI_SSID = "YOUR_WIFI_SSID";
constexpr const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

constexpr const char* OTA_HOSTNAME = "esp32-template";
constexpr const char* OTA_PASSWORD = "ota_password";
}  // namespace Config
