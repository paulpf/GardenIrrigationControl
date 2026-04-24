#include "gardencontrollerbootstrap.h"

GardenControllerBootstrap bootstrap;
GardenControllerApp &app = bootstrap.application();

void setup()
{
  app.setup();
}

void loop()
{
  app.loop();
}