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
#include "config.h"

#include "wifimanager.h"
#include "mqttmanager.h"
#include "irrigationZone.h"
#include "helper.h"
#include "StorageManager.h"

#include "esp_task_wdt.h"
#include "esp_system.h"

// ================ Variables ================
// Name is used for the hostname. This will be updated after WiFi init with actual MAC
const int CLIENT_NAME_MAX_SIZE = 50;  // Maximum size for the client name
char clientName[CLIENT_NAME_MAX_SIZE]; // Buffer for clientName

// ================ WiFi ================
WifiManager wifiManager;

// ================ MQTT ================
MqttManager mqttManager;

// ================ Irrigation zones ================
// Using an array for better scalability with 8 zones
IrrigationZone irrigationZones[MAX_IRRIGATION_ZONES];
int activeZones = 0; // Will be increased in setup

// ================ Timing ================
unsigned long previousMillisLoop1 = 0;
unsigned long previousMillisLoop2 = 0;

// ================ Main ================

void setup() 
{
  // Setup console
  Serial.begin(115200);
  Trace::log("Setup begin");

  // Initialize storage manager first
  StorageManager::getInstance().begin();
  Trace::log("StorageManager initialized");

  // Set initial client name (will be updated later)
  strncpy(clientName, "GardenController-Init", CLIENT_NAME_MAX_SIZE - 1);
  clientName[CLIENT_NAME_MAX_SIZE - 1] = '\0';

  // Setup WiFi
  wifiManager.setup(WIFI_SSID, WIFI_PWD, clientName);

  // Update client name with MAC address for unique identification
  String macFormatted = Helper::replaceChars(WiFi.macAddress(), ':', '-');
  Helper::formatToBuffer(clientName, CLIENT_NAME_MAX_SIZE, "GardenController-%s", macFormatted.c_str());
  Trace::log("Client name set: " + String(clientName));

  // Setup MQTT
  mqttManager.setup(MQTT_SERVER_IP, MQTT_SERVER_PORT, MQTT_USER, MQTT_PWD, clientName);

  // Setup all irrigation zones
  Trace::log("Initializing irrigation zones...");
  
  // Zone 1
  Helper::addIrrigationZone(ZONE1_BUTTON_PIN, ZONE1_RELAY_PIN, irrigationZones, &mqttManager, activeZones, clientName);
  
  // Load saved settings for each zone from storage
  for (int i = 0; i < activeZones; i++) 
  {
    irrigationZones[i].loadSettingsFromStorage(i);
  }
  
  Trace::log("Irrigation zones initialized: " + String(activeZones) + " zones");

  // Initialize the watchdog timer
  esp_task_wdt_init(WATCHDOG_TIMEOUT / 1000, true); // Convert milliseconds to seconds
  esp_task_wdt_add(NULL); // Add current thread to WDT watch

  Trace::log("Setup end");
}

void loop() 
{
  unsigned long currentMillis = millis();
  // Execute periodic tasks (with reduced logging)
  
  // Reset the watchdog timer in each loop iteration
  esp_task_wdt_reset();
  
  // Main timer-based events
  if (currentMillis - previousMillisLoop1 >= LOOP_INTERVAL) 
  {
    previousMillisLoop1 = currentMillis;

    // Publish MQTT data (only needed periodically)
    mqttManager.publishAllIrrigationZones();
  }

  // These functions should be called in every iteration (without delay)
  #if DEBUG_MODE
  if (currentMillis - previousMillisLoop2 >= LOOP_INTERVAL)
  #endif
  {
    previousMillisLoop2 = currentMillis;
    // Check and manage WiFi status
    wifiManager.loop();
    
    // Process MQTT messages
    mqttManager.loop();
    
    // Update irrigation zones
    for (int i = 0; i < activeZones; i++) 
    {
      irrigationZones[i].loop();
    }
  }
}