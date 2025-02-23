#include <Arduino.h>
#include <WString.h>
#include "_interfaces/EspWifiClient.h"
#include "_interfaces/OtaManager.h"
#include "_interfaces/WebserverPublisher.h"
#include "_interfaces/PublishManager.h"
#include "_interfaces/MqttPublisher.h"
#include "_interfaces/Data.h"
#include "_interfaces/ChannelController.h"

EspWifiClient espWifiClient;
OtaManager otaManager;
WebserverPublisher wsPublisher;
PublishManager publishManager;
MqttPublisher mqttPublisher;
Data data;

// from ESP32 it is GPIO 36
ChannelController chCtrl(36);

String deviceName = "GardenIC-" + WiFi.macAddress() ;

unsigned long previousMillis = 0;
const unsigned long updateInterval = 1000; // Update publish every second

void setup() 
{
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

  chCtrl.setup();
}

void loop() 
{
  // Handle OTA
  otaManager.handle();

  static unsigned long lastUpdate = 0;
    unsigned long currentMillis = millis();
  
  if (currentMillis - lastUpdate >= updateInterval) 
  {
      lastUpdate = currentMillis;
      data.setCurrentTime(currentMillis);
      publishManager.publish(data);
  }

  // wait for a second
  delay(1000);

  // turn the channel on
  chCtrl.setChannel(true);

  // wait for a second
  delay(1000);

  // turn the channel off
  chCtrl.setChannel(false);
}