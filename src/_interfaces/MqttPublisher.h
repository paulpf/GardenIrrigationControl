// MqttPublisher.h

#ifndef MQTT_PUBLISHER_H
#define MQTT_PUBLISHER_H

#include "IPublisher.h"
#include "Data.h"
#include <Arduino.h>
#include <WiFiClient.h>
#include <PubSubClient.h>

class MqttPublisher : public IPublisher
{
public:
    MqttPublisher();
    ~MqttPublisher();
    void setup(WiFiClient *wifiClient, String deviceName);
    void publish(Data &data);
private:
    static MqttPublisher *_instance;
    WiFiClient *_wifiClient;
    String _deviceName;
    PubSubClient _mqttClient;
    static void mqttCallback(char *topic, byte *payload, unsigned int length);
    void reconnectMqtt();
};

#endif // MQTT_PUBLISHER_H