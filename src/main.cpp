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

#include <WiFiClient.h>
#include "irrigationZone.h"
#include "mqttmanager.h"

#include "esp_task_wdt.h"
#include "esp_system.h"

// ================ Constants ================
int loopDelay = 1000;
const int WIFI_CONNECTION_TIMEOUT = 10000; // 10 seconds
const int WATCHDOG_TIMEOUT = 60000;

// Name is used for the hostname. It is combined with the MAC address to create a unique name.
String clientName = "GardenController-" + Tools::replaceChars(WiFi.macAddress(), ':', '-');

// ================ WiFi ================
int countToTryReconnect = 0;
int maximumCountToTryReconnect = 10;
unsigned long lastWifiCheckMillis = 0;
const unsigned long wifiCheckInterval = 30000; // Check every 30 seconds
enum WifiState 
{
  WIFI_DISCONNECTED,
  WIFI_CONNECTING,
  WIFI_CONNECTED 
};
WifiState wifiState = WIFI_DISCONNECTED;
unsigned long wifiConnectStartTime = 0;
int reconnectAttempt = 0;

// ================ MQTT ================
MqttManager mqttManager;

// ================ Irrigation zones ================
IrrigationZone irrigationZone1;

bool checkDnsResolution() 
{
  IPAddress resolvedIP;
  if(!WiFi.hostByName(MQTT_SERVER_IP, resolvedIP)) 
  {
    Trace::log("DNS resolution failed for " + String(MQTT_SERVER_IP));
    return false;
  }
  Trace::log("Resolved MQTT server to: " + resolvedIP.toString());
  return true;
}

// ================ Wifi Functions ================
void TraceWifiState()
{
  Trace::log("WiFi got IP: " + WiFi.localIP().toString());
  Trace::log("WiFi signal strength: " + String(WiFi.RSSI()) + " dBm");
  Trace::log("WiFi SSID: " + WiFi.SSID());
  Trace::log("WiFi client name: " + clientName);
}

void checkWifiSignal() 
{
  long rssi = WiFi.RSSI();
  Trace::log("WiFi signal strength: " + String(rssi) + " dBm");
  
  if (rssi < -80) 
  {
    Trace::log("Warning: Poor WiFi signal");
  }
}

void manageWifiConnection() 
{
  Trace::log("Manage WiFi connection");
  switch (wifiState) 
  {
    case WIFI_DISCONNECTED:
      Trace::log("Attempting WiFi connection...");
      WiFi.setHostname(clientName.c_str());
      WiFi.begin(WIFI_SSID, WIFI_PWD);
      wifiState = WIFI_CONNECTING;
      wifiConnectStartTime = millis();
      countToTryReconnect++;
      break;
      
    case WIFI_CONNECTING:
      if (WiFi.status() == WL_CONNECTED) 
      {
        Trace::log("WiFi connected after connection attempt " + String(countToTryReconnect));
        TraceWifiState();
        wifiState = WIFI_CONNECTED;
        countToTryReconnect = 0;
      } 
      else if (millis() - wifiConnectStartTime > WIFI_CONNECTION_TIMEOUT) 
      { // 10 second timeout
        Trace::log("WiFi connection timeout");
        WiFi.disconnect();
        wifiState = WIFI_DISCONNECTED;
        
        // If we've tried too many times, restart
        if (countToTryReconnect >= maximumCountToTryReconnect) 
        {
          Trace::log("Max reconnection attempts reached, restarting...");
          delay(1000);
          ESP.restart();
        }
      }
      break;
      
    case WIFI_CONNECTED:
      if (WiFi.status() != WL_CONNECTED) 
      {
        Trace::log("WiFi connection lost");
        WiFi.disconnect();
        wifiState = WIFI_DISCONNECTED;
      }
      break;
  }
}

void WiFiEvent(WiFiEvent_t event) 
{
  switch(event) 
  {
    case SYSTEM_EVENT_STA_CONNECTED:
      Trace::log("Connected to WiFi");
      wifiState = WIFI_CONNECTED;
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      Trace::log("Disconnected from WiFi");
      wifiState = WIFI_DISCONNECTED;
      // Can trigger reconnection here
      manageWifiConnection();
      break;
    case SYSTEM_EVENT_STA_GOT_IP:
      TraceWifiState();
      break;
    default:
      break;
  }
}

void nonBlockingWifiManagement()
{
  unsigned long currentMillis = millis();
  if (currentMillis - lastWifiCheckMillis >= wifiCheckInterval) 
  {
    lastWifiCheckMillis = currentMillis;
    manageWifiConnection();
  }
}

// ================ Main ================

void setup() 
{
  // Setup console
  Serial.begin(115200);
  Trace::log("Setup begin");

  // Setup WiFi
  WiFi.onEvent(WiFiEvent);
  manageWifiConnection();

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
  
  // Non-blocking WiFi management
  nonBlockingWifiManagement();

  // ============ Irrigation zone loop ============
  irrigationZone1.loop();


  // Non-blocking MQTT management
  mqttManager.loop();

  
  // ============ MQTT update ============
  if (mqttManager.isConnected()) 
  {
    // Irrigation zone 1
    mqttManager.publish(irrigationZone1.getMqttTopicForRelay().c_str(), 
                        irrigationZone1.getRelayState() ? "true" : "false");
    mqttManager.publish(irrigationZone1.getMqttTopicForRemainingTime().c_str(), 
                        String(irrigationZone1.getRemainingTime()).c_str());
    mqttManager.publish(irrigationZone1.getMqttTopicForSwButton().c_str(),
                        irrigationZone1.getBtnState() ? "true" : "false");

    // Publish other topics as needed
  }

  if (WiFi.status() == WL_CONNECTED) 
  {
    checkWifiSignal();
  } 
  
  delay(loopDelay);
  Trace::log("Loop end");
}