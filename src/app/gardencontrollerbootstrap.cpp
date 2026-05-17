#include "gardencontrollerbootstrap.h"

GardenControllerBootstrap::GardenControllerBootstrap()
    : _wallClockPublisher(_mqttManager, _wallClock),
      _automaticIrrigationPublisher(_mqttManager,
                                    _automaticIrrigationController),
      _diagnosticsReporter(_mqttManager),
      _scheduledIrrigationPublisher(_mqttManager, _scheduledIrrigations),
      _scheduledIrrigationCommandHandler(_scheduledIrrigations,
                                         _scheduledIrrigationPublisher,
                                         _irrigationConfig),
      _waterLevelManager(_mqttManager, _waterLevelSensor),
      _connectivityCoordinator(_wifiManager, _mqttManager),
      _wifiConnectionAwaiter(_wifiManager, _timeProvider),
      _otaLoopGuard(_otaManager),
      _app(_irrigationConfig, _systemConfig, _hardwareConfig, _wifiManager,
           _mqttManager, _otaManager, _irrigationZones, _scheduledIrrigations,
           _wallClock, _wallClockPublisher, _automaticIrrigationController,
           _automaticIrrigationPublisher, _scheduledIrrigationPublisher,
           _scheduledIrrigationCommandHandler, _diagnosticsReporter,
           _waterLevelManager,
           _connectivityCoordinator, _wifiConnectionAwaiter, _otaLoopGuard,
           _loopScheduler)
{
}

GardenControllerApp &GardenControllerBootstrap::application()
{
  return _app;
}
