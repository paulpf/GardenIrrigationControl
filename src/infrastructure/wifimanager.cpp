#include "wifimanager.h"

// Initialize the static instance pointer
WifiManager *WifiManager::_instance = nullptr;

WifiManager::WifiManager()
{
  // Store the instance pointer
  _instance = this;
}

void WifiManager::setup(String ssid, String password, String clientName)
{
  // Store the WiFi credentials and client name
  _ssid = ssid;
  _password = password;
  _clientName = clientName;

  // Setup WiFi connection
  WiFi.mode(WIFI_STA);
  WiFi.setHostname(clientName.c_str());
  WiFi.onEvent(staticWifiEventHandler);

  // Trigger initial connection explicitly to avoid relying on event ordering.
  WiFi.begin(_ssid.c_str(), _password.c_str());
  _wifiConnectStartTime = millis();
  _wifiState = WIFI_CONNECTING;

  Trace::log(TraceLevel::DEBUG, "WiFi setup complete.");
}

// Static WiFi event handler
void WifiManager::staticWifiEventHandler(WiFiEvent_t event)
{
  // Forward to the instance method if instance exists
  if (_instance)
  {
    _instance->wifiEvent(event);
  }
}

void WifiManager::wifiEvent(WiFiEvent_t event)
{
  switch (event)
  {
  case SYSTEM_EVENT_STA_START:
    Trace::log(TraceLevel::INFO, "WiFi started");
    break;
  case SYSTEM_EVENT_STA_GOT_IP:
    Trace::log(TraceLevel::INFO,
               "WiFi connected, IP: " + WiFi.localIP().toString());
    _wifiState = WIFI_CONNECTED;
    _connectedEventPending = true;
    _disconnectedEventPending = false;
    _reconnectAttempt = 0;
    break;
  case SYSTEM_EVENT_STA_DISCONNECTED:
    Trace::log(TraceLevel::INFO,
               "WiFi disconnected, attempting to reconnect...");
    _wifiState = WIFI_DISCONNECTED;
    _disconnectedEventPending = true;
    _connectedEventPending = false;
    manageConnection();
    break;
  default:
    break;
  }
}

bool WifiManager::consumeConnectedEvent()
{
  bool hadEvent = _connectedEventPending;
  _connectedEventPending = false;
  return hadEvent;
}

bool WifiManager::consumeDisconnectedEvent()
{
  bool hadEvent = _disconnectedEventPending;
  _disconnectedEventPending = false;
  return hadEvent;
}

bool WifiManager::loop()
{
  // Check WiFi connection status
  if (_wifiState == WIFI_CONNECTING && WiFi.status() == WL_CONNECTED)
  {
    Trace::log(TraceLevel::DEBUG, "WiFi connected after connection attempt " +
                                      String(_reconnectAttempt));
    _wifiState = WIFI_CONNECTED;
    _reconnectAttempt = 0;
  }
  else if (_wifiState == WIFI_DISCONNECTED &&
           millis() - _wifiConnectStartTime > WIFI_CONNECTION_TIMEOUT)
  {
    Trace::log(TraceLevel::INFO,
               "WiFi connection timeout, attempting to reconnect...");
    manageConnection();
  }

  // Return connection status: true if connected, false otherwise
  return _wifiState == WIFI_CONNECTED;
}

void WifiManager::manageConnection()
{
  if (_reconnectAttempt < _maximumCountToTryReconnect)
  {
    Trace::log(TraceLevel::INFO, "Attempting to reconnect to WiFi...");
    WiFi.disconnect();
    WiFi.begin(_ssid, _password);
    _wifiConnectStartTime = millis();
    _reconnectAttempt++;
  }
  else
  {
    Trace::log(TraceLevel::WARNING,
               "Max reconnection attempts reached, restarting...");
    delay(1000);
    ESP.restart();
  }
}