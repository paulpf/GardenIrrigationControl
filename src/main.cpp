#include <Arduino.h>
#include <WString.h>
#include "_interfaces/EspWifiClient.h"
#include "_interfaces/OtaManager.h"
#include "_interfaces/WebserverPublisher.h"
#include "_interfaces/PublishManager.h"
#include "_interfaces/MqttPublisher.h"
#include "_interfaces/Data.h"
#include "_interfaces/IrrigationManager.h"
#include "_interfaces/HardwareButton.h"

EspWifiClient espWifiClient;
OtaManager otaManager;
WebserverPublisher wsPublisher;
PublishManager publishManager;
MqttPublisher mqttPublisher;
IrrigationManager irrigationManager;
Data data;

String deviceName = "GardenIC-" + WiFi.macAddress() ;


void setup() 
{
  // IrrigationManager setup
  irrigationManager.setup();

  // EspWifiClient setup
  espWifiClient.setup(deviceName);

  // MqttPublisher setup
  mqttPublisher.setup(espWifiClient.getWifiClient(), deviceName);

  // Setup OTA
  otaManager.setup();

  // Setup WebserverPublisher	
  wsPublisher.setup();
  
  // Register WebserverPublisher with PublishManager
  publishManager.registerPublishers(&wsPublisher);

  // Register MqttPublisher with PublishManager
  publishManager.registerPublishers(&mqttPublisher);
}

void loop() 
{
  // Handle OTA
  otaManager.handle();

  // ====================================================================================
  // Read inputs ========================================================================
  // ====================================================================================

  irrigationManager.readInputs();

  // ====================================================================================
  // Process data =======================================================================
  // ====================================================================================

  // Prioritize the inputs from the publishers over the hardware buttons and the drainage valve
  // if there is a conflict between the inputs from the publishers and the hardware buttons and the drainage valve,
  // the inputs from the publishers should be prioritized.
  irrigationManager.processLogic();

  // ====================================================================================
  // Write outputs ======================================================================
  // ====================================================================================

  irrigationManager.writeOutputs();

  // publish the state of the irrigation zones and the drainage valve
  publishManager.publish(data);
}