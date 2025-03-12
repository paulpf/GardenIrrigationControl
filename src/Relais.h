// Relais.h

#ifndef RELAIS_H
#define RELAIS_H

#include <Arduino.h>

class Relais
{
  public:
    Relais();
    ~Relais();
    void setup(int gpioPin);
    void setState(bool state);
    bool getState();
  private:
    int _gpioPin;
    bool _relayOldState;
    bool _relayNewState;
    void switchRelayON();
    void switchRelayOFF();
};

#endif