#ifndef WIFICONNECTIONAWAITER_H
#define WIFICONNECTIONAWAITER_H

#include "itimeprovider.h"
#include "iwificonnectivity.h"

class WifiConnectionAwaiter
{
public:
  WifiConnectionAwaiter(IWifiConnectivity &wifi, ITimeProvider &timeProvider)
      : _wifi(wifi), _timeProvider(timeProvider)
  {
  }

  bool waitForConnection(unsigned long timeoutMs,
                         unsigned long pollDelayMs = 100)
  {
    const unsigned long start = _timeProvider.millis();
    while (!_wifi.isConnected())
    {
      if ((unsigned long)(_timeProvider.millis() - start) > timeoutMs)
      {
        return false;
      }
      _timeProvider.delayMs(pollDelayMs);
    }

    return true;
  }

private:
  IWifiConnectivity &_wifi;
  ITimeProvider &_timeProvider;
};

#endif // WIFICONNECTIONAWAITER_H
