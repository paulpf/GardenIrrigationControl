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

MqttPublisher *MqttPublisher::_instance = nullptr;

MqttPublisher::MqttPublisher()
{
    _instance = this;
}

MqttPublisher::~MqttPublisher()
{
}

void MqttPublisher::setup(WiFiClient *wifiClient, String deviceName)
{
    // Setup console
    Serial.begin(115200);

    this->_wifiClient = wifiClient;
    this->_deviceName = deviceName;
    _mqttClient.setClient(*wifiClient);
    _mqttClient.setServer(MQTT_SERVER_IP, MQTT_SERVER_PORT);
    _mqttClient.setCallback(mqttCallback);
}

void MqttPublisher::publish(Data &data)
{
    // Connect to the mqtt broker
    if (!_mqttClient.connected())
    {
        _instance->reconnectMqtt();
    }
    _mqttClient.loop();

    // Publish data
    String message = "Hello from " + _deviceName;
    if(_mqttClient.publish((_deviceName + "/topic1").c_str(), message.c_str()) == false)
    {
        Serial.println("Failed to send message to topic1");
    }

    if(_mqttClient.publish((_deviceName + "/topic2").c_str(), message.c_str()) == false)
    {
        Serial.println("Failed to send message to topic2");
    }
}

// Method to reconnect to the mqtt broker
void MqttPublisher::reconnectMqtt()
{
    // Loop until we're reconnected
    while (!_mqttClient.connected())
    {
        Serial.print("Attempting MQTT connection...");
        
        // Attempt to connect
        if (_mqttClient.connect(_deviceName.c_str(), MQTT_USER, MQTT_PWD))
        {
            Serial.println("connected");

            // Subscribe to messages
            _mqttClient.subscribe((_deviceName + "/topic1").c_str());
            _mqttClient.subscribe((_deviceName + "/topic2").c_str());
        }
        else
        {
            Serial.print("failed, rc=");
            Serial.print(_mqttClient.state());
            Serial.println(" try again in 5 seconds");
            // Wait 5 seconds before retrying
            delay(5000);
        }
    }
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
    if (String(topic) == (_instance->_deviceName + "topic1"))
    {
        // Do something
    }
    else if (String(topic) == (_instance->_deviceName + "topic2"))
    {
        // Do something
    }
    else
    {
        
    }
}
