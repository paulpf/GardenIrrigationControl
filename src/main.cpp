#include <Arduino.h>
#include <WString.h>
#include "_interfaces/EspWifiClient.h"
#include "_interfaces/OtaManager.h"
#include "_interfaces/WebserverPublisher.h"
#include "_interfaces/PublishManager.h"
#include "_interfaces/MqttPublisher.h"
#include "_interfaces/Data.h"

EspWifiClient espWifiClient;
OtaManager otaManager;
WebserverPublisher wsPublisher;
PublishManager publishManager;
MqttPublisher mqttPublisher;
Data data;

String deviceName = "GardenIC";
unsigned long previousMillis = 0;
const long interval = 1000; // Update interval in milliseconds (1 second)

void setup() 
{
  // EspWifiClient setup
  espWifiClient.setup(deviceName);

  // MqttPublisher setup
  mqttPublisher.setup(espWifiClient.getWifiClient());

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

  static unsigned long lastUpdate = 0;
  const unsigned long updateInterval = 1000; // Update every second
  unsigned long currentMillis = millis();
  
  if (currentMillis - lastUpdate >= updateInterval) 
  {
      lastUpdate = currentMillis;
      data.setCurrentTime(currentMillis);
      publishManager.publish(data);
  }
}