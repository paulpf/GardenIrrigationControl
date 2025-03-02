#include <Arduino.h>
#include "./_interfaces/HardwareButton.h"
#include <memory>

const int HWBTN_PIN = 23;

std::unique_ptr<HardwareButton> hwbtn;

void setup() 
{
  Serial.begin(115200);
  hwbtn = std::unique_ptr<HardwareButton>(new HardwareButton());
  hwbtn->setup(HWBTN_PIN);
}

void loop() 
{
  bool hwbuttonState = hwbtn->isPressed();
  
  delay(100);
}

