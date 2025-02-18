// MqttPublisher.h

#ifndef MQTT_PUBLISHER_H
#define MQTT_PUBLISHER_H

#include "IPublisher.h"
#include "Data.h"
#include <Arduino.h>
#include <WiFiClient.h>

class MqttPublisher : public IPublisher
{
public:
    MqttPublisher();
    ~MqttPublisher();
    void setup(WiFiClient *wifiClient);
    void publish(Data &data);
private:
    static MqttPublisher *instance;
    WiFiClient *wifiClient;
};

#endif // MQTT_PUBLISHER_H