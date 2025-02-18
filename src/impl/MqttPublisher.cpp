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
}

void MqttPublisher::publish(Data &data)
{
}
