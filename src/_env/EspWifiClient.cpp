#ifdef USE_PRIVATE_SECRET
#include "../../_secrets/WifiSecret.h"
#else
#include "./_secrets/WifiSecret.h"
#endif

#include <Arduino.h>
#include <WiFi.h>
#include "_interfaces/EspWifiClient.h"


EspWifiClient::EspWifiClient()
{
}

void EspWifiClient::setup(String deviceName)
{
    this->deviceName = deviceName;
    wifiClient = WiFiClient();

    deviceName += "-" + WiFi.macAddress();
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
    return &wifiClient;
}

// Method to get device name
String EspWifiClient::getDeviceName()
{
    return WiFi.getHostname();
}