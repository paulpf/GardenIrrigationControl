#include "WifiManager.h"

WifiManager::WifiManager(const char* ssid, const char* password, String clientName)
  : _ssid(ssid), _password(password), _clientName(clientName) {}

void WifiManager::setMainContext(MainContext* mainContext) {
  _mainContext = mainContext;
}

// Static pointer to access instance from static callback
static WifiManager* wifiManagerInstance = nullptr;

// Static event handler that can be passed to WiFi.onEvent()
static void staticWiFiEvent(WiFiEvent_t event) 
{
  if (wifiManagerInstance) {
    wifiManagerInstance->handleWiFiEvent(event);
  }
}

void WifiManager::traceWifiState()
{
  Trace::log("WiFi got IP: " + WiFi.localIP().toString());
  Trace::log("WiFi signal strength: " + String(WiFi.RSSI()) + " dBm");
  Trace::log("WiFi SSID: " + WiFi.SSID());
  Trace::log("WiFi client name: " + _clientName);
}

// Instance method to handle WiFi events
void WifiManager::handleWiFiEvent(WiFiEvent_t event) 
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
      manageConnection();
      break;
    case SYSTEM_EVENT_STA_GOT_IP:
      traceWifiState();
      break;
    default:
      break;
  }
}

void WifiManager::setup() 
{
  // Store instance pointer for the static callback
  wifiManagerInstance = this;
  
  // Register the static event handler
  WiFi.onEvent(staticWiFiEvent);
  manageConnection();
}

// Non-blocking WiFi management
void WifiManager::connectionLoop() 
{
  unsigned long currentMillis = millis();
  if (currentMillis - _lastWifiCheckMillis >= _wifiCheckInterval) 
  {
    _lastWifiCheckMillis = currentMillis;
    manageConnection();
  }
}

void WifiManager::manageConnection() 
{
  Trace::log("Manage WiFi connection");

  switch (wifiState) {
    case WIFI_DISCONNECTED:
      Trace::log("Attempting WiFi connection...");
      WiFi.setHostname(_clientName.c_str());
      WiFi.begin(_ssid, _password);
      wifiState = WIFI_CONNECTING;
      _wifiConnectStartTime = millis();
      _countToTryReconnect++;
      break;

    case WIFI_CONNECTING:
      if (WiFi.status() == WL_CONNECTED) {
        Trace::log("WiFi connected after connection attempt " + String(_countToTryReconnect));
        traceWifiState();
        wifiState = WIFI_CONNECTED;
        _countToTryReconnect = 0;
      } else if (millis() - _wifiConnectStartTime > _wifiConnectionTimeout) {
        Trace::log("WiFi connection timeout");
        WiFi.disconnect();
        wifiState = WIFI_DISCONNECTED;

        // If we've tried too many times, restart
        if (_countToTryReconnect >= _maximumCountToTryReconnect) {
          Trace::log("Max reconnection attempts reached, restarting...");
          delay(1000);
          ESP.restart();
        }
      }
      break;

    case WIFI_CONNECTED:
      if (WiFi.status() != WL_CONNECTED) {
        Trace::log("WiFi connection lost");
        WiFi.disconnect();
        wifiState = WIFI_DISCONNECTED;
      }
      break;
  }
}

void WifiManager::checkSignal() {
  if (millis() - _lastWifiCheckMillis >= _wifiCheckInterval) {
    _lastWifiCheckMillis = millis();
    Trace::log("WiFi signal strength: " + String(WiFi.RSSI()) + " dBm");
  }
}

bool WifiManager::isConnected() {
  return wifiState == WIFI_CONNECTED;
}

String WifiManager::getLocalIP() {
  return WiFi.localIP().toString();
}

long WifiManager::getRSSI() {
  return WiFi.RSSI();
}

String WifiManager::getSSID() {
  return WiFi.SSID();
}

String WifiManager::getClientName() {
  return _clientName;
}

bool WifiManager::checkDnsResolution(const char* ipAddress) 
{
  IPAddress resolvedIP;
  if(!WiFi.hostByName(ipAddress, resolvedIP)) 
  {
    Trace::log("DNS resolution failed for " + String(ipAddress));
    return false;
  }
  Trace::log("Resolved DNS to: " + resolvedIP.toString());
  return true;
}