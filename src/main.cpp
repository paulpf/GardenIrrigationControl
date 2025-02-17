#include <Arduino.h>
#include "_env/EspWifiClient.h"
#include "_env/OtaManager.h"
#include <string>

EspWifiClient espWifiClient;
OtaManager otaManager;

String deviceName = "GardenIC";


void setup() 
{
  // EspWifiClient setup
  espWifiClient.setup(deviceName);

  // Setup OTA
  otaManager.setup();
}

void loop() 
{
  // Handle OTA
  otaManager.handle();
}