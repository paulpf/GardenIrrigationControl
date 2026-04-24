#include "gardencontrollerbootstrap.h"

GardenControllerBootstrap::GardenControllerBootstrap()
    : _waterLevelManager(_mqttManager, _waterLevelSensor),
      _connectivityCoordinator(_wifiManager, _mqttManager),
      _wifiConnectionAwaiter(_wifiManager, _timeProvider),
      _otaLoopGuard(_otaManager),
      _app(_irrigationConfig, _systemConfig, _hardwareConfig, _wifiManager,
           _mqttManager, _otaManager, _irrigationZones, _waterLevelManager,
           _connectivityCoordinator, _wifiConnectionAwaiter, _otaLoopGuard,
           _loopScheduler)
{
}

GardenControllerApp &GardenControllerBootstrap::application()
{
  return _app;
}
