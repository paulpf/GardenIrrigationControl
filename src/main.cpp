#define TRACE

#include <Arduino.h>
#include "Trace.h"
#include <algorithm>
#include <cmath>
#include <PubSubClient.h>
#include <WiFiClient.h>
#include "esp_task_wdt.h"
#include "esp_system.h"

#include <WiFi.h>

#ifdef USE_PRIVATE_SECRET
#include "../../_secrets/WifiSecret.h"
#include "../../_configs/MqttConfig.h"
#include "../../_secrets/MqttSecret.h"
#else
#include "./_secrets/WifiSecret.h"
#include "./_config/MqttConfig.h"
#include "./_secrets/MqttSecret.h"
#endif

#include "WifiManager.h" // Include the WifiManager header

// ================ Helper functions ================
// Help function to replace characters in a string
String replaceChars(String str, char charsToReplace, char replaceWith)
{
  for (int i = 0; i < str.length(); i++)
  {
    if (str[i] == charsToReplace)
    {
      str[i] = replaceWith;
    }
  }
  return str;
}

// ================ Constants ================
int loopDelay = 1000;
const int WATCHDOG_TIMEOUT = 60000;

// Name is used for the hostname. It is combined with the MAC address to create a unique name.
String clientName = "GardenController-" + replaceChars(WiFi.macAddress(), ':', '-');

// ================ WiFi ================
unsigned long lastWifiCheckMillis = 0;
const unsigned long wifiCheckInterval = 30000; // Check every 30 seconds
int reconnectAttempt = 0;

bool synconizedBtn1NewState = false;
bool synconizedBtn1OldState = false;

// ================ Hardware buttons ================
// Button 1
int btn1GpioChannel = 23;
bool hwBtn1State = false;
unsigned long lastDebounceTime;
const int debounceDelay = 500; // debounce time in milliseconds

// ================ Software buttons (via MQTT) ================
// Button 1
bool swBtn1State = false;
bool swBtn1StateOld = false;

// ================ Relays ================
// Relay 1
int relais1GpioChannel = 22;
bool relais1State = false;

// ================ Timers ================
// for relay 1
unsigned long startTimeRel1;
int durationRel1;
int remainingTimeRel1;

// ================ MQTT ================
WiFiClient wifiClient;
PubSubClient pubSubClient(wifiClient);

// MQTT state management
enum MqttState 
{
  _MQTT_DISCONNECTED,
  _MQTT_CONNECTING,
  _MQTT_CONNECTED
};
MqttState mqttState = _MQTT_DISCONNECTED;
unsigned long lastMqttAttemptMillis = 0;
const unsigned long mqttRetryInterval = 5000; // Wait 5 seconds between connection attempts
int mqttReconnectAttempts = 0;
const int maxMqttReconnectAttempts = 5;

// ================ WifiManager Instance ================
WifiManager wifiManager(WIFI_SSID, WIFI_PWD, clientName);

void synchronizeButtonStates(bool newState) 
{
  synconizedBtn1NewState = swBtn1State = hwBtn1State = newState;
}

// Callback function to handle incoming MQTT messages
void mqttCallback(char* topic, byte* payload, unsigned int length)
{
  Trace::log("Message arrived [" + String(topic) + "]");
  String message = "";
  for (int i = 0; i < length; i++) 
  {
    message += (char)payload[i];
  }
  Trace::log("Message: " + message);

  if (String(topic).startsWith(clientName + "/swBtn1"))
  {
    swBtn1State = message == "true";
    // synchronize the new state with the hardware state
    synchronizeButtonStates(swBtn1State);
  }
}

void reconnectMqtt() 
{
  // Single connection attempt instead of blocking loop
  Trace::log("Attempting MQTT connection...");
  if (pubSubClient.connect(clientName.c_str(), MQTT_USER, MQTT_PWD)) 
  {
    Trace::log("MQTT connected");
    mqttState = _MQTT_CONNECTED;
    mqttReconnectAttempts = 0;
    
    // Subscribe to topics
    pubSubClient.subscribe((clientName + "/swBtn1").c_str());
  } 
  else 
  {
    mqttReconnectAttempts++;
    Trace::log("MQTT connection failed, rc=" + String(pubSubClient.state()) + 
               ", attempts: " + String(mqttReconnectAttempts));
    mqttState = _MQTT_DISCONNECTED;
    
    if (mqttReconnectAttempts >= maxMqttReconnectAttempts) {
      Trace::log("Maximum MQTT reconnection attempts reached, will try again later");
      mqttReconnectAttempts = 0;
    }
  }
}

void manageMqttConnection() 
{
  Trace::log("Managing MQTT connection");
  
  if (!wifiManager.isConnected()) 
  {
    Trace::log("Cannot connect to MQTT - WiFi is disconnected");
    mqttState = _MQTT_DISCONNECTED;
    return; // Can't connect to MQTT without WiFi
  }

  // DNS check before attempting connection
  if (mqttState == _MQTT_DISCONNECTED && !wifiManager.checkDnsResolution(MQTT_SERVER_IP)) 
  {
    return;  // Skip connection attempt if DNS resolution fails
  }
  
  switch (mqttState) 
  {
    case _MQTT_DISCONNECTED:
      Trace::log("Mqtt is disconnected");
      reconnectMqtt();
      lastMqttAttemptMillis = millis();
      break;
      
    case _MQTT_CONNECTED:
      if (!pubSubClient.connected()) 
      {
        Trace::log("MQTT connection lost");
        mqttState = _MQTT_DISCONNECTED;
      } 
      else 
      {
        Trace::log("MQTT loop");
        pubSubClient.loop(); // Process incoming messages and maintain connection
      }
      break;
  }
}

void publishMqtt(String topic, String payload) 
{
  if (mqttState == _MQTT_CONNECTED) 
  {
    pubSubClient.publish(topic.c_str(), payload.c_str());
  }
  else 
  {
    Trace::log("Cannot publish to MQTT - not connected");
  }
}

// ================ Hardware Functions ================
// Button 1
void IRAM_ATTR OnHwBtn1Pressed() 
{
  unsigned long now = millis();
  if (now - lastDebounceTime > debounceDelay) 
  {
    lastDebounceTime = now;
    hwBtn1State = !hwBtn1State;
    // synchronize the new state with the software state
    synchronizeButtonStates(hwBtn1State);
  }  
}

void setupButton1()
{
  pinMode(btn1GpioChannel, INPUT_PULLDOWN);
  attachInterrupt(digitalPinToInterrupt(btn1GpioChannel), OnHwBtn1Pressed, RISING);
}

// Relay 1
void setupRelais1()
{
  pinMode(relais1GpioChannel, OUTPUT);
  digitalWrite(relais1GpioChannel, HIGH);
}

void switchRelay(bool state)
{
  if (state)
  {
    Trace::log("Switching relay ON");
    digitalWrite(relais1GpioChannel, LOW);
  }
  else
  {
    Trace::log("Switching relay OFF");
    digitalWrite(relais1GpioChannel, HIGH);
  }
}

void nonBlockingMqttManagement()
{
  unsigned long currentMillis = millis();
  if (mqttState == _MQTT_DISCONNECTED && 
    currentMillis - lastMqttAttemptMillis >= mqttRetryInterval) 
  {
    manageMqttConnection();
  }
  else if (mqttState == _MQTT_CONNECTED) 
  {
    pubSubClient.loop(); // Process incoming messages
  }
}

void lastOperationsInTheLoop()
{
  if (wifiManager.isConnected()) 
  {
    // Online operations (cloud updates, etc.)
    wifiManager.checkSignal();
  } 
  else 
  {
    // Offline operations (use local controls only)
  }
}

// ================ Main ================

void setup() 
{
  synconizedBtn1NewState = false;
  synconizedBtn1OldState = false;
  hwBtn1State = false;
  swBtn1State = false;
  
  // Setup console
  Serial.begin(115200);
  Trace::log("Setup begin");

  // Setup WiFi
  wifiManager.setup();

  // Setup MQTT
  pubSubClient.setServer(MQTT_SERVER_IP, MQTT_SERVER_PORT);
  pubSubClient.setCallback(mqttCallback);
  
  // Setup button
  setupButton1();

  // Setup relais
  setupRelais1();

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
  
  // Manage WiFi connection
  wifiManager.connectionLoop();
  
  // Non-blocking MQTT management
  nonBlockingMqttManagement();

  // ============ Read ============

  // ============ Process logic ============
  // if btn1 is active, relais 1 should be active for specific time
  if (relais1State == false && synconizedBtn1NewState == true)
  {
    relais1State = true;

    // Start timer
    startTimeRel1 = millis();
    durationRel1 = 10000; // 10 seconds
  }
  // if btn1 is inactive, relais 1 should be inactive
  else if(relais1State == true && synconizedBtn1NewState == false)
  {
    relais1State = false;
    remainingTimeRel1 = 0;
  }

  // ============ Timers ============
  if (relais1State)
  {
    remainingTimeRel1 = durationRel1 - (millis() - startTimeRel1);
    Trace::log("Remaining time for relais1: " + String(remainingTimeRel1));
    if (remainingTimeRel1 <= 0)
    {
      Trace::log("Relais1 timer expired");
      remainingTimeRel1 = 0;
      relais1State = false;
      synchronizeButtonStates(false);
    }
  }
  
  // ============ Write ============
  // Update hardware depending on logic and timers
  switchRelay(relais1State);

  // ============ MQTT update ============
  if (mqttState == _MQTT_CONNECTED) 
  {
    publishMqtt(clientName + "/relais1", String(relais1State));
    publishMqtt(clientName + "/remainingTimeRel1", String(remainingTimeRel1));
    // Block MQTT updates for buttons to avoid feedback loop
    Trace::log("Publishing button state: " + String(synconizedBtn1NewState ? "true" : "false"));
    // Publish a string representation of the boolean state
    publishMqtt(clientName + "/swBtn1", synconizedBtn1NewState ? "true" : "false");
  }

  lastOperationsInTheLoop();
  
  delay(loopDelay);
  Trace::log("Loop end");
}