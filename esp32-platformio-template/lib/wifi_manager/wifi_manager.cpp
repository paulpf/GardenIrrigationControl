#include "wifi_manager.h"

#include <Arduino.h>
#include <WiFi.h>

#include "config.h"

namespace {
unsigned long lastReconnectAttempt = 0;
constexpr unsigned long RECONNECT_INTERVAL_MS = 5000;
}  // namespace

void WiFi_Manager::begin() {
  WiFi.mode(WIFI_STA);
  WiFi.setAutoReconnect(true);
  WiFi.persistent(false);
  WiFi.begin(Config::WIFI_SSID, Config::WIFI_PASSWORD);

  Serial.printf("[WiFi] Connecting to SSID: %s\n", Config::WIFI_SSID);
}

void WiFi_Manager::loop() {
  if (WiFi.status() == WL_CONNECTED) {
    return;
  }

  const unsigned long now = millis();
  if (now - lastReconnectAttempt < RECONNECT_INTERVAL_MS) {
    return;
  }

  lastReconnectAttempt = now;
  Serial.println("[WiFi] Connection lost, reconnecting...");
  WiFi.reconnect();
}
