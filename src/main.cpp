#include "../../_configs/MqttConfig.h"
#include "../../_secrets/MqttSecret.h"
#include "../../_secrets/OtaSecret.h"
#include "../../_secrets/WifiSecret.h"

#include "config.h"
#include "global_defines.h"

#include "helper.h"
#include "irrigation_zone.h"
#include "mqttmanager.h"
#include "otamanager.h"
#include "storage_manager.h"
#include "esp32waterlevelsensor.h"
#include "waterlevelmanager.h"
#include "wifimanager.h"

#include "esp_system.h"
#include "esp_task_wdt.h"

// ================ Variables ================
// Name is used for the hostname. This will be updated after WiFi init with
// actual MAC
const int CLIENT_NAME_MAX_SIZE = 50;   // Maximum size for the client name
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

struct ZoneConfig
{
  int buttonPin;
  int relayPin;
};

constexpr std::array<ZoneConfig, MAX_IRRIGATION_ZONES> ZONE_CONFIGS = {{
    {ZONE1_BUTTON_PIN, ZONE1_RELAY_PIN},
    {ZONE2_BUTTON_PIN, ZONE2_RELAY_PIN},
    {ZONE3_BUTTON_PIN, ZONE3_RELAY_PIN},
    {ZONE4_BUTTON_PIN, ZONE4_RELAY_PIN},
    {ZONE5_BUTTON_PIN, ZONE5_RELAY_PIN},
    {ZONE6_BUTTON_PIN, ZONE6_RELAY_PIN},
    {ZONE7_BUTTON_PIN, ZONE7_RELAY_PIN},
    {ZONE8_BUTTON_PIN, ZONE8_RELAY_PIN},
    {ZONE9_BUTTON_PIN, ZONE9_RELAY_PIN},
}};

// ================ Timing ================
unsigned long currentMillis = 0;            // Current time in milliseconds
unsigned long previousMillisLongLoop = 0;   // For long loop timing
unsigned long previousMillisMiddleLoop = 0; // For middle loop timing
unsigned long previousMillisShortLoop = 0;  // For short loop timing
unsigned long mainLoopStartTime = 0;        // For loop time plotting
// Encapsulates water level sensing, thresholds, lock states, and MQTT updates.
Esp32WaterLevelSensor waterLevelSensor;
WaterLevelManager waterLevelManager(mqttManager, waterLevelSensor);

void handleConnectivityEvents()
{
  if (wifiManager.consumeDisconnectedEvent())
  {
    mqttManager.forceDisconnect();
  }

  if (wifiManager.consumeConnectedEvent())
  {
    mqttManager.requestConnect();
  }
}

// ================ Main ================

// New function to initialize irrigation zones
void initIrrigationZones()
{
  Trace::log(TraceLevel::INFO, "Initializing irrigation zones...");

  // Initialize zones
  for (int i = 0; i < MAX_IRRIGATION_ZONES; i++)
  {
    Helper::addIrrigationZone(ZONE_CONFIGS[i].buttonPin,
                              ZONE_CONFIGS[i].relayPin, irrigationZones,
                              &mqttManager, i, clientName);
    irrigationZones[i].loadSettingsFromStorage(i);
  }
}

void updateClientNameFromMac()
{
  String macFormatted = Helper::replaceChars(WiFi.macAddress(), ':', '-');
  Helper::formatToBuffer(clientName, CLIENT_NAME_MAX_SIZE,
                         "GardenController-%s", macFormatted.c_str());
  Trace::log(TraceLevel::INFO, "Client name set: " + String(clientName));
}

void connectMqttIfWifiAvailable()
{
  handleConnectivityEvents();
  if (wifiManager.isConnected())
  {
    mqttManager.requestConnect();
  }
}

void setupOta()
{
#if ENABLE_OTA
  Trace::log(TraceLevel::INFO, "Setting up OTA...");
  otaManager.setup(clientName, OTA_PASSWORD);
#else
  Trace::log(TraceLevel::INFO, "OTA disabled in configuration");
  otaManager.setEnabled(false);
#endif
}

void initWatchdog()
{
  esp_task_wdt_init(WDT_TIMEOUT_SEC,
                    true); // Convert milliseconds to seconds
  esp_task_wdt_add(NULL);  // Add current thread to WDT watch
}

void waitForWifiConnection()
{
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
  waitForWifiConnection();

  updateClientNameFromMac();
  waterLevelManager.setup(clientName);

  mqttManager.setup(MQTT_SERVER_IP, MQTT_SERVER_PORT, MQTT_USER, MQTT_PWD,
                    clientName);
  connectMqttIfWifiAvailable();

  setupOta();

  // Initialize irrigation zones using the new helper function
  initIrrigationZones();

  // Initialize the watchdog timer
  initWatchdog();

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

void handleShortIntervalTasks()
{
  handleConnectivityEvents();

  // Publish MQTT messages for all irrigation zones
  mqttManager.publishAllIrrigationZones();

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

  waterLevelManager.loop(currentMillis);
}

bool shouldRunInterval(unsigned long &previousMillis, unsigned long interval)
{
  if ((unsigned long)(currentMillis - previousMillis) < interval)
  {
    return false;
  }

  previousMillis = currentMillis;
  return true;
}

bool handleOtaUpdate()
{
#if ENABLE_OTA
  otaManager.loop();

  // If OTA update is in progress, skip other operations to ensure stability
  if (otaManager.isUpdating())
  {
    return true;
  }
#endif

  return false;
}

void updateLoopTimingPlot()
{
#ifdef ENABLE_LOOP_TIME_PLOTTING
  Trace::plotLoopTime("Mainloop", 0, currentMillis - mainLoopStartTime);
  mainLoopStartTime = currentMillis;
#endif
}

void handleMiddleIntervalEvents()
{
  // Publish detailed system status periodically (every minute)
  mqttManager.publishSystemStatus();
}

void handleLongIntervalTasks()
{
  if (!wifiManager.loop())
  {
    Trace::log(TraceLevel::ERROR, "WiFi connection issue detected");
  }

  handleConnectivityEvents();
}

void loop()
{
  esp_task_wdt_reset();
  currentMillis = millis();

  // Handle OTA updates - this should be processed frequently
  if (handleOtaUpdate())
  {
    return;
  }

  updateLoopTimingPlot();

  if (shouldRunInterval(previousMillisShortLoop, SHORT_INTERVAL))
  {
    handleShortIntervalTasks();
  }

  // Handle middle loop events (e.g., WiFi management)
  if (shouldRunInterval(previousMillisMiddleLoop, MIDDLE_INTERVAL))
  {
    handleMiddleIntervalEvents();

    // Yield to allow other tasks to run
    yield();
  }

  // Handle long loop events (e.g., WiFi connection management)
  if (shouldRunInterval(previousMillisLongLoop, LONG_INTERVAL))
  {
    handleLongIntervalTasks();
  }

#ifdef ENABLE_ZONE_PLOTTING
  plotZoneStates(currentMillis);
#endif
}