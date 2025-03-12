#ifdef USE_PRIVATE_SECRET
#include "../../_secrets/WifiSecret.h"
#else
#include "./_secrets/WifiSecret.h"
#endif

#include <Arduino.h>
#include <WiFi.h>
#include "EspWifiClient.h"


EspWifiClient::EspWifiClient()
{
}

void EspWifiClient::setup(String deviceName)
{
  this->_deviceName = deviceName;
  _wifiClient = WiFiClient();

  WiFi.setHostname(deviceName.c_str());
  WiFi.begin(WIFI_SSID, WIFI_PWD);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected at IP address: ");
  Serial.println(WiFi.localIP());
}

WiFiClient *EspWifiClient::getWifiClient()
{
  return &_wifiClient;
}

void EspWifiClient::reconnectWifi()
{
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("WiFi connection lost. Reconnecting...");
    setup(_deviceName);
  }
}

// Method to get device name
String EspWifiClient::getDeviceName()
{
  return WiFi.getHostname();
}