// WifiClient.h

#ifndef ESP_WIFI_CLIENT_H
#define ESP_WIFI_CLIENT_H

#include <Arduino.h>
#include <WiFiClient.h>

class EspWifiClient
{
public:
  EspWifiClient();
  void setup(String deviceName);
  WiFiClient *getWifiClient();
  String getDeviceName();
  void reconnectWifi();

private:
  WiFiClient _wifiClient;
  String _deviceName;
  void connect();
  void reconnect();
};

#endif // ESP_WIFI_CLIENT_H