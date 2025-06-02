#ifdef USE_PRIVATE_SECRET
#include "../../_secrets/WifiSecret.h"
#include "../../_configs/MqttConfig.h"
#include "../../_secrets/MqttSecret.h"
#include "../../_secrets/OtaSecret.h"
#else
#include "./_secrets/WifiSecret.h"
#include "./_config/MqttConfig.h"
#include "./_secrets/MqttSecret.h"
#include "./_secrets/OtaSecret.h"
#endif

#include "globaldefines.h"
#include "config.h"

#include "wifimanager.h"
#include "mqttmanager.h"
#include "otamanager.h"
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

// ================ OTA ================
OtaManager otaManager;

// ================ Irrigation zones ================
// Using an array for better scalability with 8 zones
IrrigationZone irrigationZones[MAX_IRRIGATION_ZONES];
int activeZones = 0; // Will be increased in setup

// ================ Timing ================
unsigned long currentMillis = 0; // Current time in milliseconds
unsigned long previousMillisLongLoop = 0;
unsigned long previousMillisShortLoop = 0;
unsigned long mainLoopStartTime = 0; // For loop time plotting

// ================ Main ================

// New function to initialize irrigation zones
void initIrrigationZones() 
{
  // Zone configuration arrays for standard zones
  const int zoneButtons[] = {ZONE1_BUTTON_PIN, ZONE2_BUTTON_PIN, ZONE3_BUTTON_PIN, ZONE4_BUTTON_PIN,
                              ZONE5_BUTTON_PIN, ZONE6_BUTTON_PIN, ZONE7_BUTTON_PIN, ZONE8_BUTTON_PIN};
  const int zoneRelays[]  = {ZONE1_RELAY_PIN,  ZONE2_RELAY_PIN,  ZONE3_RELAY_PIN,  ZONE4_RELAY_PIN,
                              ZONE5_RELAY_PIN,  ZONE6_RELAY_PIN,  ZONE7_RELAY_PIN,  ZONE8_RELAY_PIN};
  const int numZones = sizeof(zoneButtons) / sizeof(zoneButtons[0]);

  // Initialize the standard zones
  for (int i = 0; i < numZones && activeZones < MAX_IRRIGATION_ZONES; i++) 
  {
      Helper::addIrrigationZone(zoneButtons[i], zoneRelays[i], irrigationZones, &mqttManager, activeZones, clientName);
  }
  
  // Optionally add the drainage zone if capacity allows
  if (activeZones < MAX_IRRIGATION_ZONES) 
  {
      Helper::addIrrigationZone(DRAINAGE_BUTTON_PIN, DRAINAGE_RELAY_PIN, irrigationZones, &mqttManager, activeZones, clientName);
  }

  // Load saved settings for each zone from storage
  for (int i = 0; i < activeZones; i++) 
  {
      irrigationZones[i].loadSettingsFromStorage(i);
  }

  Trace::log(TraceLevel::DEBUG, "Irrigation zones initialized: " + String(activeZones) + " zones");
}

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
    Trace::log(TraceLevel::DEBUG, "Client name set: " + String(clientName));    // Setup MQTT
    mqttManager.setup(MQTT_SERVER_IP, MQTT_SERVER_PORT, MQTT_USER, MQTT_PWD, clientName);

    // Setup OTA (Over-The-Air updates)
    #if ENABLE_OTA
    Trace::log(TraceLevel::DEBUG, "Setting up OTA...");
    otaManager.setup(clientName, OTA_PASSWORD);
    #else
    Trace::log(TraceLevel::DEBUG, "OTA disabled in configuration");
    otaManager.setEnabled(false);
    #endif

    // Initialize irrigation zones using the new helper function
    Trace::log(TraceLevel::DEBUG, "Initializing irrigation zones...");
    initIrrigationZones();

    // Initialize the watchdog timer
    esp_task_wdt_init(WATCHDOG_TIMEOUT / 1000, true); // Convert milliseconds to seconds
    esp_task_wdt_add(NULL); // Add current thread to WDT watch

    Trace::log(TraceLevel::INFO, "Setup end");
}

// Function to handle zone state plotting
void plotZoneStates(unsigned long currentTime) 
{
  static unsigned long lastPlotTime = 0;
  if ((unsigned long)(currentTime - lastPlotTime) >= TELEPLOT_INTERVAL) 
  {
    lastPlotTime = currentTime;
    char bufferName[20]; // Buffer for name string
    for (int i = 0; i < activeZones; i++) 
    {
      snprintf(bufferName, sizeof(bufferName), "Btn%d", i + 1);
      Trace::plotBoolState(bufferName, irrigationZones[i].getBtnState(), 1);
      
      snprintf(bufferName, sizeof(bufferName), "Relay%d", i + 1);
      Trace::plotBoolState(bufferName, irrigationZones[i].getRelayState(), -1);
    }
  }
}

void handleShortIntervalEvents() 
{
  // Publish MQTT data
  bool mqttPublishStatus = mqttManager.publishAllIrrigationZones();
  if (!mqttPublishStatus) 
  {
      Trace::log(TraceLevel::ERROR, "MQTT publishing failed");
  }

  // Process MQTT messages
  mqttManager.loop();
  
  // Update irrigation zones
  for (int i = 0; i < activeZones; i++) 
  {
      irrigationZones[i].loop();
      
      // Yield every third zone to prevent a long blocking loop
      if (i % 3 == 0) 
      {
          yield();
      }
  }
}

void loop() 
{
  esp_task_wdt_reset();
  currentMillis = millis();

  // Handle OTA updates - this should be processed frequently
  #if ENABLE_OTA
  otaManager.loop();
  
  // If OTA update is in progress, skip other operations to ensure stability
  if (otaManager.isUpdating()) 
  {
      return;
  }
  #endif

  #ifdef ENABLE_LOOP_TIME_PLOTTING
  Trace::plotLoopTime("Mainloop", 0, currentMillis - mainLoopStartTime);
  mainLoopStartTime = currentMillis;
  #endif

  if ((unsigned long)(currentMillis - previousMillisLongLoop) >= LONG_INTERVAL) 
  {
      previousMillisLongLoop = currentMillis;
      if (!wifiManager.loop()) 
      {
          Trace::log(TraceLevel::ERROR, "WiFi connection issue detected");
      }
      yield();
  }

  if ((unsigned long)(currentMillis - previousMillisShortLoop) >= SHORT_INTERVAL) 
  {
      previousMillisShortLoop = currentMillis;
      handleShortIntervalEvents();
  }

  #ifdef ENABLE_ZONE_PLOTTING
  plotZoneStates(currentMillis);
  #endif
}