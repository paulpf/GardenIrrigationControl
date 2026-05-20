#include "ota_update.h"

#include <Arduino.h>
#include <ArduinoOTA.h>

#include "config.h"

void OTA_Update::begin() {
  ArduinoOTA.setHostname(Config::OTA_HOSTNAME);
  ArduinoOTA.setPassword(Config::OTA_PASSWORD);

  ArduinoOTA.onStart([]() { Serial.println("[OTA] Start"); });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    const unsigned int percent = (progress * 100U) / total;
    Serial.printf("[OTA] Progress: %u%%\n", percent);
  });

  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("[OTA] Error [%u]\n", error);
  });

  ArduinoOTA.onEnd([]() { Serial.println("\n[OTA] End"); });
  ArduinoOTA.begin();

  Serial.println("[OTA] Ready");
}

void OTA_Update::handle() { ArduinoOTA.handle(); }
