// MqttPublisher.cpp

#include "_interfaces/MqttPublisher.h"
#include <Arduino.h>
#include <WiFiClient.h>

#ifdef USE_PRIVATE_SECRET
#include "../../_secrets/MqttSecret.h"
#include "../../_configs/MqttConfig.h"
#else
#include "_secret/MqttSecret.h"
#include "_config/MqttConfig.h"
#endif

MqttPublisher *MqttPublisher::instance = nullptr;

MqttPublisher::MqttPublisher()
{
    instance = this;
}

MqttPublisher::~MqttPublisher()
{
}

void MqttPublisher::setup(WiFiClient *wifiClient)
{
    this->wifiClient = wifiClient;
    mqttClient.setClient(*wifiClient);
    mqttClient.setServer(MQTT_SERVER_IP, MQTT_SERVER_PORT);
    mqttClient.setCallback(mqttCallback);
}

void MqttPublisher::publish(Data &data)
{
}

// Callback function to handle incoming mqtt messages
void MqttPublisher::mqttCallback(char *topic, byte *payload, unsigned int length)
{
    String message;

    // Convert payload to string
    for (unsigned int i = 0; i < length; i++)
    {
        message += (char)payload[i];
    }

    // Handle incoming messages
    if (String(topic) == ("topic1"))
    {
        // Do something
    }
    else if (String(topic) == ("topic2"))
    {
        // Do something
    }
    else
    {
        
    }
}
