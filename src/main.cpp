#include <Arduino.h>
#include "IrrigationZone.h"
#include "EspWifiClient.h"
#include "MqttClient.h"
#include "Data.h"
#include "Trace.h"
#include "HwButton.h"
#include "SwButton.h"
#include "Relais.h"
#include "Timer.h"

#define TRACE

IrrigationZone irrigationZone;
EspWifiClient espWifiClient;
MqttClient mqttClient;
Data data;

HwButton hwButton1;
SwButton swButton1;
Relais relais1;
Timer timer1;

String deviceName = "GardenControl";

void reconnectCommunication()
{
  // Reconnect to wifi if connection is lost
  espWifiClient.reconnectWifi();

  // Reconnect to mqtt if connection is lost
  mqttClient.reconnectMqtt();
}

void setup() 
{
  // Setup console
  Serial.begin(115200);

  Trace::log("Setup begin");

  // EspWifiClient setup
  Trace::log("EspWifiClient setup");
  espWifiClient.setup(deviceName);

  // MqttPublisher setup
  Trace::log("MqttClient setup");
  mqttClient = MqttClient();
  mqttClient.setup(espWifiClient.getWifiClient(), deviceName);

  reconnectCommunication();

  // HwButton setup
  hwButton1.setup(23);

  // SwButton setup
  swButton1.setup(&mqttClient, 1);
  swButton1.setState(LOW);

  // Relais setup
  relais1.setup(22);

  // Timer setup
  timer1.setup(&mqttClient, 1);

  // IrrigationZone setup
  Trace::log("IrrigationZone setup");
  irrigationZone.setup();
}

void loop() 
{
  Trace::log("Loop: " + String(millis()));

  // Reconnect to wifi if connection is lost
  espWifiClient.reconnectWifi();

  // Reconnect to mqtt if connection is lost
  mqttClient.reconnectMqtt();
  
  // Process incoming MQTT messages
  mqttClient.loop();

  // ============ Read ============

  bool swBtn1State = swButton1.getState();
  bool hwBtn1State = hwButton1.getState();
  int duration1 = timer1.getDuration();
  Trace::log("swBtn1State: " + String(swBtn1State) + " | duration1: " + String(duration1) + " | hwBtn1State: " + String(hwBtn1State));

  // ============ Process logic ============

  irrigationZone.setSwBtnState(swBtn1State);
  irrigationZone.setHwBtnState(hwBtn1State);
  irrigationZone.setDuration(duration1);

  bool relais1State = irrigationZone.getRelaisState(1);
  Trace::log("Relais1State: " + String(relais1State));

  // ============ Write ============
  
  // Update swButton1 state in case if relay1 is switched on/off
  swButton1.setState(relais1State);
  relais1.setState(relais1State);

  delay(1000);
}