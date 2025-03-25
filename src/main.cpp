#ifdef USE_PRIVATE_SECRET
#include "../../_secrets/WifiSecret.h"
#include "../../_configs/MqttConfig.h"
#include "../../_secrets/MqttSecret.h"
#else
#include "./_secrets/WifiSecret.h"
#include "./_config/MqttConfig.h"
#include "./_secrets/MqttSecret.h"
#endif

#include "GlobalDefines.h"

#include "esp_task_wdt.h"
#include "esp_system.h"
#include "MqttManager.h"
#include "WifiManager.h" // Include the WifiManager header
#include "HardwareButton.h"
#include "Relay.h"
#include "Timer.h"
#include "IrrigationZone.h"

// ================ Constants ================
String CLIENT_NAME = "GardenController-" + Tools::replaceChars(WiFi.macAddress(), ':', '-');
int LOOP_DELAY = 1000;
const int WATCHDOG_TIMEOUT = 60000;
int HWBTN1_GPIOPIN = 23;
int HWBTN_DEBOUNCE_DELAY = 500;
int RELAY1_GPIOPIN = 22;

WifiManager wifiManager(WIFI_SSID, WIFI_PWD, CLIENT_NAME);
MqttManager mqttManager(MQTT_SERVER_IP, MQTT_SERVER_PORT, MQTT_USER, MQTT_PWD, CLIENT_NAME);

IrrigationZone irrigationZone(HWBTN1_GPIOPIN, RELAY1_GPIOPIN, HWBTN_DEBOUNCE_DELAY, CLIENT_NAME, mqttManager);


// bool synchronizedBtn1NewState = false;
// bool hwBtn1State = false;
// bool swBtn1State = false;

// void synchronizeButtonStates(bool newState) 
// {
//   synchronizedBtn1NewState = swBtn1State = hwBtn1State = newState;
// }

// void IRAM_ATTR onHwBtn1Pressed() {
//   Trace::log("Hardware button 1 pressed");  
//   hwBtn1State = !hwBtn1State;
//   synchronizeButtonStates(hwBtn1State);  
// }

// HardwareButton hwButton1(HWBTN1_GPIOPIN, HWBTN_DEBOUNCE_DELAY);

// void handleSwBtn1Message(const String& message) {
//   Trace::log("Received message on topic " + CLIENT_NAME + "/swBtn1: " + message);
//   swBtn1State = message == "true";
//   synchronizeButtonStates(swBtn1State);
// }

// Relay relay1(RELAY1_GPIOPIN);

// void onRelay1TimerDeactivated() {
//   Trace::log("Relay 1 timer deactivated");
//   relay1.activate(false);
//   synchronizeButtonStates(false);
//   mqttManager.publish(CLIENT_NAME + "/relais1", String(relay1.isActive()));
//   mqttManager.publish(CLIENT_NAME + "/swBtn1", synchronizedBtn1NewState ? "true" : "false");  
//   mqttManager.publish(CLIENT_NAME + "/remainingTimeRel1", String(0));
// }

// void onRelay1TimerActivated() {
//   Trace::log("Relay 1 timer activated");
//   relay1.activate(true);
//   synchronizeButtonStates(true);
//   mqttManager.publish(CLIENT_NAME + "/relais1", String(relay1.isActive()));
//   mqttManager.publish(CLIENT_NAME + "/swBtn1", synchronizedBtn1NewState ? "true" : "false");  
// }

// Timer relay1Timer;

// void onRelay1TimerTick() {
//   mqttManager.publish(CLIENT_NAME + "/remainingTimeRel1", String(relay1Timer.getRemainingTime()));
// }

// void updateRelayTimerState()
// {
//   if (synchronizedBtn1NewState == true && relay1Timer.isActive() == false)
//   {
//     relay1Timer.start(10000); // Start timer for 10 seconds
//   }
//   else if (synchronizedBtn1NewState == false && relay1Timer.isActive() == true)
//   {
//     relay1Timer.stop();
//   }
// }


// ================ Main ================

void setup() 
{
  // synchronizedBtn1NewState = false;
  // hwBtn1State = false;
  // swBtn1State = false;
  
  // Setup console
  Serial.begin(115200);
  Trace::log("Setup begin");

  // Setup WiFi
  wifiManager.setup();

  // Setup MQTT
  // mqttManager.registerTopicHandler(CLIENT_NAME + "/swBtn1", [&](const String& message) {
  //   irrigationZone.handleSwBtnMessage(message);
  // });
  mqttManager.setup();
  // mqttManager.subscribeToTopic(CLIENT_NAME + "/swBtn1");
  
  // Setup IrrigationZone
  irrigationZone.setup();

  // Initialize the watchdog timer
  esp_task_wdt_init(WATCHDOG_TIMEOUT / 1000, true); // Convert milliseconds to seconds
  esp_task_wdt_add(NULL); // Add current thread to WDT watch

  Trace::log("Setup end");
}

void loop() 
{
  Trace::log("Loop start: " + String(millis()));

  // Reset the watchdog timer in each loop iteration
  esp_task_wdt_reset();
  
  // Manage WiFi connection
  wifiManager.loop();
  
  // Manage MQTT connection and loop
  mqttManager.loop(wifiManager.isConnected(), wifiManager.checkDnsResolution(MQTT_SERVER_IP));

  // Update IrrigationZone state
  irrigationZone.loop();

  delay(LOOP_DELAY);
  Trace::log("Loop end");
}