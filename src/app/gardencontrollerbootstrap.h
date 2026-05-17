#ifndef GARDENCONTROLLERBOOTSTRAP_H
#define GARDENCONTROLLERBOOTSTRAP_H

#include "automaticirrigationcontroller.h"
#include "automaticirrigationcommandhandler.h"
#include "automaticirrigationpublisher.h"
#include "arduinotimeprovider.h"
#include "connectivitycoordinator.h"
#include "diagnosticsreporter.h"
#include "esp32waterlevelsensor.h"
#include "gardencontrollerapp.h"
#include "hardwareconfig.h"
#include "irrigation_zone.h"
#include "irrigationconfig.h"
#include "loopscheduler.h"
#include "mqttmanager.h"
#include "ntpwallclock.h"
#include "otaloopguard.h"
#include "otamanager.h"
#include "scheduled_irrigation.h"
#include "scheduledirrigationcommandhandler.h"
#include "scheduledirrigationpublisher.h"
#include "systemconfig.h"
#include "waterlevelmanager.h"
#include "wallclockpublisher.h"
#include "wificonnectionawaiter.h"
#include "wifimanager.h"

class GardenControllerBootstrap
{
public:
  GardenControllerBootstrap();

  GardenControllerApp &application();

private:
  IrrigationConfig _irrigationConfig;
  SystemConfig _systemConfig;
  HardwareConfig _hardwareConfig;

  WifiManager _wifiManager;
  MqttManager _mqttManager;
  OtaManager _otaManager;
  NtpWallClock _wallClock;
  WallClockPublisher _wallClockPublisher;
  AutomaticIrrigationController _automaticIrrigationController;
  AutomaticIrrigationPublisher _automaticIrrigationPublisher;
  AutomaticIrrigationCommandHandler _automaticIrrigationCommandHandler;
  DiagnosticsReporter _diagnosticsReporter;
  IrrigationZone _irrigationZones[MAX_IRRIGATION_ZONES];
  ScheduledIrrigation _scheduledIrrigations[MAX_IRRIGATION_ZONES];
  ScheduledIrrigationPublisher _scheduledIrrigationPublisher;
  ScheduledIrrigationCommandHandler _scheduledIrrigationCommandHandler;
  Esp32WaterLevelSensor _waterLevelSensor;
  WaterLevelManager _waterLevelManager;
  ConnectivityCoordinator _connectivityCoordinator;
  ArduinoTimeProvider _timeProvider;
  WifiConnectionAwaiter _wifiConnectionAwaiter;
  OtaLoopGuard _otaLoopGuard;
  LoopScheduler _loopScheduler;
  GardenControllerApp _app;
};

#endif // GARDENCONTROLLERBOOTSTRAP_H
