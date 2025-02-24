#include <Arduino.h>
#include <WString.h>
#include "_interfaces/EspWifiClient.h"
#include "_interfaces/OtaManager.h"
#include "_interfaces/WebserverPublisher.h"
#include "_interfaces/PublishManager.h"
#include "_interfaces/MqttPublisher.h"
#include "_interfaces/Data.h"
#include "_interfaces/ChannelController.h"
#include "_interfaces/IrrigationManager.h"

EspWifiClient espWifiClient;
OtaManager otaManager;
WebserverPublisher wsPublisher;
PublishManager publishManager;
MqttPublisher mqttPublisher;

IrrigationManager irrigationManager;
Data data;

String deviceName = "GardenIC-" + WiFi.macAddress() ;

// Hardware buttons, which are used to control the irrigation zones
// Buttons are connected to the following GPIO pins
int buttonPins[] = {2, 3, 4, 5, 6, 7, 8, 9};
int buttonCount = sizeof(buttonPins) / sizeof(buttonPins[0]);

// Drainage valve, which is used to drain the water from the irrigation system
// The drainage valve is connected to the following GPIO pin
int drainageValvePin = 10;


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
  // Read data ==========================================================================
  // ====================================================================================

  // Read the state of the hardware buttons and the drainage valve
  // also read the inputs of all publishers and prioritize the inputs
  // from the publishers over the hardware buttons and the drainage valve
  // if there is a conflict between the inputs from the publishers and the
  // hardware buttons and the drainage valve, the inputs from the publishers
  // should be prioritized.

  // At first read the state of the hardware buttons
  irrigationManager.readData(data);

  // Read the state of the drainage valve
  publishManager.readData(data);

  // ====================================================================================
  // Process data =======================================================================
  // ====================================================================================

  // Prioritize the inputs from the publishers over the hardware buttons and the drainage valve
  // if there is a conflict between the inputs from the publishers and the hardware buttons and the drainage valve,
  // the inputs from the publishers should be prioritized.
  irrigationManager.processData(data);

  // ====================================================================================
  // Write data =========================================================================
  // ====================================================================================

  // Write the state of the irrigation zones and the drainage valve
  // also write the outputs of all publishers and prioritize the outputs
  // from the publishers over the irrigation zones and the drainage valve
  // if there is a conflict between the outputs from the publishers and the
  // irrigation zones and the drainage valve, the outputs from the publishers
  // should be prioritized.

  // Write the state of the irrigation zones
  irrigationManager.writeData(data);

  // publish the state of the irrigation zones and the drainage valve
  publishManager.publish(data);
}