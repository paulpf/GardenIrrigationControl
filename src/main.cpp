#include <Arduino.h>
#include <WString.h>
#include "_env/EspWifiClient.h"
#include "_env/OtaManager.h"
#include "_interfaces/WebserverPublisher.h"
#include "_interfaces/PublishManager.h"

EspWifiClient espWifiClient;
OtaManager otaManager;
WebserverPublisher wsPublisher;
PublishManager publishManager;
Data data;

String deviceName = "GardenIC";
unsigned long previousMillis = 0;
const long interval = 1000; // Update interval in milliseconds (1 second)

void setup() 
{
  // EspWifiClient setup
  espWifiClient.setup(deviceName);

  // Setup OTA
  otaManager.setup();

  // Setup WebserverPublisher	
  wsPublisher.setup();
  
  // Register WebserverPublisher with PublishManager
  publishManager.registerPublishers(&wsPublisher);
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