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
unsigned long previousMillisLongLoop = 0;
unsigned long previousMillisShortLoop = 0;
unsigned long _mainLoopStartTime = 0; // For loop time plotting

// ================ Main ================

void setup() 
{
  // Setup console
  Serial.begin(115200);
  Trace::log(TraceLevel::INFO, "Setup begin");

  // Initialize storage manager first
  StorageManager::getInstance().begin();
  Trace::log(TraceLevel::DEBUG, "StorageManager initialized");

  // Set initial client name (will be updated later)
  strncpy(clientName, "GardenController-Init", CLIENT_NAME_MAX_SIZE - 1);
  clientName[CLIENT_NAME_MAX_SIZE - 1] = '\0';

  // Setup WiFi
  wifiManager.setup(WIFI_SSID, WIFI_PWD, clientName);

  // Update client name with MAC address for unique identification
  String macFormatted = Helper::replaceChars(WiFi.macAddress(), ':', '-');
  Helper::formatToBuffer(clientName, CLIENT_NAME_MAX_SIZE, "GardenController-%s", macFormatted.c_str());
  Trace::log(TraceLevel::DEBUG, "Client name set: " + String(clientName));

  // Setup MQTT
  mqttManager.setup(MQTT_SERVER_IP, MQTT_SERVER_PORT, MQTT_USER, MQTT_PWD, clientName);

  // Setup all irrigation zones
  Trace::log(TraceLevel::DEBUG, "Initializing irrigation zones...");
  
    // Zone 1
  Helper::addIrrigationZone(ZONE1_BUTTON_PIN, ZONE1_RELAY_PIN, irrigationZones, &mqttManager, activeZones, clientName);
  // Zone 2
  Helper::addIrrigationZone(ZONE2_BUTTON_PIN, ZONE2_RELAY_PIN, irrigationZones, &mqttManager, activeZones, clientName);
  // Zone 3
  Helper::addIrrigationZone(ZONE3_BUTTON_PIN, ZONE3_RELAY_PIN, irrigationZones, &mqttManager, activeZones, clientName);
  // Zone 4
  Helper::addIrrigationZone(ZONE4_BUTTON_PIN, ZONE4_RELAY_PIN, irrigationZones, &mqttManager, activeZones, clientName);
  // Zone 5
  Helper::addIrrigationZone(ZONE5_BUTTON_PIN, ZONE5_RELAY_PIN, irrigationZones, &mqttManager, activeZones, clientName);
  // Zone 6
  Helper::addIrrigationZone(ZONE6_BUTTON_PIN, ZONE6_RELAY_PIN, irrigationZones, &mqttManager, activeZones, clientName);
  // Zone 7
  Helper::addIrrigationZone(ZONE7_BUTTON_PIN, ZONE7_RELAY_PIN, irrigationZones, &mqttManager, activeZones, clientName);
  // Zone 8
  Helper::addIrrigationZone(ZONE8_BUTTON_PIN, ZONE8_RELAY_PIN, irrigationZones, &mqttManager, activeZones, clientName);

  // Drainage zone (if needed)
  Helper::addIrrigationZone(DRAINAGE_BUTTON_PIN, DRAINAGE_RELAY_PIN, irrigationZones, &mqttManager, activeZones, clientName);
  
  // Load saved settings for each zone from storage
  for (int i = 0; i < activeZones; i++) 
  {
    irrigationZones[i].loadSettingsFromStorage(i);
  }
  
  Trace::log(TraceLevel::DEBUG, "Irrigation zones initialized: " + String(activeZones) + " zones");

  // Initialize the watchdog timer
  esp_task_wdt_init(WATCHDOG_TIMEOUT / 1000, true); // Convert milliseconds to seconds
  esp_task_wdt_add(NULL); // Add current thread to WDT watch

  Trace::log(TraceLevel::INFO, "Setup end");
}

void loop() 
{
  // Reset the watchdog timer in each loop iteration
  esp_task_wdt_reset();

  #ifdef ENABLE_LOOP_TIME_PLOTTING
  Trace::plotLoopTime("Mainloop", 0, millis() - _mainLoopStartTime);
  _mainLoopStartTime = millis(); // Reset loop start time for next iteration
  #endif
  
  unsigned long currentMillis = millis();
  // Execute periodic tasks (with reduced logging)

  // Main timer-based events
  if (currentMillis - previousMillisLongLoop >= LONG_INTERVAL) 
  {
    previousMillisLongLoop = currentMillis;

    // Check and manage WiFi status
    wifiManager.loop();
  }

  // These functions should be called in every iteration (without delay)
  if (currentMillis - previousMillisShortLoop >= SHORT_INTERVAL)
  {
    previousMillisShortLoop = currentMillis;

    // Publish MQTT data (only needed periodically)
    mqttManager.publishAllIrrigationZones();

    // Process MQTT messages
    mqttManager.loop();
    
    // Update irrigation zones
    for (int i = 0; i < activeZones; i++) 
    {
      irrigationZones[i].loop();
    }
  }

  #ifdef ENABLE_ZONE_PLOTTING
  // Plot zone states at defined interval for Teleplot
  static unsigned long lastPlotTime = 0;
  if (currentMillis - lastPlotTime >= TELEPLOT_INTERVAL) 
  {
    lastPlotTime = currentMillis;
    for (int i = 0; i < activeZones; i++) 
    {
      Trace::plotBoolState("Btn" + String(i + 1), irrigationZones[i].getBtnState(), 1);
      Trace::plotBoolState("Relay" + String(i + 1), irrigationZones[i].getRelayState(), -1);
    }
  }
  #endif
}