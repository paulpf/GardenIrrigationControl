#define TRACE

#include <Arduino.h>
#include "Trace.h"
#include <algorithm>
#include <cmath>

#include <WiFi.h>
#ifdef USE_PRIVATE_SECRET
#include "../../_secrets/WifiSecret.h"
#else
#include "./_secrets/WifiSecret.h"
#endif

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
// Name is used for the hostname. It is combined with the MAC address to create a unique name.
String clientName = "GardenController-" + replaceChars(WiFi.macAddress(), ':', '-');

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

// ================ Hardware ================
// Button 1
int btn1GpioChannel = 23;
bool btn1Pressed = false;
unsigned long lastDebounceTime;
const int debounceDelay = 500; // debounce time in milliseconds

// Relay 1
int relais1GpioChannel = 22;
bool relais1State = false;

// ================ Timer ================
unsigned long startTimeRel1;
int durationRel1;
int remainingTimeRel1;

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
      else if (millis() - wifiConnectStartTime > 10000) 
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


// ================ Hardware Functions ================
// Button 1
void IRAM_ATTR OnHwBtn1Pressed() 
{
  unsigned long now = millis();
  if (now - lastDebounceTime > debounceDelay) 
  {
    lastDebounceTime = now;
    btn1Pressed = true;
    Trace::log("Loop: Button1 pressed");
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

// ================ Main ================

void setup() 
{
  // Setup console
  Serial.begin(115200);
  Trace::log("Setup begin");

  WiFi.onEvent(WiFiEvent);

  manageWifiConnection();

  // Setup button
  setupButton1();

  // Setup relais
  setupRelais1();

  Trace::log("Setup end");
}

void loop() 
{
  Trace::log("Loop: " + String(millis()));

  // Non-blocking WiFi management
  unsigned long currentMillis = millis();
  if (currentMillis - lastWifiCheckMillis >= wifiCheckInterval) 
  {
    lastWifiCheckMillis = currentMillis;
    manageWifiConnection();
  }

  // ============ Read ============
  


  
  // ============ Process logic ============
  if (WiFi.status() == WL_CONNECTED) 
  {
    // Online operations (cloud updates, etc.)
    checkWifiSignal();
  } 
  else 
  {
    // Offline operations (use local controls only)
  }

  // Button 1 => Relay 1: switch on for 5 seconds
  if (btn1Pressed && !relais1State)
  {
    btn1Pressed = false;
    relais1State = true;

    // Start timer
    startTimeRel1 = millis();
    durationRel1 = 5000;
  }

  // Button 1 => Relay 1: switch off immediately if pressed again
  if(btn1Pressed && relais1State)
  {
    btn1Pressed = false;
    relais1State = false;
  }
  
  // ============ Write ============

  // Update hardware depending on logic and timers
  switchRelay(relais1State);

  // ============ Timers ============
  if (relais1State)
  {
    remainingTimeRel1 = durationRel1 - (millis() - startTimeRel1);
    Trace::log("Remaining time for relais1: " + String(remainingTimeRel1));
    if (remainingTimeRel1 <= 0)
    {
      relais1State = false;
    }
  }
  
  delay(loopDelay);
}