// Relais.h
#ifndef RELAY_H
#define RELAY_H

class Relay {
  public:
      Relay(int gpioChannel);
      void setup();
      void activate(bool state);
      bool isActive() const;
  
  private:
      int gpioChannel;
      bool state;
};

#endif // RELAY_H