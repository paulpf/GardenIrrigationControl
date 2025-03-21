#define TRACE

#include <Arduino.h>
#include "Trace.h"

#include <WiFi.h>
#ifdef USE_PRIVATE_SECRET
#include "../../_secrets/WifiSecret.h"
#else
#include "./_secrets/WifiSecret.h"
#endif


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

void setupWifi()
{
  WiFi.begin(WIFI_SSID, WIFI_PWD);
  WiFi.setHostname(clientName.c_str());
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected at IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println("Client name: '" + clientName + "'");
}




int btn1GpioChannel = 23;
bool btn1Pressed = false;
unsigned long _lastDebounceTime;
const int _debounceDelay = 500; // debounce time in milliseconds

void IRAM_ATTR OnHwBtn1Pressed() 
{
  unsigned long now = millis();
  if (now - _lastDebounceTime > _debounceDelay) 
  {
    _lastDebounceTime = now;
    btn1Pressed = true;
  }  
}

void setupButton1()
{
  pinMode(btn1GpioChannel, INPUT_PULLDOWN);
  attachInterrupt(digitalPinToInterrupt(btn1GpioChannel), OnHwBtn1Pressed, RISING);
}


void setup() 
{
  // Setup console
  Serial.begin(115200);
  Trace::log("Setup begin");

  // Setup wifi
  setupWifi();

  // Setup button
  setupButton1();
}



void loop() 
{
  Trace::log("Loop: " + String(millis()));

  // ============ Read ============
  if (btn1Pressed)
  {
    Trace::log("Loop: Button1 pressed");
    btn1Pressed = false;
  }


  
  // ============ Process logic ============

  
  // ============ Write ============
  
  
  delay(1000);
}