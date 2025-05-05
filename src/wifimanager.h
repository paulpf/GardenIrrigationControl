// wifimanager.h
#ifndef WIFIMANAGER_H
#define WIFIMANAGER_H

#include "globalDefines.h"

class WifiManager
{
public:
  WifiManager(/* args */);
  void setup(String ssid, String password, String clientName);
  bool loop();
  void manageConnection();
  bool checkDnsResolution();
  
  // Static WiFi event handler that will be used with WiFi.onEvent
  static void staticWifiEventHandler(WiFiEvent_t event);

private:
  // Static pointer to the instance (similar to MqttManager's implementation)
  static WifiManager* _instance;
  
  // Instance method to handle WiFi events
  void wifiEvent(WiFiEvent_t event);
  
  String _ssid;
  String _password;
  String _clientName;
  const int WIFI_CONNECTION_TIMEOUT = 10000; // 10 seconds
  int _countToTryReconnect = 0;
  int _maximumCountToTryReconnect = 10;
  unsigned long _lastWifiCheckMillis = 0;
  const unsigned long _wifiCheckInterval = 30000; // Check every 30 seconds
  enum WifiState 
  {
    WIFI_DISCONNECTED,
    WIFI_CONNECTING,
    WIFI_CONNECTED 
  };
  WifiState _wifiState = WIFI_DISCONNECTED;
  unsigned long _wifiConnectStartTime = 0;
  int _reconnectAttempt = 0;
};

#endif // WIFIMANAGER_H