#include "wifimanager.h"

// Initialize the static instance pointer
WifiManager* WifiManager::_instance = nullptr;

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
    Trace::log("WiFi setup complete.");
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
    switch(event) 
    {
        case SYSTEM_EVENT_STA_START:
            Trace::log("WiFi started, attempting to connect...");
            WiFi.begin(_ssid.c_str(), _password.c_str());
            _wifiConnectStartTime = millis();
            _wifiState = WIFI_CONNECTING;
            break;
        case SYSTEM_EVENT_STA_GOT_IP:
            Trace::log("WiFi connected, IP: " + WiFi.localIP().toString());
            _wifiState = WIFI_CONNECTED;
            break;
        case SYSTEM_EVENT_STA_DISCONNECTED:
            Trace::log("WiFi disconnected, attempting to reconnect...");
            _wifiState = WIFI_DISCONNECTED;
            manageConnection();
            break;
        default:
            break;
    }
}

void WifiManager::loop()
{
    // Check WiFi connection status
    if (_wifiState == WIFI_CONNECTING && WiFi.status() == WL_CONNECTED) 
    {
        Trace::log("WiFi connected after connection attempt " + String(_reconnectAttempt));
        _wifiState = WIFI_CONNECTED;
        _reconnectAttempt = 0;
    } 
    else if (_wifiState == WIFI_DISCONNECTED && millis() - _wifiConnectStartTime > WIFI_CONNECTION_TIMEOUT) 
    {
        Trace::log("WiFi connection timeout, attempting to reconnect...");
        manageConnection();
    }
}

void WifiManager::manageConnection()
{
    if (_reconnectAttempt < _maximumCountToTryReconnect) 
    {
        Trace::log("Attempting to reconnect to WiFi...");
        WiFi.disconnect();
        WiFi.begin(_ssid, _password);
        _wifiConnectStartTime = millis();
        _reconnectAttempt++;
    } 
    else 
    {
        Trace::log("Max reconnection attempts reached, restarting...");
        delay(1000);
        ESP.restart();
    }
}