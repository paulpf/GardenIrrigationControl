#ifdef USE_PRIVATE_SECRET
#include "../../_secrets/WifiSecret.h"
#include "../../_configs/MqttConfig.h"
#include "../../_secrets/MqttSecret.h"
#else
#include "./_secrets/WifiSecret.h"
#include "./_config/MqttConfig.h"
#include "./_secrets/MqttSecret.h"
#endif

#include "globaldefines.h"

#include "wifimanager.h"
#include "irrigationZone.h"
#include "mqttmanager.h"

#include "esp_task_wdt.h"
#include "esp_system.h"

// ================ Constants ================
int loopDelay = 1000;
const int WATCHDOG_TIMEOUT = 60000;

// Name is used for the hostname. It is combined with the MAC address to create a unique name.
String clientName = "GardenController-" + Tools::replaceChars(WiFi.macAddress(), ':', '-');

// ================ WiFi ================
WifiManager wifiManager;

// ================ MQTT ================
MqttManager mqttManager;

// ================ Irrigation zones ================
IrrigationZone irrigationZone1;


// ================ Main ================

void setup() 
{
  // Setup console
  Serial.begin(115200);
  Trace::log("Setup begin");

  // Setup WiFi - use the WiFiManager class now
  wifiManager.setup(WIFI_SSID, WIFI_PWD, clientName);

  // Setup Irrigation zones
  irrigationZone1.setup(23, 22, clientName + "/irrigationZone1"); // GPIO 23 for button, GPIO 22 for relay

  // Setup MQTT
  mqttManager.setup(MQTT_SERVER_IP, MQTT_SERVER_PORT, MQTT_USER, MQTT_PWD, clientName.c_str());
  mqttManager.addIrrigationZone(&irrigationZone1);

  // Initialize the watchdog timer
  esp_task_wdt_init(WATCHDOG_TIMEOUT / 1000, true); // Convert milliseconds to seconds
  esp_task_wdt_add(NULL); // Add current thread to WDT watch

  Trace::log("Setup end");
}

void loop() 
{
  Trace::log("Loop start: " + String(millis()));

  // Reset the watchdog timer in each loop iteration
  esp_task_wdt_reset();
  
  // Wifi management
  wifiManager.loop();

  // Irrigation zone 1 management
  irrigationZone1.loop();

  // MQTT management
  mqttManager.loop();

  // Mqtt publishing
  mqttManager.publishAllIrrigationZones();
  
  delay(loopDelay);
  Trace::log("Loop end");
}