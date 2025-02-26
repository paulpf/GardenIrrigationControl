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

// Array to track button press states
volatile bool buttonPressed[] = {false, false, false, false, false, false, false, false};

// ISR functions for each button
void IRAM_ATTR buttonISR0() { buttonPressed[0] = true; }
void IRAM_ATTR buttonISR1() { buttonPressed[1] = true; }
void IRAM_ATTR buttonISR2() { buttonPressed[2] = true; }
void IRAM_ATTR buttonISR3() { buttonPressed[3] = true; }
void IRAM_ATTR buttonISR4() { buttonPressed[4] = true; }
void IRAM_ATTR buttonISR5() { buttonPressed[5] = true; }
void IRAM_ATTR buttonISR6() { buttonPressed[6] = true; }
void IRAM_ATTR buttonISR7() { buttonPressed[7] = true; }

// Function pointer array to ISR functions
typedef void (*ISRFunctionPointer)();
ISRFunctionPointer buttonISRs[] = 
{
  buttonISR0, buttonISR1, buttonISR2, buttonISR3, 
  buttonISR4, buttonISR5, buttonISR6, buttonISR7
};

void setup() 
{
  // For each button in your array
  for (int i = 0; i < buttonCount; i++) 
  {  
    pinMode(buttonPins[i], INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(buttonPins[i]), buttonISRs[i], RISING);
  }

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
  // template code =====================================================================
  // ====================================================================================

  // Check if any button was pressed via interrupt
  for (int i = 0; i < buttonCount; i++) 
  {
    if (buttonPressed[i]) 
    {
      // Button i was pressed
      Serial.print("Button ");
      Serial.print(i);
      Serial.println(" was pressed!");
      
      // Add your button action code here
      // For example, toggle the corresponding irrigation zone
      
      // Reset the flag
      buttonPressed[i] = false;
    }
  }
  
  // ...rest of loop code...



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