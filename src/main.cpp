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

int loopDelay = 1000;
int countToTryReconnect = 0;
int maximumCountToTryReconnect = 10;
unsigned long lastWifiCheckMillis = 0;
const unsigned long wifiCheckInterval = 30000; // Check every 30 seconds
enum WifiState { WIFI_DISCONNECTED, WIFI_CONNECTING, WIFI_CONNECTED };
WifiState wifiState = WIFI_DISCONNECTED;
unsigned long wifiConnectStartTime = 0;
int reconnectAttempt = 0;
int delayTime = 0;

int btn1GpioChannel = 23;
bool btn1Pressed = false;
unsigned long _lastDebounceTime;
const int _debounceDelay = 500; // debounce time in milliseconds

int relais1GpioChannel = 22;
bool relais1State = false;


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

// Name is used for the hostname. It is combined with the MAC address to create a unique name.
String clientName = "GardenController-" + replaceChars(WiFi.macAddress(), ':', '-');

void TraceWifiState()
{
  Trace::log("WiFi got IP: " + WiFi.localIP().toString());
  Trace::log("WiFi signal strength: " + String(WiFi.RSSI()) + " dBm");
  Trace::log("WiFi SSID: " + WiFi.SSID());
  Trace::log("WiFi client name: " + clientName);
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

void IRAM_ATTR OnHwBtn1Pressed() 
{
  unsigned long now = millis();
  if (now - _lastDebounceTime > _debounceDelay) 
  {
    _lastDebounceTime = now;
    btn1Pressed = !btn1Pressed;
    Trace::log("Loop: Button1 pressed");
  }  
}

void setupButton1()
{
  pinMode(btn1GpioChannel, INPUT_PULLDOWN);
  attachInterrupt(digitalPinToInterrupt(btn1GpioChannel), OnHwBtn1Pressed, RISING);
}

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

void WiFiEvent(WiFiEvent_t event) 
{
  switch(event) 
  {
    case SYSTEM_EVENT_STA_CONNECTED:
      Trace::log("Connected to WiFi");
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      Trace::log("Disconnected from WiFi");
      // Can trigger reconnection here
      break;
    case SYSTEM_EVENT_STA_GOT_IP:
      TraceWifiState();
      break;
    default:
      break;
  }
}

// Add to your regular checks
void checkWifiSignal() 
{
  long rssi = WiFi.RSSI();
  Trace::log("WiFi signal strength: " + String(rssi) + " dBm");
  
  if (rssi < -80) 
  {
    Trace::log("Warning: Poor WiFi signal");
  }
}


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
  } 
  else 
  {
    // Offline operations (use local controls only)
  }

  if (btn1Pressed)
  {
    relais1State = true;
  }
  else
  {
    relais1State = false;
  }
  
  // ============ Write ============
  switchRelay(relais1State);
  
  delay(loopDelay);
}