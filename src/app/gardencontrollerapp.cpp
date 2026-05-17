#include "gardencontrollerapp.h"

#include "../../../_config/MqttConfig.h"
#include "../../../_secrets/MqttSecret.h"
#include "../../../_secrets/OtaSecret.h"
#include "../../../_secrets/WifiSecret.h"

#include "config.h"
#include "global_defines.h"

#include "helper.h"
#include "storage_manager.h"
#include "trace.h"

#include "esp_system.h"
#include "esp_task_wdt.h"

GardenControllerApp::GardenControllerApp(
    IrrigationConfig &irrigationConfig, SystemConfig &systemConfig,
    HardwareConfig &hardwareConfig, WifiManager &wifiManager,
    MqttManager &mqttManager, OtaManager &otaManager,
    IrrigationZone (&irrigationZones)[MAX_IRRIGATION_ZONES],
    ScheduledIrrigation (&scheduledIrrigations)[MAX_IRRIGATION_ZONES],
    IWallClock &wallClock,
    WallClockPublisher &wallClockPublisher,
    AutomaticIrrigationController &automaticIrrigationController,
    AutomaticIrrigationPublisher &automaticIrrigationPublisher,
    ScheduledIrrigationPublisher &scheduledIrrigationPublisher,
    DiagnosticsReporter &diagnosticsReporter,
    WaterLevelManager &waterLevelManager,
    ConnectivityCoordinator &connectivityCoordinator,
    WifiConnectionAwaiter &wifiConnectionAwaiter, OtaLoopGuard &otaLoopGuard,
    LoopScheduler &loopScheduler)
    : _irrigationConfig(irrigationConfig), _systemConfig(systemConfig),
      _hardwareConfig(hardwareConfig), _wifiManager(wifiManager),
      _mqttManager(mqttManager), _otaManager(otaManager),
      _irrigationZones(irrigationZones),
      _scheduledIrrigations(scheduledIrrigations),
      _wallClock(wallClock),
      _wallClockPublisher(wallClockPublisher),
      _automaticIrrigationController(automaticIrrigationController),
      _automaticIrrigationPublisher(automaticIrrigationPublisher),
      _scheduledIrrigationPublisher(scheduledIrrigationPublisher),
      _diagnosticsReporter(diagnosticsReporter),
      _waterLevelManager(waterLevelManager),
      _connectivityCoordinator(connectivityCoordinator),
      _wifiConnectionAwaiter(wifiConnectionAwaiter),
      _otaLoopGuard(otaLoopGuard), _loopScheduler(loopScheduler)
{
  _clientName[0] = '\0';
}

void GardenControllerApp::initScheduledIrrigation()
{
  Trace::log(TraceLevel::INFO, "Initializing scheduled irrigation...");

  for (int i = 0; i < MAX_IRRIGATION_ZONES; i++)
  {
    _scheduledIrrigations[i].setup(i, &_wallClock);
  }

  if (!ENABLE_SCHEDULED_IRRIGATION)
  {
    Trace::log(TraceLevel::INFO,
               "Scheduled irrigation disabled by configuration");
  }
}

void GardenControllerApp::initAutomaticIrrigation()
{
  _automaticIrrigationController.setup(false);
  Trace::log(TraceLevel::INFO, "Automatic irrigation disabled by default");
}

void GardenControllerApp::initIrrigationZones()
{
  Trace::log(TraceLevel::INFO, "Initializing irrigation zones...");

  for (int i = 0; i < MAX_IRRIGATION_ZONES; i++)
  {
    IrrigationZoneFactory::initializeZone(
        _irrigationZones[i], _irrigationConfig, _hardwareConfig.zones[i],
        _mqttManager, i, _clientName);
    _irrigationZones[i].loadSettingsFromStorage(i);
  }
}

void GardenControllerApp::resetZoneStateAfterColdStartIfNeeded()
{
  const esp_reset_reason_t resetReason = esp_reset_reason();
  const bool shouldClearZoneState = resetReason == ESP_RST_EXT ||
                                    resetReason == ESP_RST_POWERON ||
                                    resetReason == ESP_RST_BROWNOUT;
  if (!shouldClearZoneState)
  {
    return;
  }

  Trace::log(TraceLevel::INFO,
             "Cold start detected, clearing transient zone state. Reset reason=" +
                 String(static_cast<int>(resetReason)));

  for (int i = 0; i < MAX_IRRIGATION_ZONES; i++)
  {
    _irrigationZones[i].synchronizeButtonStates(false);
    _irrigationZones[i].switchRelay(false);
    _irrigationZones[i].resetTimer();

    StorageManager::getInstance().saveButtonState(i, false);
    StorageManager::getInstance().saveRelayState(i, false);
  }
}

void GardenControllerApp::updateClientNameFromMac()
{
  String macFormatted = Helper::replaceChars(WiFi.macAddress(), ':', '-');
  Helper::formatToBuffer(_clientName, CLIENT_NAME_MAX_SIZE,
                         "GardenController-%s", macFormatted.c_str());
  Trace::log(TraceLevel::INFO, "Client name set: " + String(_clientName));
}

void GardenControllerApp::setupOta()
{
#if ENABLE_OTA
  Trace::log(TraceLevel::INFO, "Setting up OTA...");
  _otaManager.setup(_clientName, OTA_PASSWORD);
#else
  Trace::log(TraceLevel::INFO, "OTA disabled in configuration");
  _otaManager.setEnabled(false);
#endif
}

void GardenControllerApp::initWatchdog()
{
  esp_task_wdt_init(_systemConfig.watchdogTimeoutMs / 1000, true);
  esp_task_wdt_add(NULL);
}

void GardenControllerApp::waitForWifiConnection()
{
  Trace::log(TraceLevel::INFO, "Waiting for WiFi connection...");
  if (!_wifiConnectionAwaiter.waitForConnection(WIFI_CONNECTION_TIMEOUT))
  {
    Trace::log(TraceLevel::ERROR, "WiFi connection timeout");
    _diagnosticsReporter.record(TraceLevel::ERROR, "WiFi connection timeout",
                                millis());
  }
}

void GardenControllerApp::initializeStorage()
{
  StorageManager::getInstance().begin();
  Trace::log(TraceLevel::INFO, "StorageManager initialized");
}

void GardenControllerApp::initializeIdentity()
{
  strncpy(_clientName, "GardenController-Init", CLIENT_NAME_MAX_SIZE - 1);
  _clientName[CLIENT_NAME_MAX_SIZE - 1] = '\0';
}

void GardenControllerApp::initializeConnectivity()
{
  updateClientNameFromMac();
  initializeDiagnostics();
  _wifiManager.setup(WIFI_SSID, WIFI_PWD, _clientName);
  waitForWifiConnection();
}

void GardenControllerApp::initializeTime()
{
  _wallClock.setup();
}

void GardenControllerApp::initializeDiagnostics()
{
  _diagnosticsReporter.setup(_clientName);
  _wallClockPublisher.setup(_clientName);
  _automaticIrrigationPublisher.setup(_clientName);
  _scheduledIrrigationPublisher.setup(_clientName);
}

void GardenControllerApp::initializeSubsystems()
{
  _waterLevelManager.setup(_clientName);
  _mqttManager.setWaterLevelManager(&_waterLevelManager);

  _mqttManager.configure(_irrigationConfig);
  _mqttManager.setup(MQTT_SERVER_IP, MQTT_SERVER_PORT, MQTT_USER, MQTT_PWD,
                     _clientName);

  setupOta();
  initIrrigationZones();
  initScheduledIrrigation();
  initAutomaticIrrigation();
  resetZoneStateAfterColdStartIfNeeded();

  _connectivityCoordinator.ensureMqttConnected();
}

void GardenControllerApp::initializeRuntimeSafety()
{
  initWatchdog();
}

void GardenControllerApp::setup()
{
  Serial.begin(115200);
  Trace::log(TraceLevel::INFO, "Setup begin");

  initializeStorage();
  initializeIdentity();
  initializeConnectivity();
  initializeTime();
  initializeSubsystems();
  initializeRuntimeSafety();

  Trace::log(TraceLevel::INFO, "Setup end");
}

void GardenControllerApp::plotZoneStates(unsigned long currentTime)
{
  static unsigned long lastPlotTime = 0;
  if ((unsigned long)(currentTime - lastPlotTime) >= TELEPLOT_INTERVAL)
  {
    lastPlotTime = currentTime;
    char bufferName[20];
    for (int i = 0; i < MAX_IRRIGATION_ZONES; i++)
    {
      snprintf(bufferName, sizeof(bufferName), "Btn%d", i + 1);
      Trace::plotBoolState(bufferName, _irrigationZones[i].getBtnState(), 1);

      snprintf(bufferName, sizeof(bufferName), "Relay%d", i + 1);
      Trace::plotBoolState(bufferName, _irrigationZones[i].getRelayState(), -1);
    }
  }
}

void GardenControllerApp::handleShortIntervalTasks()
{
  _connectivityCoordinator.handleEvents();

  _mqttManager.loop();

  for (int i = 0; i < MAX_IRRIGATION_ZONES; i++)
  {
    _irrigationZones[i].loop();
    _mqttManager.publishIrrigationZoneEvent(
        i, _irrigationZones[i].consumeEvents());

    if (i % 3 == 0)
    {
      yield();
    }
  }

  _mqttManager.publishIrrigationZoneUpdates(_currentMillis);

  _waterLevelManager.loop(_currentMillis);
}

void GardenControllerApp::updateLoopTimingPlot()
{
#ifdef ENABLE_LOOP_TIME_PLOTTING
  Trace::plotLoopTime("Mainloop", 0, _currentMillis - _mainLoopStartTime);
  _mainLoopStartTime = _currentMillis;
#endif
}

void GardenControllerApp::handleScheduledIrrigation()
{
  if (!ENABLE_SCHEDULED_IRRIGATION)
  {
    return;
  }

  if (!_automaticIrrigationController.canRunSchedules())
  {
    return;
  }

  for (int i = 0; i < MAX_IRRIGATION_ZONES; i++)
  {
    IrrigationSchedule dueSchedule;
    if (!_scheduledIrrigations[i].consumeDueSchedule(_currentMillis,
                                                     dueSchedule))
    {
      continue;
    }

    const long durationMs = (long)dueSchedule.durationMinutes * 60L * 1000L;
    if (durationMs <= 0 || durationMs > (long)_irrigationConfig.maxDurationMs)
    {
      Trace::log(TraceLevel::ERROR,
                 "Ignoring invalid schedule duration for zone " + String(i) +
                     ": " + String(dueSchedule.durationMinutes) +
                     " minutes");
      continue;
    }

    _irrigationZones[i].startForDuration((int)durationMs);
    _mqttManager.publishIrrigationZoneEvent(
        i, _irrigationZones[i].consumeEvents());
    Trace::log(TraceLevel::INFO,
               "Scheduled irrigation started for zone " + String(i) +
                   " for " + String(dueSchedule.durationMinutes) +
                   " minutes");
  }
}

void GardenControllerApp::handleMiddleIntervalEvents()
{
  _mqttManager.publishSystemStatus();
  _wallClockPublisher.publishTime();
  _automaticIrrigationPublisher.publishState();
  _scheduledIrrigationPublisher.publishState(_currentMillis);
  _diagnosticsReporter.publishReport();
  handleScheduledIrrigation();
}

void GardenControllerApp::handleLongIntervalTasks()
{
  if (!_wifiManager.loop())
  {
    Trace::log(TraceLevel::ERROR, "WiFi connection issue detected");
    _diagnosticsReporter.record(TraceLevel::ERROR,
                                "WiFi connection issue detected",
                                _currentMillis);
  }

  _connectivityCoordinator.handleEvents();
}

void GardenControllerApp::loop()
{
  esp_task_wdt_reset();
  _currentMillis = millis();

  if (_otaLoopGuard.process())
  {
    return;
  }

  updateLoopTimingPlot();

  if (_loopScheduler.shouldRun(_currentMillis, _previousMillisShortLoop,
                               _systemConfig.shortIntervalMs))
  {
    handleShortIntervalTasks();
  }

  if (_loopScheduler.shouldRun(_currentMillis, _previousMillisMiddleLoop,
                               _systemConfig.middleIntervalMs))
  {
    handleMiddleIntervalEvents();
    yield();
  }

  if (_loopScheduler.shouldRun(_currentMillis, _previousMillisLongLoop,
                               _systemConfig.longIntervalMs))
  {
    handleLongIntervalTasks();
  }

#ifdef ENABLE_ZONE_PLOTTING
  plotZoneStates(_currentMillis);
#endif
}
