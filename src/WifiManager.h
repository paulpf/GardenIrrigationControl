#ifndef WifiManager_H
#define WifiManager_H

#include "GlobalDefines.h"
#include <WiFi.h>

// Forward declaration
class MainContext;

class WifiManager {
public:
  WifiManager(const char* ssid, const char* password, String clientName);
  void setup();
  void connectionLoop();
  void checkSignal();
  bool isConnected();
  String getLocalIP();
  long getRSSI();
  String getSSID();
  String getClientName();
  void setMainContext(MainContext* mainContext);
  void handleWiFiEvent(WiFiEvent_t event);
  bool checkDnsResolution(const char* ipAddress);

  enum WifiState {
    WIFI_DISCONNECTED,
    WIFI_CONNECTING,
    WIFI_CONNECTED
  };

  WifiState wifiState = WIFI_DISCONNECTED;

private:
  const char* _ssid;
  const char* _password;
  String _clientName;
  unsigned long _wifiConnectStartTime = 0;
  int _countToTryReconnect = 0;
  const int _maximumCountToTryReconnect = 10;
  const int _wifiConnectionTimeout = 10000; // 10 seconds
  unsigned long _lastWifiCheckMillis = 0;
  const unsigned long _wifiCheckInterval = 30000; // Check every 30 seconds
  MainContext* _mainContext;
  void manageConnection();

  void traceWifiState();
};

#endif