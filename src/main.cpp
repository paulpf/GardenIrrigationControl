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
#include "dht11manager.h"

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

// ================ DHT11 Sensor ================
//Dht11Manager dht11Manager;

// ================ Irrigation zones ================
// Using an array for better scalability with 8 zones
IrrigationZone irrigationZones[MAX_IRRIGATION_ZONES];

// ================ Timing ================
unsigned long currentMillis = 0; // Current time in milliseconds
unsigned long previousMillisLongLoop = 0;
unsigned long previousMillisShortLoop = 0;
unsigned long mainLoopStartTime = 0; // For loop time plotting

// ================ Main ================

// New function to initialize irrigation zones
void initIrrigationZones() 
{
    Trace::log(TraceLevel::INFO, "Initializing irrigation zones...");

    // Zone configuration arrays
    const int zoneButtons[] = {ZONE1_BUTTON_PIN, ZONE2_BUTTON_PIN, ZONE3_BUTTON_PIN, ZONE4_BUTTON_PIN,
                                ZONE5_BUTTON_PIN, ZONE6_BUTTON_PIN, ZONE7_BUTTON_PIN, ZONE8_BUTTON_PIN, ZONE9_BUTTON_PIN};
    const int zoneRelays[]  = {ZONE1_RELAY_PIN,  ZONE2_RELAY_PIN,  ZONE3_RELAY_PIN,  ZONE4_RELAY_PIN,
                                ZONE5_RELAY_PIN,  ZONE6_RELAY_PIN,  ZONE7_RELAY_PIN,  ZONE8_RELAY_PIN, ZONE9_RELAY_PIN};

    // Initialize zones
    for (int i = 0; i < MAX_IRRIGATION_ZONES; i++) 
    {
        Helper::addIrrigationZone(zoneButtons[i], zoneRelays[i], irrigationZones, &mqttManager, i, clientName);
        irrigationZones[i].loadSettingsFromStorage(i);
    }
}

void setup() 
{
    // Setup console
    Serial.begin(115200);
    Trace::log(TraceLevel::INFO, "Setup begin");

    // Initialize storage manager first
    StorageManager::getInstance().begin();
    Trace::log(TraceLevel::INFO, "StorageManager initialized");

    // Set initial client name (will be updated later)
    strncpy(clientName, "GardenController-Init", CLIENT_NAME_MAX_SIZE - 1);
    clientName[CLIENT_NAME_MAX_SIZE - 1] = '\0';

    // Setup WiFi
    wifiManager.setup(WIFI_SSID, WIFI_PWD, clientName);

    // Wait for WiFi connection
    Trace::log(TraceLevel::INFO, "Waiting for WiFi connection...");
    unsigned long wifiConnectStart = millis();
    while (!wifiManager.isConnected()) 
    {
        if (millis() - wifiConnectStart > WIFI_CONNECTION_TIMEOUT) 
        {
            Trace::log(TraceLevel::ERROR, "WiFi connection timeout");
            break; // Exit if connection takes too long
        }
        delay(100); // Polling delay
    }

    // Update client name with MAC address for unique identification
    String macFormatted = Helper::replaceChars(WiFi.macAddress(), ':', '-');
    Helper::formatToBuffer(clientName, CLIENT_NAME_MAX_SIZE, "GardenController-%s", macFormatted.c_str());
    Trace::log(TraceLevel::INFO, "Client name set: " + String(clientName));    // Setup MQTT
    mqttManager.setup(MQTT_SERVER_IP, MQTT_SERVER_PORT, MQTT_USER, MQTT_PWD, clientName);

    // Setup OTA (Over-The-Air updates)
    #if ENABLE_OTA
    Trace::log(TraceLevel::INFO, "Setting up OTA...");
    otaManager.setup(clientName, OTA_PASSWORD);
    #else
    Trace::log(TraceLevel::INFO, "OTA disabled in configuration");
    otaManager.setEnabled(false);
    #endif    
    
    // Initialize irrigation zones using the new helper function
    initIrrigationZones();

    // Initialize DHT11 sensor
    //Trace::log(TraceLevel::DEBUG, "Initializing DHT11 sensor...");
    //dht11Manager.setup(DHT11_PIN, DHT11_TYPE, clientName);
    //mqttManager.setDht11Manager(&dht11Manager);    
    
    // Initialize the watchdog timer
    esp_task_wdt_init(WATCHDOG_TIMEOUT / 5000, true); // Convert milliseconds to seconds
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
    for (int i = 0; i < MAX_IRRIGATION_ZONES; i++) 
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
  // Publish MQTT messages for all irrigation zones
  mqttManager.publishAllIrrigationZones();
  
  // Publish DHT11 sensor data
  mqttManager.publishDht11Data();

  // Process MQTT messages
  mqttManager.loop();
  
  // Update irrigation zones
  for (int i = 0; i < MAX_IRRIGATION_ZONES; i++) 
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
      
      // Update DHT11 sensor readings
      //dht11Manager.loop();
      
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