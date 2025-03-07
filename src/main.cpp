#include <Arduino.h>
#include "./_interfaces/HardwareButton.h"
#include "./_interfaces/Relais.h"
#include <memory>

const int HWBTN_PIN = 23;
const int RELAY_PIN = 22;

std::unique_ptr<HardwareButton> hwbtn1;
std::unique_ptr<Relais> r1;

void setup() 
{
  Serial.begin(115200);
  hwbtn1 = std::unique_ptr<HardwareButton>(new HardwareButton());
  hwbtn1->setup(HWBTN_PIN);
  r1 = std::unique_ptr<Relais>(new Relais());
  r1->setup(RELAY_PIN);
}

void loop() 
{
  bool hwbuttonState = hwbtn1->isPressed();

  // Write
  r1->set(hwbuttonState);
  
  delay(10);
}
