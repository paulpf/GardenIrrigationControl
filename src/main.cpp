#include <Arduino.h>
#include "IrrigationZone.h"

//#define TRACE

IrrigationZone irrigationZone;

void setup() 
{
  Serial.begin(115200);
  irrigationZone.setup(23, 22, 5000);
}

void loop() 
{
  irrigationZone.update();

  delay(100);
}